#pragma once



// Enabled modules
#define UTL_VOIDSTREAM
#define UTL_ARGCV
#define UTL_TABLE
#define UTL_TIMER
#define UTL_SLEEP
#define UTL_RANDOM
#define UTL_MATH



// Module dependencies
#ifdef UTL_VOIDSTREAM
#include <ostream>
#endif

#ifdef UTL_ARGCV
#include <vector>
#include <string>
#include <string_view>
#endif

#ifdef UTL_TABLE
#include <ios>
#include <iostream>
#include <iomanip>
#include <functional>
#include <initializer_list>
#include <string>
#endif

#ifdef UTL_TIMER
#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#endif

#ifdef UTL_SLEEP
#include <chrono>
#include <thread>
#endif

#ifdef UTL_RANDOM
#include <cstdlib>
#include <initializer_list>
#endif

#ifdef UTL_MATH
#include <type_traits>
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4996) //_CRT_SECURE_NO_WARNINGS
	// Disables MSVC requiring "safe" versions of localtime() instead of standard ones
#endif



namespace utl {



	// ### utl::voidstream:: ###
	// "voidstream" that functions as std::ostream with no output.
	// Can be passed to interfaces that use streams to silence their output.
	//
	// # ::vstreambuf #
	// Stream buffer that overflows with no output, usage example:
	//    > std::ofstream output_stream(&vstreambuf);
	//    > output_stream << VALUE; // produces nothing
	//
	// # ::vout #
	// Output stream that produces no output, usage example:
	//    > vout << VALUE; // produces nothing
	//
	namespace voidstream {

		class VoidStreamBuf : public std::streambuf {
		public:
			inline int overflow(int c) { return c; }
		};

		class VoidStream : public std::ostream {
		public:
			inline VoidStream() : std::ostream(&buffer) {}
		private:
			VoidStreamBuf buffer;
		};

		inline VoidStreamBuf vstreambuf;
		inline VoidStream vout;
	}



	// ### utl::argcv:: ###
	// Parsers that convert argc, argv[] to std::string.
	//
	// # ::exe_path(), ::exe_path_view() #
	// Parses executable path from argcv as std::string or std::string_view.
	// Views have lower overhead, but keep pointers to original data.
	//
	// # ::command_line_args(), ::command_line_args_view() #
	// Parses command line arguments from argcv as std::string or std::string_view.
	// Views have lower overhead, but keep pointers to original data.
	//
	namespace argcv {
		inline std::string exe_path(char** argv) {
			return std::string(argv[0]);
		}

		inline std::string_view exe_path_view(char** argv) {
			return std::string_view(argv[0]);
		}

		inline std::vector<std::string> command_line_args(int argc, char** argv) {
			std::vector<std::string> arguments(argc - 1);
			for (size_t i = 0; i < arguments.size(); ++i) arguments.emplace_back(argv[i]);
			return arguments;
		}

		inline std::vector<std::string_view> command_line_args_view(int argc, char** argv) {
			std::vector<std::string_view> arguments(argc - 1);
			for (size_t i = 0; i < arguments.size(); ++i) arguments.emplace_back(argv[i]);
			return arguments;
		}
	}



	// ### utl::table:: ###
	// Functions used to display results in a tabular fashion.
	//
	// # ::create() #
	// Sets up table with given number of columns and their widths.
	//
	// # ::set_formats() #
	// (optional) Sets up column formats for better display
	//
	// # ::NONE, ::FIXED(), ::DEFAULT(), ::SCIENTIFIC(), ::BOOL() #
	// Format flags with following effects:
	// > NONE - Use default C++ formats
	// > FIXED(N) - Display floats in fixed form with N decimals, no argument assumes N = 3
	// > DEFAULT(N) - Display floats in default form with N decimals, no argument assumes N = 6
	// > SCIENTIFIC(N) - Display floats in scientific form with N decimals, no argument assumes N = 3
	// > BOOL - Display booleans as text
	//
	// # ::cell() #
	// Draws a single table cell, if multiple arguments are passed, draws each one in a new cell.
	// Accepts any type with a defined "<<" ostream operator.
	//
	namespace table {
		// Types
		using _uint = std::streamsize;
		using _ios_flags = std::ios_base::fmtflags;

		struct _Column_format {
			_ios_flags flags;
			_uint precision;
		};
		
		struct _Column {
			_uint width;
			_Column_format col_format;
		};

