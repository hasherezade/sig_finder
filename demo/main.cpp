#include <iostream>
#include <sig_finder.h>

#include "pattern_tree.h"
using namespace pattern_tree;

BYTE* load_file(const char* filename, size_t& buf_size)
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


void init_byte_signs(sig_ma::SigFinder &signFinder)
{
	DWORD start = GetTickCount();
	/*
	const BYTE pattern3[] =      { 0x40, 0x55, 0x48, 0x83, 0xec };
	const BYTE pattern3_mask[] = { 0xFF, 0xF0, 0xFF, 0xFF, 0xFF };
	*/
	// 32 bit
	//signFinder.loadSignature("prolog32_1", "55 8b ec");

	//signFinder.loadSignature("prolog32_2", "55 89 e5");
	//signFinder.loadSignature("prolog32_3", "60 89 ec");

	// 64 bit
	signFinder.loadSignature("prolog64_1", "40 ?? 4? 8? e?");
	//signFinder.loadSignature("prolog64_2", "55 48 8B EC");
	//signFinder.loadSignature("prolog64_3", "40 55 48 83 EC");

	//signFinder.loadSignature("prolog64_4", "53 48 81 EC");
	//signFinder.loadSignature("prolog64_5", "48 83 E4 f0");
	//signFinder.loadSignature("prolog64_6", "57 48 89 E7");

	//signFinder.loadSignature("prolog64_7", "48 8B C4 48 89 58 08 4C 89 48 20 4C 89 40 18 48 89 50 10 55 56 57 41 54 41 55 41 56 41 57");

	DWORD end = GetTickCount();
	std::cout << "Init signs finished in: " << (end - start) << " ms." << "\n";
}

void init_string_signs(sig_ma::SigFinder& signFinder)
{
	signFinder.loadSignature("str1", "module", false);
}

inline bool is_matching(const BYTE* loadedData, const size_t loadedSize, const BYTE* pattern, const size_t pattern_size)
{
	if (loadedSize < pattern_size) return false;

	for (size_t j = 0; j < pattern_size; j++) {
		if (loadedData[j] != pattern[j]) return false;
	}
	return true;
}

#include <iostream>
#include <iomanip>
#include <sstream>

inline std::string to_hex(const uint8_t val, int width = 2)
{
	std::stringstream ss;
	ss << std::setw(width) << std::setfill('0') << std::hex << (unsigned int)(val);
	return ss.str();
}

void show_hex_preview(BYTE* loadedData, size_t loadedSize, size_t offset, size_t previewSize)
{
	if (!previewSize || !loadedSize) return;
	for (size_t i = offset; i < loadedSize; i++) {
		if ((i - offset) >= previewSize) break;
		std::cout << to_hex(loadedData[i]) << " ";
	}
	std::cout << std::endl;
}

void walk_array1(BYTE* loadedData, size_t loadedSize)
{
	const BYTE pattern[] = { 0x40, 0x53, 0x48, 0x83, 0xec };
	size_t counter = 0;
	DWORD start = GetTickCount();
	for (size_t k = 0; k < 1; k++) {
		counter = 0;
		for (size_t i = 0; i < loadedSize; i++) {
			if ((loadedSize - i) >= sizeof(pattern)) {
				if (::memcmp(loadedData + i, pattern, sizeof(pattern)) == 0) counter++;
			}
		}
	}
	DWORD end = GetTickCount();
	std::cout << __FUNCTION__ << " Occ. counted: " << counter << " Time: " << (end - start) << " ms." << std::endl;
}

size_t find_matches(Node &rootN, BYTE loadedData[], size_t loadedSize, bool showMatches = true)
{
	std::cout << "Input: " << loadedData << " : " << loadedSize << std::endl;
	std::vector<Match> allMatches;
	DWORD start = GetTickCount();
	size_t counter = find_all_matches(rootN, loadedData, loadedSize, allMatches);
	DWORD end = GetTickCount();
	for (auto itr = allMatches.begin(); itr != allMatches.end(); ++itr) {
		Match m = *itr;
		if (showMatches) {
			std::cout << "Match: " << std::hex << m.offset << " : " << m.sign->name << " : "<< m.sign->size() << "\t";
			show_hex_preview(loadedData, loadedSize, m.offset, m.sign->size());
		}
	}
	std::cout << __FUNCTION__ << std::dec << " Occ. counted: " << counter << " Time: " << (end - start) << " ms." << std::endl;
	return counter;
}


