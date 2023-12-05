
# utl::timer

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**utl::timer** contains a number of methods for time measurement. Intended mainly for measuring code execution time without [std::chrono](https://en.cppreference.com/w/cpp/chrono) boilerplate. Outputs time as a string, prints formatted local time and date.

## Definitions
```cpp
void start() // start measurement

// Elapsed time as double
double elapsed_ms();
double elapsed_sec();
double elapsed_min();
double elapsed_hours();

// Elapsed time as string
std::string elapsed_string_ms();
std::string elapsed_string_sec();
std::string elapsed_string_min();
std::string elapsed_string_hours();

std::string elapsed_string_fullform(); // format "%H hours %M min %S sec %MS ms"

// Local date and time
std::string datetime_string();    // format "%y-%m-%d %H:%M:%S"
std::string datetime_string_id(); // format "%y-%m-%d-%H-%M-%S", works in filenames
```

## Methods
> ```cpp
> timer::start()
> ```

Sets internal start timepoint for elapsed measurements.

> ```cpp
> double timer::elapsed_ms()
> double timer::elapsed_sec()
> double timer::elapsed_min()
> double timer::elapsed_hours()
> ```

Returns elapsed time as `double`. Internally time is measured in nanoseconds.

> ```cpp
> std::string timer::elapsed_string_ms()
> std::string timer::elapsed_string_sec()
> std::string timer::elapsed_string_min()
> std::string timer::elapsed_string_hours()
> ```

Returns elapsed time as `std::string` with units.

> ```cpp
> std::string timer::elapsed_string_fullform()
> ```

Returns elapsed time in format `%H hours %M min %S sec %MS ms`.

> ```cpp
> std::string timer::datetime_string()
> std::string timer::datetime_string_id()
> ```

Returns current local date and time in format `%y-%m-%d %H:%M:%S` or `%y-%m-%d-%H-%M-%S`. Since first format is contains characters illegal in filenames, second format can be used instead.

## Example 1 (measuring time)

[ [Run this code](GODBOLT_LINK) ]
```cpp
using namespace utl;

timer::start();

std::this_thread::sleep_for(std::chrono::milliseconds(3700));

std::cout
	<< "Time elapsed during sleep_for(3700 ms):\n"
	<< timer::elapsed_string_sec() << "\n"
	<< timer::elapsed_string_fullform() << "\n";
```

Output:
```
Time elapsed during sleep_for(3700 ms):
3.711095 sec
0 hours 0 min 3 sec 712 ms
```

## Example 2 (local date and time)

[ [Run this code](GODBOLT_LINK) ]
```cpp
using namespace utl;

std::cout << "Current time is: " << timer::datetime_string() << "\n";
```

Output:
```
Current time is: 2023-12-05 02:11:34
```