		// Predefined formats
		constexpr _Column_format NONE = { std::ios::showpoint, 6 };
		
		constexpr _Column_format FIXED(_uint decimals = 3) { return { std::ios::fixed, decimals }; }
		constexpr _Column_format DEFAULT(_uint decimals = 6) { return { std::ios::showpoint, decimals }; }
		constexpr _Column_format SCIENTIFIC(_uint decimals = 3) { return { std::ios::scientific, decimals }; }

		constexpr _Column_format BOOL = { std::ios::boolalpha, 3 };

		// Internal state
		inline std::vector<_Column> _columns;
		inline int _current_column = 0;
		inline std::reference_wrapper<std::ostream> _output_stream = std::cout;

		// Table setup
		inline void create(std::initializer_list<_uint> &&widths) {
			_columns.resize(widths.size());
			for (size_t i = 0; i < _columns.size(); ++i) {
				_columns[i].width = widths.begin()[i];
				_columns[i].col_format = DEFAULT();
			}
		}

		inline void set_formats(std::initializer_list<_Column_format> &&formats) {
			for (size_t i = 0; i < _columns.size(); ++i)
				_columns[i].col_format = formats.begin()[i];
		}

		inline void set_ostream(std::ostream &new_ostream) {
			_output_stream = new_ostream;
		}

		inline void cell() {};

		template<typename T, typename... Types>
		void cell(T value, const Types... other_values) {
			const std::string left_cline = (_current_column == 0) ? "|" : "";
			const std::string right_cline = (_current_column == _columns.size() - 1) ? "|\n" : "|";
			const _ios_flags format = _columns[_current_column].col_format.flags;
			const _uint float_precision = _columns[_current_column].col_format.precision;

			// Save old stream state
			std::ios old_state(nullptr);
			old_state.copyfmt(_output_stream.get());

			// Set table formatting
			_output_stream.get() << std::resetiosflags(_output_stream.get().flags());
			_output_stream.get().flags(format);
			_output_stream.get().precision(float_precision);

			auto t = std::ios::fixed;

			// Print
			_output_stream.get()
				<< left_cline
				<< std::setw(_columns[_current_column].width) << value
				<< right_cline;

			// Return old stream state
			_output_stream.get().copyfmt(old_state);

			// Advance column counter
			_current_column = (_current_column == _columns.size() - 1) ? 0 : _current_column + 1;

			cell(other_values...);
		}

		inline void hline() {
			_output_stream.get() << "|";
			for (const auto &col : _columns) _output_stream.get() << std::string(static_cast<size_t>(col.width), '-') << "|";
			_output_stream.get() << "\n";
		}
	}



	// ### utl::timer:: ###
	// Global-state timer with built-in formatting. Functions for local date and time.
	//
	// # ::start() #
	// Starts the timer.
	// NOTE: If start() wasn't called system will use uninitialized value as a start.
	//
	// # ::elapsed_ms(), ::elapsed_sec(), ::elapsed_min(), ::elapsed_hours() #
	// Elapsed time as double.
	//
	// # ::elapsed_string_ms(), ::elapsed_string_sec(), ::elapsed_string_min(), ::elapsed_string_hours() #
	// Elapsed time as std::string.
	//
	// # ::elapsed_string:fullform() #
	// Elapsed time in format "%H hours %M min %S sec %MS ms".
	//
	// # ::datetime_string() #
	// Current local date and time in format "%y-%m-%d %H:%M:%S".
	//
	// # ::datetime_string_id() #
	// Current local date and time in format "%y-%m-%d-%H-%M-%S".
	// Less readable that usual format, but can be used in filenames which prohibit ":" usage.
	//
	namespace timer {
		using _clock = std::chrono::steady_clock;

		constexpr long long MS_IN_SEC = 1000;
		constexpr long long MS_IN_MIN = 60 * MS_IN_SEC;
		constexpr long long MS_IN_HOUR = 60 * MS_IN_MIN;

		inline _clock::time_point _start_timepoint;

		void start() {
			_start_timepoint = _clock::now();
		}

		long long _elapsed_time_as_ms() { return std::chrono::duration_cast<std::chrono::milliseconds>(_clock::now() - _start_timepoint).count(); }

