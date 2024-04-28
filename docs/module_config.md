
# utl::config

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**config** contains function for simple export/import of JSON configs.

## Definitions

```cpp
template<typename... Types>
void export_json(std::string_view path, const Types... entries);

template<typename T>
std::tuple<std::string_view, T> entry(std::string_view key, T value);
```

## Methods

> ```cpp
> config::export_json(std::string_view path, const Types... entries);
> ```

Creates JSON at `path` with given `entries`. Entries can be specified using `config::entry(key, value)`. Corresponding to the JSON standard, `value` can be an **integer**, **float**, **string** or **bool** (or any of the similar types). Arrays and nested array are also supported as long as their types are homogenous.

> ```cpp
> std::tuple<std::string_view, T> config::entry(std::string_view key, T value);
> ```

Specifies a config entry.

Performs type resolution on `value` which helps other functions resolve a proper JSON datatype without being manually specified by user.

## Example 1 (saving JSON config)

[ [Run this code]() ]
```cpp
config::export_json(
    "config.json",
    config::entry("date",              "2024.04.02"              ),
    config::entry("time_steps",        500                       ),
    config::entry("time_period",       1.24709e+2                ),
    config::entry("auxiliary_info",    true                      ),
    config::entry("scaling_functions", { "identity", "log10" }   ),
    config::entry("options",           { 17, 45 }                ),
    config::entry("coefs",             { 0.125, 0.3 }            ),
    config::entry("flags",             { true, true, false }     ),
    config::entry("matrix",            { { 0.7, -1.3 }, { 3.1 } })
        // type-homogenous 2D, 3D, 4D arrays are also fine
);
```

Output:
```
// In file "config.json"

{
    "date": "2024.04.02",
    "time_steps": 500,
    "time_period": 124.709,
    "auxiliary_info": true,
    "scaling_functions": [ "identity", "log10" ],
    "options": [ 17, 45 ],
    "coefs": [ 0.125, 0.3 ],
    "flags": [ true, true, false ],
    "matrix": [ [ 0.7, -1.3 ], [ 3.1 ] ]
}
```