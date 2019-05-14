#include <string>

#include "datum/file.hpp"
#include "datum/flonum.hpp"
#include "datum/string.hpp"

using namespace WarGrey::SCADA;

static const char linefeed = 0x0A;
static const char carriage_return = 0x0D;
static const char space = 0x20;
static const char zero = 0x30;
static const char nine = 0x39;
static const char minus = 0x2D;
static const char dot = 0x2E;

/************************************************************************************************/
bool WarGrey::SCADA::char_end_of_word(char ch) {
	return ch == space;
}

bool WarGrey::SCADA::char_end_of_line(char ch) {
	return (ch == linefeed) || (ch == carriage_return);
}

/************************************************************************************************/
int WarGrey::SCADA::peek_char(std::filebuf& src) {
	return src.sgetc();
}

int WarGrey::SCADA::read_char(std::filebuf& src) {
	return src.sbumpc();
}

std::string WarGrey::SCADA::read_text(std::filebuf& src, bool (*end_of_text)(char)) {
	std::string str;
	char ch;

	read_skip_space(src);

	while ((ch = src.sbumpc()) != EOF) {
		if (end_of_text(ch)) {
			src.sungetc();
			break;
		}

		str.push_back(ch);
	}

	return str;
}

Platform::String^ WarGrey::SCADA::read_wtext(std::filebuf& src, bool (*end_of_text)(char)) {
	return make_wstring(read_text(src, end_of_text));
}

unsigned long long WarGrey::SCADA::read_natural(std::filebuf& src) {
	unsigned long long n = 0;
	char ch;

	read_skip_space(src);

	while ((ch = src.sbumpc()) != EOF) {
		if ((ch < zero) || (ch > nine)) {
			src.sungetc();
			break;
		}

		n = n * 10 + (ch - zero);
	}

	return n;
}

long long WarGrey::SCADA::read_integer(std::filebuf& src) {
	long long n = 0;
	long long sign = 1;
	char ch;

	read_skip_space(src);

	if (src.sgetc() == minus) {
		sign = -1;
		src.snextc();
	}

	while ((ch = src.sbumpc()) != EOF) {
		if ((ch < zero) || (ch > nine)) {
			src.sungetc();
			break;
		}

		n = n * 10 + (ch - zero);
	}

	return n * sign;
}

double WarGrey::SCADA::read_flonum(std::filebuf& src) {
	double flonum = 0.0;
	double i_acc = 10.0;
	double f_acc = 1.0;
	double sign = 1.0;
	char ch;

	read_skip_space(src);

	if (src.sgetc() == minus) {
		sign = -1.0;
		src.snextc();
	}

	while ((ch = src.sbumpc()) != EOF) {
		if ((ch < zero) || (ch > nine)) {
			if (ch != dot) {
				src.sungetc();
				break;
			} else {
				i_acc = 1.0;
				f_acc = 0.1;
				continue;
			}
		}

		flonum = flonum * i_acc + double(ch - zero) * f_acc;

		if (f_acc != 1.0) {
			f_acc *= 0.1;
		}
	}

	return flonum * sign;
}

float WarGrey::SCADA::read_single_flonum(std::filebuf& src) {
	return float(read_flonum(src));
}

void WarGrey::SCADA::read_skip_space(std::filebuf& src) {
	char ch;

	while ((ch = src.sbumpc()) != EOF) {
		if (!char_end_of_word(ch)) {
			src.sungetc();
			break;
		}
	}
}

void WarGrey::SCADA::read_skip_newline(std::filebuf& src) {
	char ch;
	
	while ((ch = src.sbumpc()) != EOF) {
		if (!char_end_of_line(ch)) {
			src.sungetc();
			break;
		}
	}
}

void WarGrey::SCADA::read_skip_this_line(std::filebuf& src) {
	char ch;

	while ((ch = src.sbumpc()) != EOF) {
		if (char_end_of_line(ch)) {
			read_skip_newline(src);
			break;
		}
	}
}
