// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Macro-Module:  UTL_PROFILER
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACRO_PROFILER.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMACRO_PROFILER)
#ifndef UTLHEADERGUARD_PROFILER
#define UTLHEADERGUARD_PROFILER

// _______________________ INCLUDES _______________________

#include <chrono>      // chrono::steady_clock, chrono::duration_cast<>, std::chrono::milliseconds
#include <cstdlib>     // atexit()
#include <fstream>     // ofstream
#include <iomanip>     // setprecision(), setw()
#include <ios>         // streamsize, fixed,
#include <iostream>    // cout
#include <map>         // map<>
#include <ostream>     // ostream
#include <sstream>     // ostringstream
#include <string>      // string
#include <string_view> // string_view

// ____________________ DEVELOPER DOCS ____________________

// Macros for quick code profiling.
// Trivially simple, yet effective way of finding bottlenecks without any tooling.
//
// Can also be used to benchmark stuff "quick & dirty" due to doing all the time measuring
// and table formatting one would usually implement in their benchmarks. A proper bechmark
// suite of course would include support for automatic reruns and gather statistical data,
// but in prototying this is often not necessary.
//
// # UTL_PROFILER_REROUTE_TO_FILE(filepath) #
// Reroutes profiler summary from std::cout to a file with a given name.
//
// # UTL_PROFILER, UTL_PROFILER_LABELED() #
// Profiles the following expression/scope. If profiled scope was entered at any point of the program,
// upon exiting 'main()' the table with profiling results will be printed. Profiling results include:
// - Total program runtime
// - Total runtime of each profiled scope
// - % of total runtime taken by each profiled scope
// - Profiler labels (if using 'UTL_PROFILE_LABELED()', otherwise the label is set to "<NONE>")
// - Profiler locations: file, function, line

// ____________________ IMPLEMENTATION ____________________

// ==========================
// --- Profiler Internals ---
// ==========================

inline std::string _utl_profiler_reroute_to_filepath = "";

#define UTL_PROFILER_REROUTE_TO_FILE(filepath_) _utl_profiler_reroute_to_filepath = filepath_;

using _utl_profiler_clock         = std::chrono::steady_clock;
using _utl_profiler_time_duration = _utl_profiler_clock::duration;
using _utl_profiler_time_point    = _utl_profiler_clock::time_point;

inline const _utl_profiler_time_point _utl_profiler_program_init_time_point = _utl_profiler_clock::now();
// automatically gets program launch time so we can compute total runtime later

inline std::string _utl_profiler_format_call_site(std::string_view file, int line, std::string_view func) {
    const std::string_view filename = file.substr(file.find_last_of("/\\") + 1);

    std::ostringstream ss;
    ss << filename << ":" << line << ", " << func << "()";
    return ss.str();
}

struct _utl_profiler_record {
    std::string                 label;
    _utl_profiler_time_duration duration;
};

inline void _utl_profiler_atexit(); // predeclare, it needs '_utl_profiler' but used by '_utl_profiler'

class _utl_profiler {
private:
    std::string              call_site;
    std::string              label;
    _utl_profiler_time_point construction_time_point;

public:
    inline static std::map<std::string, _utl_profiler_record>
        records; // std::map because we do WANT sorting by call-site name

    operator bool() const { return true; } // needed so we can use 'if (auto x = _utl_profiler())' construct

public:
    _utl_profiler(std::string_view file, int line, std::string_view func, std::string_view label) {
        this->call_site               = _utl_profiler_format_call_site(file, line, func);
        this->label                   = label;
        this->construction_time_point = _utl_profiler_clock::now();

        // If profiler ever gets called => registed results output at std::exit()
        static bool first_call = true;
        if (first_call) {
            std::atexit(_utl_profiler_atexit);
            first_call = false;
        }
    }

    ~_utl_profiler() {
        const auto it                = _utl_profiler::records.find(this->call_site);
        const auto profiled_duration = _utl_profiler_clock::now() - this->construction_time_point;

        // Record with the same callsite exists => accumulate duration
        if (it != _utl_profiler::records.end()) {
            (*it).second.duration += profiled_duration;
        }
        // Otherwise => add new record with duration
        else {
            _utl_profiler::records.insert({
                std::string(this->call_site), _utl_profiler_record{this->label, profiled_duration}
            });
        }
    }
};

