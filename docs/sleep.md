# utl::sleep

**sleep** implements 3 variations of sleep() with a goal of providing improved precision.

System sleep is an alias of [std::this_thread::sleep_for()](CPPREF_LINK), as such it's subjected to OS scheduler which wakes up approximately every ~3ms, which deems it unfit for applications like real-time rendering.

Spinlock sleep is a common way of implementing thread locks, it is based in a looped time check, which ends the loop once timer tuns out. While such approach allows for a much greater precision it has a downside of constantly using corresponding CPU thread.

Hybrid version loops a short system sleep, estimating it's error mean and variance on the fly (Welford's algorithm), and once remaining time gets below the mean plus a standard deviation switches to spinlock. This results in a precision almost as good as pure spinlock, while mostly freeing CPU thread.

## Methods

'''cpp
spinlock(double ms); //   precise, uses CPU 100% of time, expected error ~0.1 ms
hybrid(double ms);   //   precise, uses CPU  ~5% of time, expected error ~0.1 ms
system(double ms);   // imprecise, uses CPU   0% of time, expected error ~1-5 ms
'''

## Example (comparing sleep precision)

[ [Run this code](GODBOLT_LINK) ]
'''cpp
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
'''

Output:
'''
'''