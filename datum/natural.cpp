#include "datum/natural.hpp"

#include "datum/bytes.hpp"
#include "datum/fixnum.hpp"

using namespace WarGrey::SCADA;

template<typename BYTE>
static int natural_from_base16(uint8* natural, const BYTE n[], size_t nstart, size_t nend, size_t capacity) {
	size_t slot = capacity - 1;
	int payload = 0U;

	do {
		uint8 lsb = ((nend > nstart) ? byte_to_hexadecimal((uint8)n[--nend], 0U) : 0U);
		uint8 msb = ((nend > nstart) ? byte_to_hexadecimal((uint8)n[--nend], 0U) : 0U);

		natural[slot--] = (msb << 4U | lsb);

		if (natural[slot + 1] > 0) {
			payload = capacity - (slot + 1);
		}
	} while (nstart < nend);

	return payload;
}

template<typename BYTE>
static size_t natural_from_base(uint8 base, uint8* natural, const BYTE n[], int nstart, int nend, size_t capacity) {
	size_t cursor = capacity - 2;
	size_t payload = 0U;

	natural[capacity - 1] = 0;

	do {
		uint16 decimal = byte_to_decimal((uint8)n[nstart++], 0U);
		
		for (size_t idx = capacity - 1; idx > cursor; idx--) {
			uint16 digit = natural[idx] * base + decimal;

			natural[idx] = (uint8)(digit & 0xFFU);
			decimal = digit >> 8;
		}

		if (decimal > 0) {
			payload = capacity - cursor;
			natural[cursor--] = (uint8)decimal;
			
		}
	} while (nstart < nend);

	return payload;
}

/*************************************************************************************************/
Natural::~Natural() {
	if (this->natural != nullptr) {
		delete [] this->natural;
	}
}

Natural::Natural() : Natural(0ULL) {}
Natural::Natural(unsigned int n) : Natural((unsigned long long)n) {}

Natural::Natural(std::string& nstr, size_t nstart, size_t nend)
	: Natural((const uint8*)nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}

Natural::Natural(std::wstring& nstr, size_t nstart, size_t nend)
	: Natural((const uint16*)nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}

Natural::Natural(Platform::String^ nstr, size_t nstart, size_t nend)
	: Natural((const uint16*)nstr->Data(), nstart, ((nend <= nstart) ? nstr->Length() : nend)) {}

Natural::Natural(uint8 base, std::string& nstr, size_t nstart, size_t nend)
	: Natural(base, (const uint8*)nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}

Natural::Natural(uint8 base, std::wstring& nstr, size_t nstart, size_t nend)
	: Natural(base, (const uint16*)nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}

Natural::Natural(uint8 base, Platform::String^ nstr, size_t nstart, size_t nend)
	: Natural(base, (const uint16*)nstr->Data(), nstart, ((nend <= nstart) ? nstr->Length() : nend)) {}

Natural::Natural(unsigned long long n) : natural(nullptr), capacity(sizeof(unsigned long long)), payload(0U) {
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
	std::string hex(fxmax((unsigned int)this->payload, 1U) * 2, '0');
	size_t payload_idx = this->capacity - this->payload;
	size_t msb_idx = 0U;

	for (size_t idx = 0; idx < this->payload; idx++) {
		uint8 ubyte = this->natural[idx + payload_idx];
		
		if (ubyte <= 0xF) {
			msb_idx++;
			hex[msb_idx++] = hexadecimal_to_byte(ubyte);
		} else {
			hex[msb_idx++] = hexadecimal_to_byte(ubyte >> 4);
			hex[msb_idx++] = hexadecimal_to_byte(ubyte & 0xF);
		}
	}

	return hex;
}

/*************************************************************************************************/
Natural::Natural(const Natural& n) : natural(nullptr), capacity(n.payload), payload(n.payload) {
	if (this->payload > 0) {
		size_t payload_idx = n.capacity - n.payload;

		this->natural = new uint8[this->capacity];
		memcpy(this->natural, n.natural + payload_idx, this->payload);
	}
}

Natural::Natural(Natural&& n) : natural(n.natural), capacity(n.capacity), payload(n.payload) {
	n.on_moved();
}

