# utl::config

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**json** module implements a lightweight JSON manipulation API.

The main goal of this module is to provide **utl** with a built-in way of managing data exchange and configuration files without introducing large dependencies. In cases where heavy-duty JSON processing is needed, it is advised specialized JSON libraries such as: [nlohmann_json](https://github.com/nlohmann/json), [simdjson](https://github.com/simdjson/simdjson) and [Glaze](https://github.com/stephenberry/glaze).

## Feature Support

| Feature | Implementation |
| - | - |
| Parsing | ✔ |
| Serialization | ✔ |
| JSON Formatting | ✔ |
| JSON Validation | **partial** |
| Unicode Support | ✔ |
| Escape Sequence Support | ✔ |
| ISO/IEC 10646 Hexadecimal Support | ✘ |
| Trait-based Type Conversions | ✔ |
| Compile-time JSON Schema | ✘ |

## Definitions

```cpp
// JSON token types
class Object;
class Array;
using String = std::string;
using Number = double;
using Bool   = bool;
class Null;

// Type traits
template <class T> constexpr bool is_object_like_v;
template <class T> constexpr bool is_array_like_v;
template <class T> constexpr bool is_string_like_v
template <class T> constexpr bool is_numeric_like_v;
template <class T> constexpr bool is_bool_like_v;
template <class T> constexpr bool is_null_like_v;

template <class T> constexpr bool is_json_type_convertible_v =
    is_string_like_v<T> || is_numeric_like_v<T> || is_bool_like_v<T> ||
    is_null_like_v<T>   || is_object_like_v<T>  || is_array_like_v<T>;

// JSON node
struct Node {
    std::string to_string(unsigned int indent_level = 0, bool skip_first_indent = false) const;
    
    Node& operator[](const std::string& key);
}

// Import/Export
Node import_string(const std::string& buffer);
Node export_string(      std::string& buffer, const Node& node);

Node import_file(const std::string& path);
void export_file(const std::string& path, const Node& node);
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

json::Node json;

json["string"]           = "lorem ipsum";
json["array"]            = { 1, 2, 3 };
json["object"]["key_1"]  = 3.14f;
json["object"]["key_2"]  = 6.28f;

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