// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::json
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_json.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_JSON)
#ifndef UTLHEADERGUARD_JSON
#define UTLHEADERGUARD_JSON

// _______________________ INCLUDES _______________________

#include <array>            // array<>
#include <charconv>         // to_chars(), from_chars()
#include <cmath>            // isfinite()
#include <codecvt>          // codecvt_utf8<>
#include <cstddef>          // size_t
#include <cuchar>           // size_t, char32_t, mbstate_t
#include <exception>        // exception
#include <fstream>          // ifstream, ofstream
#include <initializer_list> // initializer_list<>
#include <limits>           // numeric_limits<>::max_digits10, numeric_limits<>::max_exponent10
#include <map>              // map<>
#include <stdexcept>        // runtime_error
#include <string>           // string, stoul()
#include <string_view>      // string_view
#include <system_error>     // errc()
#include <type_traits>      // enable_if_t<>, void_t, is_convertible_v<>, is_same_v<>,
                            // conjunction<>, disjunction<>, negation<>
#include <utility>          // move(), declval<>()
#include <variant>          // variant<>
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Reasonably simple (if we discound reflection) parser / serializer, doen't use any intrinsics or compiler-specific
// stuff. Unlike some other implementation, doesn't include the tokenizing step - we parse everything in a single 1D
// scan over the data, constructing recursive JSON struct on the fly. The main reason we can do this so easily is is
// due to a nice quirk of JSON - parsing nodes, we can always determine node type based on a single first character,
// see '_parser::parse_node()'.
//
// Struct reflection is implemented through macros - alternative way would be to use templates with __PRETTY_FUNCTION__
// (or __FUNCSIG__) and do some constexpr string parsing to perform "magic" reflection without requiring, but that
// relies on implementation-defined format of those strings and may trash the compile times, macros work everywhere.
//
// Proper type traits and 'if constexpr' recursive introspection are a key to making APIs that can convert stuff
// between JSON and other types seamlessly, which is exactly what we do here, it even accounts for reflection.

// ____________________ IMPLEMENTATION ____________________

namespace utl::json {

// ===================
// --- Misc. utils ---
// ===================

// Codepoint -> UTF-8 string conversion using <codecvt> (deprecated in C++17, removed in C++26)
// This is kinda horrible, but there doesn't seem to a better way.
// Returns success so we can handle the error message inside the parser itself.
inline bool _unicode_codepoint_to_utf8(std::string& destination, char32_t cp) {
    std::array<char, 4> buffer; // here we will put UTF-8 string
    char32_t const*     from = &cp;
    char*               end_of_buffer;

    std::mbstate_t              state;
    std::codecvt_utf8<char32_t> codecvt;

    if (codecvt.out(state, from, from + 1, from, buffer.data(), buffer.data() + 4, end_of_buffer)) return false;

    destination.append(buffer.data(), end_of_buffer);
    return true;
}

[[nodiscard]] inline std::string _read_file_to_string(const std::string& path) {
    using namespace std::string_literals;

    // This seems the to be the fastest way of reading a text file
    // into 'std::string' without invoking OS-specific methods
    // See this StackOverflow thread:
    // [https://stackoverflow.com/questions/32169936/optimal-way-of-reading-a-complete-file-to-a-string-using-fstream]
    // And attached benchmarks:
    // [https://github.com/Sqeaky/CppFileToStringExperiments]

    std::ifstream file(path, std::ios::ate); // open file and immediately seek to the end
    if (!file.good()) throw std::runtime_error("Could not open file {"s + path + "."s);
    // NOTE: Add a way to return errors if file doesn't exist, exceptions aren't particularly good here

    const auto file_size = file.tellg(); // returns cursor pos, which is the end of file
    file.seekg(std::ios::beg);           // seek to the beginning
    std::string chars(file_size, 0);     // allocate string of appropriate size
    file.read(chars.data(), file_size);  // read into the string
    return chars;
}

template <class T>
[[nodiscard]] constexpr int _log_10_ceil(T num) noexcept {
    return num < 10 ? 1 : 1 + _log_10_ceil(num / 10);
}

[[nodiscard]] inline std::string _pretty_error(std::size_t cursor, const std::string& chars) {
    // Special case for empty buffers
    if (chars.empty()) return "";

    // "Normalize" cursor if it's at the end of the buffer
    if (cursor >= chars.size()) cursor = chars.size() - 1;

    // Get JSON line number
    std::size_t line_number = 1; // don't want to include <algorithm> just for a single std::count()
    for (std::size_t pos = 0; pos < cursor; ++pos)
        if (chars[pos] == '\n') ++line_number;

    // Get contents of the current line
    constexpr std::size_t max_left_width  = 24;
    constexpr std::size_t max_right_width = 24;

    std::size_t line_start = cursor;
    for (; line_start > 0; --line_start)
        if (chars[line_start - 1] == '\n' || cursor - line_start >= max_left_width) break;

    std::size_t line_end = cursor;
    for (; line_end < chars.size() - 1; ++line_end)
        if (chars[line_end + 1] == '\n' || line_end - cursor >= max_right_width) break;

    const std::string_view line_contents(chars.data() + line_start, line_end - line_start + 1);

    // Format output
    const std::string line_prefix =
        "Line " + std::to_string(line_number) + ": "; // fits into SSO buffer in almost all cases

    std::string res;
    res.reserve(7 + 2 * line_prefix.size() + 2 * line_contents.size());

    res += '\n';
    res += line_prefix;
    res += line_contents;
    res += '\n';
    res.append(line_prefix.size(), ' ');
    res.append(cursor - line_start, '-');
    res += '^';
    res.append(line_end - cursor, '-');
    res += " [!]";

    return res;
}

// --- Type traits ---
// -------------------

#define utl_json_define_trait(trait_name_, ...)                                                                        \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

utl_json_define_trait(_has_begin, std::declval<std::decay_t<T>>().begin());
utl_json_define_trait(_has_end, std::declval<std::decay_t<T>>().end());
utl_json_define_trait(_has_input_it, std::next(std::declval<T>().begin()));

utl_json_define_trait(_has_key_type, std::declval<typename std::decay_t<T>::key_type>());
utl_json_define_trait(_has_mapped_type, std::declval<typename std::decay_t<T>::mapped_type>());

#undef utl_json_define_trait

// Workaround for 'static_assert(false)' making program ill-formed even
// when placed inide an 'if constexpr' branch that never compiles.
// 'static_assert(_always_false_v<T)' on the the other hand doesn't,
// which means we can use it to mark branches that should never compile.
template <class>
constexpr bool _always_false_v = false;

// --- MAP macro ---
// -----------------

// This is an implementation of a classic MAP macro that applies some function macro
// to all elements of __VA_ARGS__, it looks much uglier than usual because we have to prefix
// everything with verbose 'utl_json_', but that's the price of avoiding name collisions.
//
// Created by William Swanson in 2012 and declared as public domain.
//
// Macro supports up to 365 arguments. We will need it for structure reflection.

#define utl_json_eval_0(...) __VA_ARGS__
#define utl_json_eval_1(...) utl_json_eval_0(utl_json_eval_0(utl_json_eval_0(__VA_ARGS__)))
#define utl_json_eval_2(...) utl_json_eval_1(utl_json_eval_1(utl_json_eval_1(__VA_ARGS__)))
#define utl_json_eval_3(...) utl_json_eval_2(utl_json_eval_2(utl_json_eval_2(__VA_ARGS__)))
#define utl_json_eval_4(...) utl_json_eval_3(utl_json_eval_3(utl_json_eval_3(__VA_ARGS__)))
#define utl_json_eval(...) utl_json_eval_4(utl_json_eval_4(utl_json_eval_4(__VA_ARGS__)))

#define utl_json_map_end(...)
#define utl_json_map_out
#define utl_json_map_comma ,

#define utl_json_map_get_end_2() 0, utl_json_map_end
#define utl_json_map_get_end_1(...) utl_json_map_get_end_2
#define utl_json_map_get_end(...) utl_json_map_get_end_1
#define utl_json_map_next_0(test, next, ...) next utl_json_map_out
#define utl_json_map_next_1(test, next) utl_json_map_next_0(test, next, 0)
#define utl_json_map_next(test, next) utl_json_map_next_1(utl_json_map_get_end test, next)

#define utl_json_map_0(f, x, peek, ...) f(x) utl_json_map_next(peek, utl_json_map_1)(f, peek, __VA_ARGS__)
#define utl_json_map_1(f, x, peek, ...) f(x) utl_json_map_next(peek, utl_json_map_0)(f, peek, __VA_ARGS__)

// Resulting macro, applies the function macro `f` to each of the remaining parameters
#define utl_json_map(f, ...)                                                                                           \
    utl_json_eval(utl_json_map_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0)) static_assert(true)

