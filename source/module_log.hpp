// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::log
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_log.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <cstddef>

#include <sstream>
#include <string>
#include <utility>


#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_LOG)
#ifndef UTLHEADERGUARD_LOG
#define UTLHEADERGUARD_LOG

// _______________________ INCLUDES _______________________

#include <array>         // array<>
#include <charconv>      // to_chars()
#include <chrono>        // steady_clock
#include <fstream>       // ofstream
#include <iostream>      // cout
#include <list>          // list<>
#include <mutex>         // lock_guard<>, mutex
#include <ostream>       // ostream
#include <stdexcept>     // std::runtime_error
#include <string_view>   // string_view
#include <system_error>  // errc()
#include <thread>        // this_thread::get_id()
#include <type_traits>   // is_integral_v<>, is_floating_point_v<>, is_same_v<>, is_convertible_to_v<>
#include <unordered_map> // unordered_map<>
#include <vector>        // vector<>

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

namespace utl::log {

// ======================
// --- Internal utils ---
// ======================

// - SFINAE to select localtime_s() or localtime_r() -
template <typename Arg_tm, typename Arg_time_t>
auto _available_localtime_impl(Arg_tm time_moment, Arg_time_t timer)
    -> decltype(localtime_s(std::forward<Arg_tm>(time_moment), std::forward<Arg_time_t>(timer))) {
    return localtime_s(std::forward<Arg_tm>(time_moment), std::forward<Arg_time_t>(timer));
}

template <typename Arg_tm, typename Arg_time_t>
auto _available_localtime_impl(Arg_tm time_moment, Arg_time_t timer)
    -> decltype(localtime_r(std::forward<Arg_time_t>(timer), std::forward<Arg_tm>(time_moment))) {
    return localtime_r(std::forward<Arg_time_t>(timer), std::forward<Arg_tm>(time_moment));
}

std::size_t _get_thread_index(const std::thread::id id) {
    static std::size_t next_index = 0;

    static std::mutex                                       mutex;
    static std::unordered_map<std::thread::id, std::size_t> thread_ids;
    std::lock_guard<std::mutex>                             lock(mutex);

    const auto it = thread_ids.find(id);
    if (it == thread_ids.end()) return thread_ids[id] = next_index++;
    return it->second;
}

template <class IntType, std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
unsigned int _integer_digit_count(IntType value) {
    unsigned int digits = (value <= 0) ? 1 : 0;
    // (value <  0) => we add 1 digit because of '-' in front
    // (value == 0) => we add 1 digit for '0' because the loop doesn't account for zero integers
    while (value) {
        value /= 10;
        ++digits;
    }
    return digits;
    // Note: There is probably a faster way of doing it
}

using clock = std::chrono::steady_clock;

using ms = std::chrono::microseconds;

inline const clock::time_point _program_entry_time_point = clock::now();

// =======================
// --- Stringification ---
// =======================

// --- Internal type traits ---
// ----------------------------

#define utl_log_define_trait(trait_name_, ...)                                                                         \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value;

utl_log_define_trait(_has_string_append, std::string() += std::declval<T>());
utl_log_define_trait(_has_begin, ++std::declval<T>().begin());
utl_log_define_trait(_has_end, ++std::declval<T>().end());
utl_log_define_trait(_has_get, std::get<0>(std::declval<T>()));
utl_log_define_trait(_has_tuple_size, std::tuple_size<T>::value);
utl_log_define_trait(_has_ostream_insert, std::declval<std::ostream>() << std::declval<T>());

#undef utl_log_define_trait

// --- Internal utils ---
// ----------------------

template <class>
inline constexpr bool _always_false_v = false;

template <typename T>
constexpr int _log_10_ceil(T num) {
    return num < 10 ? 1 : 1 + _log_10_ceil(num / 10);
}

template <typename T>
constexpr std::size_t _max_float_digits =
    4 + std::numeric_limits<T>::max_digits10 + std::max(2, _log_10_ceil(std::numeric_limits<T>::max_exponent10));
// should be the smallest buffer size to account for all possible 'std::to_chars()' outputs,
// see [https://stackoverflow.com/questions/68472720/stdto-chars-minimal-floating-point-buffer-size]

// --- Stringifiers ---
// --------------------

template <class T>
void append_stringified(std::string& str, const T& value);

void _append_stringified_bool(std::string& str, bool value) { str += value ? "true" : "false"; }

// Fast implementation for stringifying an integer and appending it to 'std::string'
template <class T>
void _append_stringified_integer(std::string& str, T value) {

    // Note:
    // We could count the digits of 'value', preallocate buffer for exactly however many characters
    // we need and format directly to it, however benchmarks showed that it is actually inferior to
    // just doing things the usual way with a stack-allocated middle-man buffer.
    std::array<char, std::numeric_limits<T>::digits10> buffer;
    const auto [number_end_ptr, error_code] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);