Natural& Natural::operator=(const Natural& n) {
	if (n.payload <= this->capacity) {
		this->payload = n.payload;
	} else {
		if (this->natural != nullptr) {
			delete[] this->natural;
		}

		this->payload = n.payload;
		this->capacity = this->payload;
		this->natural = new uint8[this->capacity];
	}

	if (this->payload > 0) {
		size_t payload_idx = this->capacity - this->payload;
		size_t n_idx = n.capacity - this->payload;

		memcpy(this->natural + payload_idx, n.natural + n_idx, this->payload);
	}

	return (*this);
}

Natural& Natural::operator=(Natural&& n) {
	if (this != &n) {
		if (this->natural != nullptr) {
			delete[] this->natural;
		}

		this->natural = n.natural;
		this->capacity = n.capacity;
		this->payload = n.payload;

		n.on_moved();
	}

	return (*this);
}

Natural& Natural::operator++() {
	return (*this);
}

Natural Natural::operator++(int postfix) {
	Natural snapshot(*this);

	this->operator++();

	return snapshot;
}

void Natural::on_moved() {
	this->capacity = 0U;
	this->payload = 0U;
	this->natural = nullptr;
}

/*************************************************************************************************/
Natural& Natural::operator+=(const Natural& rhs) {
	size_t mdigits = fxmax(this->payload, rhs.payload);
	size_t lcapacity = this->capacity;
	uint8* lsrc = this->natural;
	uint8* rsrc = rhs.natural;
	uint16 carry = 0U;

	if (this->capacity <= mdigits) {
		this->capacity = mdigits + 1;
		this->natural = new uint8[this->capacity];
	}

	for (size_t idx = 1; idx <= mdigits; idx++) {
		uint16 digit = carry
			+ ((idx <= this->payload) ? lsrc[lcapacity - idx] : 0U)
			+ ((idx <= rhs.payload) ? rsrc[rhs.capacity - idx] : 0U);

		if (digit > 0xFF) {
			this->natural[this->capacity - idx] = (uint8)(digit & 0xFFU);
			carry = 1U;
		} else {
			this->natural[this->capacity - idx] = (uint8)digit;
			carry = 0U;
		}
	}

	this->payload = mdigits + carry;

	if (this->natural != lsrc) {
		delete [] lsrc;
	}

	return (*this);
}

/*************************************************************************************************/
void Natural::from_memory(const uint8 nbytes[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		this->capacity = nend - nstart;
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

void Natural::from_memory(const uint16 nchars[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		this->capacity = (nend - nstart) * 2;
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

void Natural::from_base16(const uint8 nbytes[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		size_t span = nend - nstart;
		
		this->capacity = span / 2 + (span % 2);
		this->natural = new uint8[this->capacity];
		this->payload = natural_from_base16(this->natural, nbytes, nstart, nend, this->capacity);
	}
}

void Natural::from_base16(const uint16 nchars[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		size_t span = nend - nstart;
		
		this->capacity = span / 2 + (span % 2);
		this->natural = new uint8[this->capacity];
		this->payload = natural_from_base16(this->natural, nchars, nstart, nend, this->capacity);
	}
}

void Natural::from_base10(const uint8 nbytes[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		this->capacity = nend - nstart;
		this->natural = new uint8[this->capacity];
		this->payload = natural_from_base(10U, this->natural, nbytes, nstart, nend, this->capacity);
	}
}

void Natural::from_base10(const uint16 nchars[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		this->capacity = nend - nstart;
		this->natural = new uint8[this->capacity];
		this->payload = natural_from_base(10U, this->natural, nchars, nstart, nend, this->capacity);
	}
}


void Natural::from_base8(const uint8 nbytes[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		size_t span = nend - nstart;
		
		this->capacity = (span / 3 + ((span % 3 == 0) ? 0 : 1)) * 2;
		this->natural = new uint8[this->capacity];
		this->payload = natural_from_base(8U, this->natural, nbytes, nstart, nend, this->capacity);
	}
}

void Natural::from_base8(const uint16 nchars[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		size_t span = nend - nstart;
		
		this->capacity = (span / 3 + ((span % 3 == 0) ? 0 : 1)) * 2;
		this->natural = new uint8[this->capacity];
		this->payload = natural_from_base(8U, this->natural, nchars, nstart, nend, this->capacity);
	}
}
