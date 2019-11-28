#pragma once

#pragma once;

#include <string>

namespace WarGrey::SCADA {
	private class Natural {
	public:
		static WarGrey::SCADA::Natural* from_hexstring(const char nbytes[], size_t nstart, size_t nend);
		static WarGrey::SCADA::Natural* from_hexstring(const wchar_t nbytes[], size_t nstart, size_t nend);
		static WarGrey::SCADA::Natural* from_hexstring(std::string& nstr, size_t nstart = 0, size_t nend = 0);
		static WarGrey::SCADA::Natural* from_hexstring(std::wstring& nstr, size_t nstart = 0, size_t nend = 0);
		static WarGrey::SCADA::Natural* from_hexstring(Platform::String^ nstr, size_t nstart = 0, size_t nend = 0);

		template<typename BYTE, size_t N>
		static WarGrey::SCADA::Natural* from_hexstring(const BYTE(&n)[N], size_t nstart = 0, size_t nend = N) {
			return Natural::from_hexstring(n, nstart, nend);
		}

	public:
		~Natural() noexcept;
		Natural();
		Natural(unsigned int n);
		Natural(unsigned long long n);
		Natural(const char nbytes[], size_t nstart, size_t nend);
		Natural(const wchar_t nbytes[], size_t nstart, size_t nend);
		Natural(std::string& nstr, size_t nstart = 0, size_t nend = 0);
		Natural(std::wstring& nstr, size_t nstart = 0, size_t nend = 0);
		Natural(Platform::String^ nstr, size_t nstart = 0, size_t nend = 0);

		template<typename BYTE, size_t N>
		Natural(const BYTE(&n)[N], size_t nstart = 0, size_t nend = N) : Natural(n, nstart, nend) {}

	public:
		size_t length();
		size_t integer_length();

	public:
		std::string to_hexstring();

	private:
		uint8* natural;
		size_t capacity;
		size_t payload;
	};
}