    if (error_code != std::errc())
        throw std::runtime_error(
            "Integer stringification encountered std::to_chars() formatting error while serializing a value.");

    str.append(buffer.data(), number_end_ptr - buffer.data());
}

// Fast implementation for stringifying a float and appending it to 'std::string'
template <class T>
void _append_stringified_float(std::string& str, T value) {
    std::array<char, _max_float_digits<T>> buffer;
    const auto [number_end_ptr, error_code] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);

    if (error_code != std::errc())
        throw std::runtime_error(
            "Float stringification encountered std::to_chars() formatting error while serializing a value.");

    str.append(buffer.data(), number_end_ptr - buffer.data());
}

template <class T>
void _append_stringified_stringlike(std::string& str, const T& value) {
    str += value;
    // 'std::string' has operator '+=' for following types:
    // 1) 'const std::string&'
    // 2) char-like (includes all integral types)
    // 3) 'std::string_view'-convertible
}

template <class T>
void _append_stringified_string_convertible(std::string& str, const T& value) {
    str += std::string(value);
}

template <class T>
void _append_stringified_array(std::string& str, const T& value) {
    str += "{ ";
    if (value.begin() != value.end())
        for (auto it = value.begin();;) {
            append_stringified(str, *it);
            if (++it == value.end()) break; // prevents trailing comma
            str += ", ";
        }
    str += " }";
}

template <class Tuplelike, std::size_t... Idx>
void _append_stringified_tuple_impl(std::string& str, Tuplelike value, std::index_sequence<Idx...>) {
    ((Idx == 0 ? "" : str += ", ", append_stringified(str, std::get<Idx>(value))), ...);
    // fold expression '( f(args), ... )' invokes 'f(args)' for all arguments in 'args...'
    // in the same fashion, we can fold over 2 functions by doing '( ( f(args), g(args) ), ... )'
}

template <template <class...> class Tuplelike, class... Args>
void _append_stringified_tuple(std::string& str, const Tuplelike<Args...>& value) {
    str += "< ";
    _append_stringified_tuple_impl(str, value, std::index_sequence_for<Args...>{});
    str += " >";
}

template <class T>
void _append_stringified_printable(std::string& str, const T& value) {
    str += (std::ostringstream() << value).str(); // rather slow but at this point there is no other option
}

// --- Public API ---
// ------------------

