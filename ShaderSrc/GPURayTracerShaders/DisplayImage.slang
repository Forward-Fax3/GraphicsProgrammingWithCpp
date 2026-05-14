struct VertexData
{
	static float2[] data = {
		{ 1.0,  1.0},
        { 1.0, -1.0},
		{-1.0,  1.0},
		{-1.0, -1.0}
	};

    static int[] indices = {
        0, 1, 2,
        1, 3, 2
    };
};

cbuffer image
{
    float invNumberOfSamples;
    float invGamma;
};

RWTexture2D<float4> texture;

struct VertexStageOutput
{
    float4 sv_position : SV_Position;
    float2 uv : UV;
};

[shader("vertex")]
VertexStageOutput vertexMain(uint VetexIndex : SV_VertexID)
{
    VertexStageOutput output;
    float2 vetData = VertexData.data[VertexData.indices[VetexIndex]];
    output.sv_position = float4(vetData, 0.0, 1.0);
    output.uv = (vetData + 1.0f) * 0.5f;

    return output;
}

[shader("fragment")]
float4 fragmentMain(float2 uv: UV) : SV_Target
{
    uint width, height;
    texture.GetDimensions(width, height);

    uint2 pixelCoord = uint2(uv * float2(width, height));
    float4 output = pow(texture[pixelCoord] * invNumberOfSamples, invGamma);
    return output;
}
