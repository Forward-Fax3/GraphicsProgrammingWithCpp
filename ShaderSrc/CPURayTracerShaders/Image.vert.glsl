#version 460
#extension GL_ARB_shader_draw_parameters : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 32 0
void _S1(out int  _S2[6])
{

#line 32
    _S2[0] = 0;

#line 32
    _S2[1] = 1;

#line 32
    _S2[2] = 2;

#line 32
    _S2[3] = 1;

#line 32
    _S2[4] = 3;

#line 32
    _S2[5] = 2;

#line 32
    return;
}


#line 10
int  indices_0[6];


#line 32
void _S3(out vec2  _S4[4])
{

#line 5
    const vec2 _S5 = vec2(1.0, -1.0);
    const vec2 _S6 = vec2(-1.0, 1.0);
    const vec2 _S7 = vec2(-1.0, -1.0);

#line 7
    _S4[0] = vec2(1.0, 1.0);

#line 7
    _S4[1] = _S5;

#line 7
    _S4[2] = _S6;

#line 7
    _S4[3] = _S7;

#line 7
    return;
}


#line 3
vec2  data_0[4];


#line 22
layout(location = 0)
out vec2 entryPointParam_vertexMain_uv_0;


#line 22
struct VertexStageOutput_0
{
    vec4 sv_position_0;
    vec2 uv_0;
};


void main()
{

#line 29
    int  _S8[6];

#line 29
    _S1(_S8);

#line 29
    indices_0 = _S8;

#line 29
    vec2  _S9[4];

#line 29
    _S3(_S9);

#line 29
    data_0 = _S9;


    vec2 vetData_0 = data_0[indices_0[uint(gl_VertexIndex - gl_BaseVertex)]];

#line 31
    VertexStageOutput_0 output_0;

    output_0.sv_position_0 = vec4(data_0[indices_0[uint(gl_VertexIndex - gl_BaseVertex)]], vec2(1.0));
    output_0.uv_0 = (vetData_0 + 1.0) * 0.5;

    VertexStageOutput_0 _S10 = output_0;

#line 36
    gl_Position = output_0.sv_position_0;

#line 36
    entryPointParam_vertexMain_uv_0 = _S10.uv_0;

#line 36
    return;
}

