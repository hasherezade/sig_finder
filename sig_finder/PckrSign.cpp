#include "PckrSign.h"

#include <iomanip>
#include <sstream>
#include "util.h"



bool sig_ma::PckrSign::addNode(uint8_t val, sig_type vtype, uint8_t vmask)
{
	nodes.push_back(SigNode(val, vtype, vmask));

	const size_t kMaxPreview = 255;
	if (nodes.size() < kMaxPreview) {
		// add to the content preview:
		if (vtype == IMM) {
			signContent += util::to_hex(val) + " ";
		}
		else if (vtype == PARTIAL) {
			if (vmask == 0xF0) {
				signContent += util::to_hex((val >> 4), 1) + "? ";
			} else {
				signContent += "?" + util::to_hex(val, 1) + " ";
			}
		}
		else if (vtype == WILDC) {
			signContent += "?? ";
		}
	}
	else if (nodes.size() == kMaxPreview) {
		signContent += "...";
	}
	return true;
}
