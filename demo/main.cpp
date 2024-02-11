#include <iostream>
#include <sig_finder.h>
#include "util.h"

using namespace sig_finder;

typedef struct {
	BYTE *ptr;
	size_t size;
} t_pattern;

void print_results(const char caller[], size_t counter, size_t timeInMs)
{
	std::cout << caller << ":\n\t Occ. counted: " << std::dec << counter << " Time: " << timeInMs << " ms." << std::endl;
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
	print_results(__FUNCTION__, counter, (end - start));
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
	BYTE prolog32_pattern[] = {
		0x55, // PUSH EBP
		0x8b, 0xEC // MOV EBP, ESP
	};

	BYTE prolog32_2_pattern[] = {
		0x55, // PUSH EBP
		0x89, 0xE5 // MOV EBP, ESP
	};

	BYTE prolog32_3_pattern[] = {
		0x60, // PUSHAD
		0x89, 0xE5 // MOV EBP, ESP
	};

	BYTE prolog64_pattern[] = {
		0x40, 0x53,       // PUSH RBX
		0x48, 0x83, 0xEC // SUB RSP, <BYTE>
	};
	BYTE prolog64_2_pattern[] = {
		0x55,            // PUSH RBP
		0x48, 0x8B, 0xEC // MOV RBP, RSP
	};
	BYTE prolog64_3_pattern[] = {
		0x40, 0x55,      // PUSH RBP
		0x48, 0x83, 0xEC // SUB RSP, <BYTE>
	};
	BYTE prolog64_4_pattern[] = {
		0x53,            // PUSH RBX
		0x48, 0x81, 0xEC // SUB RSP, <DWORD>
	};
	BYTE prolog64_5_pattern[] = {
		0x48, 0x83, 0xE4, 0xF0 // AND rsp, FFFFFFFFFFFFFFF0; Align RSP to 16 bytes
	};
	BYTE prolog64_6_pattern[] = {
		0x57,            // PUSH RDI
		0x48, 0x89, 0xE7 // MOV RDI, RSP
	};
	BYTE prolog64_7_pattern[] = {
		 0x48, 0x8B, 0xC4, // MOV RAX, RSP
		 0x48, 0x89, 0x58, 0x08, // MOV QWORD PTR [RAX + 8], RBX
		 0x4C, 0x89, 0x48, 0x20, // MOV QWORD PTR [RAX + 0X20], R9
		 0x4C, 0x89, 0x40, 0x18, // MOV QWORD PTR [RAX + 0X18], R8
		 0x48, 0x89, 0x50, 0x10, // MOV QWORD PTR [RAX + 0X10], RDX
		 0x55, // PUSH RBP
		 0x56, // PUSH RSI
		 0x57, // PUSH RDI 
		 0x41, 0x54, // PUSH R12
		 0x41, 0x55, // PUSH R13
		 0x41, 0x56, // PUSH R14
		 0x41, 0x57 // PUSH R15
	};

	char text_pattern[] = "module";

	t_pattern patterns[] = {
		{ prolog32_pattern,   sizeof(prolog32_pattern) },
		{ prolog32_2_pattern, sizeof(prolog32_2_pattern) },
		{ prolog32_3_pattern, sizeof(prolog32_3_pattern) },
		{ prolog64_pattern,   sizeof(prolog64_pattern)   },
		{ prolog64_2_pattern, sizeof(prolog64_2_pattern) },
		{ prolog64_3_pattern, sizeof(prolog64_3_pattern) },
		{ prolog64_4_pattern, sizeof(prolog64_4_pattern) },
		{ prolog64_5_pattern, sizeof(prolog64_5_pattern) },
		{ prolog64_6_pattern, sizeof(prolog64_6_pattern) },
		{ prolog64_7_pattern, sizeof(prolog64_7_pattern) },
		{ (BYTE*)text_pattern, strlen(text_pattern) },
	};
	size_t counter = 0;
	DWORD start = GetTickCount();
	for (DWORD i = 0; i < _countof(patterns); i++) {
		counter += count_patterns(loadedData, loadedSize, patterns[i].ptr, patterns[i].size);
	}
	DWORD end = GetTickCount();
	print_results(__FUNCTION__, counter, (end - start));
	return counter;
}

