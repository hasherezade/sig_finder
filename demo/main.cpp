#include <iostream>
#include <sig_finder.h>
#include "util.h"
#include "code_patterns.h"

using namespace sig_finder;


void print_results(const char desc[], size_t counter, size_t timeInMs)
{
	float seconds = ((float)timeInMs / 1000);
	float minutes = ((float)timeInMs / 60000);
	std::cout << desc << ":\n\t Occ. counted: " << std::dec << counter << " Time: "
		<< timeInMs << " ms.";
	if (seconds > 0.5) {
		std::cout << " = " << seconds << " sec.";
	}
	if (minutes > 0.5) {
		std::cout << " = " << minutes << " min.";
	}
	std::cout << std::endl;
}


size_t find_matches(Node &rootN, BYTE loadedData[], size_t loadedSize, const char desc[], bool showMatches, bool showHexPreview = false)
{
	std::string inputStr((char*)loadedData);
	std::cout << "Input: " << inputStr << " : " << loadedSize << std::endl;
	std::vector<Match> allMatches;
	DWORD start = GetTickCount();
	size_t counter = find_all_matches(rootN, loadedData, loadedSize, allMatches);
	DWORD end = GetTickCount();
	for (auto itr = allMatches.begin(); itr != allMatches.end(); ++itr) {
		Match m = *itr;
		if (showMatches) {
			std::cout << "Match at: " << std::hex << m.offset << ", size: " << m.sign->size() << " : \"" << m.sign->name << "\"";
			if (showHexPreview) {
				std::cout << "\t";
				show_hex_preview(loadedData, loadedSize, m.offset, m.sign->size());
			}
			std::cout << std::endl;
		}
	}
	char report[] = "raw_offsets.tag";
	if (Match::saveMatches(report, allMatches, ';')) {
		std::cout << "[+] Saved matches to: " << report << std::endl;
	}
	print_results(desc, counter, (end - start));
	return counter;
}

size_t count_patterns(BYTE* buffer, size_t buf_size, BYTE* pattern_buf, size_t pattern_size)
{
	size_t count = 0;
	for (size_t i = 0; (i + pattern_size) < buf_size; i++) {
		if (memcmp(buffer + i, pattern_buf, pattern_size) == 0) {
			count++;
		}
	}
	return count;
}

size_t naive_count(BYTE* loadedData, size_t loadedSize)
{
	t_pattern patterns[_countof(patterns32) + _countof(patterns64) + 1] = { 0 };

	// init patterns list:
	size_t i = 0;
	for (size_t k = 0; k <_countof(patterns32); k++, i++)
	{
		const t_pattern& p = patterns32[k];
		patterns[i].ptr = p.ptr;
		patterns[i].size = p.size;
	}
	for (size_t k = 0; k < _countof(patterns64); k++, i++)
	{
		const t_pattern& p = patterns64[k];
		patterns[i].ptr = p.ptr;
		patterns[i].size = p.size;
	}

	const char text_pattern[] = "module";
	patterns[i].ptr = (BYTE*)text_pattern;
	patterns[i].size = strlen(text_pattern);

	// search through the list:
	size_t counter = 0;
	DWORD start = GetTickCount();
	for (DWORD i = 0; i < _countof(patterns); i++) {
		const t_pattern& p = patterns[i];
		if (!p.ptr) continue;
		counter += count_patterns(loadedData, loadedSize, p.ptr, p.size);
	}
	DWORD end = GetTickCount();
	print_results(__FUNCTION__, counter, (end - start));
	return counter;
}

size_t tree_count(BYTE* loadedData, size_t loadedSize)
{
	Node rootN;
	// init patterns list:
	for (size_t i = 0; i < _countof(patterns32); i++) {
		const t_pattern& pattern = patterns32[i];
		std::string name = "prolog32_" + std::to_string(i);
		rootN.addPattern(name.c_str(), pattern.ptr, pattern.size);
	}
	for (size_t i = 0; i < _countof(patterns64); i++) {
		const t_pattern& pattern = patterns64[i];
		std::string name = "prolog64_" + std::to_string(i);
		rootN.addPattern(name.c_str(), pattern.ptr, pattern.size);
	}
	rootN.addTextPattern("module");

	// search through the list:
	std::vector<Match> allMatches;
	DWORD start = GetTickCount();
	size_t counter = find_all_matches(rootN, loadedData, loadedSize, allMatches);
	DWORD end = GetTickCount();
	print_results(__FUNCTION__, counter, (end - start));
	return counter;
}

size_t naive_search(BYTE* loadedData, size_t loadedSize)
{
	const BYTE pattern[] = { 0x40, 0x53, 0x48, 0x83, 0xec };
	size_t counter = 0;
	DWORD start = GetTickCount();

	for (size_t i = 0; i < loadedSize; i++) {
		if ((loadedSize - i) >= sizeof(pattern)) {
			if (::memcmp(loadedData + i, pattern, sizeof(pattern)) == 0) counter++;
		}
	}

	DWORD end = GetTickCount();
	print_results(__FUNCTION__, counter, (end - start));
	return counter;
}

