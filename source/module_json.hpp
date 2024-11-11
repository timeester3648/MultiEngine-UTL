// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::json
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_json.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
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
#include <cstddef>          // size_t
#include <fstream>          // ifstream, ofstream
#include <initializer_list> // initializer_list<>
#include <limits>           // numeric_limits<>::max_digits10, numeric_limits<>::max_exponent10
#include <map>              // map<>
#include <stdexcept>        // runtime_error
#include <string>           // string
#include <string_view>      // string_view
#include <system_error>     // errc()
#include <type_traits>      // enable_if_t<>, void_t, is_convertible_v<>, is_same_v<>
#include <utility>          // move(), declval<>()
#include <variant>          // variant<>
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS
// TEMP:
// #include "../source/MACRO_PROFILER.hpp"

// ____________________ IMPLEMENTATION ____________________

namespace utl::json {

// ===================
// --- Misc. Utils ---
// ===================

inline std::string _read_file_to_string(const std::string& path) {
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

template <typename T>
constexpr int _log_10_ceil(T num) {
    return num < 10 ? 1 : 1 + _log_10_ceil(num / 10);
}

// Workaround for 'static_assert(false)' making program ill-formed even
// when placed inide an 'if constexpr' branch that never compiles.
// 'static_assert(_always_false_v<T)' on the the other hand doesn't,
// which means we can use it to mark branches that should never compile.
template <class>
inline constexpr bool _always_false_v = false;

// Type trait that checks existence of '.begin()', 'end()' members
template <typename Type, typename = void, typename = void>
struct _is_const_iterable_through : std::false_type {};

template <typename Type>
struct _is_const_iterable_through<Type, std::void_t<decltype(++std::declval<Type>().begin())>,
                                  std::void_t<decltype(std::declval<Type>().end())>> : std::true_type {};

// Type trait that checks existence of '::key_type', '::mapped_type' members
template <typename Type, typename = void, typename = void>
struct _is_assotiative : std::false_type {};

template <typename Type>
struct _is_assotiative<Type, std::void_t<typename Type::key_type>, std::void_t<typename Type::mapped_type>>
    : std::true_type {};

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
class _null_type_impl {};

template <class T>
constexpr bool is_object_like_v = _is_const_iterable_through<T>::value && _is_assotiative<T>::value;
// NOTE: Also check for 'key_type' being convertible to 'std::string'
template <class T>
constexpr bool is_array_like_v = _is_const_iterable_through<T>::value;
template <class T>
constexpr bool is_string_like_v = std::is_convertible_v<T, std::string_view>;
template <class T>
constexpr bool is_numeric_like_v = std::is_convertible_v<T, _number_type_impl>;
template <class T>
constexpr bool is_bool_like_v = std::is_same_v<T, _bool_type_impl>;
template <class T>
constexpr bool is_null_like_v = std::is_same_v<T, _null_type_impl>;

template <class T>
constexpr bool is_json_type_convertible_v = is_object_like_v<T> || is_array_like_v<T> || is_string_like_v<T> ||
                                            is_numeric_like_v<T> || is_bool_like_v<T> || is_null_like_v<T>;

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
        if (it == object.end()) it = object.insert(std::make_pair(std::string(key), Node{})).first;
        return it->second;
    }

    [[nodiscard]] const Node& operator[](std::string_view key) const {
        // 'std::map<K, V>::operator[]()' and 'std::map<K, V>::at()' don't support
        // support heterogeneous lookup, we have to reimplement them manually
        const auto& object = this->get_object();
        const auto  it     = object.find(key);
        if (it == object.end()) throw std::runtime_error("Accessing non-existent key in JSON object.");
        return it->second;
    }

    [[nodiscard]] Node& at(std::string_view key) {
        // Non-const 'operator[]' inserts non-existent keys, '.at()' should throw instead
        auto&      object = this->get_object();
        const auto it     = object.find(key);
        if (it == object.end()) throw std::runtime_error("Accessing non-existent key in JSON object.");
        return it->second;
    }

    [[nodiscard]] const Node& at(std::string_view key) const { return this->operator[](key); }

    [[nodiscard]] bool contains(std::string_view key) const {
        const auto& object = this->get_object();
        const auto  it     = object.find(key);
        return it != object.end();
    }
    
    template<class T>
    [[nodiscard]] T& value_or(std::string_view key, const T &else_value) {
        const auto& object = this->get_object();
        const auto  it     = object.find(key);
        if (it != object.end()) return it->second.get<T>();
        return else_value;
        // same thing as 'this->contains(key) ? json.at(key).get<T>() : else_value' but without a second map lookup
    }

