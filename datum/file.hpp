#pragma once

#include <fstream>
#include <string>

namespace WarGrey::SCADA {
	bool char_end_of_word(char ch);
	bool char_end_of_line(char ch);

	int peek_char(std::filebuf& src);
	int read_char(std::filebuf& src);
	
	std::string read_text(std::filebuf& src, bool (*end_of_text)(char) = char_end_of_line);
	Platform::String^ read_wtext(std::filebuf& src, bool (*end_of_text)(char) = char_end_of_line);
	Platform::String^ read_wgb18030(std::filebuf& src, bool (*end_of_text)(char) = char_end_of_line);

	unsigned long long read_natural(std::filebuf& src);
	long long read_integer(std::filebuf& src);
	double read_flonum(std::filebuf& src);
	float read_single_flonum(std::filebuf& src);

	void discard_space(std::filebuf& src);
	void discard_newline(std::filebuf& src);
	void discard_this_line(std::filebuf& src);
}
