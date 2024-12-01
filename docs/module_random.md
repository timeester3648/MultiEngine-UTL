# utl::random

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**random** adds most of the sensible random functions one would need.

Implements **XorShift64&ast;** random generator compatible with [&lt;random&gt;](https://en.cppreference.com/w/cpp/header/random), which is used internally to generate high quality random with performance slightly better than [std::rand()](https://en.cppreference.com/w/cpp/numeric/random/rand) and considerably better than classic [std::mt19937](https://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine).

## Definitions

```cpp
// XorShift64* generator
class XorShift64StarGenerator {
public:
    using result_type = uint64_t;
    
    static constexpr result_type min();
    static constexpr result_type max();
    
    void seed(uint64_t seed);
    uint64_t operator()();
}

XorShift64StarGenerator xorshift64star;

void seed(uint64_t random_seed);
void seed_with_time();          // seed with time(NULL)
void seed_with_random_device(); // seed with std::random_device

// Convenient random functions
int rand_int(int min, int max);
int rand_uint(unsigned int min, unsigned int max);

float rand_float();   // [0,1] range
float rand_float(float min, float max);

double rand_double(); // [0,1] range
double rand_double(double min, double max);

bool rand_bool();

template<class T>
const T& rand_choise(std::initializer_list<T> objects);

template<class T>
T rand_linear_combination(const T& A, const T& B);
```

## Methods

### XorShift64&ast; generator

> ```cpp
> random::XorShift64StarGenerator
> ```

Standard-compliant uniform random bit generator (see [random generator requirements](https://en.cppreference.com/w/cpp/named_req/UniformRandomBitGenerator)). Implements **XorShift64&ast;** suggested by Marsaglia G. in 2003 "Journal of Statistical Software" (see [original publication](https://www.jstatsoft.org/article/view/v008i14) and [other XorShift variantions](https://en.wikipedia.org/wiki/Xorshift)).

The engine has a small state (**8 bytes**, same as [std::minstd_rand](https://en.cppreference.com/w/cpp/numeric/random/linear_congruential_engine), in comparison [std::mt19937](https://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine) has a state of **5000 bytes**) and tends perform similar to [std::rand](https://en.cppreference.com/w/cpp/numeric/random/rand) while providing good statistical quality (see [PCG engine summary](https://www.pcg-random.org/)). Used internally by all random functions in this module.

> ```cpp
> random::xorshift64star
> ```

Global instance of `XorShift64StarGenerator`.

> ```cpp
> random::seed(uint64_t random_seed);
> random::seed_with_time();
> random::seed_with_random_device();
> ```

Seeds random with a value.

### Convenient random functions

> ```cpp
> int random::rand_int(int min, int max);
> int random::rand_uint(unsigned int min, unsigned int max);
> ```

Returns random integer in a given range.

> ```cpp
> float random::rand_float(float min, float max);
> double random::rand_double(double min, double max);
> ```

Returns random float/double in a given range.

> ```cpp
> float random::rand_float();
> double random::rand_double();
> ```

Returns random float/double in a [0, 1] range.

> ```cpp
> bool random::rand_bool();
> ```

Returns true/false randomly. Effectively same as `rand_int(0, 1)`.

> ```cpp
> const T& rand_choise(std::initializer_list<T> objects);
> ```

Returns randomly chosen object from a list.

> ```cpp
> T rand_linear_combination(const T& A, const T& B);
> ```

Returns $\alpha A + (1 - \alpha) B$, with random $0 < \alpha < 1$. Useful for vector and color operations. Object must have  a defined `operator+()` and scalar `operator*()`.

## Examples

### Getting random values

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGIAMzSrgAyeAyYAHI%2BAEaYxAGkAA6oCoRODB7evgHSyamOAqHhUSyx8f62mPYFDEIETMQEmT5%2BgZXV6XUNBEWRMXEJCvWNzdltQ929JWUBAJS2qF7EyOwc5v5hyN5YANQm/m4IBASJCiAA9OfETADuAHTAhAhe0V5Ky7KMBHdoLOcAIixCMQ8BZUMB0IZUAA3c6JYioIgEACeiTCwAA%2Bl5HLQFOcWEwhnFzpttpg4QiiFicQo7ghEol9tgTBoAIIs1lhAg7AlhCBcnYNYDIUg7ZAIBo7ABUUqF0NmewA7FY2Ts1Ts3uidsw2ApEkwVhqCLR9ir2ar1dcGOhUCwQCAlJh0Bibk8MVabSwMVhoXgVhBZqaOeqdkN0Pa0NjgyH1fs3HG9mYzB6MVyIBpRVwNBoFft/omzDGizG4wmPbb7Sm0xmdlmc8WG3sDgnzGYTABWNwMVvRoulg4FlM2l70ANN/OtxtT/tuHblu0gIeLaKjhVThszgsdrs9i195sD1tLkeYCAAWnbovbuf8E6T69jB9n88rhmdw5Xp4vV7XD83Pc7bsk17Esn0HN8MWiVBPDHPMCwfRtNxfRcIKgmDfwQ0D40PYDAN3VkNzAo8IPFVA8CUCATGVLhRTMUV/Co/4bzvQtELA5CU1I8jTyoiwaJ2OidgYxUmOnIjcJ3YC9ywlskxTWgwkwBoMV%2BaIwiYGoIDMO56LuZj4KQt8KxQ60MQU8JlNU9TNO03Sb2w2cAMktt/DNEDiEwAglgYHYNCDNlGI5Dh5loTh214PwOC0UhUE4eNLGsUNFmWTBE38HhSAITRgvmABrEBFS4O5CsVAAOdLJFKswADYAE4s0vUKOEkCLspizheDODMsqi4LSDgWAYEQFBbTRegyAoCBfjGgYtkMYAuGq7M%2BDoAg4jOCBojatTmGIZFOAynaGmRAB5aJtEwBwDt4X42EEE6GFofbetILBXmANwxFxa7XswAkjHEF78A8hw8GhTAzhezBVEu7FVmirkqjahTomuPaPCwNqCBBO1uF4cHiCgpR/j%2B%2BbzNAXr5ioAxgAUAA1PBMBuE7EkYH7%2BEEEQxHYKQZEERQVHUF7dBogwjBQaxrH0PBojOSB5lQRIakhs8wzzUwEssLhFR2M8TrMXgYTiEEsDlgN2kumoXGtUY/BokJFOmAYaLyNIBFtvRXZqKZ%2BniGi7EtzphiaTwWj0APQYELpGh90pndsYOPf94PY5mLh5gUZKVgkEKwtal7Yo4HZVFK6qz2qyQxTF4Ba2qu4NHrnYIFwQgSDS9PeB6rRZnmBAlKweJzaalrSDtdsM0i6LC86kBuuy%2BYBuGxYTmxchKGmug4giVhVhLsuK6r%2Bba/r%2BveCdVuTb0DnhFEcReevgW1DakXSBua5Emu3OOHC0hJ8NzgTrYkSNiHYqAqDF1LuXSuc0jDHwbhoJuHgWAzWIO3WYnd555RAJIOudVJBmC4KXDQ7ZaqFUWvoTgI8x4TzatPWws9MqYNIPlMwpU7isIIdVUq7Ztb%2BA0KVWqOCKEcH8PnKeHVGGUy/gbX%2BtCJFdxyqQAmqRnCSCAA%3D%3D%3D) ]
```cpp
using namespace utl;

random::seed_with_random_device();
std::cout
    << "rand_int(0, 100) = "                << random::rand_int(0, 100)                << "\n"
    << "rand_double() = "                   << random::rand_double()                   << "\n"
    << "rand_double(-5, 5) = "              << random::rand_double(-5, 5)              << "\n"
    << "rand_bool() = "                     << random::rand_bool()                     << "\n"
    << "rand_choise({1, 2, 3}) = "          << random::rand_choise({1, 2, 3})          << "\n"
    << "rand_linear_combination(2., 3.) = " << random::rand_linear_combination(2., 3.) << "\n";
```

Output:
```
rand_int(0, 100) = 82
rand_double() = 0.419907
rand_double(-5, 5) = -4.32745
rand_bool() = 0
rand_choise({1, 2, 3}) = 1
rand_linear_combination(2., 3.) = 2.12557
```

### Using XorShift64&ast; with &lt;random&gt;

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:68,endLineNumber:10,positionColumn:68,positionLineNumber:10,selectionStartColumn:68,selectionStartLineNumber:10,startColumn:68,startLineNumber:10),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++std::random_device+rd%7B%7D%3B%0A%09random::XorShift64StarGenerator+gen%7Brd()%7D%3B%0A%09std::normal_distribution%3Cdouble%3E+distr%3B%0A%09%0A%09std::cout+%3C%3C+%22Random+value+from+N(0,+1)+%3D+%22+%3C%3C+distr(gen)+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
std::random_device rd{};
random::XorShift64StarGenerator gen{rd()};
std::normal_distribution<double> distr;

std::cout << "Random value from N(0, 1) = " << distr(gen) << "\n";
```

Output:
```
Random value from N(0, 1) = 0.641989
```

### Benchmarking XorShift64&ast; with `UTL_PROFILER`

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGIM6SuADJ4DJgAcj4ARpjEIADMAKykAA6oCoRODB7evv6p6ZkCIWGRLDFxSbaY9o4CQgRMxAQ5Pn4BdpgOWQ1NBCUR0bEJyQqNza15HeP9oYPlw0kAlLaoXsTI7Bzm8aHI3lgA1CbxbggEBCkKIAD0N8RMAO4AdMCECF5RXkobsowEzzQLBuABEWIRiHgLKhgOhDKgAG43FLEVBEAgATxSoWAAH0vI5aAobiwmGNYjc9gdMMjUUR8YSFM8ECkUidsCYNABBTlc0IEQ6k0IQfmHJrAZCkQ7IBBNQ4AKnl4oRS2OAHYrNzDtrDt8cYdmGwFCkmJtdQRaCdNTytTq7jdDhZGDLScQANaHfBUKixf6HB4MdCoFiHYCMWJMIjEBS8zkATjQDHJqhRh1F4WO8RBhy4Gg0YA4eY0VtjGgTAmTqbG6BAIC8/IAbJJcQKlJh0Jns1xEiXufHS3Gg596IcFD5O4cNM9eza4wO7odlKiaCOw2EHo4jIcM0OovQFIdHu9dQw8PxiCGTIkLBopVwr9n8GNIV86gwB3IACpBXHKABKADyABiACSQTYH%2BEDmGY1a1gG6AQEs0GqiYGq8jqGGjvBEBtugSHxNa/ZluehwioIaYTsWBEUScbjblaxyWNYeCqmOl5MVmo4EDWIDYaqDo4Y0jjILiohjLRu70OyEB/ly4QgriACyXIABooZYObPPh1oYahIIDh%2B36/oBoHgZB0GwSA4JJtxuLwch6o6Zh2qWdZ1Z2YYHZrqhFi4XpM7OS53G1l4p7niwdmYGIuJPgQL4Elknp4OJGpTne05qvpBHoYFJFkQKeCUQxhW0fR1HWMxrHjhVnGxdAjDaTl2r%2BbazWtYcX4/v%2BwFgRBUFmDBwVWQQXBxnG8Rqg5PlNZhrkjWNE2howPl%2BZlAWBZZoVniQEXEFFtAxclcV4K%2BiWxT5aWaf52XtRheWiiVnFURYNGnGVL0VZYLGjtVHGPkd9UMI17UtVyuntZ1xk9WZ/VmPBwa1ipJBCAgZ4EE2vTEAA4uGG4kFNaG3TqBK0HBnkIyASPECjaMY%2BMOPrpGJBLe%2BGqrVlTnOZtYU7ZF0WxfFb5JSlN7POl12c5h93kY92bPa9dHhAxn0WN9bGMVYtUA2uwNgzqoNEbOkPdaZfXQfDLBk4GMVrHumCIQTksYRbtaqCQCio1Q6OSGMTTPLhOGYO2uuBYc0sFUV1GPYrytMV9VXsZr2Yu7xnk28O9sh/ra19mW84OkumAIn6QLYvQxBh6iIaoCkjjggAXvqTxMBijFmKF3zttBnqRkwA6WWgBKZm4pXq7RpXQVebjvgN63antBDrAwk4zqDHArKTHCJLwfiFrwqCcCPccvQoawbJgjHxDwpAEJo68rG6IBqlwzzP2qAAcV%2BSO/ZgNnGubJE3pIHed9SAHw4Lwa4t5b57xWHAWAMBEAoGDGXWI5BKClzoMMfYhhgBcAbHmPgdACCxGuBAKIoCoihCaBiTg18qHMGIBiACURtBdBgdfIEbBBAAQYLQWhe9SBYC%2BMANwYgiR0N4FgUkRhxCCPwHtboxdriCMwKoLoBIthaHIIIGooDaAnQeEwjwWBQHHUttwXgxdiBRHSJgEEmAZHAAMUYO%2BKwqAGGAAoAAangTAjwAIpEYJImQggRBiHYFIUJ8glBqFAboLg%2BhcEoBVvoE61xIArBrm%2BFRABaasJwQSmGPlwNUhxckATMPvaxkIsAZMQtUWoWQXCBkmH4RJwQ5hlAqHoNIGQ3xtN6YUN8AxunDESZ0bo9QZiDImTUdhPQZijKGHECZMzPBtD0L7ZoyyFirJWKfdYmwJAb04NvUgu9tHgMOKod%2BDZclNmlAYLc%2BDnhTg0KRXAhBmY7C4EsXgMCtBLBWAgKKWA4gNKAbwS2iRbyXP3pwSBIBoFuNIPApBaxLgEnQRATB5dwisC2Lc%2B5jycEvIbG8t5vB2zfNqXofgYTRDiCiQymJKh1CCISaQR4DwUiSNOVvEBgjwEAQJCkIeqAqA3LuQ8yQTzcE5gpe80iHgWCoIrr8/5N83EPxAJIClf9JBmC4HcjQiQ4zP3wfoTgwDSAwrhaA8BSKUWwN1WYd%2Bzx3XGobO/RIpT4gaHfnGfV1qODxCFVcxF2rXWhqqRcx1UbAX31INYjIzhJBAA%3D%3D%3D) ] (N limited by Godbolt time restrictions)
```cpp
// Benchmark different random generators
constexpr int N = 900'000'000;
constexpr std::uint64_t seed = 15;

double sum = 0.;

// Profile generating N doubles with uniform [0, 1] distribution
UTL_PROFILER("std::rand()") {
    srand(seed);
    for (int i = 0; i < N; ++i) sum += std::rand() / (static_cast<double>(RAND_MAX) + 1.);
}

UTL_PROFILER("std::minstd_rand") {
    std::minstd_rand gen{seed};
    std::uniform_real_distribution dist{0., 1.};
    for (int i = 0; i < N; ++i) sum += dist(gen);
}

UTL_PROFILER("std::mt19937") {
    std::mt19937 gen{seed};
    std::uniform_real_distribution dist{0., 1.};
    for (int i = 0; i < N; ++i) sum += dist(gen);
}

UTL_PROFILER("random::XorShift64StarGenerator") {
    utl::random::XorShift64StarGenerator gen{seed};
    std::uniform_real_distribution dist{0., 1.};
    for (int i = 0; i < N; ++i) sum += dist(gen);
}

UTL_PROFILER("random::rand_double()") {
    random::xorshift64star.seed(seed);
    for (int i = 0; i < N; ++i) sum += random::rand_double();
}

// Prevent compiler from optimizing away "unused" sum
std::cout << sum << "\n";
```

Output (compiled with `-O2` using **g++ v.11.4.0**):
```
------------------------------ UTL PROFILING RESULTS ------------------------------

 Total runtime -> 32.02 sec

 |                Call Site |                           Label |    Time | Time % |
 |--------------------------|---------------------------------|---------|--------|
 | examples.cpp:160, main() |           random::rand_double() |  4.70 s |  14.7% |
 | examples.cpp:154, main() | random::XorShift64StarGenerator |  4.74 s |  14.8% |
 | examples.cpp:148, main() |                    std::mt19937 | 10.58 s |  33.1% |
 | examples.cpp:142, main() |                std::minstd_rand |  5.96 s |  18.6% |
 | examples.cpp:137, main() |                     std::rand() |  5.28 s |  16.5% |
```