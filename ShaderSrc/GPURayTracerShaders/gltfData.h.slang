struct GeometryDescriptor
{
    uint64_t positionsOffset;
    uint64_t indicesOffset;
    uint64_t normalsOffset;
    uint64_t coloursOffset;
    uint materialIndex;
    uint pad[3]; // 48 bytes
}
