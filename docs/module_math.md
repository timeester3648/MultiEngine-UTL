# utl::math

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/math.hpp)

**math** adds various helper functions that aim to reduce code verbosity when working with mathematical expressions.

## Definitions

```cpp
// Constants
double PI           = 3.14159265358979323846;
double PI_TWO       = 2. * PI;
double PI_HALF      = 0.5 * PI;
double E            = 2.71828182845904523536;
double GOLDEN_RATIO = 1.6180339887498948482;

// Type traits
template<typename Type>
struct is_addable_with_itself;

template<typename Type>
struct is_multipliable_by_scalar;

template <typename Type>
struct is_sized;

template <typename FuncType, typename Signature>
struct is_function_with_signature;

// Basic math functions
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

template<typename IntegerType>
int kronecker_delta(IntegerType i, IntegerType j); // (i == j) ? 1 : 0

template<typename IntegerType>
int power_of_minus_one(IntegerType power); // (-1)^power

// Degrees and radians
template<typename FloatType>
FloatType deg_to_rad(FloatType degrees);

template<typename Type>
FloatType rad_to_deg(FloatType radians);

// Meshing
struct Points {
    explicit Points(std::size_t count);
};

struct Intervals {
    explicit Intervals(std::size_t count);
    Intervals(Points points);
};

template<typename FloatType>
std::vector<FloatType> linspace(FloatType L1, FloatType L2, Intervals N);

template<typename FloatType, typename FuncType>
FloatType integrate_trapezoidal(FuncType f, FloatType L1, FloatType L2, Intervals N);

// Other utils
template<typename UintType>
UintType uint_difference(UintType a, UintType b);

template<typename SizedContainer>
int ssize(const SizedContainer& container);
        
// Branchless ternary
template<typename Type>
ArithmeticType ternary_branchless(bool condition, ArithmeticType return_if_true, ArithmeticType return_if_false);

template<typename IntType>
IntType ternary_bitselect(bool condition, IntType return_if_true, IntType return_if_false);

template<typename IntType>
IntType ternary_bitselect(bool condition, IntType return_if_true); // return_if_false == 0

// Memory units
enum class MemoryUnit { BYTE, KiB, MiB, GiB, TiB, KB, MB, GB, TB };

template<typename T, MemoryUnit units = MemoryUnit::MiB>
constexpr double memory_size(std::size_t count);
```

All methods have appropriate `enable_if<>` conditions and `constexpr` qualifiers, which are omitted in documentation for reduced verbosity.

Methods that deal with floating-point values require explicitly floating-point inputs for mathematical strictness.

## Methods

### Type traits

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
> math::is_sized
> ```

`is_sized<Type>::value` returns at compile time whether `Type` objects support `.size()` method.

> ```cpp
> math::is_function_with_signature<FuncType, Signature>
> ```

`is_function_with_signature<FuncType, Signature>::value` returns at compile time whether `FuncType` is a callable with signature `Signature`.

Useful when creating functions that accept callable type as a template argument, rather than [std::function](https://en.cppreference.com/w/cpp/utility/functional/function). This is usually done to avoid overhead introduced by `std::function` type erasure, however doing so removes explicit requirements imposed on a callable signature. Using this type trait in combination with [std::enable_if](https://en.cppreference.com/w/cpp/types/enable_if) allows template approach to keep explicit signature restrictions and overload method for multiple callable types.

### Basic math functions

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
> int math::kronecker_delta(IntegerType i, IntegerType j);
> ```

