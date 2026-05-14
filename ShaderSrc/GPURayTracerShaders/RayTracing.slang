#include "gltfData.h.slang"

struct GeneralData
{
    float4x4 InvProjectionMatrix;
    float4x4 InvViewMatrix;
    uint randSeed;
};

struct [raypayload] RayPayload
{
    float4 color : read(caller) : write(caller, closesthit, miss);
    uint randSeed : read(closesthit) : write(caller, closesthit);
    uint depth : read(closesthit) : write(caller, closesthit);
};

struct PushConstantData
{
    GeneralData* generalData;
    uint64_t mainBufferData;
    uint64_t geomertyData;
};

[[push_constant]] ConstantBuffer<PushConstantData> PC;

uniform RWTexture2D<float4> resultTexture;
uniform RaytracingAccelerationStructure sceneBVH;

[shader("raygeneration")]
void rayGenShader()
{
    //printf("did gen");
    const uint2 intThreadIdx = DispatchRaysIndex().xy;
    const uint2 intScreanSize = DispatchRaysDimensions().xy;

    const float2 threadIdx = (float2)intThreadIdx;
    const float2 screenSize = (float2)intScreanSize;

    const float2 screenCoords = threadIdx / screenSize * 2.0 - 1.0;

    const float4 viewCoords = mul(float4(screenCoords, 1.0, 1.0), PC.generalData.InvProjectionMatrix);

    // Trace the ray.
    RayDesc ray;
    ray.Origin = mul(float4(0.0, 0.0, 0.0, 1.0), PC.generalData.InvViewMatrix).xyz;
    ray.Direction = mul(float4(normalize(viewCoords.xyz), 0.0), PC.generalData.InvViewMatrix).xyz;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload;
    payload.color = float4(0.0);
    payload.randSeed = PC.generalData.randSeed + intThreadIdx.x + intScreanSize.x * intThreadIdx.y;
    payload.depth = 4;
    TraceRay(sceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);

    resultTexture[intThreadIdx.xy] += payload.color;
}

[shader("miss")]
void missShader(inout RayPayload payload)
{
    //printf("did miss");
    float3 rayDirection = WorldRayDirection();

    const float scale = 1.0f;
    float t = 0.5f * (rayDirection.y + 1.0f);
    payload.color = (1.0f - t) + t * float4(0.5f, 0.7f, 1.0f, 1.0f) * scale;
}

float Rand(inout uint rngState)
{
    // Condensed version of pcg_output_rxs_m_xs_32_32, with simple conversion to floating-point [0,1].
    rngState = rngState * 747796405 + 1;
    uint word = ((rngState >> ((rngState >> 28) + 4)) ^ rngState) * 277803737;
    word = (word >> 22) ^ word;
    return float(word) / 4294967295.0f;
}

float3 diffuseReflection(float3 inRay, float3 normal, inout uint rngState)
{
    // For a random diffuse bounce direction, we follow the approach of
    // Ray Tracing in One Weekend, and generate a random point on a sphere
    // of radius 1 centered at the normal. This uses the random_unit_vector
    // function from chapter 8.5:

    const float pi = 3.14159265;

    const float theta = 2.0 * pi * Rand(rngState); // Random in [0, 2pi]
    const float u = 2.0 * Rand(rngState) - 1.0;      // Random in [-1, 1]
    const float r = sqrt(1.0 - u * u);
    const float3 direction = reflect(inRay, normal) + float3(r * cos(theta), r * sin(theta), u);

    if (dot(normal, normal) < 1e-8)
        return normal;

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
    uint geometryIndex = GeometryIndex();
    uint primitiveIndex = PrimitiveIndex();

    GeometryDescriptor* geometry = reinterpret<GeometryDescriptor*>(
        PC.geomertyData + sizeof(GeometryDescriptor) * (instanceID + geometryIndex)
    );

    uint3 attributeIndex;
    if (geometry.is16BitIndices == 1)
        attributeIndex = (uint3)reinterpret<uint16_t3*>(PC.mainBufferData + geometry.indicesOffset)[primitiveIndex];
    else // 32 bit indecies
        attributeIndex = reinterpret<uint3*>(PC.mainBufferData + geometry.indicesOffset)[primitiveIndex];

    if (payload.depth > 0)
    {
        float3 hitLocation = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
        float3 hitNormal = normalize(GetTriangleAttributes<float3>(geometry.normalsOffset, attributeIndex, barycentricCoords));
        float3 newRayDirection = diffuseReflection(WorldRayDirection(), hitNormal, payload.randSeed);

        float epsilon = max(1e-4f, RayTCurrent() * 1e-4f);
        RayDesc ray;
        ray.Origin = hitLocation + hitNormal * epsilon;
        ray.Direction = newRayDirection;
        ray.TMin = 0.001;
        ray.TMax = 10000.0;
        payload.depth--;
        TraceRay(sceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, ray, payload);
    }

    float4 colour;
    if (geometry.coloursOffset != 0xFFFFFFFFFFFFFFFFULL)
        colour = GetTriangleAttributes<float4>(geometry.coloursOffset, attributeIndex, barycentricCoords);
    else
        colour = float4(1.0);

    payload.color *= colour;
}
