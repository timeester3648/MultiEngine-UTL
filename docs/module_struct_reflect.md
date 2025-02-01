# utl::struct_reflect

[<- back to README.md](..)

**struct_reflect** is a lean `struct` reflection library based around the [map-macro](https://github.com/swansontec/map-macro).

> [!Note]
> Implementation header can be found [here](../include/UTL/struct_reflect.hpp).

## Definitions

```cpp
// Macros
#define UTL_STRUCT_REFLECT(struct_name, ...)

// Reflection
template <class S> constexpr std::string_view type_name;
template <class S> constexpr std::size_t      size;

template <class S> constexpr std::array<std::string_view, size<E>> names;
template <class S> constexpr auto field_view(S&& value);
template <class S> constexpr auto entry_view(S&& value);

template <std::size_t I, class S> constexpr auto get(S&& value);

template <class S, class Func>
constexpr void for_each(S&& value, Func&& func);

template <class S1, class S2, class Func>
constexpr void for_each(S1&& value_1, S2&& value_2, Func&& func);

template <class S, class Pred>
constexpr bool true_for_all(const S& value, Pred&& pred);

template <class S1, class S2, class Pred>
constexpr bool true_for_all(const S1& value_1, const S2& value_2, Pred&& pred);

// Other utils
template <class T, class Func>
void tuple_for_each(T&& tuple, Func&& func);

template <class T1, class T2, class Func>
void tuple_for_each(T1&& tuple_1, T2&& tuple_2, Func&& func);
```

## Methods

### Macros

> ```cpp
> #define UTL_STRUCT_REFLECT(struct_name, ...)
> ```

Registers reflection for the `struct` / `class` type `struct_name` with member variables `...`.

### Reflection

> ```cpp
> template <class S> constexpr std::string_view type_name;
> ```

Evaluates to a stringified name of struct `S`.

> ```cpp
> template <class S> constexpr std::size_t size;
> ```

Evaluates to a number of fields in the struct `S` .

> ```cpp
> template <class S> constexpr std::array<std::string_view, size<E>> names;
> ```

Evaluates to an array of stringified field names corresponding to struct `S`.

> ```cpp
> template <class S> constexpr auto field_view(S&& value);
> ```

Returns a tuple with perfectly-forwarded references corresponding to the fields of `value`.

Below is an example table for the reflection of `struct Struct { int x; };`:

| Value category                              | Forwared reference                   | Return type             |
| ------------------------------------------- | ------------------------------------ | ----------------------- |
| `value` is a const reference to a struct    | `S&&` corresponds to `const Struct&` | `std:tuple<const int&>` |
| `value` is an l-value reference to a struct | `S&&` corresponds to `Struct&`       | `std:tuple<int&>`       |
| `value` is an r-value reference to a struct | `S&&` corresponds to `Struct&&`      | `std:tuple<int&&>`      |

> [!Tip]
> This effectively means that `field_view` allows struct members to be accessed exactly as one would expect when working with struct members directly, except using a tuple API. See [examples]().

> ```cpp
> template <class S> constexpr auto entry_view(S&& value);
> ```

Returns a tuple with pairs of names and perfectly-forwarded references corresponding to the fields of `value`.

Reference forwarding logic is exactly the same as it is in `field_view()`.

> ```cpp
> template <std::size_t I, class S> constexpr auto get(S&& value);
> ```

Returns perfectly-forwarded reference to the field number `I` in `value`.

> ```cpp
> template <class S, class Func>
> constexpr void for_each(S&& value, Func&& func);
> ```

Applies function `func` to all fields of the struct `value`.

**Note:** `func` must be callable for all field types, either through overloads or templating.

> ```cpp
> template <class S1, class S2, class Func>
> constexpr void for_each(S1&& value_1, S2&& value_2, Func&& func);
> ```

Applies function `func` to all fields of a struct pair `value_1`, `value_2`.

**Note:** This is useful for defining binary functions over custom types, see [examples]().

> ```cpp
> template <class S, class Pred>
> constexpr bool true_for_all(const S& value, Pred&& pred);
> ```

Returns whether unary predicate `pred` is satisfied for all fields of the `value`.

**Note:** Predicate checks cannot be efficiently implemented in terms of `for_each()` due to potential short-circuiting of logical AND. Use this function instead.

> ```cpp
> template <class S1, class S2, class Pred>
> constexpr bool true_for_all(const S1& value_1, const S2& value_2, Pred&& pred);
> ```

Returns whether binary predicate `pred` is satisfied for all fields of a struct pair `value_1`, `value_2`.

### Other utils

> ```cpp
> template <class T, class Func>
> constexpr void tuple_for_each(T&& tuple, Func&& func)
> ```

Applies unary function `func` to all elements of the tuple `tuple`.

**Note:** This is not a part reflection, the function is provided for convenience when working with tuples in general.

> ```cpp
> template <class T1, class T2, class Func>
> constexpr void tuple_for_each(T1&& tuple_1, T2&& tuple_2, Func&& func)
> ```

Applies binary function `func` to all elements of the tuple pair `tuple_1`, `tuple_2`.

**Note:** This is not a part reflection, the function is provided for convenience when working with tuples in general.

## Examples

### Basic reflection

[ [Run this code]() ]

```cpp
// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

// Test basic reflection
static_assert( struct_reflect::type_name<Quaternion> == "Quaternion" );

static_assert( struct_reflect::size<Quaternion> == 4 );

static_assert( struct_reflect::names<Quaternion>[0] == "r" );
static_assert( struct_reflect::names<Quaternion>[1] == "i" );
static_assert( struct_reflect::names<Quaternion>[2] == "j" );
static_assert( struct_reflect::names<Quaternion>[3] == "k" );
```

### Field & entry views

[ [Run this code]() ]

```cpp
// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

// Test field & entry views
constexpr Quaternion q = { 5., 6., 7., 8. };

static_assert( struct_reflect::field_view(q) == std::tuple{ 5., 6., 7., 8. } );

static_assert( std::get<0>(struct_reflect::entry_view(q)).first  == "r" );
static_assert( std::get<0>(struct_reflect::entry_view(q)).second == 5.  );
static_assert( std::get<1>(struct_reflect::entry_view(q)).first  == "i" );
static_assert( std::get<1>(struct_reflect::entry_view(q)).second == 6.  );
static_assert( std::get<2>(struct_reflect::entry_view(q)).first  == "j" );
static_assert( std::get<2>(struct_reflect::entry_view(q)).second == 7.  );
static_assert( std::get<3>(struct_reflect::entry_view(q)).first  == "k" );
static_assert( std::get<3>(struct_reflect::entry_view(q)).second == 8.  );

static_assert( struct_reflect::get<0>(q) == 5. );
static_assert( struct_reflect::get<1>(q) == 6. );
static_assert( struct_reflect::get<2>(q) == 7. );
static_assert( struct_reflect::get<3>(q) == 8. );
```

### Using reflection to define binary operations

[ [Run this code]() ]

```cpp
// Define struct & reflection
struct Quaternion { double r, i, j, k; }; // could be any struct with a lot of fields

UTL_STRUCT_REFLECT(Quaternion, r, i, j, k);

// Define binary operation (member-wise addition)
constexpr Quaternion operator+(const Quaternion& lhs, const Quaternion &rhs) noexcept {
    Quaternion res = lhs;
    utl::struct_reflect::for_each(res, rhs, [&](auto& l, const auto& r){ l += r; });
    return res;
}

// Define binary operation with predicates (member-wise equality)
constexpr bool operator==(const Quaternion& lhs, const Quaternion &rhs) noexcept {
    bool res = true;
    utl::struct_reflect::true_for_all(lhs, rhs, [&](const auto& l, const auto& r){ return l == r; });
    return res;
}

// Test operations
static_assert( Quaternion{1, 2, 3, 4} + Quaternion{5, 6, 7, 8} == Quaternion{6, 8, 10, 12} );
```

### Iterating over a generic tuple

[ [Run this code]() ]

```cpp
using namespace utl;

std::tuple<std::string, int   > tuple_1{ "lorem", 2 };
std::tuple<const char*, double> tuple_2{ "ipsum", 3 };

// Print tuple
struct_reflect::tuple_for_each(tuple_1, [&](auto&& x){ std::cout << x << '\n'; });

// Print tuple sum
struct_reflect::tuple_for_each(tuple_1, tuple_2, [&](auto&& x, auto&& y){ std::cout << x + y << '\n'; });

// notice that tuples don't have to be homogenous,
// what matters is that binary function can be called on all corresponding pairs
```

Output:

```
lorem
2
loremipsum
5
```