Computes [Kronecker delta](https://en.wikipedia.org/wiki/Kronecker_delta) symbol: $\delta_{ii} = \begin{cases}1, \quad i = j\\0, \quad i \neq j\end{cases}$.

> ```cpp
> int math::power_of_minus_one(IntegerType power);
> ```

Computes $(-1)^{power}$ efficiently.

> ```cpp
> FloatType math::deg_to_rad(FloatType degrees);
> FloatType math::rad_to_deg(FloatType radians);
> ```

Converts degrees to radians and back.

### Meshing

> ```cpp
> struct Points {
>     explicit Points(std::size_t count);
> };
> 
> struct Intervals {
>     explicit Intervals(std::size_t count);
>     Intervals(Points points);
> };
> ```

"Strong typedefs" for grid size in 1D meshing operations, which allows caller to express their intent more clearly.

`Points` implicitly converts to `Intervals` ($N + 1$ points corresponds to $N$ intervals), allowing most meshing functions to accept both types as an input.

> ```cpp
> std::vector<FloatType> math::linspace(FloatType L1, FloatType L2, Intervals N);
> ```

Meshes $[L_1, L_2]$ range into a regular 1D grid with $N$ intervals (which corresponds to $N + 1$ grid points). Similar to `linspace()` from [Matlab](https://www.mathworks.com/products/matlab.html) and [numpy](https://numpy.org).

> ```cpp
> FloatType math::integrate_trapezoidal(FuncType f, FloatType L1, FloatType L2, Intervals N);
> ```

Numericaly computes integral $I_h = \int\limits_{L_1}^{L_2} f(x) \mathrm{d} x$ over $N$ integration intervals using [trapezoidal rule](https://en.wikipedia.org/wiki/Trapezoidal_rule).

### Other utils

> ```cpp
> UintType math::uint_difference(UintType a, UintType b);
> ```

Returns $|a - b|$ for unsigned types accounting for possible integer overflow. Useful when working with small types in image processing.

> ```cpp
> int math::ssize(const SizedContainer& container);
> ```

Returns `.size()` value of given container casted to `int`.

Useful to reduce verbosity of `static_cast<int>(container.size())` when working with `int` indexation that gets compared against container size.

> ```cpp
> ArithmeticType math::ternary_branchless(bool condition, ArithmeticType return_if_true, ArithmeticType return_if_false);
> ```

Returns `condition ? return_if_true : return_if_false` rewritten in a branchless way. Useful when working with GPU's. Requires type to be [arithmetic](https://en.cppreference.com/w/cpp/types/is_arithmetic).

> ```cpp
> IntType math::ternary_bitselect(bool condition, IntType return_if_true, IntType return_if_false);
> IntType math::ternary_bitselect(bool condition, IntType return_if_true);
> ```

Faster implementation of `ternary_branchless()` for integers. When second return is omitted, assumption `return_if_false == 0`  allows additional optimizations to be performed.

> ```cpp
> template<typename T, MemoryUnit units = MemoryUnit::MiB>
> constexpr double memory_size(std::size_t count);
> ```

Returns size in `units` occupied in memory by `count` elements of type `T`. Useful to estimate memory usage of arrays, matrices and other data structures in a human-readable way.

## Examples

### Using math type traits

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:14,endLineNumber:6,positionColumn:14,positionLineNumber:6,selectionStartColumn:14,selectionStartLineNumber:6,startColumn:14,startLineNumber:6),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++std::cout%0A++++++++%3C%3C+std::boolalpha%0A++++++++%3C%3C+%22are+doubles+addable%3F++++-%3E+%22+%3C%3C+math::is_addable_with_itself%3Cdouble%3E::value++++++++++++++%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22are+std::pairs+addable%3F+-%3E+%22+%3C%3C+math::is_addable_with_itself%3Cstd::pair%3Cint,+int%3E%3E::value+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
using namespace utl;

std::cout
    << std::boolalpha
    << "are doubles addable?    -> " << math::is_addable_with_itself<double>::value              << "\n"
    << "are std::pairs addable? -> " << math::is_addable_with_itself<std::pair<int, int>>::value << "\n";
```

Output:
```
are doubles addable?    -> true
are std::pairs addable? -> false
```

### Using basic math functions

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGIAJzSrgAyeAyYAHI%2BAEaYxHoADqgKhE4MHt6%2BAdJJKY4CoeFRLLHxXLaY9vkMQgRMxAQZPn6BFVVptfUEhZExcXoKdQ1NWa2DXT3FpRIAlLaoXsTI7BzmAMxhyN5YANQma24IBAQJCiAA9OfETADuAHTAhAhe0V5KS7KMBHdoLOcAIixCMQ8BZUMB0IZUAA3c5yAAqwXOLCYgzi5xSRnoAH1NttMHDEXcEAkEvtsCYNABBSlUsIEHYosIQek7erAZCkHbIBD1HYAKn57OhMz2AHYrNSdtKdm8wsAdsw2AoEkxlrKCLR9pKaVKZYN0CAUAsCLSZea9gd9m49mYzFTaLRGZgCAgMAodrFaKgbmziJhuQI0aoEsQ2Qx0DsCABPBIBpjABjJRzIEAmACsbgY5jMZot0ut1ttZiY0QUEAAtJJRft/sX8w3G03LW4iyjXUbS%2BWq6Lm32m4WDsWM1mc3n84ObTmUonK9XLXWc/3lxbJ4ymB2QDOGHPeyuV2ux5ns3bx6urUPpwBHYi7hf1/f7tftpBbm93x8Hi9T0/Hsd6idv2LZAXkwO9awfT8%2B2fDdXxA2IPyg5tD1/UdTwA89W0vO0sGAbEiGxa50AgLgAA4NDuGs1kXO0kJlGDN1w/DUEIphiLIii9yQlDcz/dCqUbHiiOY7FcIgZQAEkqJosw6PNBjX2EgixJfI1JK4r8sJ/Xi0NzDD5KAo9dLPAytOLLx6VEvAqCoOJGGWCB0y8LkuDFLxpMg6CgNUkALMEKybLshgHKcly3I0gdDNQk89IEhseJHGKTPoqKzAIOJmGIaNsWia5goQegFHLAhiC8TAuTWO4uDMLkqyqsUPKXBSjXS4hMuy3LDB5QritK8qdkq6raskeqqLMoykv0lLxrtVr2pywg7EwBwIBKsqXPTWr00a2jALMny5vqDrFsqZaCFWvqNq2iKWyLCb/ziva7tmjKjoWgglpWqgxCUDadtkyL9tglrXqy97PvO77aF%2BnYuG25DUsSsc1h1M9/QIRYGB2DRtVpEwxX%2BWkODmWhOHTXg/A4LRSFQThW0sawdgUBYlgDdYeFIAhNGJuYAGsQDFLg7kFsVSLWKRSLMAA2fwuA0TbSY4SQKe5mnOF4M4NE57m5jgWAYEQY0WASOg4nIShfhN%2Bh4i2QxgC4KWNC1mhaFas4IGiVXojCI7OA5732oAeWibQzr93hfjYQRA4YWho1VrBXmANwxGh8PSCwFEsRWan8H9Bw8GhTAzip3hMFUZavHS1X6UqVXaDwTqso8LBVZKvAWHToviGiZJMH%2BTAs%2BABujB1vgDGABQADU8EwG5A7jSmOf4QQRDEdgpBkQRFBUdRS9IXRygMUfTAZyx9Ebs5IDmVAEmqEuKwNWtT6sSxXJ2CtA7MXgYTiEEsCvhAOYEM0guAjCMPw5QQhhF6CUfo5RcipAEBAxIyQkEMEmH0MobQzodCGI0TwzQBinQLgIToDRMFwOweMYYhCsjlBod0GBUx4HAJZssWY%2BgyYq33rTDgOxVCkSlhWKWkhuTHwVA7O4FENA7AgLgQgJBbTixmLwLmpcZhzAQJgNi/QgFcKVrwTu6YtaU2pnwjWIAtbqK0LrA2EAkAmgSFXc2EBLam2IBEVgKxBHCNEeIu2sMpbSOkWXfARB/56BXsIUQ4hN7RJ3moVWh9SA3GuAkcOJNuGkDMT/Tggcq7OIZKgKgAihEiLEbbIwQSQmyIgB4Y2HjlFcFUdrDRfMQCSGCTLSQZgyKO3TP4QWDsDHK1IMY0xqsLG2CsW02xHSzCkTuIsvpUtSLplcmsDQpFAhSwMWsHh5j1ZzJ5gY7%2BOSpnHJsac7uKRnCSCAA%3D) ]
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

### Meshing and integrating

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:9,positionColumn:1,positionLineNumber:9,selectionStartColumn:1,selectionStartLineNumber:9,startColumn:1,startLineNumber:9),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/UTL/master/single_include/UTL.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++//+Mesh+interval+%5B0,+PI%5D+into+100+equal+intervals+%3D%3E+101+linearly+spaced+points+%0A++++auto+grid_1+%3D+math::linspace(0.,+math::PI,+math::Intervals(100))%3B%0A++++auto+grid_2+%3D+math::linspace(0.,+math::PI,+math::Points(+++101))%3B+//+same+as+above%0A%0A++++//+Get+array+memory+size%0A++++std::cout+%3C%3C+%22!'grid_1!'+occupies+%22+%3C%3C+math::memory_size%3Cdouble,+math::MemoryUnit::KB%3E(grid_1.size())+%3C%3C+%22+KB+in+memory%5Cn%5Cn%22%3B%0A%0A++++//+Integrate+a+function+over+an+interval%0A++++auto+f+%3D+%5B%5D(double+x)%7B+return+4.+/+(1.+%2B+std::tan(x))%3B+%7D%3B%0A++++double+integral+%3D+math::integrate_trapezoidal(f,+0.,+math::PI_HALF,+math::Intervals(200))%3B%0A++++std::cout+%3C%3C+%22Integral+evaluates+to:+%22+%3C%3C+integral+%3C%3C+%22+(should+be+~PI)%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'1',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
// Mesh interval [0, PI] into 100 equal intervals => 101 linearly spaced points 
auto grid_1 = math::linspace(0., math::PI, math::Intervals(100));
auto grid_2 = math::linspace(0., math::PI, math::Points(   101)); // same as above

// Get array memory size
std::cout << "'grid_1' occupies " << math::memory_size<double, math::MemoryUnit::KB>(grid_1.size()) << " KB in memory\n\n";

// Integrate a function over an interval
auto f = [](double x){ return 4. / (1. + std::tan(x)); };
double integral = math::integrate_trapezoidal(f, 0., math::PI_HALF, math::Intervals(200));
std::cout << "Integral evaluates to: " << integral << " (should be ~PI)\n";
```

Output:
```
'grid_1' occupies 0.808 KB in memory

Integral evaluates to: 3.14159 (should be ~PI)
```