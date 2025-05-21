#pragma once
#include <vector>
#include "Engine/Core/StringUtils.hpp"

class FileUtils
{
public:
	FileUtils() = default;
	~FileUtils();

	static int FileReadExternalToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename);
	static int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename);
	int FileReadToString(std::string& outString, const std::string& filename);
	int FileReadExternalToString(std::string& outString, const std::string& filename);
	int FileReadLinesToVector(std::vector<std::string>& outLines, const std::string& filename);
	static bool FileWriteFromBuffer(std::vector<uint8_t> const& buffer, const std::string& filePathName);
	bool CreateFolder(std::string const& folderPathName);
};