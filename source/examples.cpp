#include "proto_utils.hpp"
#include "proto_utils.hpp"

#include <iostream>
#include <unordered_map>
#include <tuple>
#include <set>
#include <thread>


UTL_DECLARE_ENUM_WITH_STRING_CONVERSION(Sides, LEFT, RIGHT, TOP, BOTTOM)
	// like a regular enum must be declared outside of function


int main(int argc, char* argv[]) {
	using namespace utl;

	

	// ### utl::voidstream:: ##
	std::cout << "\n\n### utl::voidstream:: ###\n\n";

	std::cout << "std::cout will print:\n";
	std::cout << "<hello there!>\n\n";

	std::cout << "voidstream::vout will not:\n";
	voidstream::vout << "<hello there!>\n\n";



	// ### utl::table:: ###
	std::cout << "\n\n### utl::table:: ###\n\n";

	table::create({ 16, 16, 16, 16, 20 });
	table::set_formats({ table::NONE, table::DEFAULT(), table::FIXED(2),table::SCIENTIFIC(3), table::BOOL });

	table::hline();
	table::cell("Method", "Threads", "Speedup", "Error", "Err. within range");
	table::hline();
	table::cell("Gauss", 16, 11.845236, 1.96e-4, false);
	table::cell("Jacobi", 16, 15.512512, 1.37e-5, false);
	table::cell("Seidel", 16, 13.412321, 1.74e-6, true);
	table::cell("Relaxation", 16, 13.926783, 1.17e-6, true);
	table::hline();



	// ### utl::sleep:: ##
	std::cout << "\n\n### utl::sleep:: ###\n\n";

	constexpr int repeats = 6;
	constexpr double sleep_duration_ms = 16.67;

	std::cout << "Sleeping for " << sleep_duration_ms << " ms.\n";

	std::cout << "\nstd::this_thread::sleep_for():\n";
	for (int i = 0; i < repeats; ++i) {
		auto start = std::chrono::steady_clock::now();

		std::this_thread::sleep_for(std::chrono::nanoseconds(static_cast<int64_t>(sleep_duration_ms * 1e6)));

		auto end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 << " ms\n";
	}
	
	std::cout << "\nsleep::spinlock():\n";
	for (int i = 0; i < repeats; ++i) {
		auto start = std::chrono::steady_clock::now();

		sleep::spinlock(sleep_duration_ms);

		auto end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 << " ms\n";
	}

	std::cout << "\nsleep::hybrid():\n";
	for (int i = 0; i < repeats; ++i) {
		auto start = std::chrono::steady_clock::now();

		sleep::hybrid(sleep_duration_ms);

		auto end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 << " ms\n";
	}

	std::cout << "\nsleep::system():\n";
	for (int i = 0; i < repeats; ++i) {
		auto start = std::chrono::steady_clock::now();

		sleep::system(sleep_duration_ms);

		auto end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 << " ms\n";
	}
	


	// ### utl::random:: ##
	std::cout << "\n\n### utl::random:: ###\n\n";

	random::seed_with_time();
	std::cout
		<< "rand_int(0, 100) = "                << random::rand_int(0, 100)                << "\n"
		<< "rand_double() = "                   << random::rand_double()                   << "\n"
		<< "rand_double(-5, 5) = "              << random::rand_double(-5, 5)              << "\n"
		<< "rand_bool() = "                     << random::rand_bool()                     << "\n"
		<< "rand_choise({1, 2, 3}) = "          << random::rand_choise({1, 2, 3})          << "\n"
		<< "rand_linear_combination(2., 3.) = " << random::rand_linear_combination(2., 3.) << "\n";



	// ### utl::math:: ###
	std::cout << "\n\n### utl::math:: ###\n\n";

	// Type traits
	std::cout
		<< std::boolalpha
		<< "are doubles addable?    -> " << math::is_addable_with_itself<double>::value << "\n"
		<< "are std::pairs addable? -> " << math::is_addable_with_itself<std::pair<int, int>>::value << "\n";

	std::cout << "\n";

	// Using math functions
	std::cout
		<< "All methods below are constexpr and type agnostic:\n"
		<< "abs(-4) = "                               << math::abs(-4)                               << "\n"
		<< "sign(-4) = "                              << math::sign(-4)                              << "\n"
		<< "sqr(-4) = "                               << math::sqr(-4)                               << "\n"
		<< "cube(-4) = "                              << math::cube(-4)                              << "\n"
		<< "deg_to_rad(180.) = "                      << math::deg_to_rad(180.)                      << "\n"
		<< "rad_to_deg(PI) = "                        << math::rad_to_deg(math::PI)                  << "\n"
		<< "midpoint(4., 5.) = "                      << math::midpoint(4., 5.)                      << "\n"
		<< "\n"
		<< "uint_difference(5u, 17u) = "              << math::uint_difference(5u, 17u)              << "\n"
		<< "\n"
		<< "ternary_branchless(true, 3.12, -4.17) = " << math::ternary_branchless(true, 3.12, -4.17) << "\n"
		<< "ternary_bitselect(true, 15, -5) = "       << math::ternary_bitselect(true, 15, -5)       << "\n"
		<< "ternary_bitselect(false, 15) = "          << math::ternary_bitselect(false, 15)          << "\n";

	// ### utl::shell:: ###
	std::cout << "\n\n### utl::shell:: ###\n\n";

	// Create temp file
	const auto temp_file_path = shell::generate_temp_file();
	const auto temp_file_text = "~~~FILE CONTENTS~~~";
	std::ofstream(temp_file_path) << temp_file_text;

	std::cout << "Temp. file path: " << temp_file_path << "\n";
	std::cout << "Temp. file text: " << temp_file_text << "\n";
	
	// Run command to show file contents (windows-specific command in this example)
	const auto command = "type " + temp_file_path;
	const auto command_result = shell::run_command(command);

	std::cout
		<< "shell::run_command(" << command << "):\n"
		<< "command_result.status = " << command_result.status << "\n"
		<< "command_result.stdout_output = " << command_result.stdout_output << "\n"
		<< "command_result.stderr_output = " << command_result.stderr_output << "\n";

	shell::clear_temp_files();

	std::cout << "Exe path:\n" << shell::get_exe_path(argv) << "\n\n";

	std::cout << "Command line arguments (if present):\n";
	for (const auto &arg : shell::get_command_line_args(argc, argv))
		std::cout << arg << "\n";

	// ### utl::stre:: ###
	std::cout << "\n\n### utl::stre:: ###\n\n";

	// Type traits
	std::cout
		<< std::boolalpha
		<< "is_printable< int >                    = " << stre::is_printable<int>::value                    << "\n"
		<< "is_iterable_through< std::list<int> >  = " << stre::is_iterable_through<std::list<int>>::value  << "\n"
		<< "is_tuple_like< std::string >           = " << stre::is_tuple_like<std::string>::value           << "\n"
		<< "is_string< const char* >               = " << stre::is_string<const char*>::value               << "\n"
		<< "is_to_str_convertible< std::set<int> > = " << stre::is_to_str_convertible<std::set<int>>::value << "\n"
		<< "\n";

	// Declare objects
	std::tuple<int, double, std::string> tup = { 2, 3.14, "text" };
	std::vector<std::vector<int>> vec_of_vecs = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9} };
	std::unordered_map<std::string, int> map = { {"key_1", 1}, {"key_2", 2} };
	std::tuple<std::vector<bool>, std::vector<std::string>, std::vector<std::pair<int, int>>> tup_of_vecs =
		{ { true, false, true }, { "text_1", "text_2" }, { {4, 5}, {7, 8} } };

	// Print objects
	std::cout
		<< "to_str(tuple):\n" << stre::to_str(tup) << "\n\n"
		<< "to_str(vector of vectors):\n" << stre::to_str(vec_of_vecs) << "\n\n"
		<< "to_str(unordered_map):\n" << stre::to_str(map) << "\n\n"
		<< "to_str(tuple of vectors with bools, strings and pairs):\n" << stre::to_str(tup_of_vecs) << "\n"
		<< "\n";

	// Inline stringstream
	const std::string str = stre::InlineStream() << "Value " << 3.14 << " is smaller than " << 6.28;
	std::cout << str << "\n";


	// ### utl::timer:: ###
	std::cout << "\n\n### utl::timer:: ###\n\n";

	std::cout << "Current time is: " << timer::datetime_string() << "\n\n";

	timer::start();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout
		<< "Time elapsed during sleep_for(100 ms):\n"
		<< timer::elapsed_string_ms() << "\n"
		<< timer::elapsed_string_sec() << "\n"
		<< timer::elapsed_string_min() << "\n"
		<< timer::elapsed_string_hours() << "\n"
		<< timer::elapsed_string_fullform() << "\n";


	// ### utl macros ###
	std::cout << "\n\n### utl macros ###\n\n";

	UTL_LOG_SET_OUTPUT(std::cerr);

	// LOG compiles always
	UTL_LOG("Block ", 17, ": responce in order. Proceeding with code ", 0, ".");

	// DLOG compiles only in Debug
	UTL_LOG_DEBUG("Texture with ID ", 15037, " seems to be corrupted.");

	// NOTE: stre::to_str() can be used to pass complex objects to logger

	std::cout << "Current platform: " << UTL_CURRENT_OS << "\n";

	// Loop than repeats N times
	UTL_REPEAT(5) {
		std::cout << "Ho\n";
	}

	// Variadic macro that outputs size of __VA_ARGS__
	#define VARIADIC_MACRO(...) UTL_VA_ARGS_COUNT(__VA_ARGS__)

	constexpr int args = VARIADIC_MACRO(1, 2., "3", A, B, (std::pair<int, int>{4, 5}), "12,3");

	std::cout << "\nSize of __VA_ARGS__: " << args << "\n";

	// Enum with string conversion
	std::cout
		<< "(enum -> string) conversion:\n"
		<< Sides::BOTTOM << " -> " << Sides::to_string(Sides::BOTTOM) << "\n"
		<< "(string -> enum) conversion:\n"
		<< "BOTTOM" << " -> " << Sides::from_string("BOTTOM") << "\n";


	return 0;
}