// ===================================
// --- JSON type conversion traits ---
// ===================================

template <class T>
using _object_type_impl = std::map<std::string, T, std::less<>>;
// 'std::less<>' declares map as transparent, which means we can `.find()` for `std::string_view` keys
template <class T>
using _array_type_impl  = std::vector<T>;
using _string_type_impl = std::string;
using _number_type_impl = double;
using _bool_type_impl   = bool;
struct _null_type_impl {
    [[nodiscard]] bool operator==(const _null_type_impl&) const noexcept {
        return true;
    } // so we can check 'Null == Null'
};

struct _dummy_type {};

// 'possible_value_type<T>::value' evaluates to:
//    - 'T::value_type' if 'T' has 'value_type'
//    - '_dummy_type' otherwise
template <class T, class = void>
struct possible_value_type {
    using type = _dummy_type;
};

template <class T>
struct possible_value_type<T, std::void_t<decltype(std::declval<typename std::decay_t<T>::value_type>())>> {
    using type = typename T::value_type;
};

// 'possible_mapped_type<T>::value' evaluates to:
//    - 'T::mapped_type' if 'T' has 'mapped_type'
//    - '_dummy_type' otherwise
template <class T, class = void>
struct possible_mapped_type {
    using type = _dummy_type;
};

template <class T>
struct possible_mapped_type<T, std::void_t<decltype(std::declval<typename std::decay_t<T>::mapped_type>())>> {
    using type = typename T::mapped_type;
};
// these type traits are a key to checking properties of 'T::value_type' & 'T::mapped_type' for a 'T' which may or may
// not have them (which is exactly the case with recursive traits that we're gonna use later to deduce convertability
// to recursive JSON). '_dummy_type' here is necessary to end the recursion of 'std::disjuction'

#define utl_json_type_trait_conjunction(trait_name_, ...)                                                              \
    template <class T>                                                                                                 \
    struct trait_name_ : std::conjunction<__VA_ARGS__> {};                                                             \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

#define utl_json_type_trait_disjunction(trait_name_, ...)                                                              \
    template <class T>                                                                                                 \
    struct trait_name_ : std::disjunction<__VA_ARGS__> {};                                                             \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

// Note:
// The reason we use 'struct trait_name : std::conjunction<...>' instead of 'using trait_name = std::conjunction<...>'
// is because 1st option allows for recursive type traits, while 'using' syntax doesn't. We have some recursive type
// traits here in form of 'is_json_type_convertible<>', which expands over the 'T' checking that 'T', 'T::value_type'
// (if exists), 'T::mapped_type' (if exists) and their other layered value/mapped types are all satisfying the
// necessary convertability trait. This allows us to make a trait which fully deduces whether some
// complex datatype can be converted to a JSON recursively.

utl_json_type_trait_conjunction(is_object_like, _has_begin<T>, _has_end<T>, _has_key_type<T>, _has_mapped_type<T>);
utl_json_type_trait_conjunction(is_array_like, _has_begin<T>, _has_end<T>, _has_input_it<T>);
utl_json_type_trait_conjunction(is_string_like, std::is_convertible<T, std::string_view>);
utl_json_type_trait_conjunction(is_numeric_like, std::is_convertible<T, _number_type_impl>);
utl_json_type_trait_conjunction(is_bool_like, std::is_same<T, _bool_type_impl>);
utl_json_type_trait_conjunction(is_null_like, std::is_same<T, _null_type_impl>);

utl_json_type_trait_disjunction(_is_directly_json_convertible, is_string_like<T>, is_numeric_like<T>, is_bool_like<T>,
                                is_null_like<T>);

utl_json_type_trait_conjunction(
    is_json_convertible,
    std::disjunction<
        // either the type itself is convertible
        _is_directly_json_convertible<T>,
        // ... or it's an array of convertible elements
        std::conjunction<is_array_like<T>, is_json_convertible<typename possible_value_type<T>::type>>,
        // ... or it's an object of convertible elements
        std::conjunction<is_object_like<T>, is_json_convertible<typename possible_mapped_type<T>::type>>>,
    // end recusion by short-circuiting conjunction with 'false' once we arrive to '_dummy_type',
    // arriving here means the type isn't convertable to JSON
    std::negation<std::is_same<T, _dummy_type>>);

#undef utl_json_type_trait_conjunction
#undef utl_json_type_trait_disjunction

// ==================
// --- Node class ---
// ==================

enum class Format { PRETTY, MINIMIZED };

class Node;
inline void _serialize_json_to_buffer(std::string& chars, const Node& node, Format format);

class Node {
public:
    using object_type = _object_type_impl<Node>;
    using array_type  = _array_type_impl<Node>;
    using string_type = _string_type_impl;
    using number_type = _number_type_impl;
    using bool_type   = _bool_type_impl;
    using null_type   = _null_type_impl;

private:
    using variant_type = std::variant<null_type, object_type, array_type, string_type, number_type, bool_type>;
    // 'null_type' should go first to ensure default-initialization creates 'null' nodes

    variant_type data{};

public:
    // -- Getters --
    // -------------

    template <class T>
    [[nodiscard]] T& get() {
        return std::get<T>(this->data);
    }

    template <class T>
    [[nodiscard]] const T& get() const {
        return std::get<T>(this->data);
    }

    [[nodiscard]] object_type& get_object() { return this->get<object_type>(); }
    [[nodiscard]] array_type&  get_array() { return this->get<array_type>(); }
    [[nodiscard]] string_type& get_string() { return this->get<string_type>(); }
    [[nodiscard]] number_type& get_number() { return this->get<number_type>(); }
    [[nodiscard]] bool_type&   get_bool() { return this->get<bool_type>(); }
    [[nodiscard]] null_type&   get_null() { return this->get<null_type>(); }

