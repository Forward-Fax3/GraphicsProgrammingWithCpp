#version 460
#extension GL_EXT_ray_tracing : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 24 0
struct RayPayload_0
{
    vec4 color_0;
    uint randSeed_0;
    uint numberOfBouncesLeft_0;
};


#line 64
rayPayloadInEXT RayPayload_0 payload_0;


#line 64
void main()
{
    vec3 rayDirection_0 = ((gl_WorldRayDirectionEXT));


    float t_0 = 0.5 * (rayDirection_0.y + 1.0);
    payload_0.color_0 = payload_0.color_0 + (1.0 - t_0 + t_0 * vec4(0.5, 0.69999998807907104, 1.0, 1.0));
    return;
}

