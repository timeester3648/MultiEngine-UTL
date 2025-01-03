// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::log
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_log.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_LOG)
#ifndef UTLHEADERGUARD_LOG
#define UTLHEADERGUARD_LOG

// _______________________ INCLUDES _______________________

#include <array>         // array<>
#include <charconv>      // to_chars()
#include <chrono>        // steady_clock
#include <cstddef>       // size_t
#include <fstream>       // ofstream
#include <iostream>      // cout
#include <iterator>      // next()
#include <limits>        // numeric_limits<>
#include <mutex>         // lock_guard<>, mutex
#include <ostream>       // ostream
#include <sstream>       // std::ostringstream
#include <stdexcept>     // std::runtime_error
#include <string>        // string
#include <string_view>   // string_view
#include <system_error>  // errc()
#include <thread>        // this_thread::get_id()
#include <type_traits>   // is_integral_v<>, is_floating_point_v<>, is_same_v<>, is_convertible_to_v<>
#include <unordered_map> // unordered_map<>
#include <utility>       // forward<>()
#include <vector>        // vector<>

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS
#include <variant>

// ____________________ IMPLEMENTATION ____________________

namespace utl::log {

// ======================
// --- Internal utils ---
// ======================

// - SFINAE to select localtime_s() or localtime_r() -
template <class TimeMoment, class TimeType>
auto _available_localtime_impl(TimeMoment time_moment, TimeType timer)
    -> decltype(localtime_s(std::forward<TimeMoment>(time_moment), std::forward<TimeType>(timer))) {
    return localtime_s(std::forward<TimeMoment>(time_moment), std::forward<TimeType>(timer));
}

template <class TimeMoment, class TimeType>
auto _available_localtime_impl(TimeMoment time_moment, TimeType timer)
    -> decltype(localtime_r(std::forward<TimeType>(timer), std::forward<TimeMoment>(time_moment))) {
    return localtime_r(std::forward<TimeType>(timer), std::forward<TimeMoment>(time_moment));
}

inline std::size_t _get_thread_index(const std::thread::id id) {
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

// ===================
// --- Stringifier ---
// ===================

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
    constexpr bool trait_name_##_v = trait_name_<T>::value

utl_log_define_trait(_has_real, std::declval<T>().real());
utl_log_define_trait(_has_imag, std::declval<T>().imag());
utl_log_define_trait(_has_begin, std::declval<T>().begin());
utl_log_define_trait(_has_end, std::declval<T>().end());
utl_log_define_trait(_has_input_it, std::next(std::declval<T>().begin()));
utl_log_define_trait(_has_get, std::get<0>(std::declval<T>()));
utl_log_define_trait(_has_tuple_size, std::tuple_size<T>::value);
utl_log_define_trait(_has_ostream_insert, std::declval<std::ostream>() << std::declval<T>());
utl_log_define_trait(_is_pad_left, std::declval<std::decay_t<T>>().is_pad_left);
utl_log_define_trait(_is_pad_right, std::declval<std::decay_t<T>>().is_pad_right);
utl_log_define_trait(_is_pad, std::declval<std::decay_t<T>>().is_pad);

// Note:
// Trait '_has_input_it' is trickier than it may seem. Just doing '++std::declval<T>().begin()' will work
// most of the time, but there are cases where it returns 'false' for very much iterable types.
//
// """
//    Although the expression '++c.begin()' often compiles, it is not guaranteed to do so: 'c.begin()' is an rvalue
//    expression, and there is no LegacyInputIterator requirement that specifies that increment of an rvalue is
///   guaranteed to work. In particular, when iterators are implemented as pointers or its operator++ is
//    lvalue-ref-qualified, '++c.begin()' does not compile, while 'std::next(c.begin())' does.
// """ (c) https://en.cppreference.com/w/cpp/iterator/next
//
// By checking if 'std::next(c.begin())' compiles we can properly check that iterator satisfies input iterator
// requirements, which means we can use it with operator '++' to iterate over the container. Trying to just
// check for operator '++' would lead to false positives, while checking '++c.begin()' would lead to false
// negatives on containers such as 'std::array'. Note that target container doesn't even have to provide
// 'T::iterator', the type gets deduced from 'c.begin()'.


#undef utl_log_define_trait

// --- Internal utils ---
// ----------------------

template <class>
inline constexpr bool _always_false_v = false;

template <class T>
constexpr int _log_10_ceil(T num) {
    return num < 10 ? 1 : 1 + _log_10_ceil(num / 10);
}

template <class T>
constexpr int _max_float_digits =
    4 + std::numeric_limits<T>::max_digits10 + std::max(2, _log_10_ceil(std::numeric_limits<T>::max_exponent10));
// should be the smallest buffer size to account for all possible 'std::to_chars()' outputs,
// see [https://stackoverflow.com/questions/68472720/stdto-chars-minimal-floating-point-buffer-size]

template <class T>
constexpr int _max_int_digits = 2 + std::numeric_limits<T>::digits10;
// +2 because 'digits10' returns last digit index rather than the number of digits
// (aka 1 less than one would expect) and doesn't account for possible '-'.
// Also note that ints use 'digits10' and not 'max_digits10' which is only valid for floats.

// --- Alignment ---
// -----------------

// To align/pad values in a stringifier we can wrap then in thin structs
// that gets some special alignment logic in stringifier formatting selector.

template <class T>
struct PadLeft {
    constexpr PadLeft(const T& val, std::size_t size) : val(val), size(size) {} // this is needed for CTAD
    const T&              val;
    std::size_t           size;
    constexpr static bool is_pad_left = true;
}; // pads value the left (aka right alignment)

template <class T>
struct PadRight {
    constexpr PadRight(const T& val, std::size_t size) : val(val), size(size) {}
    const T&              val;
    std::size_t           size;
    constexpr static bool is_pad_right = true;
}; // pads value the right (aka left alignment)

template <class T>
struct Pad {
    constexpr Pad(const T& val, std::size_t size) : val(val), size(size) {}
    const T&              val;
    std::size_t           size;
    constexpr static bool is_pad = true;
}; // pads value on both sides (aka center alignment)

// --- Stringifier ---
// -------------------

// Generic stringifier with customizable API. Formatting of specific types can be customized by inheriting it
// and overriding specific methods. This is a reference implementation that is likely to be used in other modules.
//
// For example of how to extend stringifier see 'utl::log' documentation.
//
template <class Derived>
struct StringifierBase {
    using self    = StringifierBase;
    using derived = Derived;

    // --- Type-wise methods ---
    // -------------------------

    template <class T>
    static void append_bool(std::string& buffer, const T& value) {
        buffer += value ? "true" : "false";
    }

    template <class T>
    static void append_int(std::string& buffer, const T& value) {
        std::array<char, _max_int_digits<T>> stbuff;
        const auto [number_end_ptr, error_code] = std::to_chars(stbuff.data(), stbuff.data() + stbuff.size(), value);
        if (error_code != std::errc())
            throw std::runtime_error("Stringifier encountered std::to_chars() error while serializing an integer.");
        buffer.append(stbuff.data(), number_end_ptr - stbuff.data());
    }

    template <class T>
    static void append_float(std::string& buffer, const T& value) {
        std::array<char, _max_float_digits<T>> stbuff;
        const auto [number_end_ptr, error_code] = std::to_chars(stbuff.data(), stbuff.data() + stbuff.size(), value);
        if (error_code != std::errc())
            throw std::runtime_error("Stringifier encountered std::to_chars() error while serializing a float.");
        buffer.append(stbuff.data(), number_end_ptr - stbuff.data());
    }

    template <class T>
    static void append_complex(std::string& buffer, const T& value) {
        derived::append_float(buffer, value.real());
        buffer += " + ";
        derived::append_float(buffer, value.imag());
        buffer += " i";
    }

    template <class T>
    static void append_string(std::string& buffer, const T& value) {
        buffer += value;
    }

    template <class T>
    static void append_array(std::string& buffer, const T& value) {
        buffer += "{ ";
        if (value.begin() != value.end())
            for (auto it = value.begin();;) {
                derived::append(buffer, *it);
                if (++it == value.end()) break; // prevents trailing comma
                buffer += ", ";
            }
        buffer += " }";
    }

    template <class T>
    static void append_tuple(std::string& buffer, const T& value) {
        self::_append_tuple_fwd(buffer, value);
    }

    template <class T>
    static void append_printable(std::string& buffer, const T& value) {
        buffer += (std::ostringstream() << value).str();
    }

    // --- Main API ---
    // ----------------

    template <class T>
    static void append(std::string& buffer, const T& value) {
        self::_append_selector(buffer, value);
    }

    template <class... Args>
    static void append(std::string& buffer, const Args&... args) {
        (derived::append(buffer, args), ...);
    }

    template <class... Args>
    [[nodiscard]] static std::string stringify(Args&&... args) {
        std::string buffer;
        derived::append(buffer, std::forward<Args>(args)...);
        return buffer;
    }

    template <class... Args>
    [[nodiscard]] std::string operator()(Args&&... args) {
        return derived::stringify(std::forward<Args>(args)...);
    } // allows stringifier to be used as a functor

    // --- Helpers ---
    // ---------------
private:
    template <class Tuplelike, std::size_t... Idx>
    static void _append_tuple_impl(std::string& buffer, Tuplelike value, std::index_sequence<Idx...>) {
        ((Idx == 0 ? "" : buffer += ", ", derived::append(buffer, std::get<Idx>(value))), ...);
        // fold expression '( f(args), ... )' invokes 'f(args)' for all arguments in 'args...'
        // in the same fashion, we can fold over 2 functions by doing '( ( f(args), g(args) ), ... )'
    }

    template <template <class...> class Tuplelike, class... Args>
    static void _append_tuple_fwd(std::string& buffer, const Tuplelike<Args...>& value) {
        buffer += "< ";
        self::_append_tuple_impl(buffer, value, std::index_sequence_for<Args...>{});
        buffer += " >";
    }

    template <class T>
    static void _append_selector(std::string& buffer, const T& value) {
        // Left-padded something
        if constexpr (_is_pad_left_v<T>) {
            std::string temp;
            self::_append_selector(temp, value.val);
            if (temp.size() < value.size) buffer.append(value.size - temp.size(), ' ');
            buffer += temp;
        }
        // Right-padded something
        else if constexpr (_is_pad_right_v<T>) {
            const std::size_t old_size = buffer.size();
            self::_append_selector(buffer, value.val);
            const std::size_t appended_size = buffer.size() - old_size;
            if (appended_size < value.size) buffer.append(value.size - appended_size, ' ');
            // right-padding is faster than left padding since we don't need a temporary string to get appended size
        }
        // Center-padded something
        else if constexpr (_is_pad_v<T>) {
            std::string temp;
            self::_append_selector(temp, value.val);
            if (temp.size() < value.size) {
                const std::size_t lpad_size = (value.size - temp.size()) / 2;
                const std::size_t rpad_size = value.size - lpad_size - temp.size();
                buffer.append(lpad_size, ' ');
                buffer += temp;
                buffer.append(rpad_size, ' ');
            } else buffer += temp;
        }
        // Bool
        else if constexpr (std::is_same_v<T, bool>)
            derived::append_bool(buffer, value);
        // Char
        else if constexpr (std::is_same_v<T, char>) derived::append_string(buffer, value);
        // Integral
        else if constexpr (std::is_integral_v<T>) derived::append_int(buffer, value);
        // Floating-point
        else if constexpr (std::is_floating_point_v<T>) derived::append_float(buffer, value);
        // Complex
        else if constexpr (_has_real_v<T> && _has_imag_v<T>) derived::append_complex(buffer, value);
        // 'std::string_view'-convertible (most strings and string-like types)
        else if constexpr (std::is_convertible_v<T, std::string_view>) derived::append_string(buffer, value);
        // 'std::string'-convertible (some "nastier" string-like types, mainly 'std::path')
        else if constexpr (std::is_convertible_v<T, std::string>) derived::append_string(buffer, std::string(value));
        // Array-like
        else if constexpr (_has_begin_v<T> && _has_end_v<T> && _has_input_it_v<T>) derived::append_array(buffer, value);
        // Tuple-like
        else if constexpr (_has_get_v<T> && _has_tuple_size_v<T>) derived::append_tuple(buffer, value);
        // 'std::ostream' printable
        else if constexpr (_has_ostream_insert_v<T>) derived::append_printable(buffer, value);
        // No valid stringification exists
        else static_assert(_always_false_v<T>, "No valid stringification exists for the type.");

        // Note: Using if-constexpr chain here allows us to pick and choose priority of different branches,
        // removing any possible ambiguity we could encounter doing things through SFINAE or overloads.
    }
};

// Note:
// The stringifier shines at stringifying & concatenating multiple values into the same buffer.
// Single-value is a specific case which allows all 'buffer += ...' to be replaced with most things being formatted
// straight into a newly created string. We could optimize this, but that would make require an almost full logic
// duplication and make the class cumbersome to extend since instead of a singural 'append_something()' methods there
// would be 2: 'append_something()' and 'construct_something()'. It also doesn't seem to be worth it, the difference
// in performance isn't that significat and we're still faster than most usual approaches to stringification.

// ===============================
// --- Stringifier derivatives ---
// ===============================

// "Default" customization of stringifier, here we can optimize a few things.
//
// The reason we don't include this in the original stringifier is because it's intended to be a customizable
// class that can be extended/optimized/decorated by inheriting it and overriding specific methods. The changes
// made by some optimizations wouldn't be compatible with such philosophy.
//
struct Stringifier : public StringifierBase<Stringifier> {
    using base = StringifierBase<Stringifier>;

    // template <class... Args>
    // [[nodiscard]] static std::string stringify(Args&&... args) {
    //     return StringifierBase::stringify(std::forward<Args>(args)...);
    // }

    using base::stringify;

    [[nodiscard]] static std::string stringify(int arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(long arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(long long arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(unsigned int arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(unsigned long arg) { return std::to_string(arg); }
    [[nodiscard]] static std::string stringify(unsigned long long arg) { return std::to_string(arg); }
    // for individual ints 'std::to_string()' beats 'append_int()' with <charconv> since any reasonable compiler
    // implements it using the same <charconv> routine, but formatted directly into a string upon its creation

    [[nodiscard]] static std::string stringify(std::string&& arg) { return arg; }
    // no need to do all the appending stuff for individual r-value strings, just forward them as is

    template <class... Args>
    [[nodiscard]] std::string operator()(Args&&... args) {
        return Stringifier::stringify(std::forward<Args>(args)...);
    }
};

template <class... Args>
void append_stringified(std::string& str, Args&&... args) {
    Stringifier::append(str, std::forward<Args>(args)...);
}

template <class... Args>
[[nodiscard]] std::string stringify(Args&&... args) {
    return Stringifier::stringify(std::forward<Args>(args)...);
}

template <class... Args>
void print(Args&&... args) {
    std::cout << Stringifier::stringify(std::forward<Args>(args)...);
}

template <class... Args>
void println(Args&&... args) {
    std::cout << Stringifier::stringify(std::forward<Args>(args)...) << '\n';
}

// ===============
// --- Options ---
// ===============

enum class Verbosity { ERR = 1, WARN = 2, INFO = 3, TRACE = 4 };

enum class OpenMode { REWRITE, APPEND };

enum class Colors { ENABLE, DISABLE };

struct Columns {
    bool datetime = true;
    bool uptime   = true;
    bool thread   = true;
    bool callsite = true;
    bool level    = true;
    bool message  = true;

    // Columns() : datetime(true), uptime(true), thread(true), callsite(true), level(true), message(true) {}
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
    using os_ref_wrapper = std::reference_wrapper<std::ostream>;

    std::variant<std::monostate, os_ref_wrapper, std::ofstream> os_variant;
    Verbosity                                                   verbosity;
    Colors                                                      colors;
    clock::duration                                             flush_interval;
    Columns                                                     columns;
    clock::time_point                                           last_flushed;

    friend struct _logger;

    std::ostream& ostream_ref() {
        if (const auto ref_wrapper_ptr = std::get_if<os_ref_wrapper>(&this->os_variant)) return ref_wrapper_ptr->get();
        else return std::get<std::ofstream>(this->os_variant);
        // monostate case should be impossible
    }

    // MARK:

public:
    Sink()            = delete;
    Sink(const Sink&) = delete;
    Sink(Sink&&)      = default;

    Sink(std::ofstream&& os, Verbosity verbosity, Colors colors, clock::duration flush_interval, const Columns& columns)
        : os_variant(std::move(os)), verbosity(verbosity), colors(colors), flush_interval(flush_interval),
          columns(columns) {}

    Sink(std::reference_wrapper<std::ostream> os, Verbosity verbosity, Colors colors, clock::duration flush_interval,
         const Columns& columns)
        : os_variant(os), verbosity(verbosity), colors(colors), flush_interval(flush_interval), columns(columns) {}

    // We want a way of changing sink options using its handle / reference returned by the logger
    Sink& set_verbosity(Verbosity verbosity) {
        this->verbosity = verbosity;
        return *this;
    }
    Sink& set_colors(Colors colors) {
        this->colors = colors;
        return *this;
    }
    Sink& set_flush_interval(clock::duration flush_interval) {
        this->flush_interval = flush_interval;
        return *this;
    }
    Sink& set_columns(const Columns& columns) {
        this->columns = columns;
        return *this;
    }

private:
    template <class... Args>
    void format(const Callsite& callsite, const MessageMetadata& meta, const Args&... args) {
        if (meta.verbosity > this->verbosity) return;

        thread_local std::string buffer;

        // const bool need_time = this->columns.uptime || this->flush_interval.count() != 0;
        // const clock::time_point now = [need_time]{ return need_time ? clock::now() : clock::time_point{}; }();
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

        if (this->colors == Colors::ENABLE) buffer += "\033[0m";

        buffer += '\n';

        this->ostream_ref().write(buffer.data(), buffer.size());

        // flush every message immediately
        if (this->flush_interval.count() == 0) {
            this->ostream_ref().flush();
        }
        // or flush periodically
        else if (now - this->last_flushed > this->flush_interval) {
            this->last_flushed = now;
            this->ostream_ref().flush();
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

    template <class... Args>
    void format_column_message(std::string& buffer, const Args&... args) {
        buffer += ' ';
        append_stringified(buffer, args...);
    }
};

// ====================
// --- Logger class ---
// ====================

struct _logger {
    inline static std::vector<Sink> sinks;

    static _logger& instance() {
        static _logger logger;
        return logger;
    }

    template <class... Args>
    void push_message(const Callsite& callsite, const MessageMetadata& meta, const Args&... args) {
        // When no sinks were manually created, default sink-to-terminal takes over
        if (this->sinks.empty()) {
            static Sink default_sink(std::cout, Verbosity::TRACE, Colors::ENABLE, ms(0), Columns{});
            default_sink.format(callsite, meta, args...);
        }

        for (auto& sink : this->sinks) sink.format(callsite, meta, args...);
    }
};

// =======================
// --- Sink public API ---
// =======================

inline Sink& add_ostream_sink(std::ostream& os, Verbosity verbosity = Verbosity::INFO, Colors colors = Colors::ENABLE,
                              clock::duration flush_interval = ms{}, const Columns& columns = Columns{}) {
    return _logger::instance().sinks.emplace_back(os, verbosity, colors, flush_interval, columns);
}

inline Sink& add_file_sink(const std::string& filename, OpenMode open_mode = OpenMode::REWRITE,
                           Verbosity verbosity = Verbosity::TRACE, Colors colors = Colors::DISABLE,
                           clock::duration flush_interval = ms{15}, const Columns& columns = Columns{}) {
    const auto ios_open_mode = (open_mode == OpenMode::APPEND) ? std::ios::out | std::ios::app : std::ios::out;
    return _logger::instance().sinks.emplace_back(std::ofstream(filename, ios_open_mode), verbosity, colors,
                                                  flush_interval, columns);
}

// ======================
// --- Logging macros ---
// ======================

#define UTL_LOG_ERR(...)                                                                                               \
    utl::log::_logger::instance().push_message({__FILE__, __LINE__}, {utl::log::Verbosity::ERR}, __VA_ARGS__)

#define UTL_LOG_WARN(...)                                                                                              \
    utl::log::_logger::instance().push_message({__FILE__, __LINE__}, {utl::log::Verbosity::WARN}, __VA_ARGS__)

#define UTL_LOG_INFO(...)                                                                                              \
    utl::log::_logger::instance().push_message({__FILE__, __LINE__}, {utl::log::Verbosity::INFO}, __VA_ARGS__)

#define UTL_LOG_TRACE(...)                                                                                             \
    utl::log::_logger::instance().push_message({__FILE__, __LINE__}, {utl::log::Verbosity::TRACE}, __VA_ARGS__)

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
