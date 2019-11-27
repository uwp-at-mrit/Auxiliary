#include "datum/natural.hpp"
#include "datum/fixnum.hpp"

using namespace WarGrey::SCADA;

static uint8 byte_to_hexchar(uint8 ch) {
	if (ch >= 10) {
		switch (ch) {
		case 10: ch = 'A'; break;
		case 11: ch = 'B'; break;
		case 12: ch = 'C'; break;
		case 13: ch = 'D'; break;
		case 14: ch = 'E'; break;
		case 15: ch = 'F'; break;
		}
	} else {
		ch += '0';
	}

	return ch;
}

/*************************************************************************************************/
Natural::~Natural() {
	if (this->natural != nullptr) {
		delete [] this->natural;
	}
}

Natural::Natural(unsigned int n) : Natural((unsigned long long)n) {}
Natural::Natural(std::string& nstr, size_t nstart, size_t nend) : Natural(nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}
Natural::Natural(std::wstring& nstr, size_t nstart, size_t nend) : Natural(nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}
Natural::Natural(Platform::String^ nstr, size_t nstart, size_t nend) : Natural(nstr->Data(), nstart, ((nend <= nstart) ? nstr->Length() : nend)) {}

Natural* Natural::from_hexstring(std::string& nstr, size_t nstart, size_t nend) {
	return Natural::from_hexstring(nstr.c_str, nstart, ((nend <= nstart) ? nstr.size() : nend));
}

Natural* Natural::from_hexstring(std::wstring& nstr, size_t nstart, size_t nend) {
	return Natural::from_hexstring(nstr.c_str, nstart, ((nend <= nstart) ? nstr.size() : nend));
}

Natural* Natural::from_hexstring(Platform::String^ nstr, size_t nstart, size_t nend) {
	return Natural::from_hexstring(nstr->Data(), nstart, ((nend <= nstart) ? nstr->Length() : nend));
}

Natural* Natural::from_hexstring(const char nbytes[], size_t nstart, size_t nend) {
	return nullptr;
}

Natural* Natural::from_hexstring(const wchar_t nbytes[], size_t nstart, size_t nend) {
	return nullptr;
}

Natural::Natural(unsigned long long n) : capacity(sizeof(unsigned long long)), payload(0) {
	this->natural = new uint8[this->capacity];
	
	for (size_t idx = this->capacity; idx > 0; idx--) {
		uint8 ui8 = (n & 0xFF);

		this->natural[idx - 1] = ui8;
		if (ui8 > 0) {
			this->payload = this->capacity - idx + 1;
		}

		n >>= 8;
	}
}

Natural::Natural(const char nbytes[], size_t nstart, size_t nend) : natural(nullptr), payload(0) {
	this->capacity = nend - nstart;

	if (this->capacity > 0) {
		this->natural = new uint8[this->capacity];

		for (size_t idx = 0; idx < this->capacity; idx++) {
			size_t sidx = idx + nstart;

			if (nbytes[sidx] == 0) {
				this->natural[idx] = 0;
			} else {
				this->payload = this->capacity - idx;
				memcpy(this->natural + idx, nbytes + sidx, nend - sidx);
				break;
			}
		}
	}
}

Natural::Natural(const wchar_t nchars[], size_t nstart, size_t nend) : natural(nullptr), payload(0) {
	this->capacity = (nend - nstart) * 2;

	if (this->capacity > 0) {
		this->natural = new uint8[this->capacity];
		size_t idx = 0U;

		for (size_t sidx = nstart; sidx < nend; sidx++) {
			uint16 ch = (uint16)(nchars[sidx]);

			this->natural[idx++] = (uint8)(ch >> 8);
			this->natural[idx++] = (uint8)(ch & 0xFFU);

			if (this->payload == 0) {
				if (ch > 0xFFU) {
					this->payload = this->capacity - (idx - 2);
				} else if (ch > 0) {
					this->payload = this->capacity - (idx - 1);
				}
			}
		}
	}
}

size_t Natural::length() {
	return this->payload;
}

size_t Natural::integer_length() {
	size_t s = 0;

	if (this->payload > 0) {
		s = (this->payload - 1) * 8;
		s += ::integer_length(this->natural[this->capacity - this->payload]);
	}

	return s;
}

std::string Natural::to_hexstring() {
	std::string hex(fxmax(this->payload, 1ULL) * 2, '0');
	size_t payload_idx = this->capacity - this->payload;
	size_t msb_idx = 0U;

	for (size_t idx = 0; idx < this->payload; idx++) {
		uint8 ubyte = this->natural[idx + payload_idx];
		
		if (ubyte <= 0xF) {
			msb_idx++;
			hex[msb_idx++] = byte_to_hexchar(ubyte);
		} else {
			hex[msb_idx++] = byte_to_hexchar(ubyte >> 4);
			hex[msb_idx++] = byte_to_hexchar(ubyte & 0xF);
		}
	}

	return hex;
}
