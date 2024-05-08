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
void seed_with_time(); // seed with time(NULL)

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

## Example 1 (getting random values)

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGIAJz%2BpK4AMngMmAByPgBGmMQBpAAOqAqETgwe3r4BQSlpjgJhEdEscQlBdpgOGUIETMQEWT5%2BgbaY9oUMdQ0ExVGx8YkK9Y3NOW0jvf2l5QEAlLaoXsTI7BzmAMzhyN5YANQmm24IBARJCiAA9FfETADuAHTAhAheMV5Kq7KMBI9oLCuABEWIRiHgLKhgOhDKgAG5XJLEVBEAgATyS4WAAH0vI5aAoriwmCN4lcFMtVphEciiLj8QpHggkkkjtgTBoAIIcznhAj7YnhCB8/YNYDIUj7ZAIBr7ABUcrFcPmhwA7FYufstftPlj9sw2Aokkw1jqCLQjhruZrtXcGOhUCwQCAlJh0Nj7q9sY42BB5paedr9iN0M60HjA0HtUc3DHDmYzHb3XyIBpJVwNBoVUcgfGzFGC1GY3Gk47nUnsSm0/sM1nC/XDsc4%2BYzCYAKxuBgtyMF4vHPMVh3veh%2Bxu5lsNyd9tz7UtOkCD5YxEcqyf16d59ud7s23tN/stxfDzAQAC0bclbezm3HCbX0f3M7n5cM7qHy5P58vq/vG%2B7Ha7BMeyLR8B1fbEYlQTxRxzPN7wbDdnwXcDIOgn94JA2MDyAgCd05ddQMPcDpVQPAlAgEx1S4SUzElTZKKBa9b3zBDQKQisSLIk9KIsaj9lo/Z6NVRip0InDtyA3dMObBMK1ocJMAabEARicImC6CAzEeOjHiYuDENfMtkPtbF5IiJSVLUjStJ068sJnf8JNbTYrWA4hMAIFYGH2DQAy5BieQ4RZaE4NteD8DgtFIVBOFjSxrGDSlTS2HhSAITQgsWABrEBVS4R48tVAAOTYpCKswADZ/AzC8Qo4SRwoy6LOF4S403SyKgtIOBYBgRAUEdTF6DICgIABIahl2QxgC4CrMz4OgCHiS4IBiJrVOYYg0U4VKNoaNEAHkYm0aoOtSgE2EEA6GFobbOtILAPmANwxAJHbeCwYkjHEe78Hcmo4UwS57swVRqjxdYor5DomvkmI7i2jwsCaghwSdbguqoAxgAUAA1PBMHuA6kkYd6ZEEEQxHYKRyfkJQ1Ca3RqIMIwUGsax9DwGJLkgRZUCSLpgdPEMc1MeLLC4VV9lPA6zF4eF4nBLAeb9dpOgyFx7XGPxqNCBTZiGaj8nSARtb0Y2uhmQYEmoqoagEHoxk8Fo9DtrpHb6fXrdd0YmmdnJbd9q2ykNxYKRWNYJGC0LGvumKOH2VQioq08KskKUWeAGsKseDRc/2CBcEIEh41K%2BZeA6rR5kWBBFKwBJVbqhrSCdNs0wiqL49akB2oyxYev65ZzjxchKHGuh4kiVh1iTlO04z6bs9z3PeDdYulb0fgKdEcQaa3umVHUe6mdIe47iSd7o44MLSA7%2BXOAOvEkjxfZUCoRPk9T9OpqMJe840AuHgWATWIKXLg5c0p92yiASQOcqqSDMFwZOGg2z%2BDyrNfQnBm6t3bk1Lutge6QM6tXUgOUzBFUeOQxBFUiptklpsDQRV/CwMwRwTYsdO4tSIVXK%2Bctb54K4ZXTKpBAbEDSM4SQQA%3D%3D%3D) ]
```cpp
using namespace utl;

random::seed_with_time();
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
rand_int(0, 100) = 40
rand_double() = 0.398804
rand_double(-5, 5) = -3.53699
rand_bool() = 0
rand_choise({1, 2, 3}) = 1
rand_linear_combination(2., 3.) = 2.85376
```

