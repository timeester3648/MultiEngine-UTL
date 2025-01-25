# utl::rfl

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**rfl** is a "lean and mean" reflection library. It provides `enum` and `struct`  reflection.

## Definitions

```cpp
// Enum reflection
struct Enum<E> {
    using type            = E;
    using underlying_type = std::underlying_type_t<E>;
    
    constexpr Enum(E enum_value) noexcept;
    
    // Content reflection
    constexpr static std::string_view type_name;
    
    constexpr static std::array<E,                              N> values;
    constexpr static std::array<std::string_view,               N> names;
    constexpr static std::array<std::pair<std::string_view, E>, N> entries;
    
    // String conversion
    constexpr std::string_view to_string() const;
    constexpr static type from_string(std::string_view str);
    
    // Getters
    constexpr type            get()            const noexcept;
    constexpr underlying_type get_underlying() const noexcept;
};

#define UTL_RFL_ENUM(enum_name, ...)
    
// Struct reflection
    
constexpr std::array<E, N>                              values;
constexpr std::array<std::string_view, N>               names;
constexpr std::array<std::pair<std::string_view, E>, N> entries;
```

**Alternative:**

```cpp
// Enum reflection
enum_reflect::type_name<Side>;
enum_reflect::size<Side>;

enum_reflect::names<Side>;
enum_reflect::values<Side>;
enum_reflect::entries<Side>;

enum_reflect::to_string(Side::LEFT);
enum_reflect::from_string("LEFT");

UTL_ENUM_REFLECT(Side, LEFT, RIGHT);

// Struct reflection
struct_reflect::type_name<Point>;
struct_reflect::size<Side>;

struct_reflect::names<Point>;
struct_reflect::field_view(point);
struct_reflect::const_field_view(point);
struct_reflect::entry_view(point);
struct_reflect::const_entry_view(point);

struct_reflect::get<0>(point);
struct_reflect::for_each(point);
    
UTL_STRUCT_REFLECT(Point, x, y);
```




## Methods

> ```cpp
> MASK
> ```

DESC

## Examples

### EXAMPLE_NAME

[ [Run this code]() ]
```cpp
using namespace utl;

```

Output:
```

```

## Reflection mechanism

Reflection mechanism of this library is based entirely around the [map macro](https://github.com/swansontec/map-macro) and avoids any kind of compiler-specific logic.

This also leads to a rather concise implementation — 90 lines of code cover the entirety of `enum` reflection, while structures take up ???.

### Enum reflection

Another possible approach to `enum` reflection is [magic_enum](https://github.com/Neargye/magic_enum)-like compile-time parsing of strings returned by `__PRETTY_FUNCTION__`, `__FUNCSIG__` and C++20 `std::source_location`, this has a benefit of not needing the reflection macro, however this convenience introduces several downsides:

- Significant increase in compile times due to brute-force template instantiation;
- Arbitrary enums are no longer supported, all values should reside in a certain range;
- If not implemented properly, function signature strings lead to binary bloat.

Which is why a more robust macro-based approach was chosen.

### Struct reflection