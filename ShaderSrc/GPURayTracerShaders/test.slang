#include "gltfData.h.slang"

struct GeneralData
{
    float screenWidth, screenHeight;
    float focalLength, frameHeight;
    float4 cameraDir;
    float4 cameraUp;
    float4 cameraRight;
    float4 cameraPosition;
    float4 lightDir;
    uint randSeed;
};

struct [raypayload] RayPayload
{
    float4 color             : read(caller)           : write(caller, closesthit, miss);
    uint randSeed            : read(closesthit)       : write(caller, closesthit);
};

struct PushConstantData
{
    GeneralData* generalData;
    uint64_t mainBufferData;
}

[[push_constant]] ConstantBuffer<PushConstantData> PC;

uniform RWTexture2D<float4> resultTexture;
uniform RaytracingAccelerationStructure sceneBVH;
uniform StructuredBuffer<GeometryDescriptor> geometries;

[shader("raygeneration")]
void rayGenShader()
{       
    uint2 threadIdx = DispatchRaysIndex().xy;
    if (threadIdx.x >= (int)PC.generalData.screenWidth) return;
    if (threadIdx.y >= (int)PC.generalData.screenHeight) return;

    float frameWidth = PC.generalData.screenWidth / PC.generalData.screenHeight * PC.generalData.frameHeight;
    float imageY = (threadIdx.y / PC.generalData.screenHeight - 0.5f) * PC.generalData.frameHeight;
    float imageX = (threadIdx.x / PC.generalData.screenWidth - 0.5f) * frameWidth;
    float imageZ = PC.generalData.focalLength;
    float3 rayDir = normalize(PC.generalData.cameraDir.xyz * imageZ - PC.generalData.cameraUp.xyz * imageY + PC.generalData.cameraRight.xyz * imageX);

    // Trace the ray.
    RayDesc ray;
    ray.Origin = PC.generalData.cameraPosition.xyz;
    ray.Direction = rayDir;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    payload.randSeed = PC.generalData.randSeed;
    TraceRay(sceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    resultTexture[threadIdx.xy] = payload.color;
}

[shader("miss")]
void missShader(inout RayPayload payload)
{
    float3 rayDirection = WorldRayDirection();

    const float scale = 1.0f;
    float t = 0.5f * (rayDirection.y + 1.0f);
    payload.color += (1.0f - t) + t * float4(0.5f, 0.7f, 1.0f, 1.0f) * scale;
}

float Rand(inout uint rngState)
{
    // Condensed version of pcg_output_rxs_m_xs_32_32, with simple conversion to floating-point [0,1].
    rngState = rngState * 747796405 + 1;
    uint word = ((rngState >> ((rngState >> 28) + 4)) ^ rngState) * 277803737;
    word = (word >> 22) ^ word;
    return float(word) / 4294967295.0f;
}

float3 diffuseReflection(float3 normal, inout uint rngState)
{
    // For a random diffuse bounce direction, we follow the approach of
    // Ray Tracing in One Weekend, and generate a random point on a sphere
    // of radius 1 centered at the normal. This uses the random_unit_vector
    // function from chapter 8.5:

    const float pi = 3.14159265;

    const float theta = 2.0 * pi * Rand(rngState); // Random in [0, 2pi]
    const float u = 2.0 * Rand(rngState) - 1.0;      // Random in [-1, 1]
    const float r = sqrt(1.0 - u * u);
    const float3 direction = normal + float3(r * cos(theta), r * sin(theta), u);

    return normalize(direction);
}

__generic<T : IFloat> T GetTriangleAttributes(uint64_t bufferOffset, uint3 attributeIndex, float3 barycentricCoords)
{
    T* attr = reinterpret<T*>(PC.mainBufferData + bufferOffset);

    return T(barycentricCoords.x) * attr[attributeIndex.x] +
        T(barycentricCoords.y) * attr[attributeIndex.y] +
        T(barycentricCoords.z) * attr[attributeIndex.z];
}

[shader("closesthit")]
void closestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    float3 barycentricCoords = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);

    uint instanceID = InstanceID();
    uint geomertyIndex = GeometryIndex();
    uint primitiveIndex = PrimitiveIndex();

    GeometryDescriptor geometry = geometries[instanceID + geomertyIndex];

    uint3 attributeIndex = reinterpret<uint3*>(PC.mainBufferData + geometry.indicesOffset)[primitiveIndex * 3];

    float3 hitLocation = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float3 hitNormal = normalize(GetTriangleAttributes<float3>(geometry.normalsOffset, attributeIndex, barycentricCoords));
    float3 newRayDirection = diffuseReflection(hitNormal, payload.randSeed);

    RayDesc ray;
    ray.Origin = hitLocation;
    ray.Direction = normalize(newRayDirection);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    TraceRay(sceneBVH, RAY_FLAG_NONE, 0xFF, 1, 0, 0, ray, payload);

    float4 colour;
    if (geometry.coloursOffset != uint64_t.maxValue)
        colour = GetTriangleAttributes<float4>(geometry.coloursOffset, attributeIndex, barycentricCoords);
    else
        colour = float4(float3(0.8), 1.0);
    
    payload.color *= colour;
}