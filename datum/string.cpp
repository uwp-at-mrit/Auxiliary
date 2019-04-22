#include <algorithm>

#include "datum/string.hpp"

using namespace WarGrey::SCADA;

static const char linefeed = 0x0A;
static const char carriage_return = 0x0D;
static const char space = 0x20;
static const char zero = 0x30;
static const char nine = 0x39;

static const wchar_t wlinefeed = (wchar_t)(linefeed);
static const wchar_t wcarriage_return = (wchar_t)(carriage_return);
static const wchar_t wspace = (wchar_t)(space);
static const wchar_t wzero = (wchar_t)(zero);
static const wchar_t wnine = (wchar_t)(nine);

static inline size_t integer_length(unsigned long long n) {
	return (size_t)(floor(log(double(n)) / log(2.0)) + 1.0);
}

static unsigned int newline_position(const wchar_t* src, unsigned int idx0, unsigned int idxn, unsigned int* next_idx) {
	unsigned int line_size = 0;
	unsigned int eol_size = 0;

	for (unsigned int idx = idx0; idx < idxn; idx ++) {
		if (src[idx] == wlinefeed) {
			eol_size = (((idx + 1) < idxn) && (src[idx + 1] == wcarriage_return)) ? 2 : 1;
			break;
		} else if (src[idx] == wcarriage_return) {
			eol_size = (((idx + 1) < idxn) && (src[idx + 1] == wlinefeed)) ? 2 : 1;
			break;
		}

		line_size ++;
	}

	(*next_idx) = idx0 + line_size + eol_size;

	return line_size;
}

/*************************************************************************************************/
/** WARNING
 * https://docs.microsoft.com/en-us/cpp/c-runtime-library/format-specification-syntax-printf-and-wprintf-functions
 *
 * The behavior of the `c`, `C`, `s`, and `S` type characters are Microsoft extensions.
 *   `c` and `s` standard for the comfortable characters and strings, while
 *   `C` and `S` standard for their counterparts. 
 *
 * The ISO C standard uses `c` and `s` consistently for narrow characters and strings,
 *   and `C` and `S` for wide characters and strings, in all formatting functions.
 */

Platform::String^ WarGrey::SCADA::make_wstring(const wchar_t* fmt, ...) {
	VSWPRINT(s, fmt);
	
	return s;
}

Platform::String^ WarGrey::SCADA::make_wstring(const char* bytes) {
	return make_wstring(L"%S", bytes);
}

Platform::String^ WarGrey::SCADA::make_wstring(std::string bytes) {
	return make_wstring(L"%S", bytes.c_str());
}

Platform::String^ WarGrey::SCADA::make_wstring(char ch) {
	return make_wstring(L"%c", ch);
}

size_t WarGrey::SCADA::wstrlen(const wchar_t* content) {
	return int(wcslen(content)) * 2 - 1;
}

/*************************************************************************************************/
Platform::String^ WarGrey::SCADA::flstring(double value, int precision) {
	return ((precision >= 0)
		? make_wstring(make_wstring(L"%%.%dlf", precision)->Data(), value)
		: value.ToString());
}

Platform::String^ WarGrey::SCADA::fxstring(long long value, int width) {
	return ((width > 0)
		? make_wstring(make_wstring(L"%%0%dld", width)->Data(), value)
		: value.ToString());
}

Platform::String^ WarGrey::SCADA::sstring(unsigned long long bytes, int precision) {
	static Platform::String^ units[] = { L"KB", L"MB", L"GB", L"TB" };
	static unsigned int max_idx = sizeof(units) / sizeof(Platform::String^) - 1;
	Platform::String^ size = bytes.ToString();

	if (bytes >= 1024) {
		double flsize = double(bytes) / 1024.0;
		unsigned idx = 0;

		while ((flsize >= 1024.0) && (idx < max_idx)) {
			flsize /= 1024.0;
			idx++;
		}

		size = flstring(flsize, precision) + units[idx];
	}

	return size;
}