## Example 2 (using XorShift64&ast; with &lt;random&gt;)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:68,endLineNumber:10,positionColumn:68,positionLineNumber:10,selectionStartColumn:68,selectionStartLineNumber:10,startColumn:68,startLineNumber:10),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++std::random_device+rd%7B%7D%3B%0A%09random::XorShift64StarGenerator+gen%7Brd()%7D%3B%0A%09std::normal_distribution%3Cdouble%3E+distr%3B%0A%09%0A%09std::cout+%3C%3C+%22Random+value+from+N(0,+1)+%3D+%22+%3C%3C+distr(gen)+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
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

## Example 3 (benchmarking XorShift64&ast; with UTL_PROFILER)

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGE6a4AMngMmAByPgBGmMQgkgDMpAAOqAqETgwe3r7%2ByanpAsGhESzRsQm2mPaOAkIETMQEWT5%2BUpXVGXUNBEXhUTFxiQr1jc05bcPdvSVlgwCUtqhexMjsHObxIcjeWADUJvFuCAQESQogAPQXxEwA7gB0wIQIXpFeSiuyjAT3aCwXABEWIRiHgLKhgOhDKgAG4XJLEVBEAgATySIWAAH0vI5aAoLiwmMMYhcFEsVph4YiiNjcQp7ggkkkDtgTBoAIJs9khAi7QkhCA83YNYDIUi7ZAIBq7ABUMpFMLm%2BwA7FYObsNbt3hjdsw2AokkxVlqCLQDmrOerNVcLrsLIxJYTiABrXb4KhUGLfXY3BjoVAsXbARgxJhEYgKLlsgCcaAYxNUCN2QrC%2B3iAN2XA0GjAHGzGnNUY0sYECaTw3QIBAXh5ADZJJjeUpMOg0xmuABWQscmNF6P%2B170XYKHxt3Yae7dy3RvtXXbKRE0IfB0I3RxGXapgeRegKXa3Z5ahh4fjEQMmDsWDTirgXjP4Yagt41Bh9uQAFUCmOUACUAPIAGIAJKBNgP6YoE7IWNgoEAhA5hmBWVa%2BugEBzAhSomKqXKarhw4oRAzboOh8QWr2xanrsgqCMmY4FqRtEHG4m7mvsljWHgSojue7HpsOBCViABFKrahH1I4yCYqIwxMdu9AshAP7smEAKYgAsuyAAamGWJm9wkRauFYQCfZvp%2B37/sBoHgZB0GwfBZiIQJVbAvGAmYihGEqoZeEakhICuRWHmGK2K5YRYRHGVOvl%2Bc51bHqeLAeZgYiYg%2BBBPjiGRungMmqhON6TsqJmkThMWUdRvJ4HRrHVUxLEMdYHFcaOTV8el0CMAZZUalFVq9f1uwfl%2Bv6ASBYEQVBMHYHBCH%2BSwBBcNG0bxMqXnhT1eHzYty2rUGjDhZFxXRTF/leAlJBJcQKW0GluUZXgz7Zel4UFXpUWlYNuEVUKdV8fRFiMYcDWA01liccOrW8fe92dQw3WDX17JGYNw0WWN1mTXZM0OWYKEBlWmkkEICAngQ9ZdMQADiIZriQ63YV9mo4rQyEhQTIBE8QJNkxTIw06uYYkPtr6qkdJU%2Bb5Z0XWeyWpelmUvjleVXvchUfZLeE/TRf0ZgDQPMWErFgxYEPcWxVjtbDK4I8jmpI%2BR05o6NVkTbZ02zY5%2BMsGzfppUsO6YGhDOa7h3tVqoJAKKTVDk5IkzEPcRGEZgLa2zFuza1VNUMX9hvG%2Bx4MtTxlsZuHQkhf7g5B%2Bn9vHT2xazraC6YDC3p/Oi9DEJniKBqgSSOMCABeOp3EwKJsWY53vC2CFumGTB9v5aA4mmbj1ebTH1QhF5uK%2BjknRq10EMsDDjlOSMcAsrMcB2vB%2BHmvCoJw6%2BF4DZLLMaGw8KQBCaFfCxnQgGVFwe4IDlQAA54hSAgWYWs0Yswdn0JwSQ99/6kGfhwXg5xrx/0fgsOAsAYCIBQAGTuMRyCUA7nQAY2xDDAC4LWbMfA6AEBiOcCAkR0GRBCA0FEnAf48OYMQFEf5IjaEwA4ARvA/hsEEH%2BBgtB%2BGP1IFgN4wA3BiDxNI1RmBCRGHECo/A10HB4DbucFRmBVCSJxGsLQ5BBBVHQbQR6NwREeCwOgh6PtuC8DbsQSIqRMAAj0fQlxRh/4LCoAYYACgABqeBMC3D/EkRgOj%2BCCBEGIdgbQMnyCUGodBuguD6HoSgE2%2BhHrnEgAsfuL4LEAFoKwHABKYN%2BXBlS7AaX%2BMwT9/GgiwNUtC7RJEvhcH6MYrRSBBBCH0UoAwSkpDSC%2BSZeglkFAYNMfosQSl2FGZ0EYTRPAtD0Hs0xtRDlbPmTs2whzVm7MubMmYCyFgfwpBIa%2BnA76kAfvYzBuxVAQNrA0%2BsEoDAbkYfcCcGgqK4EIMLb%2BcxeB4K0HMBYCAUpYFiMMm%2BqDSA%2Bw7NeX5T9ODYJALgyJpBCEkKWKcHElCIDUK7mEVgaxAXAtBXQiFtYoVQt4C2eFAy9B5KyeIXJshFAqHUCo4ppBbg3CSNIz5t80EqMwX%2BHESRV6oCoACoFILJBgvoZmHl0KqIeBYOQ7uiLkWRMAXEHl8DJBmC4ECjQHZowgMYcgjgeKCVEvQZgslFL8H2rMBA%2B44aXW1ggR2Dp8QNAQOjJIWsPr4iqr%2BaS3%2BdqfW9J%2BYGrNKKAGkH8WkZwkggA%3D%3D) ] (N limited by Godbolt time restrictions)
```cpp
// Benchmark different random generators
constexpr int N = 900'000'000;
constexpr std::uint64_t seed = 15;

double sum = 0.;

// Profile generating N doubles with uniform [0, 1] distribution
UTL_PROFILER_LABELED("std::rand()") {
	srand(seed);
	for (int i = 0; i < N; ++i) sum += std::rand() / (static_cast<double>(RAND_MAX) + 1.);
}

UTL_PROFILER_LABELED("std::minstd_rand") {
	std::minstd_rand gen{seed};
	std::uniform_real_distribution dist{0., 1.};
	for (int i = 0; i < N; ++i) sum += dist(gen);
}

UTL_PROFILER_LABELED("std::mt19937") {
	std::mt19937 gen{seed};
	std::uniform_real_distribution dist{0., 1.};
	for (int i = 0; i < N; ++i) sum += dist(gen);
}

UTL_PROFILER_LABELED("random::XorShift64StarGenerator") {
	utl::random::XorShift64StarGenerator gen{seed};
	std::uniform_real_distribution dist{0., 1.};
	for (int i = 0; i < N; ++i) sum += dist(gen);
}

UTL_PROFILER_LABELED("random::rand_double()") {
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