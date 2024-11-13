# utl::json

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**json** module aims to provide an intuitive JSON manipulation API similar to [nlohmann_json](https://github.com/nlohmann/json) while being a bit more lightweight and explicit about the underlying type conversions. The key difference are:

- `utl::json` doesn't introduce **any** global identifiers (including macros and operators)
- Objects support transparent comparators (which means `std::string_view` and `const char*` can be used for lookup)
- [Decent performance](#benchmarks) without relying on compiler intrinsics (TODO: Format and document benchmarks nicely)
- All JSON types map to standard library containers, no need to learn custom APIs

> [!Note]
> Despite rather competitive performance, considerably faster parsing can be achieved with custom formatters, SIMD and unordered key optimizations (see [simdjson](https://github.com/simdjson/simdjson), [Glaze](https://github.com/stephenberry/glaze) and [yyjson](https://github.com/ibireme/yyjson)), this, however often comes at the expense of user convenience or features (such as missing escape sequence handling in *simdjson* and *yyjson*, *Glaze* has it, but requires [C++20](https://en.cppreference.com/w/cpp/20)).

> [!Tip]
> Use GitHub's built-in [table of contents](https://github.blog/changelog/2021-04-13-table-of-contents-support-in-markdown-files/) to navigate this page.

## Feature Support

| Feature | Implementation | Notes |
| - | - | - |
| Parsing | ✔ |  |
| Serialization | ✔ |  |
| JSON Formatting | ✔ |  |
| JSON Validation | ✔ | Full validation with proper error messages through exceptions |
| Unicode Support | ✔ | Tested for UTF-8 |
| Escape Sequence Support | ✔ |  |
| ISO/IEC 10646 Hexadecimal Support | ✘ | Currently in the works |
| Trait-based Type Conversions | ✔ |  |
| Structure Reflection | ✘ | Requires compiler intrinsics or C++26 to implement |
| Compile-time JSON Schema | ✘ | Outside the project scope |
| Lazy Node Loading | ✘ | Outside the project scope |

## Definitions

```cpp
// JSON Node
enum class Format { PRETTY, MINIMIZED };

class Node {
    // - Member Types -
    using object_type = std::map<std::string, Node, std::less<>>;
    using array_type  = std::vector<Node>;
    using string_type = std::string;
    using bool_type   = bool;
    using null_type   = class{};
    
    // - Getters -
    template <class T>       T& get();
    template <class T> const T& get() const;
    
    object_type& get_object();
    array_type & get_array();
    string_type& get_string();
    number_type& get_number();
    bool_type  & get_bool();
    null_type  & get_null();

    const object_type& get_object() const;
    const array_type & get_array()  const;
    const string_type& get_string() const;
    const number_type& get_number() const;
    const bool_type  & get_bool()   const;
    const null_type  & get_null()   const;
    
    template <class T> bool is() const;
    
    bool is_object() const;
    bool is_array() const;
    bool is_string() const;
    bool is_number() const;
    bool is_bool() const;
    bool is_null() const;
    
    template <class T>       T* get_if();
    template <class T> const T* get_if() const;
    
    // - Object methods -
    Node      & operator[](std::string_view key);
    const Node& operator[](std::string_view key) const;
    
    Node      & at(std::string_view key);
    const Node& at(std::string_view key) const;
    
    bool contains(std::string_view key) const;
    
    template<class T> value_or(std::string_view key, const T &else_value);
    
    // - Assignment -
    Node& operator=(const Node&) = default;
    Node& operator=(Node&&)      = default;
    
    template <class T> Node& operator=(const T& value); // type-trait based conversion
    
    // - Constructors -
    Node()            = default;
    Node(const Node&) = default;
    Node(Node&&)      = default;
    
    template <class T> Node(const T& value); // type-trait based conversion
    
    // - Other -
    std::string to_string(Format format = Format::PRETTY) const;
};

// Typedefs
using Object = Node::object_type;
using Array  = Node::array_type;
using String = Node::string_type;
using Bool   = Node::bool_type;
using Null   = Node::null_type;

// Parsing
Node import_string(const std::string& buffer);
Node import_file(const std::string& filepath);
Node literals::operator""_utl_json(const char* c_str, std::size_t c_str_size);

// Serializing
void export_string(std::string& buffer, const Node& node, Format format = Format::PRETTY);
void export_file(const std::string& filepath, const Node& node, Format format = Format::PRETTY);
```

## Methods

### `Node` Class

#### Member types

> ```cpp
> using object_type = std::map<std::string, Node, std::less<>>;
> using array_type  = std::vector<Node>;
> using string_type = std::string;
> using bool_type   = bool;
> using null_type   = class{};
> ```

Definitions of the types, corresponding to all possible JSON values: objects, arrays, strings, numbers, booleans and null.

#### Getters

> ```cpp
> template <class T>       T& get();
> template <class T> const T& get() const;
> ```

If JSON node holds the value of a type `T`, returns a reference to the value, otherwise, throws [std::bad_variant_access](https://en.cppreference.com/w/cpp/utility/variant/bad_variant_access).

**Note:** Similar to [std::get](https://en.cppreference.com/w/cpp/utility/variant/get).

> ```cpp
> object_type& get_object();
> array_type & get_array();
> string_type& get_string();
> number_type& get_number();
> bool_type  & get_bool();
> null_type  & get_null();
> 
> const object_type& get_object() const;
> const array_type & get_array()  const;
> const string_type& get_string() const;
> const number_type& get_number() const;
> const bool_type  & get_bool()   const;
> const null_type  & get_null()   const;
> ```

Shortcut versions of `T& get<T>()` for all possible value types.

> ```cpp
> template <class T> bool is() const;
> ```

Returns whether JSON node contains a value of a type `T`.

**Note:** Similar to [std::holds_alternative](https://en.cppreference.com/w/cpp/utility/variant/holds_alternative).

> ```cpp
> bool is_object() const;
> bool is_array() const;
> bool is_string() const;
> bool is_number() const;
> bool is_bool() const;
> bool is_null() const;
> ```

Shortcut versions of `T& is<T>()` for all possible value types.

> ```cpp
> template <class T>       T* get_if();
> template <class T> const T* get_if() const;
> ```

Returns a `T*` pointer to the value stored at the JSON node, if stored value has a different type than `T`, returns [nullptr](https://en.cppreference.com/w/cpp/language/nullptr).

**Note:** Similar to [std::get_if](https://en.cppreference.com/w/cpp/utility/variant/get_if).

#### Object methods

> [!Important]
> Object methods can only be called for nodes that contain an object, incorrect node type will cause methods below to throw an exception.

> ```cpp
> Node      & operator[](std::string_view key);
> const Node& operator[](std::string_view key) const;
> ```

Returns a reference to the node corresponding to a given `key` in the JSON object, performs an insertion if such key does not already exist. 

> ```cpp
> Node      & at(std::string_view key);
> const Node& at(std::string_view key) const;
> ```

Returns a reference to the node corresponding to a given `key` in the JSON object, throws an exception if such key does not exist. 

> ```cpp
> bool contains(std::string_view key) const;
> ```

Returns whether JSON object node contains an entry with given `key`.

> ```cpp
> template<class T> value_or(std::string_view key, const T &else_value);
> ```

Returns value stored at given `key` in the JSON object, if no such key can be found returns `else_value`.

**Note:** Logically equivalent to `object.contains(key) ? object.at(key).get<T>() : else_value`, but faster.

#### Assignment & Constructors

> ```cpp
> template <class T> Node& operator=(const T& value);
> template <class T> Node(const T& value);
> ```

Converting assignment & constructors. Tries to convert `T` to one of the possible JSON types based on `T` traits, conversions and provided methods. If no such conversion is possible, SFINAE rejects the overload.

### Typedefs

> ```cpp
> using Object = Node::object_type;
> using Array  = Node::array_type;
> using String = Node::string_type;
> using Bool   = Node::bool_type;
> using Null   = Node::null_type;
> ```

Shorter typedefs for all existing JSON value types.

### Parsing

> ```cpp
> Node import_string(const std::string& buffer);
> ```

Imports JSON from a given string `buffer`.

> ```cpp
> Node import_file(const std::string& filepath);
> ```

Imports JSON from the file at `filepath`.

> ```cpp
> Node literals::operator""_utl_json(const char* c_str, std::size_t c_str_size);
> ```

`json::Node` custom literals.

### Serializing

> ```cpp
> void export_string(std::string& buffer, const Node& node, Format format = Format::PRETTY);
> ```

Exports JSON `node` to the target `buffer` using a given `format`. If serialization runs out of preallocated buffer, it is allowed to reallocate with more space.

> ```cpp
> void export_file(const std::string& filepath, const Node& node, Format format = Format::PRETTY);
> ```
> 

Exports JSON `node` to the file at `filepath` using a given `format`.

## Example 1 (importing/exporting JSON)

[ [Run this code]() ]
```cpp
using namespace utl;

// Export JSON
json::Node config;

config["auxiliary_info"]       = true;
config["date"]                 = "2024.04.02";
config["options"]["grid_size"] = 120;
config["options"]["phi_order"] = 5;
config["scaling_functions"]    = { "identity", "log10" };
config["time_steps"]           = 500;
config["time_period"]          = 1.24709e+2;

json::export_file("config.json", config);

// Import JSON
json::Node parsed_config = json::import_file("config.json");

std::cout << parsed_config.to_string();
```

Output:
```
{
    "auxiliary_info": true,
    "date": "2024.04.02",
    "options": {
        "grid_size": 120,
        "phi_order": 5
    },
    "scaling_functions": [
        "identity",
        "log10"
    ],
    "time_period": 124.709,
    "time_steps": 500
}
```

## Example 2 (setters & type conversions)

[ [Run this code]() ]
```cpp
using namespace utl;

json::Node json;

// Ways to assign a JSON object
json["object"]["key_1"] = 1;
json["object"]["key_2"] = 2;
json["object"]          =                                     { { "key_1", 1 }, { "key_2", 2 } };
json["object"]          =                         json::Object{ { "key_1", 1 }, { "key_2", 2 } };
json["object"]          =           std::map<std::string, int>{ { "key_1", 1 }, { "key_2", 2 } };
json["object"]          = std::unordered_map<std::string, int>{ { "key_1", 1 }, { "key_2", 2 } };

// Ways to assign a JSON array
json["array"] =            { 1, 2, 3 };
json["array"] = json::Array{ 1, 2, 3 };
json["array"] = std::vector{ 1, 2, 3 };
json["array"] =   std::list{ 1, 2, 3 };
json["array"] =    std::set{ 1, 2, 3 };

// Ways to assign a JSON string
json["string"] =                  "lorem ipsum" ;
json["string"] =     json::String("lorem ipsum");
json["string"] =      std::string("lorem ipsum");
json["string"] = std::string_view("lorem ipsum");

// ...and so on and so forth with other types, same thing with custom containers.
// All classes can convert as long as they provide std-like API.
```

## Example 3 (getters)

[ [Run this code]() ]
```cpp
using namespace utl;
using namespace json::literals;

// Create JSON from literal
auto json = R"(
    {
        "string": "lorem_ipsum",
        "array": [ 1, 2, 3 ],
        "object": {
            "key_1": 3.14,
            "key_2": 6.28
        }
    }
)"_utl_json;

// Check that node exists
assert( json.contains("string") );

// Check the type of a JSON node
assert( json["string"].is_string() );

// Get typed value from a JSON node
const auto str = json.at("string").get_string(); // '.at(key)' and '[key]' are both valid

// Iterate over a JSON object node
for (const auto &[key, value] : json.at("object").get_object())
    assert( key.front() == 'k' && value > 0 );

// Iterate over a JSON array node
for (const auto &element : json.at("array").get_array())
    assert( element > 0 );
```

## Example 4 (formatting)

[ [Run this code]() ]
```cpp
using namespace utl;

json::Node json;

json["string"]           = "lorem ipsum";
json["array"]            = { 1, 2, 3 }; 
json["object"]["key_1"]  = 3.14f; 
json["object"]["key_2"]  = 6.28f;

// Prettified JSON
std::cout
    << "--- Prettified JSON ---"
    << json.to_string()
    << "\n\n";
    
// Minimized JSON
std::cout
    << "--- Minimized JSON ---"
    << json.to_string(json::Format::MINIMIZED)
    << "\n\n";

// Serialize JSON to pre-allocated buffer
std::string buffer;
buffer.reserve(1200);
json::export_string(buffer, json, json::Format::MINIMIZED);
```

Output:
```
TODO:
```

## Benchmarks

Benchmarks for parsing and serializing of minimized JSON data corresponding to various entries in the [test suite](TODO:). 

```
|               ns/op |                op/s |    err% |     total | benchmark
|--------------------:|--------------------:|--------:|----------:|:----------
|        7,192,296.48 |              139.04 |    0.6% |      1.75 | `strings.json # Serializing # utl::json`
|       11,756,307.76 |               85.06 |    1.1% |      2.82 | `strings.json # Parsing     # utl::json`
|       11,161,407.10 |               89.59 |    1.0% |      2.68 | `strings.json # Serializing # nlohmann `
|       23,488,798.65 |               42.57 |    0.4% |      5.64 | `strings.json # Parsing     # nlohmann `
|        4,004,559.75 |              249.72 |    0.8% |      0.96 | `numbers.json # Serializing # utl::json`
|        3,470,066.09 |              288.18 |    0.2% |      0.83 | `numbers.json # Parsing     # utl::json`
|        6,748,506.95 |              148.18 |    0.4% |      1.62 | `numbers.json # Serializing # nlohmann `
|       16,767,746.74 |               59.64 |    0.5% |      4.01 | `numbers.json # Parsing     # nlohmann `
|        2,235,215.38 |              447.38 |    0.3% |      0.56 | `database.json # Serializing # utl::json`
|        8,446,221.52 |              118.40 |    1.4% |      2.03 | `database.json # Parsing     # utl::json`
|        4,989,242.17 |              200.43 |    0.7% |      1.21 | `database.json # Serializing # nlohmann `
|       12,777,209.48 |               78.26 |    0.3% |      3.06 | `database.json # Parsing     # nlohmann `
```

