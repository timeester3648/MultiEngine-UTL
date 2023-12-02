#include "proto_utils.hpp"

#include <iostream>


int main(int argc, char* argv[]) {
	using namespace utl;



	// ### utl::voidstream:: ##
	std::cout << "\n\n### utl::voidstream:: ###\n\n";

	std::cout << "std::cout will print:\n";
	std::cout << "<hello there!>\n\n";

	std::cout << "utl::voidstream::vout will not:\n";
	voidstream::vout << "<hello there!>\n\n";



	// ### utl::argcv:: ###
	std::cout << "\n\n### utl::argcv:: ###\n\n";

	std::cout << "Exe path:\n" << argcv::exe_path_view(argv) << "\n\n";

	std::cout << "Command line arguments (if present):\n";
	for (const auto &arg : argcv::command_line_args(argc, argv)) std::cout << arg << "\n";



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
	constexpr auto sleep_duration_ms = 16.67;

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
		<< "rand_int(0, 100) = " << random::rand_int(0, 100) << "\n"
		<< "rand_double() = " << random::rand_double() << "\n"
		<< "rand_double(-5, 5) = " << random::rand_double(-5, 5) << "\n"
		<< "rand_bool() = " << random::rand_bool() << "\n"
		<< "rand_choise({1, 2, 3}) = " << random::rand_choise({1, 2, 3}) << "\n"
		<< "rand_linear_combination(2., 3.) = " << random::rand_linear_combination(2., 3.) << "\n";



	// ### utl::math:: ###
	std::cout << "\n\n### utl::math:: ###\n\n";

	std::cout
		<< "All methods below are constexpr and type agnostic:\n"
		<< "\n"
		<< "math constants:\n"
		<< "PI = " << math::PI << "\n"
		<< "E = " << math::E << "\n"
		<< "\n"
		<< "abs(), sign(), sqr(), cube(), deg_to_rad(), rad_to_deg():\n"
		<< "abs(-4) = " << math::abs(-4) << "\n"
		<< "sign(-4) = " << math::sign(-4) << "\n"
		<< "sqr(-4) = " << math::sqr(-4) << "\n"
		<< "cube(-4) = " << math::cube(-4) << "\n"
		<< "deg_to_rad(180.) = " << math::deg_to_rad(180.) << "\n"
		<< "rad_to_deg(PI) = " << math::rad_to_deg(math::PI) << "\n"
		<< "\n"
		<< "uint_difference(uint - uint) with respect to unsigned overflow:\n"
		<< "uint_difference(5u, 17u) = " << math::uint_difference(5u, 17u) << "\n"
		<< "\n"
		<< "ternary_branchless(true, 3.12, -4.17) = " << math::ternary_branchless(true, 3.12, -4.17) << "\n"
		<< "ternary_bitselect(true, 15, -5) = " << math::ternary_bitselect(true, 15, -5) << "\n"
		<< "ternary_bitselect(false, 15) = " << math::ternary_bitselect(false, 15) << "\n";

	

	// ### utl::shell:: ###
	std::cout << "\n\n### utl::shell:: ###\n\n";

	std::cout
		<< "random_ascii_string(20) = " << shell::random_ascii_string(20) << "\n"
		<< "\n";

	std::cout << "Generate temp file and fill it with text:\n";
	const auto temp_file_path = shell::generate_temp_file();
	std::ofstream(temp_file_path) << "~~~FILE CONTENT~~~";
	std::cout << "generate_temp_file(): " << temp_file_path << "\n\n";

	std::cout << "Run command to show file contents, this example assumes windows:\n";
	const auto command_result = shell::run_command("type " + temp_file_path);
	std::cout
		<< "shell::run_command(\"type \" + <temp_file_path>)" << "\n"
		<< "command_result.status = " << command_result.status << "\n"
		<< "command_result.stdout_output = " << command_result.stdout_output << "\n"
		<< "command_result.stderr_output = " << command_result.stderr_output << "\n";

	shell::clear_temp_files();



	// ### utl::timer:: ###
	std::cout << "\n\n### utl::timer:: ###\n\n";

	std::cout << "Current time is: " << timer::datetime_string() << "\n\n";

	timer::start();
	std::this_thread::sleep_for(std::chrono::milliseconds(3700));
	std::cout
		<< "Time elapsed during sleep_for(3700 ms):\n"
		<< timer::elapsed_string_ms() << "\n"
		<< timer::elapsed_string_sec() << "\n"
		<< timer::elapsed_string_min() << "\n"
		<< timer::elapsed_string_hours() << "\n"
		<< timer::elapsed_string_fullform() << "\n";



	return 0;
}