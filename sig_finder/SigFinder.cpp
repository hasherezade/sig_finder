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

matched_set SigFinder::getMatching(const uint8_t *buf, long buf_size, long start_offset, match_direction md, bool stopOnFirst)
{
	long srch_size = buf_size - start_offset;
	const uint8_t* srch_bgn = buf + start_offset;
	size_t min_sig_len = tree.getMinLen();

	matched_set all_matches;

	bool skipNOPs = (md == FIXED)? true : false;

	if (md == FIXED) {
		matched mround = tree.getMatching(srch_bgn, srch_size, skipNOPs);
		mround.match_offset = srch_bgn - (buf + start_offset);
		all_matches.append(mround);
		return all_matches;
	}

	if (md == FRONT_TO_BACK) {
		while (srch_size > min_sig_len) {
			matched mround = tree.getMatching(srch_bgn, srch_size, skipNOPs);
			if (mround.signs.size() > 0) {
				mround.match_offset = srch_bgn - (buf + start_offset);
				all_matches.append(mround);
				if (stopOnFirst) {
					return all_matches;
				}
			}
			srch_size--;
			srch_bgn++;
		}

	} else if (md == BACK_TO_FRONT) {
		while (srch_size > min_sig_len && srch_size <= buf_size) {
			matched mround = tree.getMatching(srch_bgn, srch_size, skipNOPs);
			if (mround.signs.size() > 0) {
				mround.match_offset = (buf + start_offset) - srch_bgn;
				all_matches.append(mround);
				if (stopOnFirst) {
					return all_matches;
				}
			}
			srch_size++;
			srch_bgn--;
		}
	}
	return all_matches;
}
