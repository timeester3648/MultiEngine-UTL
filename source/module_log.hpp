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
#include <limits>
#include <stdexcept>
#include <string>
#include <system_error>
#include <unordered_map> // unordered_map<>
#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_LOG)
#ifndef UTLHEADERGUARD_LOG
#define UTLHEADERGUARD_LOG

// _______________________ INCLUDES _______________________

#include <array>    // array<>
#include <charconv> // to_chars()
#include <chrono>
#include <iomanip>     // setw()
#include <ios>         // left, right
#include <mutex>       // lock_guard<>, mutex
#include <ostream>     // ostream
#include <string_view> // string_view
#include <thread>      // this_thread::get_id()
#include <vector>      // vector<>


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
    static std::size_t next_index = 1;
    // we start thread indexation at 1 because otherwise <charconv> formats thread '0' to ''
    // (since it doesn't output leading zeroes), also this looks a bit nicer (subjective)

    static std::mutex                                       mutex;
    static std::unordered_map<std::thread::id, std::size_t> thread_ids;
    std::lock_guard<std::mutex>                             lock(mutex);

    auto it = thread_ids.find(id);
    if (it == thread_ids.end()) return thread_ids[id] = next_index++;
    return it->second;
}

template <class IntType, std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
unsigned int _integer_digit_count(IntType value) {
    unsigned int digits = (value < 0) ? 1 : 0;
    while (value) {
        value /= 10;
        ++digits;
    }
    return digits;
    // NOTE: There is a faster way of doing it, see http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
}

using _clock = std::chrono::steady_clock;

inline const _clock::time_point _program_entry_time_point = _clock::now();

// ===============================
// --- String formatting utils ---
// ===============================

// Grows string by 'size_increase' and returns pointer to the old ending
// in a possibly reallocated string. This function is used in multiple places
// to expand string buffer before formatting a known amount of characters into it.
inline char* _grow_string(std::string& buffer, std::size_t size_increase) {
    const std::size_t old_buffer_size = buffer.size();
    buffer.resize(old_buffer_size + size_increase);
    return buffer.data() + old_buffer_size;
}

// Fast implementation for stringifying an integer and appending it to 'std::string'
template <class IntType, std::enable_if_t<std::is_integral<IntType>::value, bool> = true>
void stringify_integer(std::string& buffer, IntType value) {
    // Note:
    // We could count the digits of 'value', preallocate buffer for exactly however many characters
    // we need and format directly to it, however benchmarks showed that it is actually inferior to
    // just doing things the usual way with a stack-allocated middle-man buffer.

    thread_local std::array<char, std::numeric_limits<IntType>::digits10> num_buffer;
    const auto result = std::to_chars(num_buffer.data(), num_buffer.data() + num_buffer.size(), value);

    if (result.ec != std::errc())
        throw std::runtime_error(
            "stringify_integer() encountered std::to_chars() formatting error while serializing a value.");

    buffer.append(num_buffer.data(), result.ptr - num_buffer.data());
}

// ===============
// --- Options ---
// ===============

enum class Verbosity {
    ERR  = 3,
    WARN = 4,
    INFO = 6
}; // levels according to https://en.wikipedia.org/wiki/Syslog specification, might add others later

enum class OpenMode { REWRITE, APPEND };

enum class Colors { ENABLE, DISABLE };

struct Columns {
    bool datetime;
    bool uptime;
    bool thread;
    bool callsite;
    bool level;
    bool message;
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
    std::ostream&      os;
    Verbosity          verbosity;
    Colors             colors;
    Columns            columns;
    _clock::duration   flush_interval;
    _clock::time_point last_flushed;

public:
    Sink() = delete;
    Sink(std::ostream& os, Verbosity verbosity, Colors colors, const Columns& columns, _clock::duration flush_interval)
        : os(os), verbosity(verbosity), colors(colors), columns(columns), flush_interval(flush_interval) {}

    template <typename... Args>
    void format(const Callsite& callsite, const MessageMetadata& meta, const Args&... args) {
        if (meta.verbosity > this->verbosity) return;

        thread_local std::string buffer;

        const _clock::time_point now = _clock::now();

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
            case Verbosity::INFO: buffer += "\033[90m"; break;
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

        // Format time straight into the buffer
        const auto append_point = _grow_string(buffer, datetime_width);

        std::strftime(append_point, datetime_width, "%Y-%m-%d %H:%M:%S", &time_moment);
        buffer.back() = ' '; // replace null-terminator added by 'strftime()' with a space
    }

    void format_column_uptime(std::string& buffer, _clock::time_point now) {
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
        stringify_integer(buffer, sec);

        buffer += '.';

        // Emulate '<< std::right << std::setw(ms_width) << std::setfill('0') << ms'
        if (ms_digits < ms_width) buffer.append(ms_width - ms_digits, '0');
        stringify_integer(buffer, ms);

        buffer += ')';
    }

    void format_column_thread(std::string& buffer) {
        buffer += '[';
        stringify_integer(buffer, _get_thread_index(std::this_thread::get_id()));
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
        stringify_integer(buffer, callsite.line);
        buffer.append(width_after_dot - _integer_digit_count(callsite.line), ' ');
    }

    void format_column_level(std::string& buffer, Verbosity level) {
        switch (level) {
        case Verbosity::ERR: buffer += " ERR  |"; return;
        case Verbosity::WARN: buffer += " WARN |"; return;
        case Verbosity::INFO: buffer += " INFO |"; return;
        }
    }

    template <typename... Args>
    void format_column_message(std::string& buffer, const Args&... args) {
        buffer += ' ';
        (buffer += ... += args); // parenthesis here are necessary
    }
};

// ====================
// --- Logger class ---
// ====================

class Logger {
private:
    inline static std::vector<Sink> sinks;

public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    template <typename... Args>
    void push_message(const Callsite& callsite, const MessageMetadata& meta, const Args&... args) {
        for (auto& sink : this->sinks) sink.format(callsite, meta, args...);
    }

    void add_sink(std::ostream& os, Verbosity verbosity, Colors colors, const Columns& columns,
                  _clock::duration flush_interval) {
        this->sinks.emplace_back(os, verbosity, colors, columns, flush_interval);
    }
};

// ======================
// --- Logging macros ---
// ======================

#define UTL_LOG_ERR(...)                                                                                               \
    utl::log::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::Verbosity::ERR}, __VA_ARGS__)

#define UTL_LOG_WARN(...)                                                                                              \
    utl::log::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::Verbosity::WARN}, __VA_ARGS__)

#define UTL_LOG_INFO(...)                                                                                              \
    utl::log::Logger::instance().push_message({__FILE__, __LINE__}, {utl::log::Verbosity::INFO}, __VA_ARGS__)

} // namespace utl::log

#endif
#endif // module utl::log
