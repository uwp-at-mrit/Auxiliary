#pragma once

#include "datum/bytes.hpp"

namespace WarGrey::SCADA {
	// These APIs are intended to ignore 'Platform::String^' and 'wchar_t*'.

	uint64 enc_hexadecimal(uint32 literal_id);
	uint64 enc_hexadecimal(const char* literal_id, size_t start = 0U);
	uint64 enc_hexadecimal(bytes& literal_id, size_t start = 0U);
	uint64 enc_hexadecimal(std::string& literal_id, size_t start = 0U);

	bytes enc_ascii(uint64 id, size_t digit_count = 5U);
	uint64 enc_hexadecimal_from_ascii(const char* literal_id, size_t digit_count, size_t start = 0U);
	uint64 enc_hexadecimal_from_ascii(bytes& literal_id, size_t digit_count, size_t start = 0U);
	uint64 enc_hexadecimal_from_ascii(std::string& literal_id, size_t digit_count, size_t start = 0U);

	uint64 enc_hardware_uid6(uint64 HW_ID);
	
	uint64 enc_hexadecimal_pad(unsigned long long hex, size_t bsize = 5U);
	bytes enc_pad(bytes bs, size_t start = 0U, size_t end = 0U);

	bytes enc_hardware_uid_encrypt(uint64 HW_ID, const uint8* M_KEY);
}
