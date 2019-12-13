#include "datum/natural.hpp"

#include "datum/bytes.hpp"
#include "datum/fixnum.hpp"

using namespace WarGrey::SCADA;

template<typename BYTE>
static size_t natural_from_base16(uint8* natural, const BYTE n[], size_t nstart, size_t nend, size_t capacity) {
	size_t slot = capacity - 1;
	size_t payload = 0U;

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

static inline size_t fixnum_length(size_t payload, size_t modulus) {
	return payload / modulus + ((payload % modulus > 0) ? 1 : 0);
}

template<typename UI>
UI fixnum_ref(const uint8* natural, size_t payload, int capacity, int slot_idx, size_t offset, size_t size) {
	UI n = 0U;

	if (payload > 0U) {
		int start0 = capacity - payload;
		int start, end;

		if (slot_idx < 0) {
			start = capacity + slot_idx * size;
		} else {
			start = capacity - (fixnum_length(payload, size) - slot_idx) * size;
		}

		start += offset;
		end = start + int(size);

		start = fxmax(start0, start);
		end = fxmin(end, capacity);

		for (int idx = start; idx < start + int(size); idx++) {
			n = (n << 8U) ^ natural[idx];
		}
	}

	return n;
}

/*************************************************************************************************/
Natural::~Natural() {
	if (this->natural != nullptr) {
		delete [] this->natural;
	}
}

Natural::Natural() : Natural(0ULL) {}

Natural::Natural(bytes& nstr, size_t nstart, size_t nend)
	: Natural(nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}

Natural::Natural(std::string& nstr, size_t nstart, size_t nend)
	: Natural((const uint8*)nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}

Natural::Natural(std::wstring& nstr, size_t nstart, size_t nend)
	: Natural((const uint16*)nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}

Natural::Natural(Platform::String^ nstr, size_t nstart, size_t nend)
	: Natural((const uint16*)nstr->Data(), nstart, ((nend <= nstart) ? nstr->Length() : nend)) {}

Natural::Natural(uint8 base, bytes& nstr, size_t nstart, size_t nend)
	: Natural(base, nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}

Natural::Natural(uint8 base, std::string& nstr, size_t nstart, size_t nend)
	: Natural(base, (const uint8*)nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}

Natural::Natural(uint8 base, std::wstring& nstr, size_t nstart, size_t nend)
	: Natural(base, (const uint16*)nstr.c_str(), nstart, ((nend <= nstart) ? nstr.size() : nend)) {}

Natural::Natural(uint8 base, Platform::String^ nstr, size_t nstart, size_t nend)
	: Natural(base, (const uint16*)nstr->Data(), nstart, ((nend <= nstart) ? nstr->Length() : nend)) {}

Natural::Natural(unsigned long long n) : natural(nullptr), capacity(sizeof(unsigned long long)), payload(0U) {
	this->natural = this->malloc(this->capacity);
	
	for (size_t idx = this->capacity; idx > 0; idx--) {
		uint8 ui8 = (n & 0xFF);

		this->natural[idx - 1] = ui8;
		if (ui8 > 0) {
			this->payload = this->capacity - idx + 1;
		}

		n >>= 8;
	}
}

bool Natural::is_zero() const {
	return ((this->payload == 0)
		|| ((this->payload == 1) // redundant checking
			&& (this->natural[this->capacity - 1] == 0)));
}

bool Natural::is_one() const {
	return ((this->payload == 1)
		&& (this->natural[this->capacity - 1] == 1));
}

size_t Natural::length() const {
	return this->payload;
}

size_t Natural::integer_length() const {
	size_t s = 0;

	if (this->payload > 0) {
		s = (this->payload - 1) * 8;
		s += ::integer_length(this->natural[this->capacity - this->payload]);
	}

	return s;
}

bytes Natural::to_bytes() {
	return bytes(this->natural + (this->capacity - this->payload), this->payload);
}

bytes Natural::to_hexstring() {
	bytes hex(fxmax((unsigned int)this->payload, 1U) * 2, '0');
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

		this->natural = this->malloc(this->capacity);
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
		this->natural = this->malloc(this->capacity);
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

void Natural::on_moved() {
	this->capacity = 0U;
	this->payload = 0U;
	this->natural = nullptr;
}

/*************************************************************************************************/
Natural& Natural::operator++() {
	size_t idx = this->capacity - 1U;
	
	do {
		uint8 digit = this->natural[idx];

		if (digit < 0xFF) {
			this->natural[idx] = digit + 1U;
			
			if (this->payload == 0) {
				this->payload++;
			}

			break;
		} else {
			size_t payload_idx = this->capacity - this->payload;
			
			this->natural[idx] = 0x00U;

			if (idx > payload_idx) {
				idx--;
			} else {
				if (idx == 0U) {
					this->recalloc(this->capacity + 1, 1);
				} else if (idx == payload_idx) {
					this->natural[idx - 1] = 1;
				}

				this->payload++;

				break;
			}
		}
	} while (1);

	return (*this);
}

Natural Natural::operator++(int postfix) {
	Natural snapshot(*this);

	this->operator++();

	return snapshot;
}

#include "syslog.hpp"

Natural& Natural::operator+=(unsigned long long rhs) {
	if (rhs > 0U) {
		size_t digits = fxmax(this->payload, sizeof(unsigned long long));
		size_t nidx = this->capacity - 1;
		uint16 carry = 0U;

		if (this->capacity <= digits) { // deadcode since sizeof(uint64) is the smallest capacity
			this->recalloc(digits + 1);
			nidx = this->capacity - 1;
		}

		do {
			uint16 digit = carry + this->natural[nidx] + (rhs & 0xFFU);

			if (digit > 0xFF) {
				this->natural[nidx] = (uint8)(digit & 0xFFU);
				rhs = (rhs >> 8U) + 1U;
			} else {
				this->natural[nidx] = (uint8)digit;
				rhs >>= 8U;
			}

			nidx--;
		} while (rhs > 0U);

		syslog(Log::Info, L"%08x: %d %d", rhs, this->payload, this->capacity - nidx);
		this->payload = fxmax(this->payload, (this->capacity - nidx - 1));
	}

	return (*this);
}

Natural& Natural::operator+=(const Natural& rhs) {
	if (!rhs.is_zero()) {
		size_t digits = fxmax(this->payload, rhs.payload);
		size_t lcapacity = this->capacity;
		uint8* lsrc = this->natural;
		uint8* rsrc = rhs.natural;
		uint16 carry = 0U;

		if (this->capacity <= digits) {
			this->capacity = digits + 1;
			this->natural = this->malloc(this->capacity);
		}

		for (size_t idx = 1; idx <= digits; idx++) {
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

		if (carry == 1) {
			this->payload = digits + carry;
			this->natural[this->capacity - this->payload] = 1;
		} else {
			this->payload = digits;
		}

		if (this->natural != lsrc) {
			delete[] lsrc;
		}
	}

	return (*this);
}

Natural& Natural::operator*=(unsigned long long rhs) {
	if ((!this->is_zero()) && (rhs != 1ULL)) {
		if (rhs == 0ULL) {
			this->bzero();
		} else {
			size_t digits = this->payload + sizeof(unsigned long long);
			uint8* product = this->malloc(digits);
			size_t j = 0U;
			
			memset(product + (digits - this->payload), '\0', this->payload);

			do {
				uint8 carry = 0U;
				uint8 v = (uint8)(rhs & 0xFFU);

				if (v > 0) {
					for (size_t i = 1; i <= this->payload; i++) {
						size_t ij = digits - i - j;
						uint16 digit = this->natural[this->capacity - i] * v + product[ij] + carry;

						product[ij] = (uint8)(digit & 0xFFU);
						carry = (uint8)(digit >> 8);
					}
				}

				product[digits - this->payload - (++j)] = carry;
				rhs >>= 8;
			} while (rhs > 0U);

			delete[] this->natural;
			this->natural = product;
			this->capacity = digits;

			digits = this->payload + j;
			this->payload = ((this->natural[this->capacity - digits] > 0U) ? digits : (digits - 1));
		}
	}

	return (*this);
}

Natural& Natural::operator*=(const Natural& rhs) {
	if ((!this->is_zero()) && (!rhs.is_one())) {
		if (rhs.is_zero()) {
			this->bzero();
		} else {
			size_t digits = this->payload + rhs.payload;
			uint8* product = this->malloc(digits);
			
			memset(product + (digits - this->payload), '\0', this->payload);

			for (size_t j = 1; j <= rhs.payload; j++) {
				uint8 carry = 0U;
				
				if (rhs.natural[rhs.capacity - j] > 0) {
					for (size_t i = 1; i <= this->payload; i++) {
						size_t ij = digits - i - j + 1;
						uint16 digit = this->natural[this->capacity - i] * rhs.natural[rhs.capacity - j] + product[ij] + carry;

						product[ij] = (uint8)(digit & 0xFFU);
						carry = (uint8)(digit >> 8);
					}
				}

				product[digits - this->payload - j] = carry;
			}

			delete[] this->natural;
			this->natural = product;
			this->capacity = digits;
			this->payload = ((this->natural[0] > 0U) ? digits : (digits - 1));
		}
	}

	return (*this);
}

/*************************************************************************************************/
Natural& Natural::operator<<=(unsigned long long rhs) {	
	if ((!this->is_zero()) && (rhs != 0U)) {
		size_t shift_byts = (size_t)(rhs / 8);
		size_t shift_bits = (size_t)(rhs % 8);
		size_t shift_load = this->payload + shift_byts;
		size_t original_idx0 = this->capacity - this->payload;
		size_t total = shift_load;

		if (shift_bits == 0U) {
			if (this->capacity < total) {
				this->recalloc(total, '\0', shift_byts);
			} else if (shift_byts > 0U) {
				memmove(this->natural + (this->capacity - shift_load), this->natural + original_idx0, this->payload);
				memset(this->natural + (this->capacity - shift_byts), '\0', shift_byts);
			}
		} else {
			const uint8* src = this->natural;
			size_t shift_diff = this->capacity - shift_load - original_idx0;
			
			if (((uint16)(src[original_idx0]) << shift_bits) > 0xFFU) {
				total++;
			}

			if (this->capacity < total) {
				shift_diff += (total - this->capacity);

				this->capacity = total;
				this->natural = this->malloc(this->capacity);
				
				/** NOTE
				 * There is no need to set other slots of `natural` to zero since
				 *   after shifting all slots will be set by algorithm.
				 */
			}

			if (shift_byts > 0) {
				this->natural[this->capacity - total] = '\0';
			}

			for (size_t idx = original_idx0; idx < original_idx0 + this->payload; idx++) {
				uint16 digit = ((uint16)(src[idx]) << shift_bits);
				size_t shift_idx = idx + shift_diff;

				if (digit <= 0xFFU) {
					this->natural[shift_idx] = (uint8)digit;
				} else {
					this->natural[shift_idx] = (uint8)(digit & 0xFFU);
					this->natural[shift_idx - 1] ^= (uint8)(digit >> 8U);
				}
			}

			if (shift_byts > 0U) {
				memset(this->natural + (this->capacity - shift_byts), '\0', shift_byts);
			}

			if (src != this->natural) {
				delete[] src;
			}
		}

		this->payload = total;
	}

	return (*this);
}

Natural& Natural::operator>>=(unsigned long long rhs) {
	if ((!this->is_zero()) && (rhs != 0U)) {
		size_t shift_byts = (size_t)(rhs / 8);
		
		if (this->payload <= shift_byts) {
			this->bzero();
		} else {
			size_t shift_bits = (size_t)(rhs % 8);
			size_t shift_cbits = 8 - shift_bits;
			size_t original_idx0 = this->capacity - this->payload;
			
			this->payload -= shift_byts;

			if (shift_bits == 0U) {
				memmove(this->natural + (this->capacity - this->payload), this->natural + original_idx0, this->payload);
				//memset(this->natural + original_idx0, '\0', shift_byts); // reduntant
			} else {
				size_t shift_endp1 = original_idx0 + this->payload;
				size_t shift_diff = this->capacity - shift_endp1 - 1;
				size_t mask = (1 << shift_bits) - 1;

				this->natural[this->capacity - 1] = (this->natural[shift_endp1 - 1] >> shift_bits);
				
				for (size_t idx = shift_endp1 - 1; idx > original_idx0; idx--) {
					size_t shift_idx = idx + shift_diff;
					uint8 digit = this->natural[idx - 1];
					uint8 tail = digit & mask;
					
					this->natural[shift_idx] = (digit >> shift_bits);

					if (tail > 0U) {
						this->natural[shift_idx + 1] ^= (tail << shift_cbits);
					}
				}

				// reduntant zeroing since other operators do not assume it
				//if (shift_byts > 0U) {
				//	memset(this->natural + original_idx0, '\0', shift_byts);
				//}

				if (this->natural[this->capacity - this->payload] == 0U) {
					this->payload--;
				}
			}
		}
	}

	return (*this);
}

Natural& Natural::operator&=(unsigned long long rhs) {
	size_t idx0 = this->capacity - fxmin(this->payload, sizeof(unsigned long long));

	this->payload = 0U;
	for (size_t idx = this->capacity; (rhs > 0U) && (idx > idx0); idx--) {
		size_t payload_idx = idx - 1U;

		this->natural[payload_idx] &= (rhs & 0xFFU);

		if (this->natural[payload_idx] > 0U) {
			this->payload = this->capacity - payload_idx;
		}

		rhs >>= 8ULL;
	}

	return (*this);
}

Natural& Natural::operator&=(const WarGrey::SCADA::Natural& rhs) {
	size_t upsize = fxmin(this->payload, rhs.payload);

	this->payload = 0U;
	for (size_t idx = 1; idx <= upsize; idx++) {
		size_t payload_idx = this->capacity - idx;

		this->natural[payload_idx] &= rhs.natural[rhs.capacity - idx];

		if (this->natural[payload_idx] > 0U) {
			this->payload = this->capacity - payload_idx;
		}
	}

	return (*this);
}

Natural& Natural::operator|=(unsigned long long rhs) {
	if (rhs > 0U) {
		size_t digits = fxmax(this->payload, sizeof(unsigned long long));
		size_t nidx = this->capacity - 1;

		if (this->capacity < digits) { // deadcode since sizeof(uint64) is the smallest capacity
			this->recalloc(digits);
			nidx = this->capacity - 1;
		}

		do {
			this->natural[nidx--] |= (rhs & 0xFFU);
			rhs >>= 8U;
		} while (rhs > 0U);

		this->payload = fxmax(this->payload, (this->capacity - nidx - 1));
	}

	return (*this);
}

Natural& Natural::operator|=(const WarGrey::SCADA::Natural& rhs) {
	if (!rhs.is_zero()) {
		size_t digits = fxmax(this->payload, rhs.payload);
		size_t lcapacity = this->capacity;
		uint8* lsrc = this->natural;
		uint8* rsrc = rhs.natural;
		
		if (this->capacity < digits) {
			this->capacity = digits;
			this->natural = this->malloc(this->capacity);
		}

		for (size_t idx = 1; idx <= digits; idx++) {
			this->natural[this->capacity - idx]
				= ((idx <= this->payload) ? lsrc[lcapacity - idx] : 0U)
				| ((idx <= rhs.payload) ? rsrc[rhs.capacity - idx] : 0U);
		}

		this->payload = digits;
		
		if (this->natural != lsrc) {
			delete[] lsrc;
		}
	}

	return (*this);
}

/*************************************************************************************************/
uint8& Natural::operator[](int idx) {
	size_t bidx = 0U;

	if (this->payload == 0U) {
		// WARNING: this is an undefined behavior.
	} else if (idx >= 0) {
		if (((size_t)idx) < this->payload) {
			bidx = this->capacity - this->payload + idx;
		}
	} else {
		if (idx >= -int(this->payload)) {
			bidx = this->capacity + idx;
		}
	}

	return this->natural[bidx];
}

size_t Natural::fixnum_count(Fixnum type) const {
	size_t modulus = 8U;

	switch (type) {
	case Fixnum::Uint64: modulus = 8U; break;
	case Fixnum::Uint32: modulus = 4U; break;
	case Fixnum::Uint16: modulus = 2U; break;
	}

	return fixnum_length(this->payload, modulus);
}

uint16 Natural::fixnum16_ref(int slot_idx, size_t offset) {
	return fixnum_ref<uint16>(this->natural, this->payload, this->capacity, slot_idx, offset, 2U);
}

uint32 Natural::fixnum32_ref(int slot_idx, size_t offset) {
	return fixnum_ref<uint32>(this->natural, this->payload, this->capacity, slot_idx, offset, 4U);
}

uint64 Natural::fixnum64_ref(int slot_idx, size_t offset) {
	return fixnum_ref<uint64>(this->natural, this->payload, this->capacity, slot_idx, offset, 8U);
}

/*************************************************************************************************/
void Natural::from_memory(const uint8 nbytes[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		this->capacity = nend - nstart;
		this->natural = this->malloc(this->capacity);

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
		this->natural = this->malloc(this->capacity);
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
		this->natural = this->malloc(this->capacity);
		this->payload = natural_from_base16(this->natural, nbytes, nstart, nend, this->capacity);
	}
}

void Natural::from_base16(const uint16 nchars[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		size_t span = nend - nstart;
		
		this->capacity = span / 2 + (span % 2);
		this->natural = this->malloc(this->capacity);
		this->payload = natural_from_base16(this->natural, nchars, nstart, nend, this->capacity);
	}
}

void Natural::from_base10(const uint8 nbytes[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		this->capacity = nend - nstart;
		this->natural = this->malloc(this->capacity);
		this->payload = natural_from_base(10U, this->natural, nbytes, int(nstart), int(nend), this->capacity);
	}
}

void Natural::from_base10(const uint16 nchars[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		this->capacity = nend - nstart;
		this->natural = this->malloc(this->capacity);
		this->payload = natural_from_base(10U, this->natural, nchars, int(nstart), int(nend), this->capacity);
	}
}


void Natural::from_base8(const uint8 nbytes[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		size_t span = nend - nstart;
		
		this->capacity = (span / 3 + ((span % 3 == 0) ? 0 : 1)) * 2;
		this->natural = this->malloc(this->capacity);
		this->payload = natural_from_base(8U, this->natural, nbytes, int(nstart), int(nend), this->capacity);
	}
}

void Natural::from_base8(const uint16 nchars[], size_t nstart, size_t nend) {
	if (nend > nstart) {
		size_t span = nend - nstart;
		
		this->capacity = (span / 3 + ((span % 3 == 0) ? 0 : 1)) * 2;
		this->natural = this->malloc(this->capacity);
		this->payload = natural_from_base(8U, this->natural, nchars, int(nstart), int(nend), this->capacity);
	}
}

void Natural::bzero() {
	this->payload = 0;

	// Method should not assume zeroed memory.
	//if (this->natural != nullptr) {
	//	memset(this->natural, '\0', this->capacity);
	//}
}

uint8* Natural::malloc(size_t size) {
	uint8* memory = new uint8[size];

	// NOTE: Method should not assume zeroed memory.

#ifdef _DEBUG
	memset(memory, size, size);
#endif

	return memory;
}

void Natural::recalloc(size_t newsize, uint8 initial, size_t shift) {
	uint8* src = this->natural;
	size_t zero_size = (this->capacity - this->payload);

	this->capacity = newsize;
	this->natural = this->malloc(this->capacity);

	{ // do copying and shifting
		size_t payload_idx0 = this->capacity - this->payload /* this is not the `zero_size` */ - shift;

		memset(this->natural, initial, this->capacity /* avoid another syscall when shifting */);
		memcpy(this->natural + payload_idx0, src + zero_size, this->payload);
	}

	delete[] src;
}
