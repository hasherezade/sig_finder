#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>

inline bool is_matching(const BYTE* loadedData, const size_t loadedSize, const BYTE* pattern, const size_t pattern_size)
{
	if (loadedSize < pattern_size) return false;

	for (size_t j = 0; j < pattern_size; j++) {
		if (loadedData[j] != pattern[j]) return false;
	}
	return true;
}

inline std::string to_hex(const uint8_t val, int width = 2)
{
	std::stringstream ss;
	ss << std::setw(width) << std::setfill('0') << std::hex << (unsigned int)(val);
	return ss.str();
}

inline void show_hex_preview(BYTE* loadedData, size_t loadedSize, size_t offset, size_t previewSize)
{
	if (!previewSize || !loadedSize) return;
	for (size_t i = offset; i < loadedSize; i++) {
		if ((i - offset) >= previewSize) break;
		std::cout << to_hex(loadedData[i]) << " ";
	}
	std::cout << std::endl;
}

inline BYTE* load_file(const char* filename, size_t& buf_size)
{
	FILE* fp = nullptr;
	fopen_s(&fp, filename, "rb");
	if (!fp) return nullptr;

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0, SEEK_SET);

	BYTE* buf = (BYTE*)::calloc(size, 1);
	if (!buf) return nullptr;

	buf_size = fread(buf, 1, size, fp);
	fclose(fp);
	std::cout << "Loaded: " << buf_size << " bytes\n";
	return buf;
}
