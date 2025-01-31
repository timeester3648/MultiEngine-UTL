# utl::enum_reflect

[<- back to README.md](https://github.com/DmitriBogdanov/UTL/tree/master)

**enum_reflect** is a lean `enum` reflection library based around the [map-macro](https://github.com/swansontec/map-macro).

> [!Note]
> Implementation header can be found [here](../include/UTL/enum_reflect.hpp).

## Definitions

```cpp
// Macros
#define UTL_ENUM_REFLECT(enum_name, ...)

// Reflection
template <class E> constexpr std::string_view type_name;
template <class E> constexpr std::size_t      size;

template <class E> constexpr std::array<std::string_view, size<E>>               names;
template <class E> constexpr std::array<E, size<E>>                              values;
template <class E> constexpr std::array<std::pair<std::string_view, E>, size<E>> entries;

template <class E> constexpr bool      is_valid(E value) noexcept;
template <class E> constexpr auto to_underlying(E value) noexcept;

template <class E> constexpr std::string_view   to_string(E value);
template <class E> constexpr E                from_string(std::string_view str);
```

## Methods

> ```cpp
> template <class E> constexpr std::string_view type_name;
> ```

Evaluates to stringified name of `E` enum.

> ```cpp
> template <class E> constexpr std::size_t size;
> ```

Evaluates to a number of elements in `E` enum.

> ```cpp
> template <class E> constexpr std::array<std::string_view, size<E>> names;
> ```

Evaluates to an array of stringified element names corresponding to `E` enum.

> ```cpp
> template <class E> constexpr std::array<E, size<E>> values;
> ```

Evaluates to an array of elements corresponding to `E` enum.

> ```cpp
> template <class E> constexpr std::array<std::pair<std::string_view, E>, size<E>> entries;
> ```

Evaluates to an array of name-value pairs corresponding to `E` enum.

> ```cpp
> template <class E> constexpr bool is_valid(E value) noexcept;
> ```

Returns whether enum-typed `value` is a valid element of `E` enum. See [examples](#reflecting-an-enum).

> ```cpp
> template <class E> constexpr auto to_underlying(E value) noexcept;
> ```

Equivalent to `static_cast<std::underlying_type_t<E>>(value)`. In C++23 can be replaced with [`std::to_underlying()`](https://en.cppreference.com/w/cpp/utility/to_underlying).

**Note:** This particular function is included for convenience and does not require `E` to be reflected.

> ```cpp
> template <class E> constexpr std::string_view to_string(E value);
> ```

Returns string corresponding to a `value` from `E` enum.

Throws [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) if `value` is not a part of enum.

> ```cpp
> template <class E> constexpr E from_string(std::string_view str);
> ```

Returns value from `E` enum corresponding to a string `str`.

Throws [`std::out_of_range`](https://en.cppreference.com/w/cpp/error/out_of_range) if `str` does not correspond to any element of the enum.

## Examples

### Reflecting an enum

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCDSAA6oCoRODB7evnoJSY4CQSHhLFEx0naYDilCBEzEBGk%2Bfly2mPY5DJXVBHlhkdGxtlU1dRmNCgOdwd2FvZIAlLaoXsTI7BzmAMzByN5YANQma24IBARxCiAA9OfETADuAHTAhAheEV5KS7KMBHdoLOcAIixCMQ8BZUMB0IZUAA3c5yAAqAXOLCYo2i5ySRnoAH1NttMHDEXcEHE4vtsCYNABBSlUxg%2BHZbVEKHZCPC7EwAdgsOwC2AAYvC9mt/jsALSNHYAJQAkgBxAASQv2osloQA8qFsMLRRo9pz/vsrNTaZdpZhHmjiDtiJgqC0UrSEQFsdhQnIALLYqUCvlueEQNlYUi8gXwkOyxXhnYarUzI202nBAg7FHBCAzfXGqk7XM7N7BYA7ZhsBRxJjLfMEWgJk3UvM7M2KTA2u0OgS0hsFozF1iYMsVlteau1mn1vOjJiOZDY5nRAgQHb0ljY232soEEAgAgATzimGxJcw%2BzcQePa21KpVezMZjP5jMO3ja2znYnVWns4U7wXS4YPlXNsNy3JIAC9z1Pdlz0vEVrzWJ9RzfXNJ0/OcakXZdAPXBwtyPBQT3vC8TAAVgsDQSNFK8RRvMw%2BUFB9c2fbMGxQvAZzQ39MLXFpcL7fCDkIilSK4CidWvB9IyVBimKQnZWPY795ww/8V244CQDwgioPJEiLDMUSqMo29YwpW9GMQ8dkI/Nivx/ZSALUnCQGhMQvH7LSsB00jyOIyjYOos8tzooUEJfWT5NspS/wcoCnJc7x3IE7SiOEgz/NFQKQEkoUZMsuTrIUuzotU2LN2c1zEsgzyUr0tLDQCqCtxM8ywrrHN3ynGyOPskrsLKr4QUqwTdJ8/47hoYhRlzQyaOChiG1C5iOtQxT0OKrCeJAAa8CG5KhIsETfPGvBJpTMTqIk%2BUpLMhtcvaqzOsKqKuNKrdtt26r9v0o6Jqm86jLMEz5tu0cWIKyK1pevq3sEQb%2BKq6CRoou4lDQBh0H%2B1lGpAYKWqWh6VqKqHNve%2BHhtSo7UYEDGZsy7LFvC8HuvWxz%2Bthnayb23TvrGqn0cxzLmoZtqwceiHOJUjb1LwBRsXi9lA207kJS5f5MxmghiDcvHGbF5niel2X5fQRXPO5HYRINdX0p2TXteF%2B78r11aJZi6GQBluWxAV%2B9zfIq3MbtlsHdFwnnsl1mt0943TePc39IDmaqDEJQHd1sPIYj17t1QbEvHR6JaB3QtY6CsMn0xiUQ%2BWrqXZ6qWnKIPOC%2BIIuS7pq74Wt%2BrRQt6uCdroms/dpv86wVvi6MUuQGa7vrx2PU7vTwfw7dzam9GEEp8y3G54u285puu7Q5XzO1/UjfNfb7Hsr3gHsuk0Ga6es/evX3PN%2BvrAms1bU75ooGR8LKOwivrYem0qDEFQCuT%2BRgPLQQgA%2BQ%2Bj5/473Lv3J2GdXZv3UpA6B2JYHAHgeSRBt4H63lQTfTuGDQF1xZtnPBMCr5wKSp9UhgNf7SQFtjIWS8DS0g4HMWgnBiK8D8BwLQpBUCcDcNYawckFhLBbOsHgpBNwSMEXMAA1iAAAbMRO4axOQAA5iIAE4NAaBMWsMwxizHGP0JwSQYjNC8GkRwXgZwNBqNcXMOAsAYCIBQNAuIdBojkEoL8UJ9AYhMiMFwXRli%2BB0AINEM4EAIiuNIBEYI1QdycFUTk5gxAdzqgiNoYC3BeC/DYIIdUDAi5ZKwK8YAbgxC0DOFU0gWAURYhWJI/AtpyjQn7FkzAqgyjDn6bwZMzQsm0DwBEa4JSPDfw0WokELACm8BGcQCIiRMD/EwL04ACyjC%2BL4AYYACgABqO0bjqn3OI1R/BBAiDEOwKQMhBDNjUFk3QjQDDnNMJYaw%2BhFlnEgHMVAcRWidLFKMdAKoQVWEsFwTk4p1RmDcbskEWBIUZiaO2PwEBXBDAaKQQIEwChFEyIkZIAhyV0uyCkLoNLegjGaBuCoYwmWcuJe0GobKegxBGLyzw9Q9CTiFdSkVEg5gKEUcseVjiOCiNIOIyR7idiqGMbosUujJCMiBUWBJdwNDmp2BAXAhASA3jWFwGYvB1FaBmHMBAmAmDj0oEIpxvAtnEW8ZqtxnBPEgG8S6wRpB/FBIWCcYcESIBRLCcQUIfZOC6v1Ya41hhTW6PNea3gmB8BEDxXoV5whRDiC%2BRW356h1kAtIDca4cRtm%2BrVS49Z7j1TDjiMOHYqAqA6r1Qao1cS80Fr1BADwLBonRHtY651vjtGxHzbosxkgzBcD1RocxnIuAJNVc40gAag1ZPcWGiNy7SA6LWJIO4uit2SA0GYsxW7dH7o0GYaQwiOBrE7Vq0NPiNFutVdijV56gORtA7spIzhJBAA%3D%3D%3D) ]

```cpp
enum class Side { LEFT = -1, RIGHT = 1, NONE = 0 };

// Register reflection
UTL_ENUM_REFLECT(Side, LEFT, RIGHT, NONE);

// ...

// Use reflection
using namespace utl;

static_assert( enum_reflect::type_name<Side> == "Side" );

static_assert( enum_reflect::size<Side> == 3 );

static_assert( enum_reflect::names<Side>[0] == "LEFT"  );
static_assert( enum_reflect::names<Side>[1] == "RIGHT" );
static_assert( enum_reflect::names<Side>[2] == "NONE"  );

static_assert( enum_reflect::values<Side>[0] == Side::LEFT  );
static_assert( enum_reflect::values<Side>[1] == Side::RIGHT );
static_assert( enum_reflect::values<Side>[2] == Side::NONE  );

static_assert( enum_reflect::entries<Side>[0].first  == "LEFT"      );
static_assert( enum_reflect::entries<Side>[1].first  == "RIGHT"     );
static_assert( enum_reflect::entries<Side>[2].first  == "NONE"      );
static_assert( enum_reflect::entries<Side>[0].second == Side::LEFT  );
static_assert( enum_reflect::entries<Side>[1].second == Side::RIGHT );
static_assert( enum_reflect::entries<Side>[2].second == Side::NONE  );

static_assert( enum_reflect::is_valid(Side{-1}) == true  );
static_assert( enum_reflect::is_valid(Side{ 1}) == true  );
static_assert( enum_reflect::is_valid(Side{ 0}) == true  );
static_assert( enum_reflect::is_valid(Side{ 2}) == false );

static_assert( enum_reflect::to_underlying(Side::LEFT ) == -1 );
static_assert( enum_reflect::to_underlying(Side::RIGHT) ==  1 );
static_assert( enum_reflect::to_underlying(Side::NONE ) ==  0 );

static_assert( enum_reflect::to_string(Side::LEFT ) == "LEFT"  );
static_assert( enum_reflect::to_string(Side::RIGHT) == "RIGHT" );
static_assert( enum_reflect::to_string(Side::NONE ) == "NONE"  );

static_assert( enum_reflect::from_string<Side>("LEFT" ) == Side::LEFT  );
static_assert( enum_reflect::from_string<Side>("RIGHT") == Side::RIGHT );
static_assert( enum_reflect::from_string<Side>("NONE" ) == Side::NONE  );
```