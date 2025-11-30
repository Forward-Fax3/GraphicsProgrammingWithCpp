#include "Log.hpp"
#include <fstream>
#include <vector>


namespace OWC
{
	template<typename T>
	std::vector<T> LoadFileToBytecode(const std::string& filepath) requires(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4)
	{
		std::ifstream fileStream(filepath, std::ios::binary | std::ios::ate);

		if (!fileStream.is_open())
		{
			Log<LogLevel::Error>("LoadFileToBytecode: Failed to open file: {}", filepath);
			return {};
		}

		std::streamsize fileSize = fileStream.tellg();
		fileStream.seekg(0, std::ios::beg);

		if (fileSize % sizeof(T) != 0)
		{
			Log<LogLevel::Error>("LoadFileToBytecode: File size is not aligned with type size for file: {}", filepath);
			return {};
		}

		std::vector<T> bytecode(fileSize / sizeof(T));
		if (!fileStream.read(reinterpret_cast<char*>(bytecode.data()), fileSize))
		{
			Log<LogLevel::Error>("LoadFileToBytecode: Failed to read file: {}", filepath);
			return {};
		}

		return bytecode;
	}
}