    [[nodiscard]] const object_type& get_object() const { return this->get<object_type>(); }
    [[nodiscard]] const array_type&  get_array() const { return this->get<array_type>(); }
    [[nodiscard]] const string_type& get_string() const { return this->get<string_type>(); }
    [[nodiscard]] const number_type& get_number() const { return this->get<number_type>(); }
    [[nodiscard]] const bool_type&   get_bool() const { return this->get<bool_type>(); }
    [[nodiscard]] const null_type&   get_null() const { return this->get<null_type>(); }

    template <class T>
    [[nodiscard]] bool is() const noexcept {
        return std::holds_alternative<T>(this->data);
    }

    [[nodiscard]] bool is_object() const noexcept { return this->is<object_type>(); }
    [[nodiscard]] bool is_array() const noexcept { return this->is<array_type>(); }
    [[nodiscard]] bool is_string() const noexcept { return this->is<string_type>(); }
    [[nodiscard]] bool is_number() const noexcept { return this->is<number_type>(); }
    [[nodiscard]] bool is_bool() const noexcept { return this->is<bool_type>(); }
    [[nodiscard]] bool is_null() const noexcept { return this->is<null_type>(); }

    template <class T>
    [[nodiscard]] T* get_if() noexcept {
        return std::get_if<T>(&this->data);
    }

    template <class T>
    [[nodiscard]] const T* get_if() const noexcept {
        return std::get_if<T>(&this->data);
    }

    // -- Object methods --
    // --------------------

    Node& operator[](std::string_view key) {
        // 'std::map<K, V>::operator[]()' and 'std::map<K, V>::at()' don't support
        // support heterogeneous lookup, we have to reimplement them manually
        if (this->is_null()) this->data = object_type(); // only 'null' converts to object automatically on 'json[key]'
        auto& object = this->get_object();
        auto  it     = object.find(key);
        if (it == object.end()) it = object.emplace(key, Node{}).first;
        return it->second;
    }

    [[nodiscard]] const Node& operator[](std::string_view key) const {
        // 'std::map<K, V>::operator[]()' and 'std::map<K, V>::at()' don't support
        // support heterogeneous lookup, we have to reimplement them manually
        const auto& object = this->get_object();
        const auto  it     = object.find(key);
        if (it == object.end())
            throw std::runtime_error("Accessing non-existent key {" + std::string(key) + "} in JSON object.");
        return it->second;
    }

    [[nodiscard]] Node& at(std::string_view key) {
        // Non-const 'operator[]' inserts non-existent keys, '.at()' should throw instead
        auto&      object = this->get_object();
        const auto it     = object.find(key);
        if (it == object.end())
            throw std::runtime_error("Accessing non-existent key {" + std::string(key) + "} in JSON object.");
        return it->second;
    }

    [[nodiscard]] const Node& at(std::string_view key) const { return this->operator[](key); }

    [[nodiscard]] bool contains(std::string_view key) const {
        const auto& object = this->get_object();
        const auto  it     = object.find(std::string(key));
        return it != object.end();
    }

    template <class T>
    [[nodiscard]] const T& value_or(std::string_view key, const T& else_value) {
        const auto& object = this->get_object();
        const auto  it     = object.find(std::string(key));
        if (it != object.end()) return it->second.get<T>();
        return else_value;
        // same thing as 'this->contains(key) ? json.at(key).get<T>() : else_value' but without a second map lookup
    }

    // -- Assignment --
    // ----------------

    // Converting assignment
    template <class T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, Node> &&
                                            !std::is_same_v<std::decay_t<T>, object_type> &&
                                            !std::is_same_v<std::decay_t<T>, array_type> &&
                                            !std::is_same_v<std::decay_t<T>, string_type> && is_json_convertible_v<T>,
                                        bool> = true>
    Node& operator=(const T& value) {
        // Don't take types that decay to Node/object/array/string to prevent
        // shadowing native copy/move assignment for those types

        // Several "type-like' characteristics can be true at the same time,
        // to resolve ambiguity we assign the following conversion priority:
        // string > object > array > bool > null > numeric

        if constexpr (is_string_like_v<T>) {
            this->data.emplace<string_type>(value);
        } else if constexpr (is_object_like_v<T>) {
            this->data.emplace<object_type>();
            auto& object = this->get_object();
            for (const auto& [key, val] : value) object[key] = val;
        } else if constexpr (is_array_like_v<T>) {
            this->data.emplace<array_type>();
            auto& array = this->get_array();
            for (const auto& elem : value) array.emplace_back(elem);
        } else if constexpr (is_bool_like_v<T>) {
            this->data.emplace<bool_type>(value);
        } else if constexpr (is_null_like_v<T>) {
            this->data.emplace<null_type>(value);
        } else if constexpr (is_numeric_like_v<T>) {
            this->data.emplace<number_type>(value);
        } else {
            static_assert(_always_false_v<T>, "Method is a non-exhaustive visitor of std::variant<>.");
        }

        return *this;
    }

    // "native" copy/move semantics for types that support it
    Node& operator=(const object_type& value) {
        this->data = value;
        return *this;
    }
    Node& operator=(object_type&& value) {
        this->data = std::move(value);
        return *this;
    }

    Node& operator=(const array_type& value) {
        this->data = value;
        return *this;
    }
    Node& operator=(array_type&& value) {
        this->data = std::move(value);
        return *this;
    }

    Node& operator=(const string_type& value) {
        this->data = value;
        return *this;
    }
    Node& operator=(string_type&& value) {
        this->data = std::move(value);
        return *this;
    }

    // Support for 'std::initializer_list' type deduction,
    // (otherwise the call is ambiguous)
    template <class T>
    Node& operator=(std::initializer_list<T> ilist) {
        // We can't just do 'return *this = array_type(value);' because compiler doesn't realize it can
        // convert 'std::initializer_list<T>' to 'std::vector<Node>' for all 'T' convertable to 'Node',
        // we have to invoke 'Node()' constructor explicitly (here it happens in 'emplace_back()')
        array_type array_value;
        array_value.reserve(ilist.size());
        for (const auto& e : ilist) array_value.emplace_back(e);
        this->data = std::move(array_value);
        return *this;
    }

    template <class T>
    Node& operator=(std::initializer_list<std::initializer_list<T>> ilist) {
        // Support for 2D brace initialization
        array_type array_value;
        array_value.reserve(ilist.size());
        for (const auto& e : ilist) {
            array_value.emplace_back();
            array_value.back() = e;
        }
        // uses 1D 'operator=(std::initializer_list<T>)' to fill each node of the array
        this->data = std::move(array_value);
        return *this;
    }

    template <class T>
    Node& operator=(std::initializer_list<std::initializer_list<std::initializer_list<T>>> ilist) {
        // Support for 3D brace initialization
        // it's dumb, but it works
        array_type array_value;
        array_value.reserve(ilist.size());
        for (const auto& e : ilist) {
            array_value.emplace_back();
            array_value.back() = e;
        }
        // uses 2D 'operator=(std::initializer_list<std::initializer_list<T>>)' to fill each node of the array
        this->data = std::move(array_value);
        return *this;
    }

    // we assume no reasonable person would want to type a 4D+ array as 'std::initializer_list<>',
    // if they really want to they can specify the type of the top layer and still be fine

    // -- Constructors --
    // ------------------

    // TEMP:
    Node& operator=(const Node&) = default;
    Node& operator=(Node&&)      = default;

