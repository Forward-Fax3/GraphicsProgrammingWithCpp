// shaders.slang

struct Uniforms
{
    float screenWidth, screenHeight;
    float focalLength, frameHeight;
    float4 cameraDir;
    float4 cameraUp;
    float4 cameraRight;
    float4 cameraPosition;
    float4 lightDir;
    uint randSeed;
    uint bounces;
};

struct Primitive
{
    float4 data0;
    float4 color;
    float3 getNormal() { return data0.xyz; }
    float3 getColor() { return color.xyz; }
};

struct [raypayload] RayPayload
{
    float4 color             : read(caller)           : write(caller, closesthit, miss);
    uint randSeed            : read(closesthit)       : write(caller, closesthit);
    uint numberOfBouncesLeft : read(closesthit, miss) : write(caller, closesthit, miss);
};

uniform RWTexture2D resultTexture;
uniform RaytracingAccelerationStructure sceneBVH;
uniform StructuredBuffer<Primitive> primitiveBuffer;
uniform Uniforms uniforms;

[shader("raygeneration")]
void rayGenShader()
{
    uint2 threadIdx = DispatchRaysIndex().xy;
    if (threadIdx.x >= (int)uniforms.screenWidth) return;
    if (threadIdx.y >= (int)uniforms.screenHeight) return;

    float frameWidth = uniforms.screenWidth / uniforms.screenHeight * uniforms.frameHeight;
    float imageY = (threadIdx.y / uniforms.screenHeight - 0.5f) * uniforms.frameHeight;
    float imageX = (threadIdx.x / uniforms.screenWidth - 0.5f) * frameWidth;
    float imageZ = uniforms.focalLength;
    float3 rayDir = normalize(uniforms.cameraDir.xyz*imageZ - uniforms.cameraUp.xyz * imageY + uniforms.cameraRight.xyz * imageX);

    // Trace the ray.
    RayDesc ray;
    ray.Origin = uniforms.cameraPosition.xyz;
    ray.Direction = rayDir;
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    payload.randSeed = uniforms.randSeed;
    payload.numberOfBouncesLeft = uniforms.bounces;
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

[shader("closesthit")]
void closestHitShader(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attr)
{
    uint primitiveIndex = PrimitiveIndex();

    if (payload.numberOfBouncesLeft > 0)
    {
        float3 hitLocation = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
        float3 hitNormal = primitiveBuffer[primitiveIndex].getNormal();
        float3 newRayDirection = diffuseReflection(hitNormal, payload.randSeed);

        payload.numberOfBouncesLeft--;

        RayDesc ray;
        ray.Origin = hitLocation;
        ray.Direction = normalize(newRayDirection);
        ray.TMin = 0.001;
        ray.TMax = 10000.0;
        TraceRay(sceneBVH, RAY_FLAG_NONE, 0xFF, 1, 0, 0, ray, payload);
    }

    float3 color = primitiveBuffer[primitiveIndex].getColor();
    payload.color *= float4(color, 1.0f);
}