#pragma once
#include "../win_types.h"

namespace sig_finder {

	namespace util {

		class CRC32
		{
		public:

			static const DWORD Seed = 0xFFFFFFFF;
			static const DWORD Poly = 0xEDB88320;

			CRC32()
				: checksum(0)
			{
				_initTable();
			}

			bool update(const BYTE* buf, size_t buf_size)
			{
				if (!buf || !buf_size) return false;
				DWORD c = checksum ^ Seed;
				for (size_t i = 0; i < buf_size; ++i) {
					c = table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
				}
				checksum = c ^ Seed;
				return true;
			}

			bool update(const BYTE element)
			{
				DWORD c = checksum ^ Seed;
				c = table[(c ^ element) & 0xFF] ^ (c >> 8);
				checksum = c ^ Seed;
				return true;
			}

			uint32_t getChecksum() const
			{
				return checksum;
			}

		private:
			void _initTable()
			{
				for (DWORD i = 0; i < 256; i++) {
					DWORD next = i;
					for (size_t j = 0; j < 8; j++) {
						if (next & 1) {
							next = Poly ^ (next >> 1);
						}
						else {
							next >>= 1;
						}
					}
					table[i] = next;
				}
			}

			DWORD table[256];
			DWORD checksum;
		};


		inline DWORD calcCRC32(const void* data, size_t dataSize)
		{
			util::CRC32 crc;
			crc.update(static_cast<const BYTE*>(data), dataSize);
			return crc.getChecksum();
		}

	}; // namespace util


}; //sig_finder
