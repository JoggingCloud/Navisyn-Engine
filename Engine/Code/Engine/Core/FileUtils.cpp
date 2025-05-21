#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <regex>
#include <windows.h> 

FileUtils::~FileUtils()
{
}

int FileUtils::FileReadExternalToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename)
{
	FILE* fileptr = nullptr;

	fopen_s(&fileptr, filename.c_str(), "rb");

	if (fileptr)
	{
		fseek(fileptr, 0, SEEK_END);

		size_t fileSize = (size_t)ftell(fileptr);

		outBuffer.resize(fileSize);

		fseek(fileptr, 0, SEEK_SET);

		fread(outBuffer.data(), fileSize, 1, fileptr);

		fclose(fileptr);

		return 0;
	}
	else
	{
		return 1;
	}
}

int FileUtils::FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename)
{
	// Get the file and open the file in binary 
	FILE* file;
	int errorCheck = fopen_s(&file, filename.c_str(), "rb");
	if (errorCheck != 0)
	{
		// Failure to open the file
		return -1;
	}

	// Check if the file has content
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (fileSize <= 0)
	{
		// File is empty
		fclose(file);
		return -1;
	}

	// Resize the output buffer
	outBuffer.resize(static_cast<size_t>(fileSize));

	// Read the file into the buffer
	size_t bytesRead = fread(outBuffer.data(), 1, fileSize, file);

	// Close the file
	fclose(file);

	if (bytesRead == static_cast<size_t>(fileSize))
	{
		// Successful file read
		return static_cast<int>(bytesRead);
	}

	// Failed to read the entire file
	return -1;
}

int FileUtils::FileReadToString(std::string& outString, const std::string& filename)
{
	std::vector<uint8_t> buffer;
	int bytesRead = FileReadToBuffer(buffer, filename);

	if (bytesRead > 0)
	{
		// Append null terminator to make a C string
		buffer.emplace_back('\0');

		// Convert the buffer to a C++ string
		outString.assign(reinterpret_cast<const char*>(buffer.data()), bytesRead);
	}

	return bytesRead;
}

int FileUtils::FileReadExternalToString(std::string& outString, const std::string& filename)
{
	std::vector<uint8_t> buffer;
	int bytesRead = FileReadExternalToBuffer(buffer, filename);

	if (bytesRead > 0)
	{
		// Append null terminator to make a C string
		buffer.emplace_back('\0');

		// Convert the buffer to a C++ string
		outString.assign(reinterpret_cast<const char*>(buffer.data()), bytesRead);
	}

	return bytesRead;
}

int FileUtils::FileReadLinesToVector(std::vector<std::string>& outLines, const std::string& filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		return -1; // Failed to open file
	}

	std::stringstream buffer;
	buffer << file.rdbuf(); // Read the entire file into a string
	file.close();

	std::string content = buffer.str(); // Store the full text

	// Use regex to split on periods, question marks, or exclamation marks followed by a space.
	std::regex sentenceRegex(R"(([^.?!]+[.?!]))");
	std::sregex_iterator it(content.begin(), content.end(), sentenceRegex);
	std::sregex_iterator end;

	while (it != end)
	{
		std::string sentence = it->str();
		if (!sentence.empty())
		{
			outLines.emplace_back(sentence);
		}
		it++;
	}

	return static_cast<int>(outLines.size());
}

bool FileUtils::FileWriteFromBuffer(std::vector<uint8_t> const& buffer, const std::string& filePathName)
{
	FILE* file;
	errno_t result = fopen_s(&file, filePathName.c_str(), "wb");
	if (result != 0)
	{
		return false;
	}

	fwrite(buffer.data(), 1, buffer.size(), file);
	fclose(file);
	return true;
}

bool FileUtils::CreateFolder(std::string const& folderPathName)
{
	return CreateDirectoryA(folderPathName.c_str(), NULL);
}
