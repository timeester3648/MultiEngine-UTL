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
   * [utl::shell::](#utlshell)
   * [utl::stre::](#utlstre)
- [Work in progress](#work-in-progress)
- [Requirements](#requirements)
- [Version history](#version-history)
- [License](#license)

<!-- TOC end -->



<!-- TOC --><a name="design-principles"></a>
## Design principles

* Library should be header only;
* Library should be portable (Windows, Linux);
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
### utl::math:: ###
Coordinate transformations, mathematical constans and helper functions.
	 
	 # ::abs(), ::sign(), ::sqr(), ::cube(), ::midpoint(), deg_to_rad(), rad_to_deg() #
	 Constexpr templated math functions, useful when writing expressions with a "textbook form" math.
	
	 # ::uint_difference() #
	 Returns abs(uint - uint) with respect to uint size and possible overflow.
	
	 # ::ternary_branchless() #
	 Branchless ternary operator. Slightly slower that regular ternary on most CPUs.
	 Should not be used unless branchess qualifier is necessary (like in GPU computation).
	
	 # ::ternary_bitselect() #
	 Faster branchless ternary for integer types.
	 If 2nd return is ommited, 0 is assumed, which allows for significant optimization.

<!-- TOC --><a name="utlshell"></a>
### utl::<wbr>shell::
Command line utils that allow simple creation of temporary files and command line
calls with stdout and stderr piping (a task surprisingly untrivial in standard C++).
	
	 # ::random_ascii_string() #
	 Creates random ASCII string of given length.
	 Uses chars in ['a', 'z'] range.
	
	 # ::generate_temp_file() #
	 Generates temporary .txt file with a random unique name, and returns it's filepath.
	 Files generated during current runtime can be deleted with ::clear_temp_files().
	 Uses relative path internally.
	
	 # ::clear_temp_files() #
	 Clears temporary files generated during current runtime.
	
	 # ::erase_temp_file() #
	 Clears a single temporary file with given filepath.
	
	 # ::run_command() #
	 Runs a command using the default system shell.
	 Returns piped status (error code), stdout and stderr.
	
<!-- TOC --><a name="utlstre"></a>	
### utl::stre::
String extensions, mainly a template ::to_str() method which works with all STL containers,
including maps, sets and tuples with any level of mutual nesting. Also includes some
expansions of <type_traits> header that allow categorizing types at compile-time.
	
	 # ::is_printable<Type> #
	 Integral constant, returns in "::value" whether Type can be printed through std::cout.
	 Criteria: Existance of operator 'ANY_TYPE operator<<(std::ostream&, Type&)'
	
	 # ::is_iterable_through<Type> #
	 Integral constant, returns in "::value" whether Type can be iterated through.
	 Criteria: Existance of .begin() and .end() with applicable operator()++
	
	 # ::is_const_iterable_through<Type> #
	 Integral constant, returns in "::value" whether Type can be const-iterated through.
	 Criteria: Existance of .cbegin() and .cend() with applicable operator()++
	
	 # ::is_tuple_like<Type> #
	 Integral constant, returns in "::value" whether Type has a tuple-like structure.
	 Tuple-like structure include std::tuple, std::pair, std::array, std::ranges::subrange (since C++20)
	 Criteria: Existance of applicable std::get<0>() and std::tuple_size()
	
	 # ::is_string<Type> #
	 Integral constant, returns in "::value" whether Type is a char string.
	 Criteria: Type can be decayed to std::string or a char* pointer
	
	 # ::is_to_str_convertible<Type> #
	 Integral constant, returns in "::value" whether Type can be converted to string through ::to_str().
	 Criteria: Existance of a valid utl::stre::to_str() overload
	
	 # ::to_str() #
	 Converts any standard container or a custom container with necessary member functions to std::string.
	 Works with tuples and tuple-like classes.
	 Works with nested containers/tuples through recursive template instantiation, which
	 resolves as long as types at the end of recursion have a valid operator<<() for ostreams.

	

<!-- TOC --><a name="work-in-progress"></a>
## Work in progress

* Coordinate transformation function in "utl::math::";
* Compile time function that abstract away common compiler-specific defines.



<!-- TOC --><a name="requirements"></a>
## Requirements

* Requires C++17 support;
* Some modules require POSIX-compliant system (Linux) or Windows.



<!-- TOC --><a name="version-history"></a>
## Version history

* 00.04
    * Added utl::stre module;
    * Switched utl::timer to use nanoseconds internally, improved precision;

* 00.03
    * Added utl::<wbr>shell module;
    * Added mindpoint(), ternary_branchess() and ternary_bitselect() to utl::math.

* 00.02
    * Finished initial draft documentation.

* 00.01
    * Uploaded initial draft with "voidstream", "argcv", "table", "timer", "sleep", "random" and "math" modules.



<!-- TOC --><a name="license"></a>
## License

This project is licensed under the MIT License - see the LICENSE.md file for details