    Node()            = default;
    Node(const Node&) = default;
    Node(Node&&)      = default;
    // Note:
    // We suffer a lot if 'object_type' move-constructor is not marked 'noexcept', if that's the case
    // 'Node' move-constructor doesn't get 'noexcept' either which means `std::vector<Node>` will copy
    // nodes instead of moving when it grows. 'std::map' is NOT required to be noexcept by the standard
    // but it is marked as such in both 'libc++' and 'libstdc++', 'VS' stdlib lacks behind in that regard.
    // See noexcept status summary here: http://howardhinnant.github.io/container_summary.html

    // Converting ctor
    template <class T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, Node> &&
                                            !std::is_same_v<std::decay_t<T>, object_type> &&
                                            !std::is_same_v<std::decay_t<T>, array_type> &&
                                            !std::is_same_v<std::decay_t<T>, string_type> && is_json_convertible_v<T>,
                                        bool> = true>
    Node(const T& value) {
        *this = value;
    }

    Node(const object_type& value) { this->data = value; }
    Node(object_type&& value) { this->data = std::move(value); }
    Node(const array_type& value) { this->data = value; }
    Node(array_type&& value) { this->data = std::move(value); }
    Node(std::string_view value) { this->data = string_type(value); }
    Node(const string_type& value) { this->data = value; }
    Node(string_type&& value) { this->data = std::move(value); }
    Node(number_type value) { this->data = value; }
    Node(bool_type value) { this->data = value; }
    Node(null_type value) { this->data = value; }

    // --- JSON Serializing public API ---
    // -----------------------------------

    [[nodiscard]] std::string to_string(Format format = Format::PRETTY) const {
        std::string buffer;
        _serialize_json_to_buffer(buffer, *this, format);
        return buffer;
    }

    void to_file(const std::string& filepath, Format format = Format::PRETTY) const {
        auto chars = this->to_string(format);
        std::ofstream(filepath).write(chars.data(), chars.size());
        // maybe a little faster than doing 'std::ofstream(filepath) << node.to_string(format)'
    }

    // --- Reflection ---
    // ------------------

    template <class T>
    [[nodiscard]] T to_struct() const {
        static_assert(
            _always_false_v<T>,
            "Provided type doesn't have a defined JSON reflection. Use 'UTL_JSON_REFLECT' macro to define one.");
        // compile-time protection against calling 'to_struct()' on types that don't have reflection,
        // we can also provide a proper error message here
        return {};
        // this is needed to silence "no return in a function" warning that appears even if this specialization
        // (which by itself should cause a compile error) doesn't get compiled

        // specializations of this template that will actually perform the conversion will be defined by
        // macros outside the class body, this is a perfectly legal thing to do, even if unintuitive compared
        // to non-template members, see https://en.cppreference.com/w/cpp/language/member_template
    }
};

// Public typedefs
using Object = Node::object_type;
using Array  = Node::array_type;
using String = Node::string_type;
using Number = Node::number_type;
using Bool   = Node::bool_type;
using Null   = Node::null_type;

// =====================
// --- Lookup Tables ---
// =====================

constexpr std::uint8_t _u8(char value) { return static_cast<std::uint8_t>(value); }

constexpr std::size_t _number_of_char_values = 256;
// always true since 'sizeof(char) == 1' is guaranteed by the standard

// Lookup table used to check if number should be escaped and get a replacement char on at the same time.
// This allows us to replace multiple checks and if's with a single array lookup that.
//
// Instead of:
//    if (c == '"') { chars += '"' }
//    ...
//    else if (c == '\t') { chars += 't' }
// we get:
//    if (const char replacement = _lookup_serialized_escaped_chars[_u8(c)]) { chars += replacement; }
//
// which ends up being a bit faster.
//
// Note:
// It is important that we explicitly cast to 'uint8_t' when indexing, depending on the platform 'char' might
// be either signed or unsigned, we don't want out array to be indexed at '-71'. While we can reasonably expect
// ASCII encoding on the platfrom (which would put all char literals that we use into the 0-127 range) other chars
// might still be negative. This shouldn't have any runtime cost as trivial int casts like this get compiled into
// the same thing as 'reinterpret_cast<>' which means no runtime logic, the bits are just treated differently.
//
constexpr std::array<char, _number_of_char_values> _lookup_serialized_escaped_chars = [] {
    std::array<char, _number_of_char_values> res{};
    // default-initialized chars get initialized to '\0',
    // ('\0' == 0) is mandated by the standard, which is why we can use it inside an 'if' condition
    res[_u8('"')]  = '"';
    res[_u8('\\')] = '\\';
    // res['/']  = '/'; escaping forward slash in JSON is allowed, but redundant
    res[_u8('\b')] = 'b';
    res[_u8('\f')] = 'f';
    res[_u8('\n')] = 'n';
    res[_u8('\r')] = 'r';
    res[_u8('\t')] = 't';
    return res;
}();

// Lookup table used to determine "insignificant whitespace" characters when
// skipping whitespace during parser. Seems to be either similar or marginally
// faster in performance than a regular condition check.
constexpr std::array<bool, _number_of_char_values> _lookup_whitespace_chars = [] {
    std::array<bool, _number_of_char_values> res{};
    // "Insignificant whitespace" according to the JSON spec:
    // [https://ecma-international.org/wp-content/uploads/ECMA-404.pdf]
    // constitues following symbols:
    // - Whitespace      (aka ' ' )
    // - Tabs            (aka '\t')
    // - Carriage return (aka '\r')
    // - Newline         (aka '\n')
    res[_u8(' ')]  = true;
    res[_u8('\t')] = true;
    res[_u8('\r')] = true;
    res[_u8('\n')] = true;
    return res;
}();

// Lookup table used to get an appropriate char for the escaped char in a 2-char JSON escape sequence.
constexpr std::array<char, _number_of_char_values> _lookup_parsed_escaped_chars = [] {
    std::array<char, _number_of_char_values> res{};
    res[_u8('"')]  = '"';
    res[_u8('\\')] = '\\';
    res[_u8('/')]  = '/';
    res[_u8('b')]  = '\b';
    res[_u8('f')]  = '\f';
    res[_u8('n')]  = '\n';
    res[_u8('r')]  = '\r';
    res[_u8('t')]  = '\t';
    return res;
}();

// ==========================
// --- JSON Parsing impl. ---
// ==========================

inline int _recursion_limit = 1000;

inline void set_recursion_limit(int max_depth) noexcept { _recursion_limit = max_depth; }

struct _parser {
    const std::string& chars;
    int                recursion_depth{};
    // we track recursion depth to handle stack allocation errors
    // (this can be caused malicious inputs with extreme level of nesting, for example, 100k array
    // opening brackets, which would cause huge recursion depth causing the stack to overflow with SIGSEGV)

    // dynamic allocation errors can be handled with regular exceptions through std::bad_alloc

    _parser() = delete;
    _parser(const std::string& chars) : chars(chars) {}

    // Parser state
    std::size_t skip_nonsignificant_whitespace(std::size_t cursor) {
        using namespace std::string_literals;

        while (cursor < this->chars.size()) {
            if (!_lookup_whitespace_chars[_u8(this->chars[cursor])]) return cursor;
            ++cursor;
        }

        throw std::runtime_error("JSON parser reached the end of buffer at pos "s + std::to_string(cursor) +
                                 " while skipping insignificant whitespace segment."s +
                                 _pretty_error(cursor, this->chars));
    }