    // -- Assignment --
    // ----------------

    // Converting assignment
    template <class T, std::enable_if_t<
                           !std::is_same_v<std::decay_t<T>, Node> && !std::is_same_v<std::decay_t<T>, object_type> &&
                               !std::is_same_v<std::decay_t<T>, array_type> &&
                               !std::is_same_v<std::decay_t<T>, string_type> && is_json_type_convertible_v<T>,
                           bool> = true>
    Node& operator=(const T& value) {
        // Don't take types that decay to Node/object/array/string to prevent
        // shadowing native copy/move assignment for those types

        constexpr bool is_object_like  = _is_const_iterable_through<T>::value && _is_assotiative<T>::value;
        constexpr bool is_array_like   = _is_const_iterable_through<T>::value;
        constexpr bool is_string_like  = std::is_convertible_v<T, std::string_view>;
        constexpr bool is_numeric_like = std::is_convertible_v<T, number_type>;
        constexpr bool is_bool_like    = std::is_same_v<T, bool_type>;
        constexpr bool is_null_like    = std::is_same_v<T, null_type>;

        // Several "type-like' characteristics can be true at the same time,
        // to resolve ambiguity we assign the following conversion priority:
        // string > object > array > bool > null > numeric

        if constexpr (is_string_like) {
            this->data.emplace<string_type>(value);
        } else if constexpr (is_object_like) {
            this->data.emplace<object_type>();
            auto& object = this->get_object();
            for (const auto& [key, val] : value) object[key] = val;
        } else if constexpr (is_array_like) {
            this->data.emplace<array_type>();
            auto& array = this->get_array();
            for (const auto& elem : value) array.emplace_back(elem);
        } else if constexpr (is_bool_like) {
            this->data.emplace<bool_type>(value);
        } else if constexpr (is_null_like) {
            this->data.emplace<null_type>(value);
        } else if constexpr (is_numeric_like) {
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

    // -- Constructors --
    // ------------------

    Node& operator=(const Node&) = default;
    Node& operator=(Node&&)      = default;

    Node()            = default;
    Node(const Node&) = default;
    Node(Node&&)      = default;

    // Converting ctor
    template <class T, std::enable_if_t<
                           !std::is_same_v<std::decay_t<T>, Node> && !std::is_same_v<std::decay_t<T>, object_type> &&
                               !std::is_same_v<std::decay_t<T>, array_type> &&
                               !std::is_same_v<std::decay_t<T>, string_type> && is_json_type_convertible_v<T>,
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

    [[nodiscard]] std::string to_string(Format format = Format::PRETTY) const {
        std::string buffer;
        _serialize_json_to_buffer(buffer, *this, format);
        return buffer;
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

constexpr std::size_t _number_of_char_values =
    256; // always true since 'sizeof(char) == 1' is guaranteed by the standard

// Lookup table used to check if number should be escaped and get a replacement char on at the same time.
// This allows us to replace multiple checks and if's with a single array lookup that.
//
// Instead of:
//    if (c == '"') { chars += '"' }
//    ...
//    else if (c == '\t') { chars += 't' }
// we get:
//    if (const char replacement = _lookup_serialized_escaped_chars[c]) { chars += replacement; }
//
// which ends up being quite noticeably faster.
//
constexpr std::array<char, _number_of_char_values> _lookup_serialized_escaped_chars = [] {
    std::array<char, _number_of_char_values> res{};
    // default-initialized chars get initialized to '\0',
    // as far as I understand ('\0' == 0) is mandated by the standard,
    // which is why we can use it inside an 'if' condition
    res['"']  = '"';
    res['\\'] = '\\';
    // res['/']  = '/'; escaping forward slash in JSON is allowed, but redundant
    res['\b'] = 'b';
    res['\f'] = 'f';
    res['\n'] = 'n';
    res['\r'] = 'r';
    res['\t'] = 't';
    return res;
}();

// Lookup table used to select appropriate node parsing methods based on the first character.
// This allows us to replace an if-else chain with a ('0' < c && c < '9') check with a single
// array lookup that selects an option in switch-case. Writing switch-case from the get go is
// also an option, but it would require either code duplication or fallthrough on '0', ..., '0',
// '-' and also 't', 'f' which seems suboptimal.
//
enum class _json_node_selector_branch {
    NONE = 0,
    OBJECT_BRANCH,
    ARRAY_BRANCH,
    STRING_BRANCH,
    NUMBER_BRANCH,
    BOOL_TRUE_BRANCH,
    BOOL_FALSE_BRANCH,
    NULL_BRANCH
};

constexpr std::array<_json_node_selector_branch, _number_of_char_values> _lookup_node_selector_branch = [] {
    std::array<_json_node_selector_branch, _number_of_char_values> res{};
    // default-initialized enums get initialized to 'static_cast<EnumName>(0)',
    // this is why we need 'NONE = 0' enum member for cases that aren't handled by the selector
    res['{'] = _json_node_selector_branch::OBJECT_BRANCH;
    res['['] = _json_node_selector_branch::ARRAY_BRANCH;
    res['"'] = _json_node_selector_branch::STRING_BRANCH;
    res['0'] = _json_node_selector_branch::NUMBER_BRANCH;
    res['1'] = _json_node_selector_branch::NUMBER_BRANCH;
    res['2'] = _json_node_selector_branch::NUMBER_BRANCH;
    res['3'] = _json_node_selector_branch::NUMBER_BRANCH;
    res['4'] = _json_node_selector_branch::NUMBER_BRANCH;
    res['5'] = _json_node_selector_branch::NUMBER_BRANCH;
    res['6'] = _json_node_selector_branch::NUMBER_BRANCH;
    res['7'] = _json_node_selector_branch::NUMBER_BRANCH;
    res['8'] = _json_node_selector_branch::NUMBER_BRANCH;
    res['9'] = _json_node_selector_branch::NUMBER_BRANCH;
    res['-'] = _json_node_selector_branch::NUMBER_BRANCH;
    res['t'] = _json_node_selector_branch::BOOL_TRUE_BRANCH;
    res['f'] = _json_node_selector_branch::BOOL_FALSE_BRANCH;
    res['n'] = _json_node_selector_branch::NULL_BRANCH;
    return res;
}();

constexpr std::array<bool, _number_of_char_values> _lookup_whitespace_chars = [] {
    std::array<bool, _number_of_char_values> res{};
    // "Insignificant whitespace" according to the JSON spec:
    // [https://ecma-international.org/wp-content/uploads/ECMA-404.pdf]
    // constitues following symbols:
    // - Whitespace      (aka ' ' )
    // - Tabs            (aka '\t')
    // - Carriage return (aka '\r')
    // - Newline         (aka '\n')
    res[' ']  = true;
    res['\t'] = true;
    res['\r'] = true;
    res['\n'] = true;
    return res;
}();

// Lookup table used to get an appropriate char for the escaped char in a 2-char JSON escape sequence.
// Allows us to avoid a chain of 8 if-else'es which ends up beings faster.
constexpr std::array<char, _number_of_char_values> _lookup_parsed_escaped_chars = [] {
    std::array<char, _number_of_char_values> res{};
    res['"']  = '"';
    res['\\'] = '\\';
    res['/']  = '/';
    res['b']  = '\b';
    res['f']  = '\f';
    res['n']  = '\n';
    res['r']  = '\r';
    res['t']  = '\t';
    return res;
}();

// ====================
// --- JSON Parsing ---
// ====================

struct _parser {
    const std::string& chars;

    _parser() = delete;
    _parser(const std::string& chars) : chars(chars) {}

    // Parser state
    std::size_t skip_nonsignificant_whitespace(std::size_t cursor) {
        using namespace std::string_literals;

        while (cursor < this->chars.size()) {
            if (!_lookup_whitespace_chars[this->chars[cursor]]) return cursor;
            ++cursor;
        }

        throw std::runtime_error("JSON parser reached the end of buffer at pos "s + std::to_string(cursor) +
                                 " while skipping insignificant whitespace segment."s);
    }

    // Parsing methods
    std::pair<std::size_t, Node> parse_node(std::size_t cursor) {
        using namespace std::string_literals;

        // Node selector assumes it is starting at a significant symbol
        // which is the first symbol of the node to be parsed

        const char c = this->chars[cursor];

        // Assuming valid JSON, we can determine node type based on a single first character
        // switch (_lookup_node_selector_branch[c]) {
        // case _json_node_selector_branch::OBJECT_BRANCH: return this->parse_object(cursor);
        // case _json_node_selector_branch::ARRAY_BRANCH: return this->parse_array(cursor);
        // case _json_node_selector_branch::STRING_BRANCH: return this->parse_string(cursor);
        // case _json_node_selector_branch::NUMBER_BRANCH: return this->parse_number(cursor);
        // case _json_node_selector_branch::BOOL_TRUE_BRANCH: return this->parse_true(cursor);
        // case _json_node_selector_branch::BOOL_FALSE_BRANCH: return this->parse_false(cursor);
        // case _json_node_selector_branch::NULL_BRANCH: return this->parse_null(cursor);
        // default:
        //     throw std::runtime_error("JSON node selector encountered unexpected marker symbol {"s +
        //     this->chars[cursor] +
        //                                      "} at pos "s + std::to_string(cursor) + " (should be one of
        //                                      {0123456789{[\"tfn})."s);
        // }
        // Seems to be even a little slower

        if (c == '{') {
            // UTL_PROFILER_LABELED("Objects")
            return this->parse_object(cursor);
        } else if (c == '[') {
            // UTL_PROFILER_LABELED("Arrays")
            return this->parse_array(cursor);
        } else if (c == '"') {
            // UTL_PROFILER_LABELED("Strings")
            return this->parse_string(cursor);
        } else if (('0' <= c && c <= '9') || (c == '-')) {
            // UTL_PROFILER_LABELED("Numbers")
            return this->parse_number(cursor);
        } else if (c == 't') {
            // UTL_PROFILER_LABELED("Trues")
            return this->parse_true(cursor);
        } else if (c == 'f') {
            // UTL_PROFILER_LABELED("Falses")
            return this->parse_false(cursor);
        } else if (c == 'n') {
            // UTL_PROFILER_LABELED("Nulls")
            return this->parse_null(cursor);
        }
        throw std::runtime_error("JSON node selector encountered unexpected marker symbol {"s + this->chars[cursor] +
                                 "} at pos "s + std::to_string(cursor) + " (should be one of {0123456789{[\"tfn})."s);
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
                                     "} after the pair key at pos "s + std::to_string(cursor) + " (should be {:})."s);
        ++cursor; // move past the colon ':'
        cursor = this->skip_nonsignificant_whitespace(cursor);

        // Parse pair value
        Node value;
        std::tie(cursor, value) = this->parse_node(cursor);

        parent.emplace(std::pair{std::move(key), std::move(value)});

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
            }
        }

        throw std::runtime_error("JSON object node reached the end of buffer while parsing object contents.");
    }

    std::size_t parse_array_element(std::size_t cursor, Array& parent) {
        using namespace std::string_literals;

        // Array element parser assumes it is starting at the first symbol of some JSON node

        // Parse pair key
        Node value;
        std::tie(cursor, value) = this->parse_node(cursor);

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
            }
        }

