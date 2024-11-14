# utl::json

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**json** module aims to provide an intuitive JSON manipulation API similar to [nlohmann_json](https://github.com/nlohmann/json) while being a bit more lightweight and explicit about the underlying type conversions. The key difference are:

- `utl::json` doesn't introduce **any** global identifiers (including macros and operators)
- Objects support transparent comparators (which means `std::string_view` and `const char*` can be used for lookup)
- [Decent performance](#benchmarks) without relying on compiler intrinsics (TODO: Format and document benchmarks nicely)
- All JSON types map to standard library containers, no need to learn custom APIs
- Simple integration (single header, less that a `1k` lines)

> [!Note]
> Despite rather competitive performance, considerably faster parsing can be achieved with custom formatters, SIMD and unordered key optimizations (see [simdjson](https://github.com/simdjson/simdjson), [Glaze](https://github.com/stephenberry/glaze), [RapidJSON](https://github.com/Tencent/rapidjson)  and [yyjson](https://github.com/ibireme/yyjson)), this, however often comes at the expense of user convenience (like RapidJSON) or features (such as missing escape sequence handling in *simdjson* and *yyjson*, *Glaze* has it, but requires [C++20](https://en.cppreference.com/w/cpp/20)).

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
====== BENCHMARKING ON DATA: `strings.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               18.95 |               52.78 |    2.2% |      1.37 | `utl::json`
|    31.0% |               61.12 |               16.36 |    0.6% |      4.35 | `nlohmann`
|    57.4% |               32.99 |               30.31 |    0.3% |      2.34 | `PicoJSON`
|   127.3% |               14.89 |               67.18 |    1.5% |      1.06 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               19.32 |               51.76 |    0.5% |      1.40 | `utl::json`
|    29.5% |               65.52 |               15.26 |    2.5% |      4.64 | `nlohmann`
|    54.9% |               35.19 |               28.42 |    1.2% |      2.50 | `PicoJSON`
|   105.9% |               18.24 |               54.81 |    0.5% |      1.30 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               12.81 |               78.06 |    0.5% |      0.91 | `utl::json`
|    44.5% |               28.78 |               34.74 |    0.4% |      2.06 | `nlohmann`
|    33.6% |               38.17 |               26.20 |    0.4% |      2.74 | `PicoJSON`
|    74.3% |               17.24 |               58.02 |    3.8% |      1.20 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               16.61 |               60.22 |    2.0% |      1.17 | `utl::json`
|    53.1% |               31.28 |               31.97 |    2.7% |      2.21 | `nlohmann`
|    41.8% |               39.73 |               25.17 |    0.5% |      2.84 | `PicoJSON`
|    95.1% |               17.46 |               57.26 |    1.9% |      1.26 | `RapidJSON`


====== BENCHMARKING ON DATA: `numbers.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               10.67 |               93.70 |    0.2% |      0.76 | `utl::json`
|    21.1% |               50.68 |               19.73 |    0.8% |      3.61 | `nlohmann`
|    25.9% |               41.29 |               24.22 |    1.7% |      2.94 | `PicoJSON`
|   119.6% |                8.93 |              112.03 |    1.3% |      0.63 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               12.17 |               82.14 |    1.0% |      0.86 | `utl::json`
|    23.2% |               52.37 |               19.09 |    0.5% |      3.74 | `nlohmann`
|    27.7% |               44.00 |               22.73 |    0.8% |      3.13 | `PicoJSON`
|   129.6% |                9.40 |              106.42 |    1.3% |      0.66 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               11.08 |               90.25 |    0.4% |      0.80 | `utl::json`
|    63.3% |               17.50 |               57.15 |    0.5% |      1.25 | `nlohmann`
|    13.8% |               80.21 |               12.47 |    0.3% |      5.70 | `PicoJSON`
|    61.1% |               18.13 |               55.15 |    2.4% |      1.28 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               14.12 |               70.82 |    3.4% |      1.00 | `utl::json`
|    69.7% |               20.26 |               49.37 |    1.0% |      1.44 | `nlohmann`
|    16.5% |               85.42 |               11.71 |    2.5% |      6.13 | `PicoJSON`
|    73.3% |               19.27 |               51.89 |    1.4% |      1.42 | `RapidJSON`


====== BENCHMARKING ON DATA: `database.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               24.79 |               40.35 |    6.7% |      1.81 | `utl::json`
|    46.8% |               53.01 |               18.87 |    3.4% |      3.80 | `nlohmann`
|    64.3% |               38.57 |               25.92 |    8.6% |      2.71 | `PicoJSON`
|   256.1% |                9.68 |              103.34 |    4.1% |      0.70 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               26.92 |               37.15 |    4.9% |      1.92 | `utl::json`
|    46.3% |               58.11 |               17.21 |    6.1% |      4.11 | `nlohmann`
|    64.3% |               41.86 |               23.89 |    9.5% |      2.96 | `PicoJSON`
|   230.4% |               11.68 |               85.59 |    0.7% |      0.85 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               11.77 |               84.94 |    2.7% |      0.87 | `utl::json`
|    49.6% |               23.74 |               42.12 |    0.2% |      1.69 | `nlohmann`
|    33.1% |               35.54 |               28.13 |    1.1% |      2.55 | `PicoJSON`
|   114.9% |               10.25 |               97.60 |    3.2% |      0.71 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               13.86 |               72.14 |    6.0% |      1.03 | `utl::json`
|    53.5% |               25.91 |               38.59 |    1.5% |      1.87 | `nlohmann`
|    37.0% |               37.42 |               26.72 |    0.4% |      2.67 | `PicoJSON`
|   128.1% |               10.82 |               92.42 |    0.2% |      0.77 | `RapidJSON`


====== BENCHMARKING ON DATA: `unordered_object.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               19.99 |               50.02 |    1.0% |      1.48 | `utl::json`
|    65.3% |               30.63 |               32.65 |    3.0% |      2.21 | `nlohmann`
|    94.7% |               21.10 |               47.39 |    2.6% |      1.51 | `PicoJSON`
|   454.8% |                4.40 |              227.46 |    0.5% |      0.32 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               20.04 |               49.90 |    2.1% |      1.50 | `utl::json`
|    62.4% |               32.12 |               31.13 |    2.7% |      2.29 | `nlohmann`
|    93.1% |               21.53 |               46.44 |    1.1% |      1.54 | `PicoJSON`
|   347.5% |                5.77 |              173.41 |    0.2% |      0.41 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               12.57 |               79.57 |    5.5% |      0.86 | `utl::json`
|   129.9% |                9.67 |              103.40 |    6.3% |      0.77 | `nlohmann`
|   109.0% |               11.53 |               86.76 |    5.3% |      0.90 | `PicoJSON`
|   348.4% |                3.61 |              277.26 |    0.2% |      0.26 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               13.98 |               71.52 |    7.2% |      0.90 | `utl::json`
|    92.1% |               15.18 |               65.89 |    7.1% |      1.08 | `nlohmann`
|   104.4% |               13.40 |               74.65 |    5.7% |      1.00 | `PicoJSON`
|   263.3% |                5.31 |              188.31 |    0.2% |      0.38 | `RapidJSON`
```

> [!Note]
> The main weak-point of `utl::json` from the performance standpoint is parsing of JSON objects that contain a large amount of keys.
>
> Unfortunately, the issue is mostly caused by `std::map` insertion and iteration speed, which dominates the runtime. A truly proper container for JSON object representation doesn't really exist in the standard library.
>
> Perhaps I'll find a way to implement one that can be "slotted into" the implementation as a template parameter that allows us to trade a standard API for even better performance.
