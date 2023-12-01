


# Prototyping utils

This is a collection of various utilities that aim to provide a set of concise "helpers" that allow prototying with minimal boilerplate. Most of the following modules were created during my work in gamedev and university projects.

## Design principles

* Library should be header only;
* Each module should be self-contained and reside in a separate namespace;
* Each module should toggleable through a define;
* Boilerplate code on user side should be minimal;
* Modern C++ idioms should be prefered both externally and internally;
* Header should contain a duplicate set of documentation;
* Usage examples should be provided for all major functions.

## Modules

### utl::voidstream::
"voidstream" that functions as std::ostream with no output.
Can be passed to interfaces that use streams to silence their output.
	
	 # ::vstreambuf #
	 Stream buffer that overflows with no output, usage example:
	    > std::ofstream output_stream(&vstreambuf);
	    > output_stream << VALUE; // produces nothing
	
	 # ::vout #
	 Output stream that produces no output, usage example:
	    > vout << VALUE; // produces nothing
	
### utl::argcv::
Parsers that convert argc, argv[] to std::string.
	
	 # ::exe_path(), ::exe_path_view() #
	 Parses executable path from argcv as std::string or std::string_view.
	 Views have lower overhead, but keep pointers to original data.
	
	 # ::command_line_args(), ::command_line_args_view() #
	 Parses command line arguments from argcv as std::string or std::string_view.
	 Views have lower overhead, but keep pointers to original data.
	

## Requirements

* Requires C++17 support;
* Requires POSIX-compliant system or Windows.

## Version history

* 00.01
    * Uploaded initial draft with "voidstream", "argcv", "table", "timer", "sleep", "random" and "math" modules.

## License

This project is licensed under the MIT License - see the LICENSE.md file for details