/*************************************************************************************************/
Platform::String^ WarGrey::SCADA::substring(Platform::String^ src, int start, int endplus1) {
	Platform::String^ substr = nullptr;
	int max_size = src->Length();
	const wchar_t* pool = (src->Data() + start);
	int subsize = ((endplus1 > 0) ? std::min(endplus1, max_size) : src->Length()) - start;

	if (subsize > 0) {
		substr = ref new Platform::String(pool, subsize);
	}

	return substr;
}

std::string WarGrey::SCADA::make_nstring(const char* fmt, ...) {
	VSNPRINT(s, fmt);

	return s;
}

std::string WarGrey::SCADA::make_nstring(const wchar_t* wbytes) {
	return make_nstring("%S", wbytes);
}

std::string WarGrey::SCADA::make_nstring(Platform::String^ wstr) {
	return make_nstring("%S", wstr->Data());
}

std::string WarGrey::SCADA::binumber(unsigned long long n, size_t bitsize) {
	static char support[64];
	size_t size = ((bitsize < 1) ? ((n == 0) ? 1 : integer_length(n)) : bitsize);
	char* pool = ((size > (sizeof(support) / sizeof(char))) ? new char[size] : support);

	for (size_t idx = size; idx > 0; idx--) {
		pool[idx - 1] = (((n >> (size - idx)) & 0b1) ? '1' : '0');
	}

	std::string str(pool, size);

	if (pool != support) {
		delete[] pool;
	}

	return str;
}

/**************************************************************************************************/
Platform::String^ WarGrey::SCADA::string_first_line(Platform::String^ src) {
	const wchar_t* wsrc = src->Data();
	unsigned int total = src->Length();
	unsigned int line_size = newline_position(wsrc, 0, total, &total);
	
	return ref new Platform::String(wsrc, line_size);
}

std::list<Platform::String^> WarGrey::SCADA::string_lines(Platform::String^ src, bool skip_empty_line) {
	std::list<Platform::String^> lines;
	unsigned int nidx = 0;
	unsigned int total = src->Length();
	const wchar_t* wsrc = src->Data();

	while (total > 0) {
		unsigned int line_size = newline_position(wsrc, 0, total, &nidx);

		if ((line_size > 0) || (!skip_empty_line)) {
			lines.push_back(ref new Platform::String(wsrc, line_size));
		}

		wsrc += nidx;
		total -= nidx;
	}

	return lines;
}

/************************************************************************************************/
long long WarGrey::SCADA::scan_integer(const unsigned char* src, size_t* pos, size_t total, bool skip_trailing_space) {
	long long value = 0;

	while ((*pos) < total) {
		char c = src[(*pos)];

		if ((c < zero) || (c > nine)) {
			break;
		}

		value = value * 10 + (c - zero);
		(*pos) += 1;
	}

	if (skip_trailing_space) {
		scan_skip_space(src, pos, total);
	}

	return value;
}

size_t WarGrey::SCADA::scan_skip_space(const unsigned char* src, size_t* pos, size_t total) {
	size_t idx = (*pos);

	while ((*pos) < total) {
		char c = src[(*pos)];

		if (c != space) {
			break;
		}

		(*pos) += 1;
	}

	return (*pos) - idx;
}

size_t WarGrey::SCADA::scan_skip_newline(const unsigned char* src, size_t* pos, size_t total) {
	size_t idx = (*pos);

	while ((*pos) < total) {
		char c = src[(*pos)];

		if ((c != linefeed) && (c != carriage_return)) {
			break;
		}

		(*pos) += 1;
	}

	return (*pos) - idx;
}

size_t WarGrey::SCADA::scan_skip_this_line(const unsigned char* src, size_t* pos, size_t total) {
	size_t idx = (*pos);

	while ((*pos) < total) {
		char c = src[(*pos)];

		if ((c == linefeed) || (c == carriage_return)) {
			scan_skip_newline(src, pos, total);
			break;
		}

		(*pos) += 1;
	}

	return (*pos) - idx;
}
