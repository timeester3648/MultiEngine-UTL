# utl::json

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**json** module aims to provide an intuitive JSON manipulation API similar to [nlohmann_json](https://github.com/nlohmann/json) while being a bit more lightweight and explicit about the underlying type conversions. The few key features & differences are:

- `utl::json` doesn't introduce any invasive macros or operators
- Objects support transparent comparators (which means `std::string_view` and `const char*` can be used for lookup)
- [Decent performance](#benchmarks) without relying on compiler intrinsics
- All JSON types map to standard library containers, no need to learn custom APIs
- Simple integration (single header, barely over `1k` lines)
- [Nice error messages](#error-handling)
- [Full support for class reflection](#complex-structure-reflection)

> [!Note]
> Despite rather competitive performance, considerably faster parsing can be achieved with custom formatters, SIMD and unordered key optimizations (see [simdjson](https://github.com/simdjson/simdjson), [Glaze](https://github.com/stephenberry/glaze), [RapidJSON](https://github.com/Tencent/rapidjson)  and [yyjson](https://github.com/ibireme/yyjson)), this, however often comes at the expense of user convenience (like with *RapidJSON*) or features (such as missing escape sequence handling in *simdjson* and *yyjson*, *Glaze* has it all, but requires [C++23](https://en.cppreference.com/w/cpp/23)).

> [!Tip]
> Use GitHub's built-in [table of contents](https://github.blog/changelog/2021-04-13-table-of-contents-support-in-markdown-files/) to navigate this page.

## Feature Support

| Feature | Implementation | Notes |
| - | - | - |
| Parsing | ✔ |  |
| Serialization | ✔ |  |
| JSON Formatting | ✔ |  |
| JSON Validation | ✔ | Almost complete[¹](#tests) validation with proper error messages through exceptions |
| Unicode Support | ✔ | Supports UTF-8 |
| Control Character Escape Sequence Support | ✔ |  |
| Unicode HEX Sequence Support | ✔ | Supports UTF-8 |
| Trait-based Type Conversions | ✔ |  |
| Structure Reflection | ✔ | Arbitrary reflection including nested types and containers[²](#complex-structure-reflection) |
| Compile-time JSON Schema | ✘ | Outside the project scope, can be emulated with reflection |
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
    using number_type = double;
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
    bool is_array()  const;
    bool is_string() const;
    bool is_number() const;
    bool is_bool()   const;
    bool is_null()   const;
    
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
    
    // Serializing
    std::string          to_string(                           Format format = Format::PRETTY) const;
    void                 to_file(const std::string& filepath, Format format = Format::PRETTY) const;
    template <class T> T to_struct()                                                          const;
};

// Typedefs
using Object = Node::object_type;
using Array  = Node::array_type;
using String = Node::string_type;
using Number = Node::number_type;
using Bool   = Node::bool_type;
using Null   = Node::null_type;

// Parsing
Node                    from_string(const std::string& chars   );
Node                    from_file(  const std::string& filepath);
template <class T> Node from_struct(const           T& value   );

Node literals::operator""_utl_json(const char* c_str, std::size_t c_str_size);
void set_recursion_limit(int max_depth);

// Reflection
#define UTL_JSON_REFLECT(struct_name, ...)

template <class T> constexpr bool is_reflected_struct;
```

## Methods

### `Node` Class

#### Member types

> ```cpp
> using object_type = std::map<std::string, Node, std::less<>>;
> using array_type  = std::vector<Node>;
> using string_type = std::string;
> using number_type = double;
> using bool_type   = bool;
> using null_type   = class{};
> ```

Definitions of the types, corresponding to all possible JSON values according to [ECMA-404 specification](https://ecma-international.org/wp-content/uploads/ECMA-404.pdf): objects, arrays, strings, numbers, booleans and null.

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

#### Serializing

> ```cpp
> std::string to_string(Format format = Format::PRETTY) const;
> ```

Serializes JSON node to a string using a given `format`.

> ```cpp
> void to_file(const std::string& filepath, Format format = Format::PRETTY) const;
> ```

Serializes JSON node to the file at `filepath` using a given `format`.

> ```cpp
> template <class T> T to_struct() const;
> ```

Serializes JSON node to the structure / class object of type `T`.

The type `T` must be reflected with `UTL_JSON_REFLECT()` macro, otherwise compilation fails with a proper assertion.

### Parsing

> ```cpp
> Node from_string(const std::string& buffer);
> ```

Parses JSON from a given string `buffer`.

> ```cpp
> Node from_file(const std::string& filepath);
> ```

Parses JSON from the file at `filepath`.

> ```cpp
> template <class T> Node from_struct(const T& value);
> ```

Parses JSON from structure / class object `value`.

The type `T` must be reflected with `UTL_JSON_REFLECT()` macro, otherwise compilation fails with a proper assertion.

> ```cpp
> Node literals::operator""_utl_json(const char* c_str, std::size_t c_str_size);
> ```

`json::Node` custom literals.

> ```cpp
> void set_recursion_limit(int max_depth) noexcept;
> ```

Sets max recursion depth during parsing, default value is `1000`.

**Note:** JSON parsers need recursion depth limit to prevent malicious inputs (such as 100'000+ nested object opening braces) from causing stack overflows, instead we get a controllable error.

### Typedefs

> ```cpp
> using Object = Node::object_type;
> using Array  = Node::array_type;
> using String = Node::string_type;
> using Number = Node::number_type;
> using Bool   = Node::bool_type;
> using Null   = Node::null_type;
> ```

Shorter typedefs for all existing JSON value types.

### Reflection

> ```cpp
> #define UTL_JSON_REFLECT(struct_name, ...)
> ```

Reflects structure / class `struct_name` with member variables `...`.

Declaring this macro defines methods `Node::to_struct<struct_name>()` and `from_struct(const struct_name&)` for parsing and serialization.

**Note 1:** Reflection supports nested classes, each class should be reflected with a macro and `to_struct()` / `from_struct()` will call each other recursively whenever appropriate. Containers of reflected classes are also supported with any level of nesting. See [examples](#structure-reflection).

**Note 2:** Reflection does not impose any strict limitations on member variable types, it uses the same set of type traits as other methods to deduce appropriate conversions. It is expected however, that array-like member variables should support `.resize()` ([std::vector](https://en.cppreference.com/w/cpp/container/vector) and [std::list](https://en.cppreference.com/w/cpp/container/list) satisfy that) or provide an API similar to [std::array](https://en.cppreference.com/w/cpp/container/array). For object-like types it is expected that new elements can be inserted with `operator[]` [std::map](https://en.cppreference.com/w/cpp/container/map) and [std::unordered_map](https://en.cppreference.com/w/cpp/container/unordered_map) satisfy that).

> ```cpp
> template <class T> constexpr bool is_reflected_struct;
> ```

Evaluates to `true` if `T` was reflected with `UTL_JSON_REFLECT()`, `false` otherwise.

## Examples 

### Parse/serialize JSON

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:23,endLineNumber:7,positionColumn:23,positionLineNumber:7,selectionStartColumn:23,selectionStartLineNumber:7,startColumn:23,startLineNumber:7),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++//+Export+JSON%0A++++json::Node+config%3B%0A%0A++++config%5B%22auxiliary_info%22%5D+++++++%3D+true%3B%0A++++config%5B%22date%22%5D+++++++++++++++++%3D+%222024.04.02%22%3B%0A++++config%5B%22options%22%5D%5B%22grid_size%22%5D+%3D+120%3B%0A++++config%5B%22options%22%5D%5B%22phi_order%22%5D+%3D+5%3B%0A++++config%5B%22scaling_functions%22%5D++++%3D+%7B+%22identity%22,+%22log10%22+%7D%3B%0A++++config%5B%22time_steps%22%5D+++++++++++%3D+500%3B%0A++++config%5B%22time_period%22%5D++++++++++%3D+1.24709e%2B2%3B%0A%0A++++config.to_file(%22config.json%22)%3B%0A%0A++++//+Import+JSON%0A++++config+%3D+json::from_file(%22config.json%22)%3B%0A%0A++++std::cout+%3C%3C+config.to_string()%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

// Serialize JSON
json::Node config;

config["auxiliary_info"]       = true;
config["date"]                 = "2024.04.02";
config["options"]["grid_size"] = 120;
config["options"]["phi_order"] = 5;
config["scaling_functions"]    = { "identity", "log10" };
config["time_steps"]           = 500;
config["time_period"]          = 1.24709e+2;

config.to_file("config.json");

// Parse JSON
config = json::from_file("config.json");

std::cout << config.to_string();
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

### Setters & type conversions

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGISdKuADJ4DJgAcj4ARpjEIGbSAA6oCoRODB7evv5JKWkCIWGRLDFxCbaY9o4CQgRMxASZPn4BFVXptfUEhRHRsfHSCnUNTdmtQ109xaUDAJS2qF7EyOwc5gDMocjeWADUJutuCAQEiQogAPQXxEwA7gB0wIQIXlFeSsuyjAT3aCwXABEWIRiHgLKhgOhDKgAG4XRLEVBEAgAT0SoWAAH0vI5aAoLiwmENYhctjtMPDEURsbiFPcEIlEgdsCYNABBVkcsybBjbLx7A5uWh4IbMzkbMn8zD7Q5KAhi9mc0IEXaE0IQZW7erAZCkXbIBD1XYAKmN2phs32AHYrOzdvbdu8MbtmGwFIkmCtHQRaAdbRy7Q7tApnCBwhhpcGBH7xYH7VddgB1JgohS7IhahSpYAMLW7ABSQgA8uFdqgotpMA5OQ7dlGGCYAKxWMxmcuV6utpsApstswAa0wKMxXHMZm7MoBu1H639tfrvbH7ar8q7jZ7zbHg%2BH47XU4OU/Hs5rQZDDc3reXnfH69rd8n98fT%2Bfj5MNutFn2re3I7Heq41oAnqb6fluQ6YruZh6mYgGATGcZ1mei6XhWK5jhOz4Hi%2B2HzmeIAgEWqHVu%2BIFfgO4Gjq2/6AcBJHfuBkHQbBb49seCELhebZEauN5Tph6x8c%2BQzoPhhJMrKBAiSAQygkYerKmKJF0eRw6UVB040R%2BZE/oxuwwSxcFsWyuHRpxV48RhT5YcJ%2BFeAwJBYMQmDoJiYmCjZ0kELJwDyYIimgcpP5qdRLG0aB9E7n%2BenMVarH%2Bie8YXEmKZphmRLZrmTAFsWpb1DcKIJYhpl9nlKbobeWHYaRXDQXq6yGXOp7FWOpUFXuD71vhbLEPl1W1bs9UsfBxlNeeJU9WV7XWZJ%2BEwiuJB9XpdUNYVHHjb1U0CQ6HnCqK741UtA0rexSGca15X7lt20zdJmDyvt/WDbFw2radfaEl5eCqBdD6kYt%2BmxWFA16pIMVxbsCYfaCKxpuYABs6aMCGxBpvU0o0GE6aoKgr3Na2BBIwtm37kpAWfgdANAVp6wg2DQOkdIuyNpppFw3qVpg8dAYjYlyWpljmYZXmhYlrsMkYrjY1juLRg/ZVOG1mOtAkJgLC7HgZw%2BGOMqNfaa3S59svE3enUgLU3kQErKtqxrCha62szDSZUutjLwBy1dtYeW7lutsrTk25rLBjo7RnO8hZhux7U7e4bWIwngmC3L7Zj%2B6r6tByHL0IQm9x54Y6Bi6gZaZQwhchrs/ANAguy3M8ZYEAgsTpmimAKHqCisNKjfOnXjf6u8RBq2gghMKEsR0oVCZsrQtD6gYWZt/qhj6gIc0NJmuzK0Ym%2BN0OuwIrCeB7MJAC0wqDrsbLKAAkvcsY87sTkEEsuYaC9z3shw8y0Jwja8H4DgWhSCoE4G4aw1gi5LC9BsHgpACCaG/vMfsIArRcHuGgq0AAOdYUgsFmDhgATi4BoRs%2BhOCSAAYgkBnBeDnA0PAxB8w4CwBgIgFAqAWDonoGQCgEA/jcP6NsQwwAuBww0AwmgtACYo0oFEahURQj1BRJwOBijmDEBRIRDsCDuC8D%2BGwQQRYGC0BUUA3gWA3jADcGIPEqiLGqxEeIcxpB8BOQcHgOa5wXGYFUFWHEqxgHKkqNQ4UUQbiaI8Fgahn0WD2NIOvKIKRMAAkcUYYURgmF8AMMABQAA1ROtwiyJEYPE/gggRBiHYFIGQghFAqHUC43QNUDCZNMJYaw%2Bg8BRHOJAeYqBEjVAYN40%2BwkDztJbBYLgHNT5FjMLwWEsRQRYF6RAeYdgVzpBcGXUYfgarBHHtMfoNVkipCGbsvQpz8gMCmH0OINUNkeJqMMRonhmh6EeUMzoDRbklGObYF5FyHkvN%2BTMLg6zFjLGqT/P%2BVCXGgI4LsVQWC4anzhqDYRO8xH3A0Di3YEBcCEBIF%2BXBsxeC6K0LMeYTcmCOUoDCjglDSBxMbAwwBwCEV0JAAwil39SAsPYYsU4OJyCUAEXQWI4Qu6cGRai9F88RHTjhjinFvBnJEuWXocpwhRDiBqdq%2BpahqHNNILcG4iR7EMv/qQdlCzOBFhxIkHEZYqBIpRWijFrTgBKpVRofFHguESuICS8F5KmHIP8MqohkgzBcBRaQwhaCxHkMZbwFlbLqGctsNyxh5iqWkBQWYLB9wi2xrhlgxs0z1gaCwYQyQbNf4cHWHCjltDc2UoZfMm1ma228vzevVIzhJBAA%3D%3D%3D) ]
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

json["matrix"] = { { 1, 2 }, { 3, 4 } }; // matrices & tensors are fine too
json["tensor"] = { { { 1, 2 }, { 3, 4 } }, { { 4, 5 }, { 6, 7 } } };

// Ways to assign a JSON string
json["string"] =                  "lorem ipsum" ;
json["string"] =     json::String("lorem ipsum");
json["string"] =      std::string("lorem ipsum");
json["string"] = std::string_view("lorem ipsum");

// ...and so on and so forth with other types, same thing with custom containers.
// All classes can convert as long as they provide std-like API.
```

### Getters

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:44,endLineNumber:34,positionColumn:44,positionLineNumber:34,selectionStartColumn:44,selectionStartLineNumber:34,startColumn:44,startLineNumber:34),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A++++using+namespace+json::literals%3B%0A%0A++++//+Create+JSON+from+literal%0A++++auto+json+%3D+R%22(%0A++++++++%7B%0A++++++++++++%22string%22:+%22lorem_ipsum%22,%0A++++++++++++%22array%22:+%5B+1,+2,+3+%5D,%0A++++++++++++%22object%22:+%7B%0A++++++++++++++++%22key_1%22:+3.14,%0A++++++++++++++++%22key_2%22:+6.28%0A++++++++++++%7D%0A++++++++%7D%0A++++)%22_utl_json%3B%0A%0A++++//+Check+that+node+exists%0A++++assert(+json.contains(%22string%22)+)%3B%0A%0A++++//+Check+the+type+of+a+JSON+node%0A++++assert(+json%5B%22string%22%5D.is_string()+)%3B%0A%0A++++//+Get+typed+value+from+a+JSON+node%0A++++const+auto+str+%3D+json.at(%22string%22).get_string()%3B+//+!'.at(key)!'+and+!'%5Bkey%5D!'+are+both+valid%0A%0A++++//+Iterate+over+a+JSON+object+node%0A++++for+(const+auto+%26%5Bkey,+value%5D+:+json.at(%22object%22).get_object())%0A++++++++assert(+key.front()+%3D%3D+!'k!'+%26%26+value.get_number()+%3E+0+)%3B%0A%0A++++//+Iterate+over+a+JSON+array+node%0A++++for+(const+auto+%26element+:+json.at(%22array%22).get_array())%0A++++++++assert(+element.get_number()+%3E+0+)%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
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
    assert( key.front() == 'k' && value.get_number() > 0 );

// Iterate over a JSON array node
for (const auto &element : json.at("array").get_array())
    assert( element.get_number() > 0 );
```

### Formatting

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:12,positionColumn:1,positionLineNumber:12,selectionStartColumn:1,selectionStartLineNumber:12,startColumn:1,startLineNumber:12),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++json::Node+json%3B%0A%0A++++json%5B%22string%22%5D+++++++++++%3D+%22lorem+ipsum%22%3B%0A++++json%5B%22array%22%5D++++++++++++%3D+%7B+1,+2,+3+%7D%3B+%0A++++json%5B%22object%22%5D%5B%22key_1%22%5D++%3D+3.14%3B+%0A++++json%5B%22object%22%5D%5B%22key_2%22%5D++%3D+6.28%3B%0A%0A++++//+Prettified/Minimized+JSON%0A++++std::cout%0A++++++++%3C%3C+%22---+Prettified+JSON+---%22%0A++++++++%3C%3C+%22%5Cn%5Cn%22%0A++++++++%3C%3C+json.to_string()%0A++++++++%3C%3C+%22%5Cn%5Cn%22%0A++++++++%3C%3C+%22---+Minimized+JSON+---%22%0A++++++++%3C%3C+%22%5Cn%5Cn%22%0A++++++++%3C%3C+json.to_string(json::Format::MINIMIZED)%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

json::Node json;

json["string"]           = "lorem ipsum";
json["array"]            = { 1, 2, 3 }; 
json["object"]["key_1"]  = 3.14; 
json["object"]["key_2"]  = 6.28;

// Prettified/Minimized JSON
std::cout
    << "--- Prettified JSON ---"
    << "\n\n"
    << json.to_string()
    << "\n\n"
    << "--- Minimized JSON ---"
    << "\n\n"
    << json.to_string(json::Format::MINIMIZED);
```

Output:
```
--- Prettified JSON ---

{
    "array": [
        1,
        2,
        3
    ],
    "object": {
        "key_1": 3.14,
        "key_2": 6.28
    },
    "string": "lorem ipsum"
}

--- Minimized JSON ---

{"array":[1,2,3],"object":{"key_1":3.14,"key_2":6.28},"string":"lorem ipsum"}
```

### Error handling

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:6,endLineNumber:19,positionColumn:6,positionLineNumber:19,selectionStartColumn:6,selectionStartLineNumber:19,startColumn:6,startLineNumber:19),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++const+auto+invalid_json+%3D+R%22(%0A++++++++%7B%0A++++++++++++%22key_1%22:+%22value_1%22,%0A++++++++++++%22key_2%22:++value_2%22,%0A++++++++++++%22key_3%22:+%22value_3%22%0A++++++++%7D%0A++++)%22%3B%0A%0A++++try+%7B%0A++++++++json::from_string(invalid_json)%3B%0A++++%7D%0A++++catch+(std::runtime_error+%26e)+%7B%0A++++++++std::cerr+%3C%3C+%22ERROR:+Caught+exception:%5Cn%5Cn%22+%3C%3C+e.what()%3B%0A++++%7D%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

const auto invalid_json = R"(
    {
        "key_1": "value_1",
        "key_2":  value_2",
        "key_3": "value_3"
    }
)";

try {
    json::from_string(invalid_json);
}
catch (std::runtime_error &e) {
    std::cerr << "ERROR: Caught exception:\n\n" << e.what();
}
```

Output:
```
ERROR: Caught exception:

JSON node selector encountered unexpected marker symbol {v} at pos 48 (should be one of {0123456789{["tfn}).
Line 4:         "key_2":  value_2",
        ------------------^-------- [!]
```

### Structure reflection

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGEgMykrgAyeAyYAHI%2BAEaYxCBmGqQADqgKhE4MHt6%2BASlpGQKh4VEssfGJtpj2jgJCBEzEBNk%2BflyBdpgOmfWNBMWRMXEJSQoNTS257bbj/WGDZcOJAJS2qF7EyOwc5v5hyN5YANQm/m4IBATJCiAA9LfETADuAHTAhAhe0V5Km7KMBBeaBYtwAIixCMQ8BZUMB0IZUAA3W7JYioIgEACeyTCwAA%2Bl5HLQFLcWEwxnFbvtDpgUWiiASiQoXghkslTtgTBoAIJc7ljYheBxHDwMGjAE4AdisPKOcqO0VQnnlKqOTC8qjoeEamLxYX4J38oKOBEFmFOMu5KrG6BAIAFuKO8IImFVbtOxvMZkSZkkLw0fo0Zi9Fr5KrD8oFQoIRwA8slagwFFLLW65WEY8Aoeg8ekAF6uj1HLiJUOytMZo7JBB4PEkLDEQ3GgCsZat8pMkuNqATmQUbfD5blNrtiK6RGIpzcI/tptxHKOClEtFxeKoXgY3QEyaLnas3rwWEEhExXtIJ29tFhXA0Ia7A8jBFt9rwBbxMbTbscbFzLuun6LZsNFvfxU3ldB1miehPxguVv0wPFkjiJx0DTIsuBeX1JQ0ABOc1LGDUC%2BU7UE2z5OQABVgjxAApIRYwiPEACVsAAMWCbA3AoiBRXFc91U1FcdT1MVUHPZ1MHPHtEwUc8lzEVd103GTz3g39MGuVS8B/JCoQwZY20o6i6IY5i2I4rieIEcU7XjFSjizQ9c1fSSqxrOtiAbAyiJ5PlKzJMIIGWFMIzlH5HWYNgFGSJgtiOQlaDIocjnuI5lEaJQjhMiIjioNEWEXU1o1CtVCVQNM0DFPAJSLXjqr3EiHzlbQFGcEAIgwV0WoEDsjSObq/BAPLUBYX9BQcCBKvFbzLRKmc0EJQ03CnC8zAAWg2o56nGmMiCy%2Bico2taTGbNwGC9JaVoGl4GQdIwgqS9s5VSoRkIUgt9oYk1yqjBwSvVPbfm1FcCxzKbqqbfrWoYG7UDG4qzjq4AOQenynpOZLyV%2BAgICOcHgBeAStWE/Vyt60i%2BqB97MDB6zqsJjVieIXVSZVGb/oUbHcfxl4JNg8MjSLKmQZpvEeb5/m5XZzHObiHG8bpgnpL7N5s2cj6PSFt6Rdpqqld7bdVac/NXWl9HUpeS3DFQ1qjgEYj7x5DhVloThm14PwOC0UhUE4ZbLGsRd1k2QszH8HhSAITRndWABrEBJQwxPJQADnDyQU7MAA2HCb2bfROEkD3o59zheBuJIo6953SDgWAYEQFARpxegyAoSbm7oYYDkMYAuCz4C%2BDoF1iBuCBohL6Iwh1TgI6n5hmdjaJtHHWfeGBNhBFjBhaExEusG%2BYA3DEYk19ILAySMcRq/PvBiHHPAxxuG/MFULpCW2b2M2qEuV2iR5mYeCwCXOcLAz5jmIIqJQoJMCX2ACuIw0dVhUAMMABQAA1PAmAnjxkYGffgggRBiHYFIGQghFAqHUDfXQXB9C9xQNYaw%2Bg8DRBuJAVYyttycDWjaD0pgA6WC4JKI4a1YxmF4EiOI2ZMBsKClUGomQXAMHcJ4VoegQjzFKOUPQqR0iJkmG0fIejMgDC0cMWhnQtwMF6BMVRUx5EPzqLMUxQx4gWNmAYvQYw%2BguMWG41YrUNhbAkC7N2xcb6%2Bw4EcVQKcs5rSzpIPGBgjDFizv6f0RwIC4EICQC84dli8CrloZYqwECYCYA2SgoSOBF1IGAoCpBPbe0ieXEAlckG1wbhAJA6wriEnIJQYELc4gRFYNsGJcSElJN7qk9J3saY5OzHoAhwhlwkOkCsihagS40NIE8R4yQ17VPdo0kukTYyEmSItVAVBomxPiYknuKT%2B5zMyR4FgwzGy7C4AUyOSC44gEkGknOkgzBcFiRoZsOFE79wLjU3g9SkhNIkWXWwbS/nVxKaQeOZgU6YUzv3FOzYhH%2BA0CnHCQK4X%2BHCc01FRSY5wvEaciJdL/mkAgekZwkggA%3D) ]

```cpp
struct Config {
    bool        auxiliary_info = true;
    std::string date           = "2024.04.02";

    struct Options {
        int grid_size = 120;
        int phi_order = 5;
    } options;

    std::vector<std::string> scaling_functions = {"identity", "log10"};
    std::size_t              time_steps        = 500;
    double                   time_period       = 1.24709e+2;
};

UTL_JSON_REFLECT(Config, auxiliary_info, date, options, scaling_functions, time_steps, time_period);
UTL_JSON_REFLECT(Config::Options, grid_size, phi_order);

// ...

using namespace utl;

// Parse JSON from struct
auto       config = Config{};
json::Node json   = json::from_struct(config);

// Test the result
std::cout << "--- Struct to JSON ---\n" << json.to_string();

// Serialize JSON to struct
auto serialized_config = json.to_struct<Config>();

// Test the result
assert( config.auxiliary_info    == serialized_config.auxiliary_info    );
assert( config.date              == serialized_config.date              );
assert( config.options.grid_size == serialized_config.options.grid_size );
// ...and so on
```

Output:
```
--- Struct to JSON ---
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

### Complex structure reflection

> [!Note]
> There are no particular limitations on what kinds of nested structures can be reflected, as long as there is a logically sound path to converting one thing to another `utl::json` will figure out a way.

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGIAKykrgAyeAyYAHI%2BAEaYxBIA7KQADqgKhE4MHt6%2BASlpGQKh4VEssfFcSXaYDplCBEzEBNk%2BfoHVtQL1jQTFkTFxibYNTS257SO9Yf1lg5UAlLaoXsTI7BzmAMxhyN5YANQmm24IBATJCiAA9FfETADuAHTAhAhe0V5Kq7KMBI9oLCuABEWIRiHgLKhgOhDKgAG5XZLEVBEAgAT2SYWAAH0vI5aAoriwmAoCHErjs9phEciiLj8QpHghkskjtgTBoAIIczk3fZCTAEfZeZL7BSoNj7AHJeiqfbhUmYdBigjELwOBQ80lqhz7ZSoMJCkwJKxc/bm/boZbRej7VSkfZoh0ALyOpu5CSBbp5WtV6qFABUSQBrQ4mnkWlXoEAgbVY/ZhZJ47HJJgEBDes0W0nR2Oq%2BPLc7J1PpzOcyNW962yP7RxsbG0PCgghl41ezbu306wMh0KksPuyM5mPE1nHYd58FGB1BhTBtn7UfY1BUbENOeajsRi189N4BQJg80cLb82nqMxuE1IjEI5uCdXhwkO/6w1shfE/OqZer1KGzeDjuVy1gg%2B6HvsYjivsx6YOe558g%2B17PuOBC5o%2BN53ohT63ihuajphqExnG056gagjvps7KUQuZIMOKxA/tio4KIxf6CAB8HAY83EgWB9zLLQyqQag%2ByxNB0w%2Bp6rZcnIAbBNiABSQgAPIRNiABK2AAGLBNgbgBhAr6CA69qOi68xlrJ8lKapGnabp%2BkQLOwYOomxZpggDqFkmBAph5Dp1pgDZNoQFlbjJcmKSpamaTpekGc5fYEA6S4rmuIYKClabgt%2BaVsQQChhZ2XKGouTBhBA8wDuenzxswbAKKmazCgQtDSeWFpoHRPZzkltYhsF/ZHEC1VZjW%2Bx8gAskwoorv1G7npGxqAeNS0mocZhmOuwbYlw5hmA6y0bWYbm%2BVwjwwi2m2HZt3nJudl37Q6lQaI8YYjW2pCLatR37dt2JmE9A7HadAMXWmQP7XdvlmODV0HfsXAABz%2BG9bbvd9Z6el9Y01lN2V4HKc2bCN%2BWarja0ratWMWMDGgOvT%2BwaJ9wNcAzDPve9OMddTNN089DMs0djOM1wnNtpjGO4xL4XcrjfLKI0Sj7DZETQciLAqt23ORruoEHsJlrWvQBsG1KCCYGmaL7PcTDW3NxEvFQaLxtq/pHiQlqYB8wAvEY54TmgeKHMcd77No4p%2BCAVAa9ibsOBA/2NqS8yPHSjuVa2Ulchwiy0Jw/i8H4HBaKQqCcG41jWGKyyrJgG2bDwpAEJoueLMGIAJOdXcJEjjeSEjZgAGwAJxcBogT5xwkhF63ZecLwlz0y3Je56QcCwDAiAoBKmL0GQFAQNKdCDLshjAFwQ8aPTNC0GSxCXBA0Rz9EYSNGinBN6/zDEGiynRNoa8n9eAAjYIIZSDBaAf1XqQLAPs3BiAJMA2BmBiRGHEDA/AxBrx4CvJcGBmBVA1DxOsUuhpMBT1Lo2aIdxf4eCwHPfMLBkFXmINENImAgSoPPo2IwrdFhUAMMABQAA1PAmB7jKWSIwZB/BBAiDEOwKQMhBCKBUOoGBug2YGD4aYSw1h9B4GiJcSAixUDJEcAIfBABaHMw09FWEsJUfY1jlJmF4PCOI4IsAmMqrYChODnAQFcGMPwbMQjTFKOUPQqR0iWKyJ4VoMSCjxL6FEuY/j7DxO6KMRJuQ2YdGyZMNJAwKjDB6KEvQpIeglNmBURY4oVhrAkHnAus8YHlw4HaJGQ9rFD0kFKHRwBEZD0eK9DQ%2BwIC4EIJ7LYXB5i8BXloeYiwLZMCwPEPxU8Z6kGYf4emxdS6dMXiAZe/D15bwgEgaG5BKDH33hEVg6xVA9L6QMs%2BRgRljLGbwJUMzvF6DkcIUQ4hlFArUWoOeWjSD3DuMkYBrSOCF1IIcjxnBlJ4h8vsOaLzen9MGefL54zJkeBYHvOIDd5mLP4e3EAkhRmj0kGYZGV9/Ajy7pffQnAdl7IOXPY5thTnNxpaQDuZgkaPHFcyoeKNKibA0EjEe9KuUcE2O0o5C9hWrxWSq9xKL%2BWaqWW3UgrD0jOEkEAA%3D%3D) ]

```cpp
// Set up some complex nested structs
struct Point {
    double x, y, z;
};

struct Task {
    std::string input_path;
    std::string output_path;
    double      time_limit;
};

struct TaskList {
    std::map<std::string, Task> map_of_tasks;
    // this is fine
    
    std::vector<std::vector<Point>> matrix_of_points;
    // this is also fine
    
    // std::vector<std::vector<std::vector<std::map<std::string, Point>>>> tensor_of_maps_of_points;
    // ... this would also be fine
    
    // std::array<std::unordered_map<std::string, Task>, 4> array_of_maps_of_tasks;
    // ... and so will be this
};

UTL_JSON_REFLECT(Point, x, y, z);
UTL_JSON_REFLECT(Task, input_path, output_path, time_limit);
UTL_JSON_REFLECT(TaskList, map_of_tasks, matrix_of_points);

// ...

using namespace utl;

const TaskList task_list = {
    // Map of tasks
    {
        { "task_1", { "input_1.dat", "output_1.dat", 170. } },
        { "task_2", { "input_2.dat", "output_2.dat", 185. } }
    },
    // Matrix of 3D points
    {
        { { 0, 0, 0}, { 1, 0, 0 } },
        { { 0, 1, 0}, { 0, 0, 1 } }
    }
};

// Parse JSON from struct,
// this also doubles as a cheaty way of stringifying structs for debugging
std::cout << json::from_struct(task_list).to_string();
```

Output:
```
{
    "map_of_tasks": {
        "task_1": {
            "input_path": "input_1.dat",
            "output_path": "output_1.dat",
            "time_limit": 170
        },
        "task_2": {
            "input_path": "input_2.dat",
            "output_path": "output_2.dat",
            "time_limit": 185
        }
    },
    "matrix_of_points": [
        [
            {
                "x": 0,
                "y": 0,
                "z": 0
            },
            {
                "x": 1,
                "y": 0,
                "z": 0
            }
        ],
        [
            {
                "x": 0,
                "y": 1,
                "z": 0
            },
            {
                "x": 0,
                "y": 0,
                "z": 1
            }
        ]
    ]
}
```

## Tests

`utl::json` parsing was [tested](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/tests/module_json.cpp) using the standard [RFC-8259](https://datatracker.ietf.org/doc/html/rfc8259) compliance [testing suite](https://github.com/nst/JSONTestSuite/) with following metrics:

| Metric | Compliance | Note |
| - | - | - |
| Parser accepts valid RFC-8259 JSON | **100%** | Full conformance |
| Parser rejects invalid RFC-8259 JSON | **93.6%** | Missing conformance of 6.4% is due to parser imposing less restrictions on the floating point format, it will accepts values such as `2.`, `01`, `2.e+3` and etc., which go beyond the default JSON specification. |

Parsing and serialization also satisfies [C++ `<charconv>`](https://en.cppreference.com/w/cpp/header/charconv) float round-trip guarantees (which means floats serialized by `utl::json` will be recovered to the exact same value when parsed again by the library).

## Benchmarks

[Benchmarks](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/benchmarks/benchmark_json.cpp) for parsing and serializing of minimized JSON data corresponding to various entries in the [test suite](https://github.com/DmitriBogdanov/prototyping_utils/tree/master/benchmarks/data). 

```
====== BENCHMARKING ON DATA: `strings.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               16.03 |               62.40 |    4.6% |      0.81 | `utl::json`
|    25.3% |               63.43 |               15.76 |    0.4% |      3.11 | `nlohmann`
|    47.5% |               33.71 |               29.66 |    0.3% |      1.65 | `PicoJSON`
|    94.4% |               16.98 |               58.90 |    0.2% |      0.83 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               15.85 |               63.08 |    0.7% |      0.78 | `utl::json`
|    24.4% |               64.88 |               15.41 |    0.2% |      3.19 | `nlohmann`
|    45.6% |               34.77 |               28.76 |    0.5% |      1.69 | `PicoJSON`
|    88.1% |               17.99 |               55.60 |    0.4% |      0.88 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               11.90 |               84.05 |    0.2% |      0.59 | `utl::json`
|    42.2% |               28.22 |               35.44 |    0.4% |      1.39 | `nlohmann`
|    33.1% |               36.00 |               27.78 |    0.2% |      1.76 | `PicoJSON`
|    75.5% |               15.77 |               63.42 |    0.5% |      0.77 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               12.67 |               78.93 |    0.7% |      0.62 | `utl::json`
|    41.7% |               30.41 |               32.88 |    3.4% |      1.52 | `nlohmann`
|    33.2% |               38.20 |               26.18 |    0.6% |      1.88 | `PicoJSON`
|    75.9% |               16.69 |               59.92 |    1.2% |      0.82 | `RapidJSON`


====== BENCHMARKING ON DATA: `numbers.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               10.95 |               91.35 |    0.7% |      0.54 | `utl::json`
|    22.9% |               47.79 |               20.92 |    0.6% |      2.35 | `nlohmann`
|    28.2% |               38.78 |               25.79 |    0.5% |      1.89 | `PicoJSON`
|   144.6% |                7.57 |              132.11 |    0.4% |      0.37 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               11.26 |               88.84 |    0.2% |      0.55 | `utl::json`
|    22.6% |               49.88 |               20.05 |    0.5% |      2.51 | `nlohmann`
|    28.1% |               39.99 |               25.00 |    1.4% |      1.96 | `PicoJSON`
|   128.9% |                8.73 |              114.51 |    3.1% |      0.43 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               11.18 |               89.44 |    1.0% |      0.55 | `utl::json`
|    65.2% |               17.14 |               58.33 |    0.4% |      0.85 | `nlohmann`
|    13.9% |               80.48 |               12.43 |    0.6% |      3.95 | `PicoJSON`
|    63.9% |               17.51 |               57.13 |    1.1% |      0.85 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               13.43 |               74.48 |    1.0% |      0.66 | `utl::json`
|    65.0% |               20.64 |               48.44 |    0.6% |      1.02 | `nlohmann`
|    16.3% |               82.14 |               12.17 |    0.4% |      4.04 | `PicoJSON`
|    74.3% |               18.06 |               55.38 |    0.4% |      0.89 | `RapidJSON`


====== BENCHMARKING ON DATA: `database.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               22.88 |               43.71 |    2.5% |      1.14 | `utl::json`
|    43.6% |               52.44 |               19.07 |    2.7% |      2.55 | `nlohmann`
|    71.7% |               31.89 |               31.35 |    3.5% |      1.69 | `PicoJSON`
|   221.3% |               10.34 |               96.73 |    1.4% |      0.52 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               28.24 |               35.41 |    4.5% |      1.34 | `utl::json`
|    48.9% |               57.78 |               17.31 |    2.3% |      2.84 | `nlohmann`
|    69.0% |               40.91 |               24.44 |    3.4% |      2.01 | `PicoJSON`
|   223.8% |               12.62 |               79.23 |    1.9% |      0.62 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               12.38 |               80.80 |    2.4% |      0.61 | `utl::json`
|    49.6% |               24.95 |               40.08 |    0.6% |      1.22 | `nlohmann`
|    33.7% |               36.72 |               27.23 |    1.2% |      1.82 | `PicoJSON`
|   124.9% |                9.91 |              100.94 |    2.4% |      0.48 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               16.18 |               61.79 |    1.1% |      0.79 | `utl::json`
|    62.0% |               26.09 |               38.33 |    0.4% |      1.29 | `nlohmann`
|    42.9% |               37.72 |               26.51 |    0.5% |      1.85 | `PicoJSON`
|   123.7% |               13.08 |               76.44 |    0.9% |      0.64 | `RapidJSON`
```

### Some thoughts on implementation

The main weak-point of `utl::json` from the performance point of view is parsing of large JSON dictionaries.

Unfortunately, the issue is mostly caused by `std::map` insertion, which dominates the runtime. A truly suitable for the purpose container doesn't really exist in the standard library, and would need a custom implementation like in `RapidJSON`, which would reduce the standard library interoperability thus going against the main purpose of this library which is simplicity of use.

Flat maps and async maps seem like the way to go, slotting in a custom flat map implementation into `json::_object_type_impl` allowed `utl::json` to beat `RapidJSON` on all serializing tasks and brought `database.json` parsing a more or less even ground:

```
// Using associative API wrapper for std::vector of pairs instead of std::map we can bridge the performance gap
// General-case usage however suffers, which is why this decision was ruled against

====== BENCHMARKING ON DATA: `database.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               13.22 |               75.65 |    0.6% |      0.95 | `utl::json`
|    24.7% |               53.57 |               18.67 |    3.6% |      3.93 | `nlohmann`
|    43.5% |               30.42 |               32.87 |    0.7% |      2.19 | `PicoJSON`
|   139.5% |                9.48 |              105.51 |    0.2% |      0.68 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               14.28 |               70.02 |    0.5% |      1.03 | `utl::json`
|    25.8% |               55.36 |               18.06 |    0.5% |      3.99 | `nlohmann`
|    41.1% |               34.75 |               28.78 |    0.2% |      2.50 | `PicoJSON`
|   130.4% |               10.95 |               91.33 |    0.4% |      0.79 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |                8.47 |              118.00 |    0.2% |      0.61 | `utl::json`
|    35.7% |               23.74 |               42.12 |    0.3% |      1.71 | `nlohmann`
|    23.6% |               35.85 |               27.89 |    0.4% |      2.57 | `PicoJSON`
|    94.4% |                8.97 |              111.43 |    0.3% |      0.65 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               10.09 |               99.12 |    0.4% |      0.73 | `utl::json`
|    39.2% |               25.74 |               38.85 |    0.5% |      1.85 | `nlohmann`
|    26.8% |               37.61 |               26.59 |    0.4% |      2.71 | `PicoJSON`
|    93.7% |               10.77 |               92.87 |    0.2% |      0.78 | `RapidJSON`
```