size_t tree_count(BYTE* loadedData, size_t loadedSize)
{
	BYTE prolog32_pattern[] = {
		0x55, // PUSH EBP
		0x8b, 0xEC // MOV EBP, ESP
	};

	BYTE prolog32_2_pattern[] = {
		0x55, // PUSH EBP
		0x89, 0xE5 // MOV EBP, ESP
	};

	BYTE prolog32_3_pattern[] = {
		0x60, // PUSHAD
		0x89, 0xE5 // MOV EBP, ESP
	};

	BYTE prolog64_pattern[] = {
		0x40, 0x53,       // PUSH RBX
		0x48, 0x83, 0xEC // SUB RSP, <BYTE>
	};
	BYTE prolog64_2_pattern[] = {
		0x55,            // PUSH RBP
		0x48, 0x8B, 0xEC // MOV RBP, RSP
	};
	BYTE prolog64_3_pattern[] = {
		0x40, 0x55,      // PUSH RBP
		0x48, 0x83, 0xEC // SUB RSP, <BYTE>
	};
	BYTE prolog64_4_pattern[] = {
		0x53,            // PUSH RBX
		0x48, 0x81, 0xEC // SUB RSP, <DWORD>
	};
	BYTE prolog64_5_pattern[] = {
		0x48, 0x83, 0xE4, 0xF0 // AND rsp, FFFFFFFFFFFFFFF0; Align RSP to 16 bytes
	};
	BYTE prolog64_6_pattern[] = {
		0x57,            // PUSH RDI
		0x48, 0x89, 0xE7 // MOV RDI, RSP
	};
	BYTE prolog64_7_pattern[] = {
		 0x48, 0x8B, 0xC4, // MOV RAX, RSP
		 0x48, 0x89, 0x58, 0x08, // MOV QWORD PTR [RAX + 8], RBX
		 0x4C, 0x89, 0x48, 0x20, // MOV QWORD PTR [RAX + 0X20], R9
		 0x4C, 0x89, 0x40, 0x18, // MOV QWORD PTR [RAX + 0X18], R8
		 0x48, 0x89, 0x50, 0x10, // MOV QWORD PTR [RAX + 0X10], RDX
		 0x55, // PUSH RBP
		 0x56, // PUSH RSI
		 0x57, // PUSH RDI 
		 0x41, 0x54, // PUSH R12
		 0x41, 0x55, // PUSH R13
		 0x41, 0x56, // PUSH R14
		 0x41, 0x57 // PUSH R15
	};
	Node rootN;

	rootN.addPattern("prolog32_pattern", prolog32_pattern, sizeof(prolog32_pattern));
	rootN.addPattern("prolog32_2_pattern", prolog32_2_pattern, sizeof(prolog32_2_pattern));
	rootN.addPattern("prolog32_3_pattern", prolog32_3_pattern, sizeof(prolog32_3_pattern));

	rootN.addPattern("prolog64_pattern", prolog64_pattern, sizeof(prolog64_pattern));
	rootN.addPattern("prolog64_2_pattern", prolog64_2_pattern, sizeof(prolog64_2_pattern));
	rootN.addPattern("prolog64_3_pattern", prolog64_3_pattern, sizeof(prolog64_3_pattern));

	rootN.addPattern("prolog64_4_pattern", prolog64_4_pattern, sizeof(prolog64_4_pattern));
	rootN.addPattern("prolog64_5_pattern", prolog64_5_pattern, sizeof(prolog64_5_pattern));
	rootN.addPattern("prolog64_6_pattern", prolog64_6_pattern, sizeof(prolog64_6_pattern));
	rootN.addPattern("prolog64_7_pattern", prolog64_7_pattern, sizeof(prolog64_7_pattern));
	rootN.addTextPattern("module");
	std::vector<Match> allMatches;

	DWORD start = GetTickCount();
	size_t counter = find_all_matches(rootN, loadedData, loadedSize, allMatches);
	DWORD end = GetTickCount();
	print_results(__FUNCTION__, counter, (end - start));
	return counter;
}

void naive_search(BYTE* loadedData, size_t loadedSize)
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
}

void tree_search(BYTE* loadedData, size_t loadedSize)
{
	Node rootN;
	const BYTE pattern[] = { 0x40, 0x53, 0x48, 0x83, 0xec };
	rootN.addPattern("pattern", pattern, sizeof(pattern));
	std::vector<Match> allMatches;
	DWORD start = GetTickCount();
	size_t counter = find_all_matches(rootN, loadedData, loadedSize, allMatches);
	DWORD end = GetTickCount();
	print_results(__FUNCTION__, counter, (end - start));
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

	if (!aho_corasic_test()) return (-1);
	if (!aho_corasic_test2()) return (-1);
	if (!aho_corasic_test3()) return (-1);
	if (!aho_corasic_test4()) return (-1);
	if (!aho_corasic_test5()) return (-1);

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
	naive_search(loadedData, loadedSize);
	tree_search(loadedData, loadedSize);
	std::cout << "---\n";
	naive_count(loadedData, loadedSize);
	tree_count(loadedData, loadedSize);
	std::cout << "---\n";
	multi_search(loadedData, loadedSize);

	std::vector<Signature*> signatures;
	if (!sig_finder::Signature::loadFromFile(argv[2], signatures)) {
		std::cerr << "Could not load signatures from file!\n";
		return 2;
	}
	Node rootN;
	rootN.addPatterns(signatures);
	find_matches(rootN, loadedData, loadedSize, false);

	std::cout << "Finished!\n";
	return 0;
}
