#pragma once

#include "datum/string.hpp"

namespace WarGrey::SCADA {
	uint64 enc_hexadecimal(uint32 literal_id);
	uint64 enc_hexadecimal(const char* literal_id, size_t start = 0U);
	uint64 enc_hexadecimal(const wchar_t* literal_id, size_t start = 0U);
	uint64 enc_hexadecimal(std::string& literal_id, size_t start = 0U);
	uint64 enc_hexadecimal(Platform::String^ literal_id, size_t start = 0U);

	std::string enc_ascii(uint64 id, size_t digit_count = 5U);
	uint64 enc_hexadecimal_from_ascii(const char* literal_id, size_t digit_count, size_t start = 0U);
	uint64 enc_hexadecimal_from_ascii(const wchar_t* literal_id, size_t digit_count, size_t start = 0U);
	uint64 enc_hexadecimal_from_ascii(std::string& literal_id, size_t digit_count, size_t start = 0U);
	uint64 enc_hexadecimal_from_ascii(Platform::String^ literal_id, size_t digit_count, size_t start = 0U);

	uint64 enc_hardware_uid6(uint64 HW_ID);
	
	std::string enc_hardware_uid_encrypt(uint64 HW_ID, const uint8* M_KEY);
}
