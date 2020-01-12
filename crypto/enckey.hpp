#pragma once

#include "datum/natural.hpp"

namespace WarGrey::DTPM {
	// These APIs are intended to use `Natural` as its primitive data type instead of `uint64`s and `String`s.
	// Hint: Strings(std::basic_string<unsigned char>) are just base-256 Naturals.

	WarGrey::DTPM::Natural enc_natural(uint64 literal_id);
	WarGrey::DTPM::Natural enc_natural(const char* literal_id, size_t digit_count, size_t start = 0U);
	WarGrey::DTPM::Natural enc_natural(WarGrey::SCADA::bytes& literal_id, size_t start = 0U);
	WarGrey::DTPM::Natural enc_natural(std::string& literal_id, size_t start = 0U);

	WarGrey::SCADA::bytes enc_ascii(uint64 id);
	WarGrey::SCADA::bytes enc_ascii(WarGrey::DTPM::Natural& id);

	WarGrey::DTPM::Natural enc_natural_from_ascii(const char* literal_id, size_t digit_count, size_t start = 0U);
	WarGrey::DTPM::Natural enc_natural_from_ascii(WarGrey::SCADA::bytes& literal_id, size_t digit_count, size_t start = 0U);
	WarGrey::DTPM::Natural enc_natural_from_ascii(std::string& literal_id, size_t digit_count, size_t start = 0U);

	WarGrey::DTPM::Natural enc_natural_pad(WarGrey::DTPM::Natural bs);
	WarGrey::DTPM::Natural enc_natural_unpad(WarGrey::DTPM::Natural bs);
	WarGrey::DTPM::Natural enc_hardware_uid6(WarGrey::DTPM::Natural HW_ID);

	WarGrey::DTPM::Natural enc_cell_permit_encrypt(const WarGrey::DTPM::Natural& HW_ID, const WarGrey::DTPM::Natural& cell_key);
	WarGrey::DTPM::Natural enc_cell_permit_decrypt(const WarGrey::DTPM::Natural& HW_ID, const WarGrey::DTPM::Natural& en_cell_key);

	WarGrey::DTPM::Natural enc_cell_permit_checksum(const char* name, size_t nsize, uint32 expiry_date,
		const WarGrey::DTPM::Natural& eck1, const WarGrey::DTPM::Natural& eck2);

	WarGrey::DTPM::Natural enc_cell_permit_checksum(const char* name, size_t nsize, uint32 expiry_year, uint32 expiry_month, uint32 expiry_day,
		const WarGrey::DTPM::Natural& eck1, const WarGrey::DTPM::Natural& eck2);

	WarGrey::DTPM::Natural enc_cell_permit_checksum(const WarGrey::DTPM::Natural& HW_ID,
		const char* name, size_t nsize, uint32 expiry_date,
		const WarGrey::DTPM::Natural& ck1, const WarGrey::DTPM::Natural& ck2);

	WarGrey::DTPM::Natural enc_cell_permit_checksum(const WarGrey::DTPM::Natural& HW_ID,
		const char* name, size_t nsize, uint32 expiry_year, uint32 expiry_month, uint32 expiry_day,
		const WarGrey::DTPM::Natural& ck1, const WarGrey::DTPM::Natural& ck2);
}
