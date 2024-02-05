#include "SigFinder.h"

#include <iostream>
#include <fstream>

using namespace sig_ma;

//----------------------------------------------------

/* read file with signatures */
size_t SigFinder::loadSignaturesFromFile(const std::string &fname)
{
	std::ifstream input;
	input.open(fname);
	if (!input.is_open()) {
		//std::cout << "File not found: " << fname << std::endl;
		return 0;
	}
	const size_t num = tree.loadFromFile(input);
	input.close();
	return num;
}

bool SigFinder::loadSignature(const std::string &sigName, const std::string &sigContent)
{
	return tree.loadSignature(sigName, sigContent);
}

matched SigFinder::getMatching(const uint8_t *buf, long buf_size, long start_offset, match_direction md)
{
	long srch_size = buf_size - start_offset;
	const uint8_t* srch_bgn = buf + start_offset;
	size_t min_sig_len = tree.getMinLen();
	
	matched matched_set;
	matched_set.match_offset = 0;
	const bool stopOnFirst = true;
	bool skipNOPs = true;// (md == FIXED) ? true : false;

	if (md == FIXED) {
		matched_set = tree.getMatching(srch_bgn, srch_size, skipNOPs);
		matched_set.match_offset += srch_bgn - (buf + start_offset);
		return matched_set; 
	}

	if (md == FRONT_TO_BACK) {
		while (srch_size > min_sig_len) {
			matched_set = tree.getMatching(srch_bgn, srch_size, skipNOPs);

			if (stopOnFirst && matched_set.signs.size() > 0) {
				matched_set.match_offset += srch_bgn - (buf + start_offset);
				return matched_set;
			}
			srch_size--;
			srch_bgn++;
		}

	} else if (md == BACK_TO_FRONT) {
		while (srch_size > min_sig_len && srch_size <= buf_size) {
			matched_set = tree.getMatching(srch_bgn, srch_size, skipNOPs);
			if (stopOnFirst && matched_set.signs.size() > 0) {
				matched_set.match_offset += (buf + start_offset) - srch_bgn;
				return matched_set;
			}
			srch_size++;
			srch_bgn--;
		}
	}
	return matched_set; /* empty set */
}