// =================================
// --- Profiler Table Formatting ---
// =================================

inline void _utl_profiler_atexit() {
    namespace chr = std::chrono;

    const auto total_runtime = _utl_profiler_clock::now() - _utl_profiler_program_init_time_point;
    // const auto total_runtime = std::chrono::duration_cast<std::chrono::milliseconds>();

    // Choose whether to print or reroute output to file
    std::ostream* ostr = &std::cout;
    std::ofstream output_file;

    if (!_utl_profiler_reroute_to_filepath.empty()) {
        output_file.open(_utl_profiler_reroute_to_filepath);
        ostr = &output_file;
    }

    // Convenience functions
    const auto duration_to_sec = [](_utl_profiler_time_duration duration) -> double {
        return chr::duration_cast<chr::nanoseconds>(duration).count() / 1e9;
    };

    const auto duration_percentage = [&](double duration_sec) -> double {
        const double total_runtime_sec = duration_to_sec(total_runtime);
        return (duration_sec / total_runtime_sec) * 100.;
    };

    const auto float_printed_size = [](double value, std::streamsize precision, decltype(std::fixed) format,
                                       std::string_view postfix) -> std::streamsize {
        std::ostringstream ss;
        ss << std::setprecision(precision) << format << value << postfix;
        return ss.str().size(); // can be done faster but we don't really care here
    };

    const auto repeat_hline_symbol = [](std::streamsize repeats) -> std::string {
        return std::string(static_cast<size_t>(repeats), '-');
    };

    constexpr std::streamsize  duration_precision = 2;
    constexpr auto             duration_format    = std::fixed;
    constexpr std::string_view duration_postfix   = " s";

    constexpr std::streamsize  percentage_precision = 1;
    constexpr auto             percentage_format    = std::fixed;
    constexpr std::string_view percentage_postfix   = "%";

    // Collect max length of each column (for proper formatting)
    constexpr std::string_view column_name_call_site  = "Call Site";
    constexpr std::string_view column_name_label      = "Label";
    constexpr std::string_view column_name_duration   = "Time";
    constexpr std::string_view column_name_percentage = "Time %";

    std::streamsize max_length_call_site  = column_name_call_site.size();
    std::streamsize max_length_label      = column_name_label.size();
    std::streamsize max_length_duration   = column_name_duration.size();
    std::streamsize max_length_percentage = column_name_percentage.size();

    for (const auto& record : _utl_profiler::records) {
        const auto&  call_site    = record.first;
        const auto&  label        = record.second.label;
        const double duration_sec = duration_to_sec(record.second.duration);

        // 'Call Site' column
        const std::streamsize length_call_site = call_site.size();
        if (max_length_call_site < length_call_site) max_length_call_site = length_call_site;

        // 'Label' column
        const std::streamsize length_label = label.size();
        if (max_length_label < length_label) max_length_label = length_label;

        // 'Time' column
        const std::streamsize length_duration =
            float_printed_size(duration_sec, duration_precision, duration_format, duration_postfix);
        if (max_length_duration < length_duration) max_length_duration = length_duration;

        // 'Time %' column
        const auto            percentage = duration_percentage(duration_sec);
        const std::streamsize length_percentage =
            float_printed_size(percentage, percentage_precision, percentage_format, percentage_postfix);
        if (max_length_percentage < length_percentage) max_length_percentage = length_percentage;
    }

    // Print formatted profiler header
    constexpr std::string_view HEADER_TEXT = " UTL PROFILING RESULTS ";

    const std::streamsize total_table_length = sizeof("| ") - 1 + max_length_call_site + sizeof(" | ") - 1 +
                                               max_length_label + sizeof(" | ") - 1 + max_length_duration +
                                               sizeof(" | ") - 1 + max_length_percentage + sizeof(" |") -
                                               1; // -1 because sizeof(char[]) accounts for invisible '\0' at the end

    const std::streamsize header_text_length = HEADER_TEXT.size();
    const std::streamsize header_left_pad    = (total_table_length - header_text_length) / 2;
    const std::streamsize header_right_pad   = total_table_length - header_text_length - header_left_pad;

    *ostr << "\n"
          << repeat_hline_symbol(header_left_pad + 1) << HEADER_TEXT << repeat_hline_symbol(header_right_pad + 1)
          << '\n'
          // + 1 makes header hline extend 1 character past the table on both sides
          << "\n"
          << " Total runtime -> " << std::setprecision(duration_precision) << duration_format
          << duration_to_sec(total_runtime) << " sec\n"
          << "\n";

    // Print formatted table header
    *ostr << " | " << std::setw(max_length_call_site) << column_name_call_site << " | " << std::setw(max_length_label)
          << column_name_label << " | " << std::setw(max_length_duration) << column_name_duration << " | "
          << std::setw(max_length_percentage) << column_name_percentage << " |\n";

    *ostr << " |"
          << repeat_hline_symbol(max_length_call_site + 2) // add 2 to account for delimers not having spaces in hline
          << "|" << repeat_hline_symbol(max_length_label + 2) << "|" << repeat_hline_symbol(max_length_duration + 2)
          << "|" << repeat_hline_symbol(max_length_percentage + 2) << "|\n";

    *ostr << std::setfill(' '); // reset the fill so we don't mess with table contents


    // Print formatted table contents
    for (const auto& record : _utl_profiler::records) {
        const auto&  call_site    = record.first;
        const auto&  label        = record.second.label;
        const double duration_sec = duration_to_sec(record.second.duration);
        const double percentage   = duration_percentage(duration_sec);

        // Joint floats with their postfixes into a single string so they are properly handled by std::setw()
        // (which only affects the first value leading to a table misaligned by postfix size)
        std::ostringstream ss_duration;
        ss_duration << std::setprecision(duration_precision) << duration_format << duration_sec << duration_postfix;

        std::ostringstream ss_percentage;
        ss_percentage << std::setprecision(percentage_precision) << percentage_format << percentage
                      << percentage_postfix;

        *ostr << " | " << std::setw(max_length_call_site) << call_site << " | " << std::setw(max_length_label) << label
              << " | " << std::setw(max_length_duration) << ss_duration.str() << " | "
              << std::setw(max_length_percentage) << ss_percentage.str() << " |\n";
    }
}

