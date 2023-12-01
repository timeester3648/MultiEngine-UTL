# Prototyping utils

This is a collection of various utilities that aim to provide a set of concise "helpers" that allow prototying with minimal boilerplate. Most of the following modules were created during my work in gamedev and university projects.

<!-- TOC start -->

- [Design principles](#design-principles)
- [Modules](#modules)
   * [utl::voidstream::](#utlvoidstream)
   * [utl::argcv::](#utlargcv)
   * [utl::table::](#utltable)
   * [utl::timer::](#utltimer)
   * [utl::sleep::](#utlsleep)
   * [utl::random::](#utlrandom)
   * [utl::math::](#utlmath)
- [Work in progress](#work-in-progress)
- [Requirements](#requirements)
- [Version history](#version-history)
- [License](#license)

<!-- TOC end -->



<!-- TOC --><a name="design-principles"></a>
## Design principles

* Library should be header only;
* Each module should be self-contained and reside in a separate namespace;
* Each module should toggleable through a define;
* Boilerplate code on user side should be minimal;
* Modern C++ idioms should be prefered both externally and internally;
* Header should contain a duplicate set of documentation;
* Usage examples should be provided for all major functions.



<!-- TOC --><a name="modules"></a>
## Modules

<!-- TOC --><a name="utlvoidstream"></a>
### utl::voidstream::
"voidstream" that functions like std::ostream with no output.
Can be passed to interfaces that use streams to silence their output.
	
	 # ::vstreambuf #
	 Stream buffer that overflows with no output, usage example:
	    > std::ofstream output_stream(&vstreambuf);
	    > output_stream << VALUE; // produces nothing
	
	 # ::vout #
	 Output stream that produces no output, usage example:
	    > vout << VALUE; // produces nothing
	
<!-- TOC --><a name="utlargcv"></a>
### utl::argcv::
Parsers that convert argc, argv[] to std::string.
	
	 # ::exe_path(), ::exe_path_view() #
	 Parses executable path from argcv as std::string or std::string_view.
	 Views have lower overhead, but keep pointers to original data.
	
	 # ::command_line_args(), ::command_line_args_view() #
	 Parses command line arguments from argcv as std::string or std::string_view.
	 Views have lower overhead, but keep pointers to original data.
	
<!-- TOC --><a name="utltable"></a>
### utl::table::
Functions used to display results in a tabular fashion.
	
	 # ::create() #
	 Sets up table with given number of columns and their widths.
	
	 # ::set_formats() #
	 (optional) Sets up column formats for better display
	
	 # ::NONE, ::FIXED(), ::DEFAULT(), ::SCIENTIFIC(), ::BOOL() #
	 Format flags with following effects:
	 > NONE - Use default C++ formats
	 > FIXED(N) - Display floats in fixed form with N decimals, no argument assumes N = 3
	 > DEFAULT(N) - Display floats in default form with N decimals, no argument assumes N = 6
	 > SCIENTIFIC(N) - Display floats in scientific form with N decimals, no argument assumes N = 3
	 > BOOL - Display booleans as text
	
	 # ::cell() #
	 Draws a single table cell, if multiple arguments are passed, draws each one in a new cell.
	 Accepts any type with a defined "<<" ostream operator.
	
<!-- TOC --><a name="utltimer"></a>
### utl::timer::
Global-state timer with built-in formatting. Functions for local date and time.
	
	 # ::start() #
	 Starts the timer.
	 NOTE: If start() wasn't called system will use uninitialized value as a start.
	
	 # ::elapsed_ms(), ::elapsed_sec(), ::elapsed_min(), ::elapsed_hours() #
	 Elapsed time as double.
	
	 # ::elapsed_string_ms(), ::elapsed_string_sec(), ::elapsed_string_min(), ::elapsed_string_hours() #
	 Elapsed time as std::string.
	
	 # ::elapsed_string:fullform() #
	 Elapsed time in format "%H hours %M min %S sec %MS ms".
	
	 # ::datetime_string() #
	 Current local date and time in format "%y-%m-%d %H:%M:%S".
	
	 # ::datetime_string_id() #
	 Current local date and time in format "%y-%m-%d-%H-%M-%S".
	 Less readable that usual format, but can be used in filenames which prohibit ":" usage.
	
<!-- TOC --><a name="utlsleep"></a>
### utl::sleep::
Various implementation of sleep(), used for precise delays.
	
	 # ::spinlock() #
	 Best precision, uses CPU.
	 Expected error: ~1 ms
	
	 # ::hybrid() #
	 Recommended option, similar precision to spinlock with minimal CPU usage.
	 Uses system sleep with statistically estimated error with spinlock sleep at the end.
	 Adjusts system sleep error estimate with every call.
	 Expected error: ~1-2 ms
	
	 # ::system() #
	 Worst precision, frees CPU.
	 Expected error: ~10-20 ms

<!-- TOC --><a name="utlrandom"></a>
### utl::random::
Various convenient random functions, utilizes rand() internally.
If good quality random is necessary use std::random generators instead.
	
	 # ::rand_int(), ::rand_uint(), ::rand_float(), ::rand_double() #
	 Random value in [min, max] range.
	 Floats with no parameters assume range [0, 1].
	
	 # ::rand_bool() #
	 Randomly chooses 0 or 1.
	
	 # ::rand_choise() #
	 Randomly chooses a value from initializer list.
	
	 # ::rand_linear_combination() #
	 Produces "c A + (1-c) B" with random "0 < c < 1" assuming objects "A", "B" support arithmetic operations.
	 Useful for vector and color operations.

<!-- TOC --><a name="utlmath"></a>
### utl::math::
Coordinate transformations, mathematical constans and helper functions.
	 
	 # ::abs(), ::sign(), ::sqr(), ::cube(), deg_to_rad(), rad_to_deg() #
	 Constexpr templated functions, useful when writing expressions with a "textbook form" math.
	
	 # ::uint_difference() #
	 Returns abs(uint - uint) with respect to uint size and possible overflow.
		


<!-- TOC --><a name="work-in-progress"></a>
## Work in progress

* "utl::`shell`::" module that allows cross-platform access to command line commants;
* Coordinate transformation function in "utl::math::";
* Bitselect adn branchless ternary in "utl::math::";



<!-- TOC --><a name="requirements"></a>
## Requirements

* Requires C++17 support;
* Some modules require POSIX-compliant system (Linux) or Windows.



<!-- TOC --><a name="version-history"></a>
## Version history

* 00.01
    * Uploaded initial draft with "voidstream", "argcv", "table", "timer", "sleep", "random" and "math" modules.



<!-- TOC --><a name="license"></a>
## License

This project is licensed under the MIT License - see the LICENSE.md file for details