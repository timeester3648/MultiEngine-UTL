
# utl::sleep

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**sleep** implements 3 variations of sleep() function with a goal of providing improved precision.

## Definitions
```cpp
spinlock(double ms); //   precise, uses CPU 100% of time, expected error ~0.1 ms
hybrid(double ms);   //   precise, uses CPU  ~5% of time, expected error ~0.1 ms
system(double ms);   // imprecise, uses CPU   0% of time, expected error ~1-5 ms
```

## Methods
> ```cpp
> sleep::system(double ms)
> ```

System sleep is an alias of [std::this_thread::sleep_for()](https://en.cppreference.com/w/cpp/thread/sleep_for), as such it's subjected to OS scheduler which wakes up approximately every ~3ms, with delays even more inconsistent for larger sleep times, which deems it unfit for applications like real-time rendering.

> ```cpp
> sleep::spinlock(double ms)
> ```

Spinlock sleep is a common way of implementing thread locks, it is based in a looped time check, which ends the loop once timer tuns out. While such approach allows for a much greater precision it has a downside of constantly using corresponding CPU thread.

> ```cpp
> sleep::hybrid(double ms)
> ```

Hybrid version loops a short system sleep, estimating it's error mean and variance on the fly (Welford's algorithm), and once remaining time gets below the mean plus a standard deviation switches to spinlock. This results in a precision almost as good as pure spinlock, while mostly freeing CPU thread.

## Example (comparing sleep precision)

[ [Run this code](GODBOLT_LINK) ]
```cpp
using namespace utl;

constexpr int repeats = 6;
constexpr double sleep_duration_ms = 16.67;

std::cout << "Sleeping for " << sleep_duration_ms << " ms.\n";

std::cout << "\nsleep::spinlock():\n";
for (int i = 0; i < repeats; ++i) {
	auto start = std::chrono::steady_clock::now();

	sleep::spinlock(sleep_duration_ms);

	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 << " ms\n";
}

std::cout << "\nsleep::hybrid():\n";
for (int i = 0; i < repeats; ++i) {
	auto start = std::chrono::steady_clock::now();

	sleep::hybrid(sleep_duration_ms);

	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 << " ms\n";
}

std::cout << "\nsleep::system():\n";
for (int i = 0; i < repeats; ++i) {
	auto start = std::chrono::steady_clock::now();

	sleep::system(sleep_duration_ms);

	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6 << " ms\n";
}
```

Output:
```
Sleeping for 16.67 ms.

sleep::spinlock():
16.6713 ms
16.6709 ms
16.6708 ms
16.6711 ms
16.6709 ms
16.6715 ms

sleep::hybrid():
16.6728 ms
16.6713 ms
16.6713 ms
16.6711 ms
16.671 ms
16.6713 ms

sleep::system():
20.0231 ms
30.344 ms
30.678 ms
30.6268 ms
30.6562 ms
30.1724 ms
```