// ========================
// --- Profiler Codegen ---
// ========================

#define _utl_profiler_concat_tokens(a, b) a##b
#define _utl_profiler_concat_tokens_wrapper(a, b) _utl_profiler_concat_tokens(a, b)
#define _utl_profiler_add_line_number_to_variable_name(varname_) _utl_profiler_concat_tokens_wrapper(varname_, __LINE__)
// This macro creates token 'varname_##__LINE__' from 'varname_'.
//
// The reason we can't just write it as is, is that function-macros only expands their macro-arguments
// if neither the stringizing operator # nor the token-pasting operator ## are applied to the arguments
// inside the macro body.
//
// Which means in a simple 'varname_##__LINE__' macro '__LINE__' doesn't expand to it's value.
//
// We can get around this fact by introducing indirection,
// '__LINE__' gets expanded in '_utl_profiler_concat_tokens_wrapper()'
// and then tokenized and concatenated in '_utl_profiler_concat_tokens()'

#define UTL_PROFILER_LABELED(label_)                                                                                   \
    if (auto _utl_profiler_add_line_number_to_variable_name(profiler_) =                                               \
            _utl_profiler(__FILE__, __LINE__, __func__, label_))
// We add line number to profiler variable name to prevent nested profiler scopes from shadowing
// each other's 'profiler_' variable. While such shadowing has no effect on an actual behavior,
// it does cause a warning from most compilers.
//
// Such solution isn't perfect (2 scopes can be nested on the same line in deffirent files), but it
// is good enough and there is no way to get a better 'unique varname' without resorting to
// specific compiler extensions.

#define UTL_PROFILER UTL_PROFILER_LABELED("<NONE>")

#endif
#endif // macro-module UTL_PROFILER
