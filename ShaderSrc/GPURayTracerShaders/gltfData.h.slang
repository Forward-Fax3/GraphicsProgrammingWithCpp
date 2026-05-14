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