    // Parsing methods
    std::pair<std::size_t, Node> parse_node(std::size_t cursor) {
        using namespace std::string_literals;

        // Node selector assumes it is starting at a significant symbol
        // which is the first symbol of the node to be parsed

        const char c = this->chars[cursor];

        // Assuming valid JSON, we can determine node type based on a single first character
        if (c == '{') {
            return this->parse_object(cursor);
        } else if (c == '[') {
            return this->parse_array(cursor);
        } else if (c == '"') {
            return this->parse_string(cursor);
        } else if (('0' <= c && c <= '9') || (c == '-')) {
            return this->parse_number(cursor);
        } else if (c == 't') {
            return this->parse_true(cursor);
        } else if (c == 'f') {
            return this->parse_false(cursor);
        } else if (c == 'n') {
            return this->parse_null(cursor);
        }
        throw std::runtime_error("JSON node selector encountered unexpected marker symbol {"s + this->chars[cursor] +
                                 "} at pos "s + std::to_string(cursor) + " (should be one of {0123456789{[\"tfn})."s +
                                 _pretty_error(cursor, this->chars));

        // Note: using a lookup table instead of an 'if' chain doesn't seem to offer any performance benefits here
    }

    std::size_t parse_object_pair(std::size_t cursor, Object& parent) {
        using namespace std::string_literals;

        // Object pair parser assumes it is starting at a '"'

        // Parse pair key
        std::string key; // allocating a string here is fine since we will std::move() into a map key
        std::tie(cursor, key) = this->parse_string(cursor); // may point 'this->current_node' to a new node

        // Handle stuff inbetween
        cursor = this->skip_nonsignificant_whitespace(cursor);
        if (this->chars[cursor] != ':')
            throw std::runtime_error("JSON object node encountered unexpected symbol {"s + this->chars[cursor] +
                                     "} after the pair key at pos "s + std::to_string(cursor) + " (should be {:})."s +
                                     _pretty_error(cursor, this->chars));
        ++cursor; // move past the colon ':'
        cursor = this->skip_nonsignificant_whitespace(cursor);

        // Parse pair value
        if (++this->recursion_depth > _recursion_limit)
            throw std::runtime_error("JSON parser has exceeded maximum allowed recursion depth of "s +
                                     std::to_string(_recursion_limit) +
                                     ". If stated depth wasn't caused by an invalid input, "s +
                                     "recursion limit can be increased with json::set_recursion_limit()."s);

        Node value;
        std::tie(cursor, value) = this->parse_node(cursor);

        --this->recursion_depth;

        // Note 1:
        // The question of wheter JSON allows duplicate keys is non-trivial but the resulting answer is NO.
        // JSON is goverened by 2 standards:
        // 1) ECMA-404 https://ecma-international.org/wp-content/uploads/ECMA-404.pdf
        //    which doesn't say anything about duplicate kys
        // 2) RFC-8259 https://www.rfc-editor.org/rfc/rfc2119
        //    which states "The names within an object SHOULD be unique.",
        //    however as defined in RFC-2119 https://www.rfc-editor.org/rfc/rfc2119:
        //       "SHOULD This word, or the adjective "RECOMMENDED", mean that there may exist valid reasons in
        //       particular circumstances to ignore a particular item, but the full implications must be understood
        //       and carefully weighed before choosing a different course."
        // which means at the end of the day duplicate keys are discouraged but still valid

        // Note 2:
        // There is no standard specification on which JSON value should be prefered in case of duplicate keys.
        // This is considered implementation detail as per RFC-8259:
        //    "An object whose names are all unique is interoperable in the sense that all software implementations
        //    receiving that object will agree on the name-value mappings. When the names within an object are not
        //    unique, the behavior of software that receives such an object is unpredictable. Many implementations
        //    report the last name/value pair only. Other implementations report an error or fail to parse the object,
        //    and some implementations report all of the name/value pairs, including duplicates."

        // Note 3:
        // We could easily check for duplicate keys since 'std::map<>::emplace()' returns insertion success as a bool
        // (the same isn't true for 'std::map<>::emplace_hint()' which returns just the iterator), however we will
        // not since that goes against the standard

        // Note 4:
        // 'parent.emplace_hint(parent.end(), ...)' can drastically speed up parsing of sorted JSON object, however
        // since most JSONs in the wild aren't sorted we will resort to a more generic option of regular '.emplace()'
        parent.emplace(std::move(key), std::move(value));

        return cursor;
    }

    std::pair<std::size_t, Object> parse_object(std::size_t cursor) {
        using namespace std::string_literals;

        ++cursor; // move past the opening brace '{'

        // Empty object that will accumulate child nodes as we parse them
        Object object_value;

        // Handle 1st pair
        cursor = this->skip_nonsignificant_whitespace(cursor);
        if (this->chars[cursor] != '}') {
            cursor = this->parse_object_pair(cursor, object_value);
        } else {
            ++cursor; // move past the closing brace '}'
            return {cursor, std::move(object_value)};
        }

        // Handle other pairs

        // Since we are staring past the first pair, all following pairs are gonna be preceded by a comma.
        //
        // Strictly speaking, commas in objects aren't necessary for decoding a JSON, this is
        // a case of redundant information, included into the format to make it more human-readable.
        // { "key_1":1 "key_1":2 "key_3":"value" } <- enough information to parse without commas.
        //
        // However commas ARE in fact necessary for array parsing. By using commas to detect when we have
        // to parse another pair, we can reuse the same algorithm for both objects pairs and array elements.
        //
        // Doing so also has a benefit of inherently adding comma-presense validation which we would have
        // to do manually otherwise.

        while (cursor < this->chars.size()) {
            cursor       = this->skip_nonsignificant_whitespace(cursor);
            const char c = this->chars[cursor];

            if (c == ',') {
                ++cursor; // move past the comma ','
                cursor = this->skip_nonsignificant_whitespace(cursor);
                cursor = this->parse_object_pair(cursor, object_value);
            } else if (c == '}') {
                ++cursor; // move past the closing brace '}'
                return {cursor, std::move(object_value)};
            } else {
                throw std::runtime_error(
                    "JSON array node could not find comma {,} or object ending symbol {}} after the element at pos "s +
                    std::to_string(cursor) + "."s + _pretty_error(cursor, this->chars));
            }
        }

        throw std::runtime_error("JSON object node reached the end of buffer while parsing object contents." +
                                 _pretty_error(cursor, this->chars));
    }

    std::size_t parse_array_element(std::size_t cursor, Array& parent) {
        using namespace std::string_literals;

        // Array element parser assumes it is starting at the first symbol of some JSON node

        // Parse pair key
        if (++this->recursion_depth > _recursion_limit)
            throw std::runtime_error("JSON parser has exceeded maximum allowed recursion depth of "s +
                                     std::to_string(_recursion_limit) +
                                     ". If stated depth wasn't caused by an invalid input, "s +
                                     "recursion limit can be increased with json::set_recursion_limit()."s);

        Node value;
        std::tie(cursor, value) = this->parse_node(cursor);

        --this->recursion_depth;

        parent.emplace_back(std::move(value));

        return cursor;
    }

