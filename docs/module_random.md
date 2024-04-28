

# utl::random

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**random** adds most of the sensible random functions one would need. Uses [std::rand](https://en.cppreference.com/w/cpp/numeric/random/rand) internally.

For scientific applications with higher random quality requirements use [&lt;random&gt;](https://en.cppreference.com/w/cpp/header/random) Mersenne Twister generator.

## Definitions

```cpp
void seed(unsigned int random_seed);
void seed_with_time(); // seed with time(NULL)

int rand_int(int min, int max);
int rand_uint(unsigned int min, unsigned int max);

float rand_float(); // [0,1] range
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
> random::seed(unsigned int random_seed);
> random::seed_with_time();
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

## Example (getting random values)

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