


# utl::math

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**math** adds various helper functions that aim to reduce code verbosity when working with mathematical expressions.

## Definitions

```cpp
// Constants
double PI = 3.14159265358979323846;
double PI_TWO = 2. * PI;
double PI_HALF = 0.5 * PI;
double E = 2.71828182845904523536;
double GOLDEN_RATIO = 1.6180339887498948482;

// Unary type traits
template<typename Type>
struct is_addable_with_itself;

template<typename Type>
struct is_multipliable_by_scalar;

// Standard math functions
template<typename Type>
Type abs(Type x);

template<typename Type>
Type sign(Type x);

template<typename Type>
Type sqr(Type x); // x^2

template<typename Type>
Type cube(Type x); // x^3

template<typename Type>
Type midpoint(Type a, Type b); // (a + b) / 2

// Degrees and radians
template<typename Type>
Type deg_to_rad(Type degrees);

template<typename Type>
Type rad_to_deg(Type radians);

// Misc helpers
template<typename UintType>
UintType uint_difference(UintType a, UintType b);
		
// Branchless ternary
template<typename Type>
Type ternary_branchless(bool condition, Type return_if_true, Type return_if_false);

template<typename IntType>
IntType ternary_bitselect(bool condition, IntType return_if_true, IntType return_if_false);

template<typename IntType>
IntType ternary_bitselect(bool condition, IntType return_if_true); // return_if_false == 0
```

All methods have appropriate `enable_if<>` conditions and `constexpr` qualifiers, which are omitted in documentation for reduced verbosity.

Methods that deal with floating-point values require explicitly floating-point inputs for mathematical strictness.

## Methods

> ```cpp
> math::is_addable_with_itself
> ```

`is_addable_with_itself<Type>::value` returns at compile time whether `Type` objects can be added with `operator+()`.

(see and  [&lt;type_traits&gt;](https://en.cppreference.com/w/cpp/header/type_traits) and [UnaryTypeTrait](https://en.cppreference.com/w/cpp/named_req/UnaryTypeTrait))

> ```cpp
> math::is_multipliable_by_scalar
> ```

`is_multipliable_by_scalar<Type>::value` returns at compile time whether `Type` objects can be multiplied by a floating point scalar with `operator*()`.

> ```cpp
> Type math::abs(Type x);
> Type math::sign(Type x);
> Type math::sqr(Type x);
> Type math::cube(Type x);
> ```

Returns $|x|$, $\mathrm{sign} (x)$, $x^2$ or $x^3$ of an appropriate type.

> ```cpp
> Type math::midpoint(Type a, Type b);
> ```

Returns $\dfrac{a + b}{2}$ of an appropriate type. Can be used with vectors or other custom types that have defined `operator+()` and scalar `operator*()`.

> ```cpp
> Type math::deg_to_rad(Type degrees);
> Type math::rad_to_deg(Type radians);
> ```

Converts degrees to radians and back.

> ```cpp
> UintType math::uint_difference(UintType a, UintType b);
> ```

Returns $|a - b|$ for unsigned types accounting for possible integer overflow. Useful when working with small types in image processing.

> ```cpp
> Type math::ternary_branchless(bool condition, Type return_if_true, Type return_if_false);
> ```

Return `condition ? return_if_true : return_if_false` rewritten in a branchless way. Useful when working with GPU's.

> ```cpp
> IntType math::ternary_bitselect(bool condition, IntType return_if_true, IntType return_if_false);
> IntType math::ternary_bitselect(bool condition, IntType return_if_true);
> ```

Faster implementation of `ternary_branchless()` for integers. When second return is omitted, assumption `return_if_false == 0`  allows additional optimizations to be performed.

## Example 1 (type traits)

[ [Run this code](GODBOLT_LINK) ]
```cpp
using namespace utl;

std::cout
	<< std::boolalpha
	<< "are doubles addable?    -> " << math::is_addable_with_itself<double>::value << "\n"
	<< "are std::pairs addable? -> " << math::is_addable_with_itself<std::pair<int, int>>::value << "\n";
```

Output:
```
are doubles addable?    -> true
are std::pairs addable? -> false
```

## Example 2 (using math functions)

[ [Run this code](GODBOLT_LINK) ]
```cpp
using namespace utl;

std::cout
		<< "All methods below are constexpr and type agnostic:\n"
		<< "abs(-4) = "                               << math::abs(-4)                               << "\n"
		<< "sign(-4) = "                              << math::sign(-4)                              << "\n"
		<< "sqr(-4) = "                               << math::sqr(-4)                               << "\n"
		<< "cube(-4) = "                              << math::cube(-4)                              << "\n"
		<< "deg_to_rad(180.) = "                      << math::deg_to_rad(180.)                      << "\n"
		<< "rad_to_deg(PI) = "                        << math::rad_to_deg(math::PI)                  << "\n"
		<< "\n"
		<< "uint_difference(5u, 17u) = "              << math::uint_difference(5u, 17u)              << "\n"
		<< "\n"
		<< "ternary_branchless(true, 3.12, -4.17) = " << math::ternary_branchless(true, 3.12, -4.17) << "\n"
		<< "ternary_bitselect(true, 15, -5) = "       << math::ternary_bitselect(true, 15, -5)       << "\n"
		<< "ternary_bitselect(false, 15) = "          << math::ternary_bitselect(false, 15)          << "\n";
```

Output:
```
All methods below are constexpr and type agnostic:
abs(-4) = 4
sign(-4) = -1
sqr(-4) = 16
cube(-4) = -64
deg_to_rad(180.) = 3.14159
rad_to_deg(PI) = 180

uint_difference(5u, 17u) = 12

ternary_branchless(true, 3.12, -4.17) = 3.12
ternary_bitselect(true, 15, -5) = 15
ternary_bitselect(false, 15) = 0
```