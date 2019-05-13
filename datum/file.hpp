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

	double read_flonum(std::filebuf& src);
	float read_single_flonum(std::filebuf& src);
	void read_skip_space(std::filebuf& src);
	void read_skip_newline(std::filebuf& src);
	void read_skip_this_line(std::filebuf& src);
}
