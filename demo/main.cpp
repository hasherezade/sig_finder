#include <iostream>
#include <sig_finder.h>

#include <pattern_tree.h>
using namespace sig_finder;

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
			std::cout << "Match: " << std::hex << m.offset << " : " << m.sign->name << "  ["<< m.sign->size() << "]\t";
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
	//rootN.addPattern("prolog32_1", pattern, sizeof(pattern));

	//const BYTE pattern2[] = { 0x55, 0x48, 0x8B, 0xec };
	//rootN.addPattern("prolog32_2", pattern2, sizeof(pattern2));

	const BYTE pattern3[] = { 0x40, 0x55, 0x48, 0x83, 0xec };
	const BYTE pattern3_mask[] = { 0xFF, 0x00, 0xF0, 0xF0, 0xF0 };
	Signature sign("prolog32_3", pattern3, sizeof(pattern3), pattern3_mask);

	std::cout << sign.name << " : " << sign.toByteStr() << "\n";

	//rootN.addTextPattern("module");
	Signature* sign1 = Signature::loadFromByteStr("prolog1", "40 ?? 4? 8? e?");
	std::cout << sign1->name << " : " << sign1->toByteStr() << "\n";
	if (!sign1) {
		std::cout << "Could not load the signature!\n";
		return;
	}
	rootN.addPattern(*sign1);
	find_matches(rootN, loadedData, loadedSize, false);
}

bool aho_corasic_test()
{
	Node rootN;
	BYTE loadedData[] = "GCATCG";
	size_t loadedSize = sizeof(loadedData);

	rootN.addTextPattern("ACC");
	rootN.addTextPattern("ATC");
	rootN.addTextPattern("CAT");
	rootN.addTextPattern("CATC");
	rootN.addTextPattern("GCG");

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

	rootN.addTextPattern("hers");
	rootN.addTextPattern("his");
	rootN.addTextPattern("he");
	rootN.addTextPattern("she");
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

	rootN.addTextPattern("hers");
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

	rootN.addTextPattern("he");
	rootN.addTextPattern("hehehehe");
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

	rootN.addTextPattern("hers");
	rootN.addTextPattern("his");
	rootN.addTextPattern("he");

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

	if (argc < 3) {
		std::cout << " Args: <input_file> <patterns_file>\n";
		return 0;
	}

	size_t loadedSize = 0;
	BYTE *loadedData = load_file(argv[1], loadedSize);
	if (!loadedData) {
		std::cout << "Failed to load!\n";
		return 1;
	}

	std::vector<Signature*> signatures;
	if (!sig_finder::Signature::loadFromFile(argv[2], signatures)) {
		std::cerr << "Could not load signatures from file!\n";
		return 2;
	}
	Node rootN;
	rootN.addPatterns(signatures);
	find_matches(rootN, loadedData, loadedSize, true);

	std::cout << "Finished!\n";
	return 0;
}
