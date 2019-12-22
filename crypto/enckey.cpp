#include "crypto/enckey.hpp"
#include "crypto/blowfish.hpp"

#include "datum/string.hpp"

using namespace WarGrey::SCADA;

static Natural inline make_HW_ID(uint64 l0000, uint64 l000, uint64 l00, uint64 l0, uint64 l) {
	return Natural((l0000 << 32U) ^ (l000 << 24U) ^ (l00 << 16U) ^ (l0 << 8U) ^ l);
}

static Natural inline make_HW_ID(const uint8* id, size_t start) {
	return make_HW_ID(
		hexadecimal_ref(id, start, 0U),
		hexadecimal_ref(id, start + 1, 0U),
		hexadecimal_ref(id, start + 2, 0U),
		hexadecimal_ref(id, start + 3, 0U),
		hexadecimal_ref(id, start + 4, 0U));
}

template<typename B>
static Natural ascii_to_hexadecimal(B* ascii, size_t digit_count, size_t start) {
	size_t end = start + digit_count * 2U;
	Natural id(0U);

	for (size_t idx = start; idx < end; idx += 2U) {
		uint8 digit = ((uint8)(ascii[idx]) - '0') * 10 + ((uint8)(ascii[idx + 1]) - '0');

		if (digit > 40U) {
			digit--;
		}

		id = ((id << 8U) ^ (digit - 30U));
	}

	return id;
}

/**************************************************************************************************/
Natural WarGrey::SCADA::enc_natural(uint32 literal_id) {
	uint64 l0000 = (literal_id >> 16U) & 0xFU;
	uint64 l000 = (literal_id >> 12U) & 0xFU;
	uint64 l00 = (literal_id >> 8U) & 0xFU;
	uint64 l0 = (literal_id >> 4U) & 0xFU;
	uint64 l = literal_id & 0xFU;

	return make_HW_ID(
		(literal_id >> 16U) & 0xFU,
		(literal_id >> 12U) & 0xFU,
		(literal_id >> 8U) & 0xFU,
		(literal_id >> 4U) & 0xFU,
		literal_id & 0xFU);
}

Natural WarGrey::SCADA::enc_natural(const char* literal_id, size_t start) {
	return make_HW_ID((uint8*)literal_id, start);
}

Natural WarGrey::SCADA::enc_natural(bytes& literal_id, size_t start) {
	return make_HW_ID(literal_id.c_str(), start);
}

Natural WarGrey::SCADA::enc_natural(std::string& literal_id, size_t start) {
	return enc_natural(literal_id.c_str(), start);
}

/**************************************************************************************************/
bytes WarGrey::SCADA::enc_ascii(uint64 id, size_t digit_count) {
	size_t hexsize = digit_count * 2U;
	bytes ascii(hexsize, '0');

	for (size_t idx = hexsize; ((idx > 0U) && (id > 0U)); idx -= 2U, id >>= 8U) {
		uint8 digit = (uint8)(id & 0xFU) + 30U;

		if (digit >= 40U) {
			digit++;
		}

		ascii[idx - 2] = digit / 10 + '0';
		ascii[idx - 1] = digit % 10 + '0';
	}

	return ascii;
}

bytes WarGrey::SCADA::enc_ascii(Natural& id) {
	size_t hexsize = id.length() * 2U;
	bytes ascii(hexsize, '0');

	for (size_t idx = 0; idx < id.length(); idx++) {
		uint8 digit = id[int(idx)] + 30U;

		if (digit >= 40U) {
			digit++;
		}

		ascii[idx * 2U + 0U] = (digit / 10 + '0');
		ascii[idx * 2U + 1U] = (digit % 10 + '0');
	}

	return ascii;
}

Natural WarGrey::SCADA::enc_natural_from_ascii(const char* ascii, size_t digit_count, size_t start) {
	return ascii_to_hexadecimal(ascii, digit_count, start);
}

Natural WarGrey::SCADA::enc_natural_from_ascii(bytes& literal_id, size_t digit_count, size_t start) {
	return ascii_to_hexadecimal(literal_id.c_str(), digit_count, start);
}

Natural WarGrey::SCADA::enc_natural_from_ascii(std::string& literal_id, size_t digit_count, size_t start) {
	return enc_natural_from_ascii(literal_id.c_str(), digit_count, start);
}

/**************************************************************************************************/
Natural WarGrey::SCADA::enc_hardware_uid6(Natural HW_ID) {
	return (HW_ID << 8) ^ HW_ID[0];
}

Natural WarGrey::SCADA::enc_hardware_uid_encrypt(Natural& HW_ID, const Natural& M_KEY) {
	BlowfishCipher bf(M_KEY.to_bytes().c_str(), M_KEY.length());
	
	return HW_ID;
}

/**************************************************************************************************/
Natural WarGrey::SCADA::enc_natural_pad(Natural n) {
	size_t size = n.length();
	size_t remainder = size % 8;
	
	if (remainder > 0) {
		size_t padsize = 8 - remainder;

		for (uint8 idx = 0; idx < padsize; idx++) {
			n <<= 8U;
			n += padsize;
		}
	}

	return n;
}
