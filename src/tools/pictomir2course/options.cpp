#include "options.h"

#include <iostream>

namespace pictomir2course
{

using std::string;

string options::input_file_name = string();
string options::output_file_name = string();
string options::program_name_;

bool options::show_help_and_exit = true;
int options::exit_code = 127;


#if defined(WIN32) || defined(_WIN32)
static const string SEP = string("\\");
#else
static const string SEP = string("/");
#endif

void options::parse(int argc, char *argv[])
{
	show_help_and_exit = false;

	program_name_ = string(argv[0]);
	size_t sep_pos = program_name_.rfind(SEP);
	if (sep_pos != string::npos) {
		program_name_ = program_name_.substr(sep_pos + 1);
	}

	size_t positional_index = 0u;
	for (int i = 1; i < argc; ++i) {
		string opt(argv[i]);

		if ("/?" == opt || "/h" == opt || "/help" == opt) {
			// Windows-style option
			show_help_and_exit = true;
			exit_code = 0;
			return;
		}

		size_t minus_count = 0u;
		for (;
			minus_count < opt.length() && opt.at(minus_count) == '-';
			minus_count ++
		) {}

		if (minus_count) {
			// named parameter
			string prefix = opt.substr(0u, minus_count);
			opt = opt.substr(minus_count);
			string key;
			string value;
			size_t equal_pos = opt.find('=');
			if (equal_pos == string::npos) {
				key = opt;
			} else {
				key = opt.substr(0u, equal_pos);
				value = opt.substr(equal_pos + 1u);
			}
			if (!parse_named_parameter(prefix, key, value)) {
				return;
			}
		} else {
			// positional parameter
			if (!parse_positional_parameter(positional_index, opt)) {
				return;
			}
			++ positional_index;
		}
	}

	if (show_help_and_exit) {
		exit_code = 0;
		return;
	} else if (input_file_name.length() == 0) {
		exit_code = 127;
		return;
	} else if (output_file_name.length() == 0) {
		exit_code = 0;
		create_default_output_file_name();
	} else {
		exit_code = 0;
	}
}

bool options::parse_positional_parameter(size_t index, const string &value)
{
	if (index > 0) {
		std::cerr << "Extra positional parameter: " << value << std::endl << std::endl;
		return false;
	}

	input_file_name = value;
	return true;
}

bool options::parse_named_parameter(
	const string &prefix,
	const string &key,
	const string &value)
{
	if ("help" == key || "h" == key) {
		show_help_and_exit = true;
		return true;
	} else if ("output" == key || "o" == key) {
		output_file_name = value;
		return true;
	} else {
		std::cerr << "Unknown parameter: " << prefix << key;
		if (value.length() > 0) {
			std::cerr << "=" << value;
		}
		std::cerr << std::endl << std::endl;
		return false;
	}
}

void options::create_default_output_file_name()
{
	output_file_name = input_file_name;
	size_t dot_pos = output_file_name.rfind(".pm.json");
	if (dot_pos != string::npos) {
		output_file_name.resize(dot_pos);
	}
	output_file_name += ".kurs.xml";
}

void options::print_help()
{
	std::cerr << "Usage:" << std::endl;
	std::cerr << "\t" << program_name_
		<< " [--output=FILENAME.kurs.xml] FILENAME.pm.json" << std::endl << std::endl;
	std::cerr << "Options:" << std::endl;
	std::cerr << "\tFILENAME.kurs.xml\tForce output file name (optional)" << std::endl;
	std::cerr << "\tFILENAME.pm.json\tInput file name (required)" << std::endl;
}

}