        throw std::runtime_error("JSON array node reached the end of buffer while parsing object contents.");
    }

    std::pair<std::size_t, String> parse_string(std::size_t cursor) {
        using namespace std::string_literals;

        // Empty string that will accumulate characters as we parse them
        std::string string_value;

        ++cursor; // move past the opening quote '"'

        while (cursor < this->chars.size()) {
            const char c = this->chars[cursor];

            // Handle escape sequences inside the string
            if (c == '\\') {
                ++cursor; // move past the backslash '\'
                if (cursor >= this->chars.size())
                    throw std::runtime_error(
                        "JSON string node reached the end of buffer while parsing an escape sequence at pos "s +
                        std::to_string(cursor) + "."s);

                const char escaped_char = this->chars[cursor];

                // // Some character are added as is
                // if (escaped_char == '"' || escaped_char == '\\' || escaped_char == '/') string_value += escaped_char;
                // // Some are interpreted based specifically on the 2-character escape
                // else if (escaped_char == 'b') string_value += '\b';
                // else if (escaped_char == 'f') string_value += '\f';
                // else if (escaped_char == 'n') string_value += '\n';
                // else if (escaped_char == 'r') string_value += '\r';
                // else if (escaped_char == 't') string_value += '\t';

                if (const char replacement_char = _lookup_parsed_escaped_chars[escaped_char])
                    string_value += replacement_char;
                // Escaped unicode codepoints (=> another 4 escaped characters after the current)
                else if (escaped_char == 'u')
                    throw std::runtime_error(
                        "JSON string node encountered unicode hex value while parsing an escape sequence at pos "s +
                        std::to_string(cursor) + ", which is not currently supported."s);
                else
                    throw std::runtime_error("JSON string node encountered unexpected character {"s + escaped_char +
                                             "} while parsing an escape sequence at pos "s + std::to_string(cursor) +
                                             "."s);

                // This covers all non-hex escape sequences according to ECMA-404 specification
                // [https://ecma-international.org/wp-content/uploads/ECMA-404.pdf] (page 4)

                ++cursor; // move past the escaped character
                continue;
            }
            // NOTE: Hex escape sequences are not supported yet

            // Reached the end of the string
            if (c == '"') {
                ++cursor; // move past the closing quote '"'
                return {cursor, string_value};
            }

            string_value += c;
            ++cursor;
        }

        throw std::runtime_error("JSON string node reached the end of buffer while parsing string contents.");
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
                                         std::to_string(cursor) + "."s);
            else if (error_code == std::errc::result_out_of_range)
                throw std::runtime_error(
                    "JSON number node parsed to number larger than its possible binary representation at pos "s +
                    std::to_string(cursor) + "."s);
        }

        return {numer_end_ptr - this->chars.data(), number_value};
    }

    std::pair<std::size_t, Bool> parse_true(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 4;

        if (cursor + token_length >= this->chars.size())
            throw std::runtime_error("JSON bool node reached the end of buffer while parsing {true}.");

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 't' && //
            this->chars[cursor + 1] == 'r' && //
            this->chars[cursor + 2] == 'u' && //
            this->chars[cursor + 3] == 'e';   //

        if (!parsed_correctly)
            throw std::runtime_error("JSON bool node could not parse {true} at pos "s + std::to_string(cursor) + "."s);

        return {cursor + token_length, Bool(true)};
    }

    std::pair<std::size_t, Bool> parse_false(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 5;

        if (cursor + token_length >= this->chars.size())
            throw std::runtime_error("JSON bool node reached the end of buffer while parsing {false}.");

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 'f' && //
            this->chars[cursor + 1] == 'a' && //
            this->chars[cursor + 2] == 'l' && //
            this->chars[cursor + 3] == 's' && //
            this->chars[cursor + 4] == 'e';   //

        if (!parsed_correctly)
            throw std::runtime_error("JSON bool node could not parse {false} at pos "s + std::to_string(cursor) + "."s);

        return {cursor + token_length, Bool(false)};
    }

    std::pair<std::size_t, Null> parse_null(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 4;

        if (cursor + token_length >= this->chars.size())
            throw std::runtime_error("JSON null node reached the end of buffer while parsing {null}.");

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 'n' && //
            this->chars[cursor + 1] == 'u' && //
            this->chars[cursor + 2] == 'l' && //
            this->chars[cursor + 3] == 'l';   //

        if (!parsed_correctly)
            throw std::runtime_error("JSON null node could not parse {null} at pos "s + std::to_string(cursor) + "."s);

        return {cursor + token_length, Null()};
    }
};


