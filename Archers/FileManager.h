#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

class FileManager
{
public:
	static std::vector<char> ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate);

		if (!file.is_open())
		{
			return {};
		}

		std::size_t fileSize = (std::size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}
};