    std::pair<std::size_t, Array> parse_array(std::size_t cursor) {
        using namespace std::string_literals;

        ++cursor; // move past the opening bracket '['

        // Empty object that will accumulate child nodes as we parse them
        Array array_value;

        // Handle 1st pair
        cursor = this->skip_nonsignificant_whitespace(cursor);
        if (this->chars[cursor] != ']') {
            cursor = this->parse_array_element(cursor, array_value);
        } else {
            ++cursor; // move past the closing bracket ']'
            return {cursor, std::move(array_value)};
        }

        // Handle other pairs
        // (the exact same way we do with objects, see the note here)
        while (cursor < this->chars.size()) {
            cursor       = this->skip_nonsignificant_whitespace(cursor);
            const char c = this->chars[cursor];

            if (c == ',') {
                ++cursor; // move past the comma ','
                cursor = this->skip_nonsignificant_whitespace(cursor);
                cursor = this->parse_array_element(cursor, array_value);
            } else if (c == ']') {
                ++cursor; // move past the closing bracket ']'
                return {cursor, std::move(array_value)};
            } else {
                throw std::runtime_error(
                    "JSON array node could not find comma {,} or array ending symbol {]} after the element at pos "s +
                    std::to_string(cursor) + "."s + _pretty_error(cursor, this->chars));
            }
        }

        throw std::runtime_error("JSON array node reached the end of buffer while parsing object contents." +
                                 _pretty_error(cursor, this->chars));
    }

    inline void parse_unicode_codepoint_from_hex(std::size_t cursor, std::string& string_value) {
        using namespace std::string_literals;

        if (cursor >= this->chars.size() + 4)
            throw std::runtime_error("JSON string node reached the end of buffer while"s +
                                     "parsing a 5-character escape sequence at pos "s + std::to_string(cursor) + "."s +
                                     _pretty_error(cursor, this->chars));
        // Standard library is absolutely HORRIBLE when it comes to Unicode support.
        // Literally every single encoding function in <cuchar>/<string>/<codecvt> is a
        // crime against common sense, API safety and performace, which is why we do this
        // inefficient nonsense and pray that there's not gonna be a lot of escaped unicode
        // in the data. This also adds another 2 #include's just by itself. Supports UTF-8.
        const std::string hex(this->chars.data() + cursor + 1, 4);
        // standard functions only support std::string or null-terminated char*,
        // there's no way (that I'm aware of) to view into the data and parse it without making a copy,
        // thankfully a string of 4 characters should always fit into the SSO buffer
        const char32_t    unicode_char = std::stoul(hex, nullptr, 16);
        if (!_unicode_codepoint_to_utf8(string_value, unicode_char))
            throw std::runtime_error("JSON string node could not parse unicode codepoint {"s + hex +
                                     "} while parsing an escape sequence at pos "s + std::to_string(cursor) + "."s +
                                     _pretty_error(cursor, this->chars));
    }

    std::pair<std::size_t, String> parse_string(std::size_t cursor) {
        using namespace std::string_literals;

        // Empty string that will accumulate characters as we parse them
        std::string string_value;

        ++cursor; // move past the opening quote '"'

        // Serialize string while handling escape sequences.
        //
        // Doing 'string_value += c' for every char is ~50-60% slower than appending whole string at once,
        // which is why we 'buffer' appends by keeping track of 'segment_start' and 'cursor', and appending
        // whole chunks of the buffer to 'string_value' when we encounter an escape sequence or end of the string.
        //
        for (std::size_t segment_start = cursor; cursor < this->chars.size(); ++cursor) {
            const char c = this->chars[cursor];

            // Reached the end of the string
            if (c == '"') {
                string_value.append(this->chars.data() + segment_start, cursor - segment_start);
                ++cursor; // move past the closing quote '"'
                return {cursor, std::move(string_value)};
            }
            // Handle escape sequences inside the string
            else if (c == '\\') {
                ++cursor; // move past the backslash '\'

                string_value.append(this->chars.data() + segment_start, cursor - segment_start - 1);
                // can't buffer more than that since we have to insert special characters now.

                const char escaped_char = this->chars[cursor];

                // 2-character escape sequences
                if (const char replacement_char = _lookup_parsed_escaped_chars[_u8(escaped_char)]) {
                    if (cursor >= this->chars.size())
                        throw std::runtime_error("JSON string node reached the end of buffer while"s +
                                                 "parsing a 2-character escape sequence at pos "s +
                                                 std::to_string(cursor) + "."s + _pretty_error(cursor, this->chars));
                    string_value += replacement_char;
                }
                // 6-character escape sequences (escaped unicode HEX codepoints)
                else if (escaped_char == 'u') {
                    parse_unicode_codepoint_from_hex(cursor, string_value);
                    cursor += 4; // move past first 'uXXX' symbols, last symbol will be covered by the loop '++cursor'
                } else {
                    throw std::runtime_error("JSON string node encountered unexpected character {"s + escaped_char +
                                             "} while parsing an escape sequence at pos "s + std::to_string(cursor) +
                                             "."s + _pretty_error(cursor, this->chars));
                }

                // This covers all non-hex escape sequences according to ECMA-404 specification
                // [https://ecma-international.org/wp-content/uploads/ECMA-404.pdf] (page 4)

                // moving past the escaped character will be done by the loop '++cursor'
                segment_start = cursor + 1;
                continue;
            }
            // Reject unescaped control characters (codepoints U+0000 to U+001F)
            else if (_u8(c) <= 31)
                throw std::runtime_error(
                    "JSON string node encountered unescaped ASCII control character character \\"s +
                    std::to_string(static_cast<int>(c)) + " at pos "s + std::to_string(cursor) + "."s +
                    _pretty_error(cursor, this->chars));
        }

        throw std::runtime_error("JSON string node reached the end of buffer while parsing string contents." +
                                 _pretty_error(cursor, this->chars));
    }

    std::pair<std::size_t, Number> parse_number(std::size_t cursor) {
        using namespace std::string_literals;

        Number number_value;

        const auto [numer_end_ptr, error_code] =
            std::from_chars(this->chars.data() + cursor, this->chars.data() + this->chars.size(), number_value);

        // Note:
        // std::from_chars() converts the first complete number it finds in the string,
        // for example "42 meters" would be converted to 42. We rely on that behaviour here.

        if (error_code != std::errc()) {
            // std::errc(0) is a valid enumeration value that represents success
            // even though it does not appear in the enumerator list (which starts at 1)
            if (error_code == std::errc::invalid_argument)
                throw std::runtime_error("JSON number node could not be parsed as a number at pos "s +
                                         std::to_string(cursor) + "."s + _pretty_error(cursor, this->chars));
            else if (error_code == std::errc::result_out_of_range)
                throw std::runtime_error(
                    "JSON number node parsed to number larger than its possible binary representation at pos "s +
                    std::to_string(cursor) + "."s + _pretty_error(cursor, this->chars));
        }

        return {numer_end_ptr - this->chars.data(), number_value};
    }