size_t tree_search(BYTE* loadedData, size_t loadedSize)
{
	Node rootN;
	const BYTE pattern[] = { 0x40, 0x53, 0x48, 0x83, 0xec };
	rootN.addPattern("pattern", pattern, sizeof(pattern));
	std::vector<Match> allMatches;
	DWORD start = GetTickCount();
	size_t counter = find_all_matches(rootN, loadedData, loadedSize, allMatches);
	DWORD end = GetTickCount();
	print_results(__FUNCTION__, counter, (end - start));
	return counter;
}


void multi_search(BYTE* loadedData, size_t loadedSize)
{
	Node rootN;

	const BYTE pattern[] = { 0x40, 0x53, 0x48, 0x83, 0xec };
	rootN.addPattern("prolog32_1", pattern, sizeof(pattern));

	const BYTE pattern2[] = { 0x55, 0x48, 0x8B, 0xec };
	rootN.addPattern("prolog32_2", pattern2, sizeof(pattern2));

	const BYTE pattern3[] = { 0x40, 0x55, 0x48, 0x83, 0xec };
	const BYTE pattern3_mask[] = { 0xFF, 0x00, 0xF0, 0xF0, 0xF0 };
	Signature sign("prolog32_3", pattern3, sizeof(pattern3), pattern3_mask);

	std::cout << sign.name << " : " << sign.toByteStr() << "\n";

	rootN.addTextPattern("module");

	Signature* sign1 = Signature::loadFromByteStr("prolog1", "40 ?? 4? 8? e?");
	std::cout << sign1->name << " : " << sign1->toByteStr() << "\n";
	if (!sign1) {
		std::cout << "Could not load the signature!\n";
		return;
	}
	rootN.addPattern(*sign1);

	find_matches(rootN, loadedData, loadedSize, __FUNCTION__, false);
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

	if (find_matches(rootN, loadedData, loadedSize, __FUNCTION__, true) == 3) {
		return true;
	}
	std::cerr << "[-]" << __FUNCTION__ << " : Test failed.\n";
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
	if (find_matches(rootN, loadedData, loadedSize, __FUNCTION__, true) == 3) {
		return true;
	}
	std::cerr << "[-]" << __FUNCTION__ << " : Test failed.\n";
	return false;
}

bool aho_corasic_test3()
{
	Node rootN;

	BYTE loadedData[] = "h he her hers";
	size_t loadedSize = sizeof(loadedData);

	rootN.addTextPattern("hers");
	if (find_matches(rootN, loadedData, loadedSize, __FUNCTION__, true) == 1) {
		return true;
	}
	std::cerr << "[-]" << __FUNCTION__ << " : Test failed.\n";
	return false;
}

bool aho_corasic_test4()
{
	Node rootN;

	BYTE loadedData[] = "hehehehehe";
	size_t loadedSize = sizeof(loadedData);

	rootN.addTextPattern("he");
	rootN.addTextPattern("hehehehe");
	if (find_matches(rootN, loadedData, loadedSize, __FUNCTION__, true) == 7) {
		return true;
	}
	std::cerr << "[-]" << __FUNCTION__ << " : Test failed.\n";
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
	
	if (find_matches(rootN, loadedData, loadedSize, __FUNCTION__, true) == 0) {
		return true;
	}
	std::cerr << "[-]" << __FUNCTION__ << " : Test failed.\n";
	return false;
}

int main(int argc, char *argv[])
{
	if (!aho_corasic_test()) return (-1);
	if (!aho_corasic_test2()) return (-1);
	if (!aho_corasic_test3()) return (-1);
	if (!aho_corasic_test4()) return (-1);
	if (!aho_corasic_test5()) return (-1);
	std::cout << "[+] All passed.\n";

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
	size_t nRes = naive_search(loadedData, loadedSize);
	size_t tRes = tree_search(loadedData, loadedSize);
	if (nRes != tRes) {
		std::cerr << "[-] Test failed.\n";
		return (-2);
	}
	std::cout << "---\n";
	nRes = naive_count(loadedData, loadedSize);
	tRes = tree_count(loadedData, loadedSize);
	if (nRes != tRes) {
		std::cerr << "[-] Test failed.\n";
		return (-2);
	}
	std::cout << "---\n";
	multi_search(loadedData, loadedSize);

	std::vector<Signature*> signatures;
	if (!sig_finder::Signature::loadFromFile(argv[2], signatures)) {
		std::cerr << "Could not load signatures from file!\n";
		return 2;
	}
	Node rootN;
	rootN.addPatterns(signatures);
	find_matches(rootN, loadedData, loadedSize, "from_sig_file", false);

	std::cout << "Finished!\n";
	return 0;
}