void walk_array2(BYTE* loadedData, size_t loadedSize)
{
	Node rootN;

	//const BYTE pattern[] = { 0x40, 0x53, 0x48, 0x83, 0xec };
	//Node::addPattern(&rootN, "prolog32_1", pattern, sizeof(pattern));

	//const BYTE pattern2[] = { 0x55, 0x48, 0x8B, 0xec };
	//Node::addPattern(&rootN, "prolog32_2", pattern2, sizeof(pattern2));

	const BYTE pattern3[] = { 0x40, 0x55, 0x48, 0x83, 0xec };
	const BYTE pattern3_mask[] = { 0xFF, 0x00, 0xF0, 0xF0, 0xF0 };
	Signature sign("prolog32_3", pattern3, sizeof(pattern3), pattern3_mask);

	std::cout << sign.name << " : " << sign.toByteStr() << "\n";

	//Node::addTextPattern(&rootN, "module");
	Signature* sign1 = Signature::loadFromByteStr("prolog1", "40 ?? 4? 8? e?");
	std::cout << sign1->name << " : " << sign1->toByteStr() << "\n";
	if (!sign1) {
		std::cout << "Could not load the signature!\n";
		return;
	}
	Node::addPattern(&rootN, *sign1);
	find_matches(rootN, loadedData, loadedSize, false);
}

bool aho_corasic_test()
{
	Node rootN;
	BYTE loadedData[] = "GCATCG";
	size_t loadedSize = sizeof(loadedData);

	Node::addTextPattern(&rootN, "ACC");
	Node::addTextPattern(&rootN, "ATC");
	Node::addTextPattern(&rootN, "CAT");
	Node::addTextPattern(&rootN, "CATC");
	Node::addTextPattern(&rootN, "GCG");

	if (find_matches(rootN, loadedData, loadedSize) == 3) {
		return true;
	}
	std::cerr << __FUNCTION__ << " : Test failed.\n";
	return false;
}

bool aho_corasic_test2()
{
	Node rootN;
	BYTE loadedData[] = "ushers";
	size_t loadedSize = sizeof(loadedData);

	Node::addTextPattern(&rootN, "hers");
	Node::addTextPattern(&rootN, "his");
	Node::addTextPattern(&rootN, "he");
	Node::addTextPattern(&rootN, "she");
	if (find_matches(rootN, loadedData, loadedSize) == 3) {
		return true;
	}
	std::cerr << __FUNCTION__ << " : Test failed.\n";
	return false;
}

bool aho_corasic_test3()
{
	Node rootN;

	BYTE loadedData[] = "h he her hers";
	size_t loadedSize = sizeof(loadedData);

	Node::addTextPattern(&rootN, "hers");
	if (find_matches(rootN, loadedData, loadedSize) == 1) {
		return true;
	}
	std::cerr << __FUNCTION__ << " : Test failed.\n";
	return false;
}

bool aho_corasic_test4()
{
	Node rootN;

	BYTE loadedData[] = "hehehehehe";
	size_t loadedSize = sizeof(loadedData);

	Node::addTextPattern(&rootN, "he");
	Node::addTextPattern(&rootN, "hehehehe");
	if (find_matches(rootN, loadedData, loadedSize) == 7) {
		return true;
	}
	std::cerr << __FUNCTION__ << " : Test failed.\n";
	return false;
}

bool aho_corasic_test5()
{
	Node rootN;

	BYTE loadedData[] = "something";
	size_t loadedSize = sizeof(loadedData);

	Node::addTextPattern(&rootN, "hers");
	Node::addTextPattern(&rootN, "his");
	Node::addTextPattern(&rootN, "he");

	if (find_matches(rootN, loadedData, loadedSize) == 0) {
		return true;
	}
	std::cerr << __FUNCTION__ << " : Test failed.\n";
	return false;
}

int main(int argc, char *argv[])
{

	aho_corasic_test();
	aho_corasic_test2();
	aho_corasic_test3();
	aho_corasic_test4();
	aho_corasic_test5();

	if (argc < 2) {
		std::cout << " Args: <filename>\n";
		return 0;
	}

	size_t loadedSize = 0;
	BYTE *loadedData = load_file(argv[1], loadedSize);
	if (!loadedData) {
		std::cout << "Failed to load!\n";
		return 0;
	}

	sig_ma::SigFinder signFinder;

	init_byte_signs(signFinder);
	//init_string_signs(signFinder);

	//for (size_t i = 0; i < 20; i++) {
		//walk_array1(loadedData, loadedSize);
		walk_array2(loadedData, loadedSize);
	//}

	DWORD start = GetTickCount();
	sig_ma::matched_set mS = signFinder.getMatching(loadedData, loadedSize, 0, sig_ma::FRONT_TO_BACK, false);
	DWORD end = GetTickCount();

	std::cout << "Old matcher" << std::dec << " Occ. counted: " << mS.size() << " Time: " << (end - start) << " ms." << std::endl;
	//std::cout << "All matched: " << mS.size() << " Time: " << (end - start) << " ms." << std::endl;

#ifdef _PRINT_ALL
	for (auto itr = mS.matchedSigns.begin(); itr != mS.matchedSigns.end(); ++itr) {
		std::cout << "Offset: " << std::hex << itr->match_offset << "\n";
		std::cout << loadedData + itr->match_offset << "\n";
		std::cout << "----\n";
	}
#endif
	std::cout << "Finished!\n";
	return 0;
}
