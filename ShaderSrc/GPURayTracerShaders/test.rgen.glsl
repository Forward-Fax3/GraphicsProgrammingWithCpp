#version 460
#extension GL_EXT_ray_tracing : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 32 0
layout(binding = 2)
uniform accelerationStructureEXT sceneBVH_0;


#line 31
layout(rgba32f)
layout(binding = 1)
uniform image2D resultTexture_0;


#line 3
struct Uniforms_0
{
    float screenWidth_0;
    float screenHeight_0;
    float focalLength_0;
    float frameHeight_0;
    vec4 cameraDir_0;
    vec4 cameraUp_0;
    vec4 cameraRight_0;
    vec4 cameraPosition_0;
    vec4 lightDir_0;
    uint randSeed_0;
    uint bounces_0;
};


#line 3
struct GlobalParams_0
{
    Uniforms_0 uniforms_0;
};


#line 19
layout(binding = 0)
layout(std140) uniform block_GlobalParams_0
{
    Uniforms_0 uniforms_0;
}globalParams_0;
struct RayPayload_0
{
    vec4 color_0;
    uint randSeed_1;
    uint numberOfBouncesLeft_0;
};


#line 19339 1
layout(location = 0)
rayPayloadEXT
RayPayload_0 p_0;


#line 24 0
RayPayload_0 RayPayload_x24init_0(vec4 color_1, uint randSeed_2, uint numberOfBouncesLeft_1)
{

#line 24
    RayPayload_0 _S1;

    _S1.color_0 = color_1;
    _S1.randSeed_1 = randSeed_2;
    _S1.numberOfBouncesLeft_0 = numberOfBouncesLeft_1;

#line 24
    return _S1;
}


#line 19086 1
struct RayDesc_0
{
    vec3 Origin_0;
    float TMin_0;
    vec3 Direction_0;
    float TMax_0;
};


#line 37 0
void main()
{
    uvec3 _S2 = ((gl_LaunchIDEXT));

#line 39
    uvec2 threadIdx_0 = _S2.xy;
    uint _S3 = threadIdx_0.x;

#line 40
    if(_S3 >= uint(int(globalParams_0.uniforms_0.screenWidth_0)))
    {

#line 40
        return;
    }

#line 41
    uint _S4 = threadIdx_0.y;

#line 41
    if(_S4 >= uint(int(globalParams_0.uniforms_0.screenHeight_0)))
    {

#line 41
        return;
    }

#line 47
    vec3 rayDir_0 = normalize(globalParams_0.uniforms_0.cameraDir_0.xyz * globalParams_0.uniforms_0.focalLength_0 - globalParams_0.uniforms_0.cameraUp_0.xyz * ((float(_S4) / globalParams_0.uniforms_0.screenHeight_0 - 0.5) * globalParams_0.uniforms_0.frameHeight_0) + globalParams_0.uniforms_0.cameraRight_0.xyz * ((float(_S3) / globalParams_0.uniforms_0.screenWidth_0 - 0.5) * (globalParams_0.uniforms_0.screenWidth_0 / globalParams_0.uniforms_0.screenHeight_0 * globalParams_0.uniforms_0.frameHeight_0)));


    RayDesc_0 ray_0;
    ray_0.Origin_0 = globalParams_0.uniforms_0.cameraPosition_0.xyz;
    ray_0.Direction_0 = rayDir_0;
    ray_0.TMin_0 = 0.00100000004749745;
    ray_0.TMax_0 = 10000.0;
    RayPayload_0 payload_0 = RayPayload_x24init_0(vec4(0.0, 0.0, 0.0, 0.0), 0U, 0U);
    payload_0.randSeed_1 = globalParams_0.uniforms_0.randSeed_0;
    payload_0.numberOfBouncesLeft_0 = globalParams_0.uniforms_0.bounces_0;
    RayDesc_0 _S5 = ray_0;

#line 58
    p_0 = payload_0;

#line 58
    traceRayEXT(sceneBVH_0, 0U, 255U, 0U, 0U, 0U, _S5.Origin_0, _S5.TMin_0, _S5.Direction_0, _S5.TMax_0, 0);

#line 58
    payload_0 = p_0;

    imageStore((resultTexture_0), (ivec2(threadIdx_0.xy)), payload_0.color_0);
    return;
}

