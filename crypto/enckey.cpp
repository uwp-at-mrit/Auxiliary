#include "crypto/enckey.hpp"
#include "crypto/blowfish.hpp"

#include "datum/bytes.hpp"

using namespace WarGrey::SCADA;

static uint64 inline make_HW_ID(uint64 l0000, uint64 l000, uint64 l00, uint64 l0, uint64 l) {
	return (l0000 << 32U) ^ (l000 << 24U) ^ (l00 << 16U) ^ (l0 << 8U) ^ l;
}

template<typename B>
static uint64 ascii_to_hexadecimal(B* ascii, size_t digit_count, size_t start) {
	size_t end = start + digit_count * 2U;
	uint64 id = 0U;

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
uint64 WarGrey::SCADA::enc_hexadecimal(uint32 literal_id) {
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

uint64 WarGrey::SCADA::enc_hexadecimal(const char* literal_id, size_t start) {
	const uint8* id = (uint8*)literal_id;

	return make_HW_ID(
		hexadecimal_ref(id, start, 0U),
		hexadecimal_ref(id, start + 1, 0U),
		hexadecimal_ref(id, start + 2, 0U),
		hexadecimal_ref(id, start + 3, 0U),
		hexadecimal_ref(id, start + 4, 0U));
}

uint64 WarGrey::SCADA::enc_hexadecimal(const wchar_t* literal_id, size_t start) {
	return make_HW_ID(
		byte_to_hexadecimal((unsigned char)(literal_id[start]), 0U),
		byte_to_hexadecimal((unsigned char)(literal_id[start + 1U]), 0U),
		byte_to_hexadecimal((unsigned char)(literal_id[start + 2U]), 0U),
		byte_to_hexadecimal((unsigned char)(literal_id[start + 3U]), 0U),
		byte_to_hexadecimal((unsigned char)(literal_id[start + 4U]), 0U));
}

uint64 WarGrey::SCADA::enc_hexadecimal(std::string& literal_id, size_t start) {
	return enc_hexadecimal(literal_id.c_str(), start);
}

uint64 WarGrey::SCADA::enc_hexadecimal(Platform::String^ literal_id, size_t start) {
	return enc_hexadecimal(literal_id->Data(), start);
}

/**************************************************************************************************/
std::string WarGrey::SCADA::enc_ascii(uint64 id, size_t digit_count) {
	size_t hexsize = digit_count * 2U;
	std::string ascii(hexsize, '0');

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

uint64 WarGrey::SCADA::enc_hexadecimal_from_ascii(const char* ascii, size_t digit_count, size_t start) {
	return ascii_to_hexadecimal(ascii, digit_count, start);
}

uint64 WarGrey::SCADA::enc_hexadecimal_from_ascii(const wchar_t* ascii, size_t digit_count, size_t start) {
	return ascii_to_hexadecimal(ascii, digit_count, start);
}

uint64 WarGrey::SCADA::enc_hexadecimal_from_ascii(std::string& literal_id, size_t digit_count, size_t start) {
	return enc_hexadecimal_from_ascii(literal_id.c_str(), digit_count, start);
}

uint64 WarGrey::SCADA::enc_hexadecimal_from_ascii(Platform::String^ literal_id, size_t digit_count, size_t start) {
	return enc_hexadecimal_from_ascii(literal_id->Data(), digit_count, start);
}

/**************************************************************************************************/
uint64 WarGrey::SCADA::enc_hardware_uid6(uint64 HW_ID) {
	return (HW_ID << 8) ^ (HW_ID >> 32U);
}

std::string WarGrey::SCADA::enc_hardware_uid_encrypt(uint64 HW_ID, const uint8* M_KEY) {
	BlowfishCipher bf(M_KEY, 5);
	
	return hexnumber(bf.encrypt(HW_ID), 16);
}
