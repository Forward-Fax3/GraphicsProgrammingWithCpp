struct GeometryDescriptor
{
    uint64_t positionsOffset;
    uint64_t indicesOffset;
    uint64_t normalsOffset;
    uint64_t coloursOffset;
    uint materialIndex;
    uint is16BitIndices; // boolean
    uint pad[2]; // 48 bytes
}

struct LightData
{
    float3 position;
    uint type;
    float3 direction;
    float intensity;
    float3 colour;
    float range;
    float innerConeAngle;
    float outerConeAngle;
    uint pad[2];
}
