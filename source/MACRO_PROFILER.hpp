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

#include <algorithm>   // sort()
#include <chrono>      // chrono::steady_clock, chrono::duration_cast<>, std::chrono::milliseconds
#include <cstdlib>     // atexit()
#include <fstream>     // ofstream
#include <iomanip>     // setprecision(), setw()
#include <ios>         // streamsize, fixed,
#include <iostream>    // cout
#include <ostream>     // ostream
#include <sstream>     // ostringstream
#include <string>      // string
#include <string_view> // string_view
#include <vector>      // vector<>

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

namespace utl::profiler {

// ==========================
// --- Profiler Internals ---
// ==========================

inline std::string _format_call_site(std::string_view file, int line, std::string_view func) {
    const std::string_view filename = file.substr(file.find_last_of("/\\") + 1);

    return (std::ostringstream() << filename << ":" << line << ", " << func << "()").str();
}

using _clock      = std::chrono::steady_clock;
using _duration   = _clock::duration;
using _time_point = _clock::time_point;

inline const _time_point _program_entry_time_point = _clock::now();

struct _record {
    const char* file;
    int         line;
    const char* func;
    const char* label;
    _duration   accumulated_time;
};

inline void _utl_profiler_atexit(); // predeclaration, implementation has circular dependency with 'RecordManager'

// =========================
// --- Profiler Classess ---
// =========================

class RecordManager {
private:
    _record data;

public:
    inline static std::vector<_record> records;
    bool                               busy;

    void add_time(_duration time) noexcept { this->data.accumulated_time += time; }

    RecordManager() = delete;

    RecordManager(const char* file, int line, const char* func, const char* label)
        : data({file, line, func, label, _duration(0)}), busy(false) {
        // 'file', 'func', 'label' are guaranteed to be string literals, since we want to
        // have as little overhead as possible during runtime, we can just save raw pointers
        // and convert them to nicer types like 'std::string_view' later in the formatting stage

        // Profiler ever gets called => register result output at std::exit()
        static bool first_call = true;
        if (first_call) {
            std::atexit(_utl_profiler_atexit);
            first_call = false;
        }
    }

    ~RecordManager() { records.emplace_back(this->data); }
};

class ScopeTimer {
private:
    _time_point    start;
    RecordManager* record_manager;
    // we could use 'std::optional<std::reference_wrapper<RecordManager>>',
    // but that would inctroduce more dependencies for no real reason

public:
    constexpr operator bool() const noexcept { return true; }
    // we needed so we can use 'if constexpr (const auto x = ScopeTimer())' construct

    ScopeTimer(RecordManager* manager) {
        // Busy manager => we're in recursion, don't count time
        if (manager->busy) {
            this->record_manager = nullptr;
            return;
        }
        this->record_manager       = manager;
        this->record_manager->busy = true;
        this->start                = _clock::now();
    }

    ~ScopeTimer() {
        if (!this->record_manager) return;
        this->record_manager->add_time(_clock::now() - this->start);
        this->record_manager->busy = false;
    }
};

// ==================================
// --- Profiler Exit & Formatting ---
// ==================================

inline void _utl_profiler_atexit() {

    const auto total_runtime = _clock::now() - _program_entry_time_point;

    std::ostream* ostr = &std::cout;

    // Convenience functions
    const auto duration_to_sec = [](_duration duration) -> double {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() / 1e9;
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

    // Sort records by their accumulated time
    std::sort(RecordManager::records.begin(), RecordManager::records.end(),
              [](const _record& l, const _record& r) { return l.accumulated_time < r.accumulated_time; });

    // Collect max length of each column (for proper formatting)
    constexpr std::string_view column_name_call_site  = "Call Site";
    constexpr std::string_view column_name_label      = "Label";
    constexpr std::string_view column_name_duration   = "Time";
    constexpr std::string_view column_name_percentage = "Time %";

    std::streamsize max_length_call_site  = column_name_call_site.size();
    std::streamsize max_length_label      = column_name_label.size();
    std::streamsize max_length_duration   = column_name_duration.size();
    std::streamsize max_length_percentage = column_name_percentage.size();

    for (const auto& record : RecordManager::records) {
        const std::string call_site    = _format_call_site(record.file, record.line, record.func);
        const std::string label        = record.label;
        const double      duration_sec = duration_to_sec(record.accumulated_time);

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
    for (const auto& record : RecordManager::records) {
        const std::string call_site    = _format_call_site(record.file, record.line, record.func);
        const std::string label        = record.label;
        const double      duration_sec = duration_to_sec(record.accumulated_time);
        const double      percentage   = duration_percentage(duration_sec);

        // Join floats with their postfixes into a single string so they are properly handled by std::setw()
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
#define _utl_profiler_add_uuid(varname_) _utl_profiler_concat_tokens_wrapper(varname_, __LINE__)
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

#define UTL_PROFILER(label_)                                                                                           \
    constexpr bool _utl_profiler_add_uuid(utl_profiler_macro_guard_) = true;                                           \
                                                                                                                       \
    static_assert(_utl_profiler_add_uuid(utl_profiler_macro_guard_), "UTL_PROFILE is a multi-line macro.");            \
                                                                                                                       \
    static utl::profiler::RecordManager _utl_profiler_add_uuid(utl_profiler_record_manager_)(__FILE__, __LINE__,       \
                                                                                             __func__, label_);        \
                                                                                                                       \
    if constexpr (const utl::profiler::ScopeTimer _utl_profiler_add_uuid(utl_profiler_scope_timer_){                   \
                      &_utl_profiler_add_uuid(utl_profiler_record_manager_)})
// Note 1:
//
//    constexpr bool ... = true;
//    static_assert(..., "UTL_PROFILE is a multi-line macro.");
//
// is reponsible for preventing accidental errors caused by using macro like this:
//
//    for (...) UTL_PROFILER("") function(); // will only loop the first line of the multi-line macro
//
// If someone tries to write it like this, the constexpr bool variable will be "pulled" into a narrower scope,
// causing 'static_assert()' to fail due to using undeclared identifier. Since the line with undeclared identifier
// gets expanded, user will be able to see the assert message.
//
// Note 2:
//
// By separating "record management" into a static variable and "actual timing" into a non-static one,
// we can avoid additional overhead from having to locate the record, corresponding to the profiled source location.
// (an operation that require a non-trivial vector/map lookup with string comparisons)
//
// Static variable initializes its record once and timer does the bare minimum of work - 2 calls to 'now()' to get
// timing, one addition to accumulated time and a check for recursion (so it can skip time appropriately).
//
//  Note 3:
//
// _utl_profiler_add_uuid(...) ensures no identifier collisions when several profilers exist in a single scope.
// Since in this context 'uuid' is a line number, the only case in which ids can collide is when multiple profilers
// are declated on the same line, which I assume no sane person would do. And even if they would, that would simply
// lead to a compiler error. Can't really do better than that without resorting to non-standard macros like
// '__COUNTER__' for 'uuid' creation

} // namespace utl::profiler

#endif
#endif // macro-module UTL_PROFILER
