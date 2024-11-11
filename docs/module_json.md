# utl::config

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**json** module aims to provide an intuitive JSON manipulation API similar to [nlohmann_json](https://github.com/nlohmann/json) while being a bit more lightweight and explicit about the underlying type conversions. The key difference are:

- `utl::json` doesn't introduce **any** global identifiers (including macros and operators)
- Objects support transparent comparators (which means `std::string_view` and `const char*` can be used for lookup)
- [Decent performance](#benchmarks) without relying on compiler intrinsics (TODO: Format and document benchmarks nicely)
- All JSON types map to standard library containers, no need to learn custom APIs

> [!Note]
> Despite rather competitive performance, considerably faster parsing can be achieved with custom formatters, SIMD and unordered key optimizations (see [simdjson](https://github.com/simdjson/simdjson), [Glaze](https://github.com/stephenberry/glaze) and [yyjson](https://github.com/ibireme/yyjson)), this, however often comes at the expense of user convenience or features (such as missing escape sequence handling in *simdjson* and *yyjson*, *Glaze* has it, but requires [C++20](https://en.cppreference.com/w/cpp/20)).

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
| Compile-time JSON Schema | ✘ | Requires compiler intrinsics or C++26 to implement |
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

> ```cpp
> config::export_json(std::string_view path, const Types... entries);
> ```

Creates JSON at `path` with given `entries`. Entries can be specified using `config::entry(key, value)`. Corresponding to the JSON standard, `value` can be an **integer**, **float**, **string** or **bool** (or any of the similar types). Arrays and nested arrays are also supported as long as their types are homogenous.

> ```cpp
> std::tuple<std::string_view, T> config::entry(std::string_view key, T value);
> ```

Specifies a config entry.

Performs type resolution on `value` which helps other functions resolve a proper JSON datatype without being manually specified by user.

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