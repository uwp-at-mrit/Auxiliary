#pragma once

#include "datum/natural.hpp"

namespace WarGrey::SCADA {
	// These APIs are intended to use `Natural` as its primitive data type instead of `uint64`s and `String`s.
	// Hint: Strings(std::basic_string<unsigned char>) are just base-256 Naturals.

	WarGrey::SCADA::Natural enc_natural(uint64 literal_id);
	WarGrey::SCADA::Natural enc_natural(const char* literal_id, size_t digit_count, size_t start = 0U);
	WarGrey::SCADA::Natural enc_natural(bytes& literal_id, size_t start = 0U);
	WarGrey::SCADA::Natural enc_natural(std::string& literal_id, size_t start = 0U);

	bytes enc_ascii(uint64 id);
	bytes enc_ascii(WarGrey::SCADA::Natural& id);

	WarGrey::SCADA::Natural enc_natural_from_ascii(const char* literal_id, size_t digit_count, size_t start = 0U);
	WarGrey::SCADA::Natural enc_natural_from_ascii(bytes& literal_id, size_t digit_count, size_t start = 0U);
	WarGrey::SCADA::Natural enc_natural_from_ascii(std::string& literal_id, size_t digit_count, size_t start = 0U);

	WarGrey::SCADA::Natural enc_natural_pad(WarGrey::SCADA::Natural bs);
	WarGrey::SCADA::Natural enc_natural_unpad(WarGrey::SCADA::Natural bs);
	WarGrey::SCADA::Natural enc_hardware_uid6(WarGrey::SCADA::Natural HW_ID);

	WarGrey::SCADA::Natural enc_cell_permit_encrypt(const WarGrey::SCADA::Natural& HW_ID, const WarGrey::SCADA::Natural& cell_key);
	WarGrey::SCADA::Natural enc_cell_permit_decrypt(const WarGrey::SCADA::Natural& HW_ID, const WarGrey::SCADA::Natural& en_cell_key);

	WarGrey::SCADA::Natural enc_cell_permit_checksum(const char* name, size_t nsize, uint32 expiry_date,
		const WarGrey::SCADA::Natural& eck1, const WarGrey::SCADA::Natural& eck2);

	WarGrey::SCADA::Natural enc_cell_permit_checksum(const WarGrey::SCADA::Natural& HW_ID,
		const char* name, size_t nsize, uint32 expiry_date,
		const WarGrey::SCADA::Natural& ck1, const WarGrey::SCADA::Natural& ck2);
}