		// Elapsed time
		double elapsed_ms() { return static_cast<double>(_elapsed_time_as_ms()); }
		double elapsed_sec() { return static_cast<double>(_elapsed_time_as_ms()) / MS_IN_SEC; }
		double elapsed_min() { return static_cast<double>(_elapsed_time_as_ms()) / MS_IN_MIN; }
		double elapsed_hours() { return static_cast<double>(_elapsed_time_as_ms()) / MS_IN_HOUR; }

		// Elapsed string
		std::string elapsed_string_ms() { return std::to_string(elapsed_ms()) + " ms"; }
		std::string elapsed_string_sec() { return std::to_string(elapsed_sec()) + " sec"; }
		std::string elapsed_string_min() { return std::to_string(elapsed_min()) + " min"; }
		std::string elapsed_string_hours() { return std::to_string(elapsed_hours()) + " hours"; }

		// Full form
		std::string elapsed_string_fullform() {
			long long unaccounted_ms = _elapsed_time_as_ms();

			long long ms = 0;
			long long min = 0;
			long long sec = 0;
			long long hours = 0;

			if (unaccounted_ms > MS_IN_HOUR) {
				hours += unaccounted_ms / MS_IN_HOUR;
				unaccounted_ms -= hours * MS_IN_HOUR;
			}

			if (unaccounted_ms > MS_IN_MIN) {
				min += unaccounted_ms / MS_IN_MIN;
				unaccounted_ms -= min * MS_IN_MIN;
			}

			if (unaccounted_ms > MS_IN_SEC) {
				sec += unaccounted_ms / MS_IN_SEC;
				unaccounted_ms -= sec * MS_IN_SEC;
			}

			ms = unaccounted_ms;

			return std::to_string(hours) + " hours " + std::to_string(min) + " min " + std::to_string(sec) + " sec " + std::to_string(ms) + " ms ";
		}

		// Date string
		std::string datetime_string() {
			std::time_t t = std::time(nullptr);
			char mbstr[100];
			std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
			return std::string(mbstr);
		}

		std::string datetime_string_id() {
			std::time_t t = std::time(nullptr);
			char mbstr[100];
			std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d-%H-%M-%S", std::localtime(&t));
			return std::string(mbstr);
		}
	}



	// ### utl::sleep:: ###
	// Various implementation of sleep(), used for precise delays.
	//
	// # ::spinlock() #
	// Best precision, uses CPU.
	// Expected error: ~1 ms
	//
	// # ::hybrid() #
	// Recommended option, similar precision to spinlock with minimal CPU usage.
	// Uses system sleep with estimated error with spinlock sleep at the end.
	// Adjusts system sleep error estimate with every call.
	// Expected error: ~1-2 ms
	//
	// # ::system() #
	// Worst precision, frees CPU.
	// Expected error: ~10-20 ms
	namespace sleep {
		using _clock = std::chrono::steady_clock;
		using _chrono_ns = std::chrono::nanoseconds;

		inline void spinlock(double ms) {
			const long long ns = static_cast<int64_t>(ms * 1e6);
			const auto start_timepoint = _clock::now();

			volatile int i = 0; // volatile prevents standard-compliant compilers from optimizing away the loop
			while (std::chrono::duration_cast<_chrono_ns>(_clock::now() - start_timepoint).count() < ns) { ++i; }
		}

		inline void hybrid(double ms) {
			static double estimate = 5e-3; // initial sleep_for() error estimate
			static double mean = estimate;
			static double m2 = 0;
			static int64_t count = 1;

			// We treat sleep_for(1 ms) as a random variate "1 ms + random_value()"
			while (ms > estimate) {
				const auto start = _clock::now();
				std::this_thread::sleep_for(_chrono_ns(static_cast<int64_t>(1e6)));
				const auto end = _clock::now();

				const double observed = std::chrono::duration_cast<_chrono_ns>(end - start).count() / 1e6;
				ms -= observed;

				++count;

				// Welford's algorithm for mean and unbiased variance estimation 
				const double delta = observed - mean;
				mean += delta / count;
				m2 += delta * (observed - mean); // intermediate values 'm2' reduce numerical instability
				const double variance = sqrt(m2 / (count - 1));

				estimate = mean + variance; // set estimate 1 standard deviation above the mean
				// can be adjusted to make estimate more or less pessimistic
			}

			utl::sleep::spinlock(ms);
		}

		inline void system(double ms) {
			std::this_thread::sleep_for(_chrono_ns(static_cast<int64_t>(ms * 1e6)));
		}
	}

	

