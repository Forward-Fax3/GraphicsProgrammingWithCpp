#include <vector>
#include <string>


namespace OWC
{
	template<typename T>
	std::vector<T> LoadFileToBytecode(const std::string& filepath);
}

#include "LoadFile.inl"
