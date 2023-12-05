

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

[ [Run this code](GODBOLT_LINK) ]
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