	// ### utl::random:: ###
	// Various convenient random functions, utilizes rand() internally.
	// If good quality random is necessary use std::random generators instead.
	//
	// # ::rand_int(), ::rand_uint(), ::rand_float(), ::rand_double() #
	// Random value in [min, max] range.
	// Floats with no parameters assume range [0, 1].
	//
	// # ::rand_bool() #
	// Randomly chooses 0 or 1.
	//
	// # ::rand_choise() #
	// Randomly chooses a value from initializer list.
	//
	// # ::rand_linear_combination() #
	// Produces "c A + (1-c) B" with random "0 < c < 1" assuming objects "A", "B" support arithmetic operations.
	// Useful for vector and color operations.
	//
	namespace random {

		inline void seed(unsigned int random_seed) { srand(random_seed); }
		inline void seed_with_time() { srand(static_cast<unsigned int>(time(NULL))); }

		inline int rand_int(int min, int max) { return min + rand() % (max - min + 1); }
		inline int rand_uint(unsigned int min, unsigned int max) { return min + static_cast<unsigned int>(rand()) % (max - min + 1u); }

		inline float rand_float() { return rand() / (RAND_MAX + 1.f); }
		inline float rand_float(float min, float max) { return min + (max - min) * rand_float(); }

		inline double rand_double() { return rand() / (RAND_MAX + 1.); }
		inline double rand_double(double min, double max) { return min + (max - min) * rand_double(); }

		inline bool rand_bool() { return static_cast<bool>(rand() % 2); }

		template<class T>
		const T& rand_choise(std::initializer_list<T> objects) {
			return objects.begin()[rand_int(0, objects.size() - 1)];
		}

		template<class T>
		T rand_linear_combination(const T& A, const T& B) { // random linear combination of 2 colors/vectors/etc
			const auto coef = rand_double();
			return A * coef + B * (1. - coef);
		}
	}



	// ### utl::math:: ###
	// Coordinate transformations, mathematical constans and helper functions.
	// 
	// ::abs(), ::sign(), ::sqr(), ::cube(), deg_to_rad(), rad_to_deg()
	// Constexpr templated functions, useful when writing expressions with a "textbook form" math.
	//
	// ::uint_difference()
	// Returns abs(uint - uint) with respect to uint size and possible overflow.
	//
	namespace math {
		// Constants
		constexpr double PI = 3.14159265358979323846;
		constexpr double PI_TWO = 2. * PI;
		constexpr double PI_HALF = 0.5 * PI;
		constexpr double E = 2.71828182845904523536;
		constexpr double GOLDEN_RATIO = 1.6180339887498948482;

		// Convenience functions
		template<typename Type, typename = std::enable_if_t<std::is_scalar<Type>::value>>
		constexpr Type abs(Type x) { return (x > Type(0)) ? x : -x; }

		template<typename Type, typename = std::enable_if_t<std::is_scalar<Type>::value>>
		constexpr Type sign(Type x) { return (x > Type(0)) ? Type(1) : Type(-1); }

		template<typename Type, typename = std::enable_if_t<std::is_arithmetic<Type>::value>>
		constexpr Type sqr(Type x) { return x * x; }

		template<typename Type, typename = std::enable_if_t<std::is_arithmetic<Type>::value>>
		constexpr Type cube(Type x) { return x * x * x; }

		template<typename Type, typename = std::enable_if_t<std::is_floating_point<Type>::value>>
		constexpr Type deg_to_rad(Type degrees) {
			constexpr Type FACTOR = Type(PI / 180.);
			return degrees * FACTOR;
		}

		template<typename Type, typename = std::enable_if_t<std::is_floating_point<Type>::value>>
		constexpr Type rad_to_deg(Type radians) {
			constexpr Type FACTOR = Type(180. / PI);
			return radians * FACTOR;
		}

		template<typename UintType, typename = std::enable_if_t<std::is_integral<UintType>::value>>
		constexpr UintType uint_difference(UintType a, UintType b) {
			// Cast to widest type if there is a change values don't fit into a regular 'int'
			using WiderIntType = std::conditional_t<(sizeof(UintType) >= sizeof(int)), int64_t, int>;

			return static_cast<UintType>(utl::math::abs(static_cast<WiderIntType>(a) - static_cast<WiderIntType>(b)));
		}
	}
}