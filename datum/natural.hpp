#pragma once

#pragma once;

#include <string>

namespace WarGrey::SCADA {
	private class Natural {
	public:
		~Natural() noexcept;
		
		Natural();
		Natural(unsigned long long n);
		
		Natural(std::string& nstr, size_t nstart = 0, size_t nend = 0);
		Natural(std::wstring& nstr, size_t nstart = 0, size_t nend = 0);
		Natural(Platform::String^ nstr, size_t nstart = 0, size_t nend = 0);
		Natural(uint8 base, std::string& nstr, size_t nstart = 0, size_t nend = 0);
		Natural(uint8 base, std::wstring& nstr, size_t nstart = 0, size_t nend = 0);
		Natural(uint8 base, Platform::String^ nstr, size_t nstart = 0, size_t nend = 0);

		template<typename BYTE, size_t N>
		Natural(const BYTE(&ns)[N], size_t nstart = 0, size_t nend = N)
			: Natural((const BYTE*)ns, nstart, nend) {}

		template<typename BYTE, size_t N>
		Natural(uint8 base, const BYTE(&ns)[N], size_t nstart = 0, size_t nend = N)
			: Natural(base, (const BYTE*)ns, nstart, nend) {}

		template<typename BYTE>
		Natural(const BYTE ns[], size_t nstart, size_t nend)
			: Natural((uint8)0U, ns, nstart, nend) {}

		template<typename BYTE>
		Natural(uint8 base, const BYTE ns[], size_t nstart, size_t nend) : natural(nullptr), capacity(0U), payload(0U) {
			switch (base) {
			case 16: this->from_base16(ns, nstart, nend); break;
			case 10: this->from_base10(ns, nstart, nend); break;
			case 8:  this->from_base8(ns, nstart, nend); break;
			default: this->from_memory(ns, nstart, nend);
			}
		}

	public:
		Natural(const WarGrey::SCADA::Natural& n);
		Natural(WarGrey::SCADA::Natural&& n);

		//auto operator<=>(const WarGrey::SCADA::Natural& chs);

		WarGrey::SCADA::Natural& operator=(const WarGrey::SCADA::Natural& n);
		WarGrey::SCADA::Natural& operator=(WarGrey::SCADA::Natural&& n);

	public:
		WarGrey::SCADA::Natural& operator++();
		WarGrey::SCADA::Natural operator++(int postfix);

		WarGrey::SCADA::Natural& operator+=(unsigned long long rhs);
		WarGrey::SCADA::Natural& operator+=(const WarGrey::SCADA::Natural& rhs);
		friend WarGrey::SCADA::Natural operator+(WarGrey::SCADA::Natural lhs, const WarGrey::SCADA::Natural& rhs) { lhs += rhs; return lhs; }
		friend WarGrey::SCADA::Natural operator+(WarGrey::SCADA::Natural lhs, unsigned long long rhs) { lhs += rhs; return lhs; }

	public:
		size_t length();
		size_t integer_length();

	public:
		std::string to_hexstring();

	private:
		void from_memory(const uint8 nbytes[], size_t nstart, size_t nend);
		void from_memory(const uint16 nchars[], size_t nstart, size_t nend);
		void from_base16(const uint8 nbytes[], size_t nstart, size_t nend);
		void from_base16(const uint16 nchars[], size_t nstart, size_t nend);
		void from_base10(const uint8 nbytes[], size_t nstart, size_t nend);
		void from_base10(const uint16 nchars[], size_t nstart, size_t nend);
		void from_base8(const uint8 nbytes[], size_t nstart, size_t nend);
		void from_base8(const uint16 nchars[], size_t nstart, size_t nend);

	private:
		void on_moved();

	private:
		uint8* natural;
		size_t capacity;
		size_t payload;
	};
}