template <class T>
void append_stringified(std::string& str, const T& value) {
    // Bool
    if constexpr (std::is_same_v<T, bool>) _append_stringified_bool(str, value);
    // Char
    else if constexpr (std::is_same_v<T, char>) _append_stringified_stringlike(str, value);
    // Integral
    else if constexpr (std::is_integral_v<T>) _append_stringified_integer(str, value);
    // Floating-point
    else if constexpr (std::is_floating_point_v<T>) _append_stringified_float(str, value);
    // 'std::string_view'-convertible (most strings and string-like types)
    else if constexpr (std::is_convertible_v<T, std::string_view>) _append_stringified_stringlike(str, value);
    // 'std::string'-convertible (some "nastier" string-like types, mainly 'std::path')
    else if constexpr (std::is_convertible_v<T, std::string>) _append_stringified_string_convertible(str, value);
    // Array-like
    else if constexpr (_has_begin_v<T> && _has_end_v<T>) _append_stringified_array(str, value);
    // Tuple-like
    else if constexpr (_has_get_v<T> && _has_tuple_size_v<T>) _append_stringified_tuple(str, value);
    // 'std::ostream' printable
    else if constexpr (_has_ostream_insert_v<T>) _append_stringified_printable(str, value);
    // No valid stringification exists
    else static_assert(_always_false_v<T>, "No valid stringification exists for the type.");

    // Note: Using if-constexpr chain here allows us to pick and choose priority of different branches,
    // removing any possible ambiguity we could encounter doing things through SFINAE or overloads.
}

template <class... Args>
std::string stringify(Args&&... args) {
    std::string buffer;
    (append_stringified(buffer, std::forward<Args>(args)), ...);
    return buffer;
}

template <class... Args>
void print(Args&&... args) {
    std::cout << stringify(std::forward<Args>(args)...);
}

template <class... Args>
void println(Args&&... args) {
    std::cout << stringify(std::forward<Args>(args)...) << '\n';
}

// ===============
// --- Options ---
// ===============

enum class Verbosity { ERR = 1, WARN = 2, INFO = 3, TRACE = 4 };

enum class OpenMode { REWRITE, APPEND };

enum class Colors { ENABLE, DISABLE };

struct Columns {
    bool datetime;
    bool uptime;
    bool thread;
    bool callsite;
    bool level;
    bool message;

    Columns() : datetime(true), uptime(true), thread(true), callsite(true), level(true), message(true) {}
};

struct Callsite {
    std::string_view file;
    int              line;
};

struct MessageMetadata {
    Verbosity verbosity;
};

constexpr bool operator<(Verbosity l, Verbosity r) { return static_cast<int>(l) < static_cast<int>(r); }
constexpr bool operator<=(Verbosity l, Verbosity r) { return static_cast<int>(l) <= static_cast<int>(r); }

// ==================
// --- Sink class ---
// ==================

class Sink {
private:
    std::ostream&     os;
    Verbosity         verbosity;
    Colors            colors;
    clock::duration   flush_interval;
    Columns           columns;
    clock::time_point last_flushed;

public:
    Sink()            = delete;
    Sink(const Sink&) = delete;
    Sink(Sink&&)      = default;

    Sink(std::ostream& os, Verbosity verbosity, Colors colors, clock::duration flush_interval, const Columns& columns)
        : os(os), verbosity(verbosity), colors(colors), flush_interval(flush_interval), columns(columns) {}

    template <typename... Args>
    void format(const Callsite& callsite, const MessageMetadata& meta, const Args&... args) {
        if (meta.verbosity > this->verbosity) return;

        thread_local std::string buffer;

        const clock::time_point now = clock::now();

        // To minimize logging overhead we use string buffer, append characters to it and then write the whole buffer
        // to `std::ostream`. This avoids the inherent overhead of ostream formatting (caused largely by
        // virtualization, syncronization and locale handling, neither of which are relevant for the logger).
        //
        // This buffer gets reused between calls. Note the 'thread_local', if buffer was just a class member, multiple
        // threads could fight trying to clear and resize the buffer while it's being written to by another thread.
        //
        // Buffer may grow when formatting a message longer that any one that was formatted before, otherwise we just
        // reuse the reserved memory and no new allocations take place.

        buffer.clear();

        // Format columns one-by-one
        if (this->colors == Colors::ENABLE) switch (meta.verbosity) {
            case Verbosity::ERR: buffer += "\033[31;1m"; break;
            case Verbosity::WARN: buffer += "\033[33m"; break;
            case Verbosity::INFO: buffer += "\033[37m"; break;
            case Verbosity::TRACE: buffer += "\033[90m"; break;
            }

        if (this->columns.datetime) this->format_column_datetime(buffer);
        if (this->columns.uptime) this->format_column_uptime(buffer, now);
        if (this->columns.thread) this->format_column_thread(buffer);
        if (this->columns.callsite) this->format_column_callsite(buffer, callsite);
        if (this->columns.level) this->format_column_level(buffer, meta.verbosity);
        if (this->columns.message) this->format_column_message(buffer, args...);

        if (this->colors == Colors::ENABLE) { buffer += "\033[0m"; }

        buffer += '\n';

        this->os.write(buffer.data(), buffer.size());

        // flush every message immediately
        if (this->flush_interval.count() == 0) {
            this->os.flush();
            return;
        }
        // or flush periodically
        if (now - this->last_flushed > this->flush_interval) {
            this->last_flushed = now;
            this->os.flush();
        }
    }