    std::pair<std::size_t, Bool> parse_true(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 4;

        if (cursor + token_length > this->chars.size())
            throw std::runtime_error("JSON bool node reached the end of buffer while parsing {true}." +
                                     _pretty_error(cursor, this->chars));

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 't' && //
            this->chars[cursor + 1] == 'r' && //
            this->chars[cursor + 2] == 'u' && //
            this->chars[cursor + 3] == 'e';   //

        if (!parsed_correctly)
            throw std::runtime_error("JSON bool node could not parse {true} at pos "s + std::to_string(cursor) + "."s +
                                     _pretty_error(cursor, this->chars));

        return {cursor + token_length, Bool(true)};
    }

    std::pair<std::size_t, Bool> parse_false(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 5;

        if (cursor + token_length > this->chars.size())
            throw std::runtime_error("JSON bool node reached the end of buffer while parsing {false}." +
                                     _pretty_error(cursor, this->chars));

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 'f' && //
            this->chars[cursor + 1] == 'a' && //
            this->chars[cursor + 2] == 'l' && //
            this->chars[cursor + 3] == 's' && //
            this->chars[cursor + 4] == 'e';   //

        if (!parsed_correctly)
            throw std::runtime_error("JSON bool node could not parse {false} at pos "s + std::to_string(cursor) + "."s +
                                     _pretty_error(cursor, this->chars));

        return {cursor + token_length, Bool(false)};
    }

    std::pair<std::size_t, Null> parse_null(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 4;

        if (cursor + token_length > this->chars.size())
            throw std::runtime_error("JSON null node reached the end of buffer while parsing {null}." +
                                     _pretty_error(cursor, this->chars));

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 'n' && //
            this->chars[cursor + 1] == 'u' && //
            this->chars[cursor + 2] == 'l' && //
            this->chars[cursor + 3] == 'l';   //

        if (!parsed_correctly)
            throw std::runtime_error("JSON null node could not parse {null} at pos "s + std::to_string(cursor) + "."s +
                                     _pretty_error(cursor, this->chars));

        return {cursor + token_length, Null()};
    }
};


// ==============================
// --- JSON Serializing impl. ---
// ==============================

template <bool prettify>
inline void _serialize_json_recursion(const Node& node, std::string& chars, unsigned int indent_level = 0,
                                      bool skip_first_indent = false) {
    using namespace std::string_literals;
    constexpr std::size_t indent_level_size = 4;
    const std::size_t     indent_size       = indent_level_size * indent_level;

    // first indent should be skipped when printing after a key
    //
    // Example:
    //
    // {
    //     "object": {              <- first indent skipped (Object)
    //         "something": null    <- first indent skipped (Null)
    //     },
    //     "array": [               <- first indent skipped (Array)
    //          1,                  <- first indent NOT skipped (Number)
    //          2                   <- first indent NOT skipped (Number)
    //     ]
    // }
    //
    // We handle 'prettify' segments through 'if constexpr'
    // to avoid  any "trace" overhead on non-prettified serializing
    //

    // Note:
    // The fastest way to append strings to a preallocated buffer seems to be with '+=':
    //    > chars += string_1; chars += string_2; chars += string_3;
    //
    // Using operator '+' slows things down due to additional allocations:
    //    > chars +=  string_1 + string_2 + string_3; // slow
    //
    // '.append()' performs exactly the same as '+=', but has no overload for appending single chars.
    // However it does have an overload for appending N of some character, which is why we use if for indentation.
    //
    // 'std::ostringstream' is painfully slow compared to regular appends
    // so it's out of the question.

    if constexpr (prettify)
        if (!skip_first_indent) chars.append(indent_size, ' ');

    // JSON Object
    if (auto* ptr = node.get_if<Object>()) {
        const auto& object_value = *ptr;

        // Skip all logic for empty objects
        if (object_value.empty()) {
            chars += "{}";
            return;
        }

        chars += '{';
        if constexpr (prettify) chars += '\n';

        for (auto it = object_value.cbegin();;) {
            if constexpr (prettify) chars.append(indent_size + indent_level_size, ' ');
            // Key
            chars += '"';
            chars += it->first;
            if constexpr (prettify) chars += "\": ";
            else chars += "\":";
            // Value
            _serialize_json_recursion<prettify>(it->second, chars, indent_level + 1, true);
            // Comma
            if (++it != object_value.cend()) { // prevents trailing comma
                chars += ',';
                if constexpr (prettify) chars += '\n';
            } else {
                if constexpr (prettify) chars += '\n';
                break;
            }
        }

        if constexpr (prettify) chars.append(indent_size, ' ');
        chars += '}';
    }
    // JSON Array
    else if (auto* ptr = node.get_if<Array>()) {
        const auto& array_value = *ptr;

        // Skip all logic for empty arrays
        if (array_value.empty()) {
            chars += "[]";
            return;
        }

        chars += '[';
        if constexpr (prettify) chars += '\n';

        for (auto it = array_value.cbegin();;) {
            // Node
            _serialize_json_recursion<prettify>(*it, chars, indent_level + 1);
            // Comma
            if (++it != array_value.cend()) { // prevents trailing comma
                chars += ',';
                if constexpr (prettify) chars += '\n';
            } else {
                if constexpr (prettify) chars += '\n';
                break;
            }
        }
        if constexpr (prettify) chars.append(indent_size, ' ');
        chars += ']';
    }
    // String
    else if (auto* ptr = node.get_if<String>()) {
        const auto& string_value = *ptr;

        chars += '"';

        // Serialize string while handling escape sequences.
        /// Without escape sequences we could just do 'chars += string_value'.
        //
        // Since appending individual characters is ~twice as slow as appending the whole string, we use a
        // "buffered" way of appending, appending whole segments up to the currently escaped char.
        // Strings with no escaped chars get appended in a single call.
        //
        std::size_t segment_start = 0;
        for (std::size_t i = 0; i < string_value.size(); ++i) {
            if (const char escaped_char_replacement = _lookup_serialized_escaped_chars[_u8(string_value[i])]) {
                chars.append(string_value.data() + segment_start, i - segment_start);
                chars += '\\';
                chars += escaped_char_replacement;
                segment_start = i + 1; // skip over the "actual" technical character in the string
            }
        }
        chars.append(string_value.data() + segment_start, string_value.size() - segment_start);

        chars += '"';
    }
    // Number
    else if (auto* ptr = node.get_if<Number>()) {
        const auto& number_value = *ptr;

        constexpr int max_exponent = std::numeric_limits<Number>::max_exponent10;
        constexpr int max_digits =
            4 + std::numeric_limits<Number>::max_digits10 + std::max(2, _log_10_ceil(max_exponent));
        // should be the smallest buffer size to account for all possible 'std::to_chars()' outputs,
        // see [https://stackoverflow.com/questions/68472720/stdto-chars-minimal-floating-point-buffer-size]

        std::array<char, max_digits> buffer;

        const auto [number_end_ptr, error_code] =
            std::to_chars(buffer.data(), buffer.data() + buffer.size(), number_value);

        if (error_code != std::errc())
            throw std::runtime_error(
                "JSON serializing encountered std::to_chars() formatting error while serializing value {"s +
                std::to_string(number_value) + "}."s);

        const std::string_view number_string(buffer.data(), number_end_ptr - buffer.data());

        // Save NaN/Inf cases as strings, since JSON spec doesn't include IEEE 754.
        // (!) May result in non-homogenous arrays like [ 1.0, "inf" , 3.0, 4.0, "nan" ]
        if (std::isfinite(number_value)) {
            chars.append(buffer.data(), number_end_ptr - buffer.data());
        } else {
            chars += '"';
            chars.append(buffer.data(), number_end_ptr - buffer.data());
            chars += '"';
        }
    }
    // Bool
    else if (auto* ptr = node.get_if<Bool>()) {
        const auto& bool_value = *ptr;
        chars += (bool_value ? "true" : "false");
    }
    // Null
    else if (node.is<Null>()) {
        chars += "null";
    }
}

