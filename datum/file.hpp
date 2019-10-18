#pragma once

#include <fstream>
#include <string>

#include "datum/flonum.hpp"

namespace WarGrey::SCADA {
	bool open_input_binary(std::filebuf& src, Platform::String^ in_port);
	bool open_output_binary(std::wofstream& src, Platform::String^ out_port, unsigned int flprecision = 0);

	bool char_end_of_word(char ch);
	bool char_end_of_line(char ch);
	bool char_end_of_field(char ch);

	int peek_char(std::filebuf& src);
	int read_char(std::filebuf& src);
	
	std::string read_text(std::filebuf& src, bool (*end_of_text)(char) = char_end_of_line);
	Platform::String^ read_wtext(std::filebuf& src, bool (*end_of_text)(char) = char_end_of_line);
	Platform::String^ read_wgb18030(std::filebuf& src, bool (*end_of_text)(char) = char_end_of_line);

	bool read_bool(std::filebuf& src);
	unsigned long long read_natural(std::filebuf& src);
	long long read_integer(std::filebuf& src);
	double read_flonum(std::filebuf& src);
	float read_single_flonum(std::filebuf& src);

	void discard_space(std::filebuf& src);
	void discard_newline(std::filebuf& src);
	void discard_this_line(std::filebuf& src);

	std::wostream& write_bool(std::wostream& stream, bool b);
	std::wostream& write_wtext(std::wostream& stream, Platform::String^ text);
	std::wostream& write_position(std::wostream& stream, WarGrey::SCADA::double2 pos, Platform::String^ sep = " ");
	std::wostream& write_newline(std::wostream& stream);

	template<typename E>
	std::wostream& write_wtext(std::wostream& stream, E id, bool append_newline = true) {
		write_wtext(stream, id.ToString());

		if (append_newline) {
			write_newline(stream);
		}

		return stream;
	}
}