    void format_column_datetime(std::string& buffer) {
        std::time_t timer = std::time(nullptr);
        std::tm     time_moment{};

        _available_localtime_impl(&time_moment, &timer);

        constexpr std::size_t datetime_width = sizeof("yyyy-mm-dd HH:MM:SS");
        // size includes the null terminator added by 'strftime()'

        // Format time straight into the buffer
        std::array<char, datetime_width> strftime_buffer;
        std::strftime(strftime_buffer.data(), strftime_buffer.size(), "%Y-%m-%d %H:%M:%S", &time_moment);

        strftime_buffer.back() = ' '; // replace null-terminator added by 'strftime()' with a space
        buffer.append(strftime_buffer.data(), strftime_buffer.size());
    }

    void format_column_uptime(std::string& buffer, clock::time_point now) {
        const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - _program_entry_time_point);
        const auto sec        = (elapsed_ms / 1000).count();
        const auto ms         = (elapsed_ms % 1000).count(); // is 'elapsed_ms - 1000 * full_seconds; faster?

        const unsigned int sec_digits = _integer_digit_count(sec);
        const unsigned int ms_digits  = _integer_digit_count(ms);

        constexpr unsigned int sec_width = 4;
        constexpr unsigned int ms_width  = 3;

        buffer += '(';

        // Emulate '<< std::right << std::setw(sec_width) << sec'
        if (sec_digits < sec_width) buffer.append(sec_width - sec_digits, ' ');
        append_stringified(buffer, sec);

        buffer += '.';

        // Emulate '<< std::right << std::setw(ms_width) << std::setfill('0') << ms'
        if (ms_digits < ms_width) buffer.append(ms_width - ms_digits, '0');
        append_stringified(buffer, ms);

        buffer += ')';
    }

    void format_column_thread(std::string& buffer) {
        buffer += '[';
        constexpr std::size_t thread_id_width = sizeof("thread") - 1;
        const auto            thread_id       = _get_thread_index(std::this_thread::get_id());
        if (_integer_digit_count(thread_id) < thread_id_width) buffer.append(thread_id_width - thread_id, ' ');
        append_stringified(buffer, thread_id);
        buffer += ']';
    }

    void format_column_callsite(std::string& buffer, const Callsite& callsite) {
        constexpr std::streamsize width_before_dot = 28;
        constexpr std::streamsize width_after_dot  = 4;

        // Get just filename from the full path
        std::string_view filename = callsite.file.substr(callsite.file.find_last_of("/\\") + 1);

        // Emulate '<< std::right << std::setw(width_before_dot) << filename'
        // trim first characters if it's too long
        if (filename.size() < width_before_dot) buffer.append(width_before_dot - filename.size(), ' ');
        else filename.remove_prefix(width_before_dot - filename.size());

        buffer += filename;
        buffer += ':';
        // Emulate '<< std::left << std::setw(width_after_dot) << line'
        append_stringified(buffer, callsite.line);
        buffer.append(width_after_dot - _integer_digit_count(callsite.line), ' ');
    }

    void format_column_level(std::string& buffer, Verbosity level) {
        switch (level) {
        case Verbosity::ERR: buffer += "   ERR|"; return;
        case Verbosity::WARN: buffer += "  WARN|"; return;
        case Verbosity::INFO: buffer += "  INFO|"; return;
        case Verbosity::TRACE: buffer += " TRACE|"; return;
        }
    }

    template <typename... Args>
    void format_column_message(std::string& buffer, const Args&... args) {
        buffer += ' ';
        (append_stringified(buffer, args), ...);
        //(buffer += ... += args); // parenthesis here are necessary
    }
};

