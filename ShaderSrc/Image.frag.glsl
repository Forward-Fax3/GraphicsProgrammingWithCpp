#version 460
layout(row_major) uniform;
layout(row_major) buffer;

#line 19 0
struct SLANG_ParameterGroup_image_natural_0
{
    float invNumberOfSamples_0;
    float invGamma_0;
};


#line 20
layout(binding = 0)
layout(std140) uniform block_SLANG_ParameterGroup_image_natural_0
{
    float invNumberOfSamples_0;
    float invGamma_0;
}image_0;

#line 977 1
layout(binding = 1)
uniform sampler2D image_texture_0;


#line 12555
layout(location = 0)
out vec4 entryPointParam_fragmentMain_0;


#line 12555
layout(location = 0)
in vec2 uv_0;


#line 41 0
void main()
{

#line 41
    entryPointParam_fragmentMain_0 = pow((texture((image_texture_0), (uv_0))) * image_0.invNumberOfSamples_0, vec4(image_0.invGamma_0));

#line 41
    return;
}

