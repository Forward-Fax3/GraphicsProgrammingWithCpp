#version 460
#extension GL_EXT_ray_tracing : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 16 0
struct Primitive_0
{
    vec4 data0_0;
    vec4 color_0;
};


#line 33
layout(std430, binding = 3) readonly buffer StructuredBuffer_Primitive_t_0 {
    Primitive_0 _data[];
} primitiveBuffer_0;

#line 32
layout(binding = 2)
uniform accelerationStructureEXT sceneBVH_0;


#line 24
struct RayPayload_0
{
    vec4 color_1;
    uint randSeed_0;
    uint numberOfBouncesLeft_0;
};


#line 19339 1
layout(location = 0)
rayPayloadEXT
RayPayload_0 p_0;


#line 20 0
vec3 Primitive_getNormal_0(Primitive_0 this_0)
{

#line 20
    return this_0.data0_0.xyz;
}


#line 73
float Rand_0(inout uint rngState_0)
{

    uint _S1 = rngState_0 * 747796405U + 1U;

#line 76
    rngState_0 = _S1;
    uint word_0 = ((_S1 >> ((_S1 >> 28) + 4U)) ^ _S1) * 277803737U;

    return float((word_0 >> 22) ^ word_0) / 4.294967296e+09;
}

vec3 diffuseReflection_0(vec3 normal_0, inout uint rngState_1)
{

#line 91
    float _S2 = Rand_0(rngState_1);

#line 91
    float theta_0 = 6.28318548202514648 * _S2;
    float _S3 = Rand_0(rngState_1);

#line 92
    float u_0 = 2.0 * _S3 - 1.0;
    float r_0 = sqrt(1.0 - u_0 * u_0);


    return normalize(normal_0 + vec3(r_0 * cos(theta_0), r_0 * sin(theta_0), u_0));
}


#line 21
vec3 Primitive_getColor_0(Primitive_0 this_1)
{

#line 21
    return this_1.color_0.xyz;
}


#line 19312 1
rayPayloadInEXT RayPayload_0 payload_0;


#line 19156
struct BuiltInTriangleIntersectionAttributes_0
{
    vec2 barycentrics_0;
};


#line 19156
hitAttributeEXT BuiltInTriangleIntersectionAttributes_0 attr_0;


#line 19086
struct RayDesc_0
{
    vec3 Origin_0;
    float TMin_0;
    vec3 Direction_0;
    float TMax_0;
};


#line 100 0
void main()
{
    uint primitiveIndex_0 = ((gl_PrimitiveID));

    if((payload_0.numberOfBouncesLeft_0) > 0U)
    {
        vec3 _S4 = ((gl_WorldRayOriginEXT));

#line 106
        vec3 _S5 = ((gl_WorldRayDirectionEXT));

#line 106
        float _S6 = ((gl_RayTmaxEXT));

#line 106
        vec3 hitLocation_0 = _S4 + _S5 * _S6;

        vec3 newRayDirection_0 = diffuseReflection_0(Primitive_getNormal_0(primitiveBuffer_0._data[uint(primitiveIndex_0)]), payload_0.randSeed_0);

        payload_0.numberOfBouncesLeft_0 = payload_0.numberOfBouncesLeft_0 - 1U;

        RayDesc_0 ray_0;
        ray_0.Origin_0 = hitLocation_0;
        ray_0.Direction_0 = normalize(newRayDirection_0);
        ray_0.TMin_0 = 0.00100000004749745;
        ray_0.TMax_0 = 10000.0;
        RayDesc_0 _S7 = ray_0;

#line 117
        p_0 = payload_0;

#line 117
        traceRayEXT(sceneBVH_0, 0U, 255U, 1U, 0U, 0U, _S7.Origin_0, _S7.TMin_0, _S7.Direction_0, _S7.TMax_0, 0);

#line 117
        payload_0 = p_0;

#line 104
    }

#line 121
    payload_0.color_1 = payload_0.color_1 * vec4(Primitive_getColor_0(primitiveBuffer_0._data[uint(primitiveIndex_0)]), 1.0);
    return;
}

