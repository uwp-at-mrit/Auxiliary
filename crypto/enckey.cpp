#include "crypto/enckey.hpp"
#include "crypto/blowfish.hpp"

#include "datum/bytes.hpp"

using namespace WarGrey::SCADA;

/**************************************************************************************************/
template<typename B>
static Natural bytes_to_natural(B* bs, size_t digit_count, size_t start) {
	size_t end = start + digit_count;
	Natural id(0U);

	for (size_t idx = 0; idx < digit_count; idx++) {
		uint8 digit = byte_to_hexadecimal(bs[start + idx], 0U);

		id <<= 8U;
		id += hexadecimal_to_byte(digit);
	}

	return id;
}

/**************************************************************************************************/
Natural WarGrey::SCADA::enc_natural(uint64 literal_id) {
	Natural n;
	uint8 idx = 0;

	while (literal_id > 0U) {
		uint8 digit = literal_id & 0xFU;
		size_t shift = n.length() * 8U;

		n ^= ((uint64)(hexadecimal_to_byte(digit)) << shift);

		literal_id >>= 4U;
	}

	return n;
}

Natural WarGrey::SCADA::enc_natural(const char* literal_id, size_t digit_count, size_t start) {
	return bytes_to_natural(literal_id, digit_count, start);
}

Natural WarGrey::SCADA::enc_natural(bytes& literal_id, size_t start) {
	return bytes_to_natural(literal_id.c_str(), literal_id.size() - start, start);
}

Natural WarGrey::SCADA::enc_natural(std::string& literal_id, size_t start) {
	return bytes_to_natural(literal_id.c_str(), literal_id.size() - start, start);
}

/**************************************************************************************************/
bytes WarGrey::SCADA::enc_ascii(uint64 id) {
	return enc_natural(id).to_hexstring();
}

bytes WarGrey::SCADA::enc_ascii(Natural& id) {
	return id.to_hexstring();
}

Natural WarGrey::SCADA::enc_natural_from_ascii(const char* ascii, size_t digit_count, size_t start) {
	return Natural(16U, (const uint8*)ascii, start, start + digit_count * 2U);
}

Natural WarGrey::SCADA::enc_natural_from_ascii(bytes& literal_id, size_t digit_count, size_t start) {
	return Natural(16U, literal_id.c_str(), start, start + digit_count * 2U);
}

Natural WarGrey::SCADA::enc_natural_from_ascii(std::string& literal_id, size_t digit_count, size_t start) {
	return enc_natural_from_ascii(literal_id.c_str(), digit_count, start);
}

/**************************************************************************************************/
Natural WarGrey::SCADA::enc_hardware_uid6(Natural HW_ID) {
	return HW_ID[0] ^ (HW_ID << 8U);
}

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

Natural WarGrey::SCADA::enc_natural_unpad(Natural n) {
	n >>= (8U * n[-1]);

	return n;
}

/**************************************************************************************************/
Natural WarGrey::SCADA::enc_cell_permit_encrypted_key(const Natural& HW_ID, const Natural& key) {
	const size_t key_size = 8U;
	uint8 cipher[key_size];
	Natural HW_ID6 = enc_hardware_uid6(HW_ID);
	BlowfishCipher bf(HW_ID6.to_bytes().c_str(), HW_ID6.length());
	size_t key_remainder = key.length() % 8U;

	if (key_remainder > 0U) {
		bf.encrypt(enc_natural_pad(key).to_bytes().c_str(), 0U, key_size, cipher, 0U, key_size);
	} else {
		bf.encrypt(key.to_bytes().c_str(), 0U, key_size, cipher, 0U, key_size);
	}

	return Natural(cipher);
}