// ========================
// --- JSON Serializing ---
// ========================

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

        for (auto it = object_value.cbegin(); it != object_value.cend();) {
            if constexpr (prettify) chars.append(indent_size + indent_level_size, ' ');
            // Key
            chars += '"';
            chars += it->first;
            if constexpr (prettify) chars += "\": ";
            else chars += "\":";
            // Value
            _serialize_json_recursion<prettify>(it->second, chars, indent_level + 1, true);
            // Comma
            if (++it != object_value.cend()) chars += ','; // prevents trailing comma
            if constexpr (prettify) chars += '\n';
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

        for (auto it = array_value.cbegin(); it != array_value.cend();) {
            // Node
            _serialize_json_recursion<prettify>(*it, chars, indent_level + 1);
            // Comma
            if (++it != array_value.cend()) chars += ','; // prevents trailing comma
            if constexpr (prettify) chars += '\n';
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
            if (const char escaped_char_replacement = _lookup_serialized_escaped_chars[string_value[i]]) {
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

        auto [number_end_ptr, error_code] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), number_value);

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

// ==========================================
// --- Parsing / Serializing API wrappers ---
// ==========================================

inline Node import_string(const std::string& buffer) {
    _parser           parser(buffer);
    const std::size_t json_start = parser.skip_nonsignificant_whitespace(0); // skip leading whitespace
    auto result_node = parser.parse_node(json_start).second; // starts parsing recursively from the root node

    return result_node;
}

inline void export_string(std::string& buffer, const Node& node, Format format = Format::PRETTY) {
    _serialize_json_to_buffer(buffer, node, format);
    // NOTE: Allocating here kinda defeats the whole purpose of saving to a pre-existing buffer,
    // but for now I'll keep it for the sake of having a more uniform API
}

inline Node import_file(const std::string& filepath) {
    const std::string chars = _read_file_to_string(filepath);

    return import_string(chars);
}

inline void export_file(const std::string& filepath, const Node& node, Format format = Format::PRETTY) {
    auto chars = node.to_string(format);
    std::ofstream(filepath).write(chars.data(), chars.size());
    // a little faster than doing 'std::ofstream(path) << node.to_string(format)'
}

namespace literals {
inline Node operator""_utl_json(const char* c_str, std::size_t c_str_size) {
    return import_string(std::string(c_str, c_str_size));
}
} // namespace literals

} // namespace utl::json

#endif
#endif // module utl::json
