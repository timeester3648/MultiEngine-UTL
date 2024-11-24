# utl::json

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**json** module aims to provide an intuitive JSON manipulation API similar to [nlohmann_json](https://github.com/nlohmann/json) while being a bit more lightweight and explicit about the underlying type conversions. The key difference are:

- `utl::json` doesn't introduce **any** global identifiers (including macros and operators)
- Objects support transparent comparators (which means `std::string_view` and `const char*` can be used for lookup)
- [Decent performance](#benchmarks) without relying on compiler intrinsics
- All JSON types map to standard library containers, no need to learn custom APIs
- Simple integration (single header, barely over `1k` lines)
- [Nice error messages](#error-handling)

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
using Number = Node::number_type;
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

## Examples 

### Import/Export JSON

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:23,endLineNumber:7,positionColumn:23,positionLineNumber:7,selectionStartColumn:23,selectionStartLineNumber:7,startColumn:23,startLineNumber:7),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++//+Export+JSON%0A++++json::Node+config%3B%0A%0A++++config%5B%22auxiliary_info%22%5D+++++++%3D+true%3B%0A++++config%5B%22date%22%5D+++++++++++++++++%3D+%222024.04.02%22%3B%0A++++config%5B%22options%22%5D%5B%22grid_size%22%5D+%3D+120%3B%0A++++config%5B%22options%22%5D%5B%22phi_order%22%5D+%3D+5%3B%0A++++config%5B%22scaling_functions%22%5D++++%3D+%7B+%22identity%22,+%22log10%22+%7D%3B%0A++++config%5B%22time_steps%22%5D+++++++++++%3D+500%3B%0A++++config%5B%22time_period%22%5D++++++++++%3D+1.24709e%2B2%3B%0A%0A++++config.to_file(%22config.json%22)%3B%0A%0A++++//+Import+JSON%0A++++config+%3D+json::from_file(%22config.json%22)%3B%0A%0A++++std::cout+%3C%3C+config.to_string()%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

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

config.to_file("config.json");

// Import JSON
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

### Setters & Type Conversions

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

### Error Handling

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:6,endLineNumber:19,positionColumn:6,positionLineNumber:19,selectionStartColumn:6,selectionStartLineNumber:19,startColumn:6,startLineNumber:19),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++const+auto+invalid_json+%3D+R%22(%0A++++++++%7B%0A++++++++++++%22key_1%22:+%22value_1%22,%0A++++++++++++%22key_2%22:++value_2%22,%0A++++++++++++%22key_3%22:+%22value_3%22%0A++++++++%7D%0A++++)%22%3B%0A%0A++++try+%7B%0A++++++++utl::json::from_string(invalid_json)%3B%0A++++%7D%0A++++catch+(std::runtime_error+%26e)+%7B%0A++++++++std::cerr+%3C%3C+%22ERROR:+Caught+exception:%5Cn%5Cn%22+%3C%3C+e.what()%3B%0A++++%7D%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

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
    utl::json::from_string(invalid_json);
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
|   100.0% |               19.14 |               52.24 |    0.3% |      1.38 | `utl::json`
|    32.4% |               59.05 |               16.93 |    0.4% |      4.26 | `nlohmann`
|    54.5% |               35.15 |               28.45 |    0.8% |      2.53 | `PicoJSON`
|   118.7% |               16.13 |               62.00 |    0.2% |      1.16 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               19.49 |               51.31 |    0.4% |      1.40 | `utl::json`
|    32.2% |               60.52 |               16.52 |    0.5% |      4.37 | `nlohmann`
|    55.8% |               34.92 |               28.64 |    0.3% |      2.52 | `PicoJSON`
|   121.7% |               16.01 |               62.45 |    0.1% |      1.15 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               12.45 |               80.34 |    0.1% |      0.91 | `utl::json`
|    43.8% |               28.45 |               35.15 |    0.2% |      2.05 | `nlohmann`
|    33.3% |               37.38 |               26.75 |    0.2% |      2.69 | `PicoJSON`
|    81.3% |               15.32 |               65.28 |    0.2% |      1.11 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               15.45 |               64.72 |    0.6% |      1.12 | `utl::json`
|    51.2% |               30.16 |               33.15 |    0.2% |      2.18 | `nlohmann`
|    39.2% |               39.41 |               25.38 |    0.2% |      2.84 | `PicoJSON`
|    95.1% |               16.26 |               61.51 |    0.2% |      1.17 | `RapidJSON`


====== BENCHMARKING ON DATA: `numbers.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               10.82 |               92.46 |    0.5% |      0.78 | `utl::json`
|    23.6% |               45.80 |               21.84 |    0.1% |      3.30 | `nlohmann`
|    28.1% |               38.51 |               25.97 |    0.2% |      2.77 | `PicoJSON`
|   144.4% |                7.49 |              133.47 |    0.1% |      0.54 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               11.17 |               89.56 |    0.1% |      0.80 | `utl::json`
|    23.1% |               48.29 |               20.71 |    0.2% |      3.48 | `nlohmann`
|    27.3% |               40.89 |               24.45 |    0.2% |      2.95 | `PicoJSON`
|   126.3% |                8.84 |              113.14 |    0.4% |      0.64 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               10.93 |               91.52 |    0.4% |      0.83 | `utl::json`
|    65.2% |               16.77 |               59.63 |    0.7% |      1.24 | `nlohmann`
|    13.5% |               80.96 |               12.35 |    0.9% |      5.93 | `PicoJSON`
|    64.2% |               17.02 |               58.77 |    0.2% |      1.23 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               13.11 |               76.29 |    0.2% |      0.94 | `utl::json`
|    65.8% |               19.93 |               50.18 |    0.3% |      1.44 | `nlohmann`
|    15.7% |               83.44 |               11.98 |    0.2% |      6.05 | `PicoJSON`
|    68.8% |               19.05 |               52.50 |    0.1% |      1.37 | `RapidJSON`


====== BENCHMARKING ON DATA: `database.json` ======

| relative |               ms/op |                op/s |    err% |     total | Parsing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:-----------------------
|   100.0% |               20.31 |               49.25 |    1.0% |      1.48 | `utl::json`
|    42.6% |               47.66 |               20.98 |    0.3% |      3.44 | `nlohmann`
|    62.5% |               32.51 |               30.76 |    0.5% |      2.35 | `PicoJSON`
|   206.4% |                9.84 |              101.62 |    0.3% |      0.71 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Parsing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:------------------------
|   100.0% |               21.43 |               46.65 |    0.7% |      1.54 | `utl::json`
|    42.2% |               50.77 |               19.70 |    0.2% |      3.66 | `nlohmann`
|    61.3% |               34.96 |               28.61 |    1.2% |      2.51 | `PicoJSON`
|   194.5% |               11.02 |               90.74 |    0.2% |      0.79 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing minimized JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:---------------------------
|   100.0% |               11.69 |               85.54 |    1.0% |      0.84 | `utl::json`
|    50.3% |               23.24 |               43.04 |    0.4% |      1.68 | `nlohmann`
|    33.2% |               35.22 |               28.39 |    0.2% |      2.54 | `PicoJSON`
|   116.9% |               10.00 |              100.04 |    0.3% |      0.72 | `RapidJSON`

| relative |               ms/op |                op/s |    err% |     total | Serializing prettified JSON
|---------:|--------------------:|--------------------:|--------:|----------:|:----------------------------
|   100.0% |               13.70 |               72.98 |    0.3% |      1.01 | `utl::json`
|    52.6% |               26.03 |               38.41 |    0.2% |      1.88 | `nlohmann`
|    36.4% |               37.63 |               26.57 |    0.4% |      2.70 | `PicoJSON`
|   114.2% |               12.00 |               83.32 |    0.2% |      0.87 | `RapidJSON`
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
