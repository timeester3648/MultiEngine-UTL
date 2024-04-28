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
template<typename FloatType>
FloatType deg_to_rad(FloatType degrees);

template<typename Type>
FloatType rad_to_deg(FloatType radians);

// Misc helpers
template<typename UintType>
UintType uint_difference(UintType a, UintType b);

template<typename FloatType>
std::vector<FloatType> linspace(FloatType min, FloatType max, size_t N);

template<typename SizedContainer>
int ssize(const SizedContainer& container);
		
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
> FloatType math::deg_to_rad(FloatType degrees);
> FloatType math::rad_to_deg(FloatType radians);
> ```

Converts degrees to radians and back.

> ```cpp
> UintType math::uint_difference(UintType a, UintType b);
> ```

Returns $|a - b|$ for unsigned types accounting for possible integer overflow. Useful when working with small types in image processing.

> ```cpp
> std::vector<FloatType> math::linspace(FloatType min, FloatType max, size_t N);
> ```

Returns $N$ evenly spaced number in $[min, max]$ range.

> ```cpp
> int math::ssize(const SizedContainer& container);
> ```

Returns `.size()` value of given container casted to `int`.

Useful to reduce verbosity of `static_cast<int>(container.size())` when working with `int` indexation that gets compared against container size.

> ```cpp
> Type math::ternary_branchless(bool condition, Type return_if_true, Type return_if_false);
> ```

Returns `condition ? return_if_true : return_if_false` rewritten in a branchless way. Useful when working with GPU's.

> ```cpp
> IntType math::ternary_bitselect(bool condition, IntType return_if_true, IntType return_if_false);
> IntType math::ternary_bitselect(bool condition, IntType return_if_true);
> ```

Faster implementation of `ternary_branchless()` for integers. When second return is omitted, assumption `return_if_false == 0`  allows additional optimizations to be performed.

## Example 1 (type traits)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:14,endLineNumber:6,positionColumn:14,positionLineNumber:6,selectionStartColumn:14,selectionStartLineNumber:6,startColumn:14,startLineNumber:6),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++std::cout%0A++++++++%3C%3C+std::boolalpha%0A++++++++%3C%3C+%22are+doubles+addable%3F++++-%3E+%22+%3C%3C+math::is_addable_with_itself%3Cdouble%3E::value+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22are+std::pairs+addable%3F+-%3E+%22+%3C%3C+math::is_addable_with_itself%3Cstd::pair%3Cint,+int%3E%3E::value+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
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

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCAArKQADqgKhE4MHt6%2BekkpjgJBIeEsUTHxdpgOaUIETMQEGT5%2BXLaY9nkM1bUEBWGR0XG2NXUNWc0KQ93BvcX9sQCUtqhexMjsHOYAzMHI3lgA1CYbbggEBAkKIAD0l8RMAO4AdMCECF4RXkorsowED2gslwAIixCMQ8BZUMB0IZUAA3S4JYioIgEACeCWCwAA%2Bl5HLQFJcWExxtFLgolitMAikUQcXiFA8EAkEodsCYNABBdkc4IEPZE4IQXl7WrAZCkPbIBC1PYAKllothcwOAHYrJy9pq9h9MXtmGwFAkmKttQRaId1VyNVrxugQCglgRuVqXQcjoc3AczGYObRaPzMAQEBgFHsorRUHcRcRMJKBCTVIiRQx0Hs0QlY0xgAxko5kCATLE3AxzGZna7NR6PV6zEwIgoIABaSTKw6AmsVztd7tutzVolB%2B11hvN5U98fdqtHGuF4ul8sVqee0spbNNltu9ulic711L/lMQcgVcMddj3e7/fzosl70Lvfu6crgCOxDPm47F4v%2B4HSGPr/fL9L0fZc7xvedrUXECa2QN5MHfNtPyA8cf0PP9YKiQDkJ7K8wLnO9IIfPsn29LBsVpW50AgLgAA4NAeVsNi3b1sK1VCjzIrEKKYKjaPo89sNwstwIIjkuyEyiuNQLEyIgZQAElGOYsxWJddi/0k2lZN/e0FIE4DiNA4T8LLQi1Og68TPvczDJrLxeRkvAqCoaJGFWCBYi8CUuBVLwlKQlDoJ0kB7MERznNchh3M87zfP0ycLLw29TLEzshNnZLrLYxKzAIaJmGIVEsQiW4ooQegFAbAhiC8TAJQ2B4uDMCVm0alV/O3dT7Ty4gCqKkrDClCqqpquq9gapqWskNrGNsyzMrM7K5u9Hq%2BuKwhygcCBqtq7z4j2Rt5g/bcoNs4LVtqfqNtaCoCG20a9pao7TureaINSl6SNy/LLvWghNruqgxCUPaOpYhKzrQ7qfsKv6AeoYGxq4Z7xJyjL5w2S17xjAhlgYPYNAtbkTBVQFuQ4BZaE4WJeD8DgtFIVBOD7SxrD2cllhNTYeFIAhNAphYAGsQBVLgHlFlUaI2KQaLMAA2ABOLgNHiKmOEkWn%2BcZzheAuDRef5hY4FgGBEAdFgMXoMgKAgf5Lf6HZDGALg5Y0fWaFoHqLggCItYiYJLs4Hn/b6gB5CJtFuoPeH%2BNhBFDhhaFRLWsHeYA3DEfFo9ILAiSMcR6d4fAY0qWFMAuQvSEwVQKlxNYGd5VotdoPABsKjwsC16q8BYaOFioAxgAUAA1PBMDuUOMzpnn%2BEEEQxHYKQZEERQVHUSvdGaAwjBQaxrH0VuLkgBZUASdoK8bW021MVnLB8g7Q7MXg4WiMEsCPiAFgBtIXBTEYmirimHoRQSjZGSKkAQ/8wG5DSMAvoMQxg3UqAITowxPCND0N/FBEw4EzAQYMLoUCxg4KmCA/oXAv4UlWBISm1NNaVyZhwPYqgaJy0bHLSQkpt7AD2C7B49ENB7AgLgQgJAvTSzmLwPmhc5gLAQJgHi/RP76E4BrUgvdYj6zpgzRhusQD62kVoI2psIBIEdAkXE5BKB2zoNEUIrA1gsLYRwrhTteFy34fw3gmB8BEDfnoWewhRDiCXoE1eagtab1IHcW4CQ%2B4qI4DTUg2jn6cFDriCxfJUBUGYaw9hnDHZGHcZ4wREAPAW1scQcRFCpGGyFiASQHjFaSDMLRV2sQFaixdgktRGitFa10bYfRBsZH1LMDRB44zWlyxorEHyGwNA0QVo0hJGx6E6J1iMoxtCOBP2SQMzZhiBakDLsQFIzhJBAA%3D%3D) ]
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