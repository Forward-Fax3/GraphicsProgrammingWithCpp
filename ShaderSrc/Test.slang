struct VertexData
{
	static float2[] data = {
		{-0.5,  0.5},
        { 0.5, -0.5},
		{-0.5, -0.5},
		{ 0.5,  0.5}
	};
};

struct Fragment
{
    float4 colour;
}

struct VertexStageOutput
{
//    CoarseVertex    coarseVertex    : CoarseVertex;
    float4          sv_position     : SV_Position;
};

[shader("vertex")]
VertexStageOutput vertexMain(uint VetexIndex : SV_VertexID)
{
    VertexStageOutput output;

//    float3 position = assembledVertex.position;
//    float3 color    = assembledVertex.color;

//    output.coarseVertex.color = color;
    output.sv_position = float4(VertexData.data[VetexIndex % 4], 1.0);

    return output;
}

[shader("fragment")]
Fragment fragmentMain() : SV_Target
{
//	float3 colour = coarseVertex.color;

    Fragment output;
    output.colour = float4(1.0, 0.0, 0.0, 1.0);
    return output;
}
