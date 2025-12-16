#version 460
layout(row_major) uniform;
layout(row_major) buffer;

#line 18 0
struct SLANG_ParameterGroup_image_natural_0
{
    float invNumberOfSamples_0;
};


#line 19
layout(binding = 0)
layout(std140) uniform block_SLANG_ParameterGroup_image_natural_0
{
    float invNumberOfSamples_0;
}image_0;

#line 977 1
layout(binding = 1)
uniform sampler2D image_texture_0;


#line 977
layout(location = 0)
out vec4 entryPointParam_fragmentMain_0;


#line 977
layout(location = 0)
in vec2 uv_0;


#line 40 0
void main()
{

#line 40
    entryPointParam_fragmentMain_0 = (texture((image_texture_0), (uv_0))) * image_0.invNumberOfSamples_0;

#line 40
    return;
}