// ====================
// --- Logger class ---
// ====================

class Logger {
private:
    inline static std::vector<Sink>        sinks;
    inline static std::list<std::ofstream> managed_files;
    // we don't want 'managed_files' to reallocate its elements at any point
    // since that would leave corresponding sinks with dangling references,
    // which is why list is used

public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    template <typename... Args>
    void push_message(const Callsite& callsite, const MessageMetadata& meta, const Args&... args) {
        // When no sinks were manually created, default sink-to-terminal takes over
        if (this->sinks.empty()) {
            static Sink default_sink(std::cout, Verbosity::TRACE, Colors::ENABLE, ms(0), Columns{});
            default_sink.format(callsite, meta, args...);
        }

        for (auto& sink : this->sinks) sink.format(callsite, meta, args...);
    }

    Sink& emplace_sink(std::ostream& os, Verbosity verbosity, Colors colors, clock::duration flush_interval,
                       const Columns& columns) {
        return this->sinks.emplace_back(os, verbosity, colors, flush_interval, columns);
    }

    std::ofstream& emplace_managed_file(const std::string& filename, OpenMode open_mode) {
        const auto ios_open_mode = (open_mode == OpenMode::APPEND) ? std::ios::out | std::ios::app : std::ios::out;
        return this->managed_files.emplace_back(filename, ios_open_mode);
    }
};

// =======================
// --- Sink public API ---
// =======================

Sink& add_terminal_sink(std::ostream& os, Verbosity verbosity = Verbosity::INFO, Colors colors = Colors::ENABLE,
                        clock::duration flush_interval = ms{}, const Columns& columns = Columns{}) {
    return Logger::instance().emplace_sink(os, verbosity, colors, flush_interval, columns);
}

Sink& add_file_sink(const std::string& filename, OpenMode open_mode = OpenMode::REWRITE,
                    Verbosity verbosity = Verbosity::TRACE, Colors colors = Colors::DISABLE,
                    clock::duration flush_interval = ms{15}, const Columns& columns = Columns{}) {
    auto& os = Logger::instance().emplace_managed_file(filename, open_mode);
    return Logger::instance().emplace_sink(os, verbosity, colors, flush_interval, columns);
}

// ======================
// --- Logging macros ---
// ======================

#define UTL_LOG_ERR(...)                                                                                               \
    utl::log::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::Verbosity::ERR}, __VA_ARGS__)

#define UTL_LOG_WARN(...)                                                                                              \
    utl::log::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::Verbosity::WARN}, __VA_ARGS__)

#define UTL_LOG_INFO(...)                                                                                              \
    utl::log::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::Verbosity::INFO}, __VA_ARGS__)

#define UTL_LOG_TRACE(...)                                                                                             \
    utl::log::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::Verbosity::TRACE}, __VA_ARGS__)

#ifdef _DEBUG
#define UTL_LOG_DERR(...) UTL_LOG_ERR(__VA_ARGS__)
#define UTL_LOG_DWARN(...) UTL_LOG_WARN(__VA_ARGS__)
#define UTL_LOG_DINFO(...) UTL_LOG_INFO(__VA_ARGS__)
#define UTL_LOG_DTRACE(...) UTL_LOG_TRACE(__VA_ARGS__)
#else
#define UTL_LOG_DERR(...)
#define UTL_LOG_DWARN(...)
#define UTL_LOG_DINFO(...)
#define UTL_LOG_DTRACE(...)
#endif


} // namespace utl::log

#endif
#endif // module utl::log