inline void _serialize_json_to_buffer(std::string& chars, const Node& node, Format format) {
    if (format == Format::PRETTY) _serialize_json_recursion<true>(node, chars);
    else _serialize_json_recursion<false>(node, chars);
}

// ===============================
// --- JSON Parsing public API ---
// ===============================

[[nodiscard]] inline Node from_string(const std::string& chars) {
    _parser           parser(chars);
    const std::size_t json_start = parser.skip_nonsignificant_whitespace(0); // skip leading whitespace
    auto [end_cursor, node]      = parser.parse_node(json_start); // starts parsing recursively from the root node
    // Check for invalid trailing sumbols

    using namespace std::string_literals;
    for (auto cursor = end_cursor; cursor < chars.size(); ++cursor)
        if (!_lookup_whitespace_chars[_u8(chars[cursor])])
            throw std::runtime_error("Invalid trailing symbols encountered after the root JSON node at pos "s +
                                     std::to_string(cursor) + "."s + _pretty_error(cursor, chars));

    return std::move(node); // implicit tuple blocks copy elision, we have to move() manually
}
[[nodiscard]] inline Node from_file(const std::string& filepath) {
    const std::string chars = _read_file_to_string(filepath);
    return from_string(chars);
}

namespace literals {
[[nodiscard]] inline Node operator""_utl_json(const char* c_str, std::size_t c_str_size) {
    return from_string(std::string(c_str, c_str_size));
}
} // namespace literals

// ============================
// --- Structure reflection ---
// ============================

// --- from-struct utils ---
// -------------------------

template <class T>
constexpr bool _is_reflected_struct = false;
// this trait allows us to "mark" all reflected struct types, we use it to handle nested classes
// and call 'to_struct()' / 'from_struct()' recursively whenever necessary

template <class T>
[[nodiscard]] utl::json::Node from_struct(const T&) {
    static_assert(_always_false_v<T>,
                  "Provided type doesn't have a defined JSON reflection. Use 'UTL_JSON_REFLECT' macro to define one.");
    // compile-time protection against calling 'from_struct()' on types that don't have reflection,
    // we can also provide a proper error message here
    return {};
    // this is needed to silence "no return in a function" warning that appears even if this specialization
    // (which by itself should cause a compile error) doesn't get compiled
}

template <class T>
void _assign_value_to_node(Node& node, const T& value) {
    if constexpr (is_json_convertible_v<T>) node = value;
    // it is critical that the trait above performs DEEP check for JSON convertability and not a shallow one,
    // we want to detect things like 'std::vector<int>' as convertible, but not things like 'std::vector<MyStruct>',
    // these should expand over their element type / mapped type further until either they either reach
    // the reflected 'MyStruct' or end up on a dead end, which means an impossible conversion
    else if constexpr (_is_reflected_struct<T>) node = from_struct(value);
    else if constexpr (is_object_like_v<T>) {
        node = Object{};
        for (const auto& [key, val] : value) {
            Node single_node;
            _assign_value_to_node(single_node, val);
            node.get_object().emplace(key, std::move(single_node));
        }
    } else if constexpr (is_array_like_v<T>) {
        node = Array{};
        for (const auto& elem : value) {
            Node single_node;
            _assign_value_to_node(single_node, elem);
            node.get_array().emplace_back(std::move(single_node));
        }
    } else static_assert(_always_false_v<T>, "Could not resolve recursive conversion from 'T' to 'json::Node'.");
}

#define utl_json_from_struct_assign(fieldname_) _assign_value_to_node(json[#fieldname_], val.fieldname_);

// --- to-struct utils ---
// -----------------------

// Assigning JSON node to a value for arbitrary type is a bit of an "incorrect" problem,
// since we can't possinly know the API of the type we're assigning stuff to.
// Object-like and array-like types need special handling that expands their nodes recursively,
// we can't directly assign 'std::vector<Node>' to 'std::vector<double>' like we would we simpler types.
template <class T>
void _assign_node_to_value_recursively(T& value, const Node& node) {
    if constexpr (is_string_like_v<T>) value = node.get_string();
    else if constexpr (is_object_like_v<T>) {
        const auto object = node.get_object();
        for (const auto& [key, val] : object) _assign_node_to_value_recursively(value[key], val);
    } else if constexpr (is_array_like_v<T>) {
        const auto array = node.get_array();
        value.resize(array.size());
        for (std::size_t i = 0; i < array.size(); ++i) _assign_node_to_value_recursively(value[i], array[i]);
    } else if constexpr (is_bool_like_v<T>) value = node.get_bool();
    else if constexpr (is_null_like_v<T>) value = node.get_null();
    else if constexpr (is_numeric_like_v<T>) value = node.get_number();
    else if constexpr (_is_reflected_struct<T>) value = node.to_struct<T>();
    else static_assert(_always_false_v<T>, "Method is a non-exhaustive visitor of std::variant<>.");
}

// Not sure how to generically handle array-like types with compile-time known size,
// so we're just gonna make a special case for 'std::array'
template <class T, std::size_t N>
void _assign_node_to_value_recursively(std::array<T, N>& value, const Node& node) {
    using namespace std::string_literals;

    const auto array = node.get_array();

    if (array.size() != value.size())
        throw std::runtime_error("JSON to structure serializer encountered non-mathing std::array size of "s +
                                 std::to_string(value.size()) + ", corresponding node has a size of "s +
                                 std::to_string(array.size()) + "."s);

    for (std::size_t i = 0; i < array.size(); ++i) _assign_node_to_value_recursively(value[i], array[i]);
}

#define utl_json_to_struct_assign(fieldname_) _assign_node_to_value_recursively(val.fieldname_, this->at(#fieldname_));

// --- Codegen ---
// ---------------

#define UTL_JSON_REFLECT(struct_name_, ...)                                                                            \
                                                                                                                       \
    template <>                                                                                                        \
    constexpr bool utl::json::_is_reflected_struct<struct_name_> = true;                                               \
                                                                                                                       \
    template <>                                                                                                        \
    utl::json::Node utl::json::from_struct<struct_name_>(const struct_name_& val) {                                    \
        utl::json::Node json;                                                                                          \
        /* map 'json["<FIELDNAME>"] = val.<FIELDNAME>;' */                                                             \
        utl_json_map(utl_json_from_struct_assign, __VA_ARGS__);                                                        \
        return json;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    template <>                                                                                                        \
    auto utl::json::Node::to_struct<struct_name_>() const->struct_name_ {                                              \
        struct_name_ val;                                                                                              \
        /* map 'val.<FIELDNAME> = this->at("<FIELDNAME>").get<decltype(val.<FIELDNAME>)>();' */                        \
        utl_json_map(utl_json_to_struct_assign, __VA_ARGS__);                                                          \
        return val;                                                                                                    \
    }                                                                                                                  \
                                                                                                                       \
    static_assert(true)


} // namespace utl::json

#endif
#endif // module utl::json