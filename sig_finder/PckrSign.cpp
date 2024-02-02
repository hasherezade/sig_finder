#include "PckrSign.h"

#include <iomanip>
#include <sstream>


namespace sig_ma {
	namespace util {

		std::string to_hex(const uint8_t val, int width = 2)
		{
			std::stringstream ss;
			ss << std::setw(width) << std::setfill('0') << std::hex << (unsigned int)(val);
			return ss.str();
		}

	};
};

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
