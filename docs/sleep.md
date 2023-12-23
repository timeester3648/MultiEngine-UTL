
# utl::sleep

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**sleep** implements 3 variations of sleep() function with a goal of providing improved precision.

## Definitions
```cpp
spinlock(double ms); //   precise, uses CPU 100% of time, expected error ~0.01 ms
hybrid(double ms);   //   precise, uses CPU  ~5% of time, expected error ~0.01 ms
system(double ms);   // imprecise, uses CPU   0% of time, expected error ~0.1-5 ms
```

## Methods
> ```cpp
> sleep::system(double ms);
> ```

System sleep is an alias of [std::this_thread::sleep_for()](https://en.cppreference.com/w/cpp/thread/sleep_for), as such it's subjected to OS scheduler which wakes up approximately every ~3ms (usually, true value depends on the system), with delays even more inconsistent for larger sleep times, which deems it unfit for applications like real-time rendering.

> ```cpp
> sleep::spinlock(double ms);
> ```

Spinlock sleep is a common way of implementing thread locks, it is based in a looped time check, which ends the loop once timer tuns out. While such approach allows for a much greater precision it has a downside of constantly using corresponding CPU thread.

> ```cpp
> sleep::hybrid(double ms);
> ```

Hybrid version loops a short system sleep, estimating it's error mean and variance on the fly (Welford's algorithm), and once remaining time gets below the mean plus a standard deviation switches to spinlock. This results in a precision almost as good as pure spinlock, while mostly freeing CPU thread.

## Example (comparing sleep precision)

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGISWakrgAyeAyYAHI%2BAEaYxCBmAKykAA6oCoRODB7evv6BaRmOAqHhUSyx8Um2mPbFDEIETMQEOT5%2BATV1WY3NBKWRMXEJyQpNLW15nWN9A%2BWVIwCUtqhexMjsHOYAzGHI3lgA1CbbbggEBCkKIAD0N8RMAO4AdMCECF7RXkrrsowEzzQLBuABEWIRiHgLKhgOhDKgAG43FLEVBEAgATxSYWAAH0vI5aAobiwmGM4jcFKt1phkaiiPjCQpnggUikTtgTBoAIJc7lhAiHUlhCCLY4AdisPMOMsO3xxh2YbAUKSYGzlBFoJylvOlsrQDHJqhRhwFh2ImBSmCYBAUx22IMOADZtXzZYcDUaTehVtF6IcFPRLbj0GsbVlcSw7SdHVwnc8neLXTy3bKxugQChVoKTm5c8czGYhEHsUZDvxiAWzPa86cAyWQ2H6pHo6d8%2BZq1HniZEm4GB3k7rue705m0ASa%2B3Cz2%2B4HMJbMyqwrRUMgANaikAz/vT7Y690Vw4QM14e2OjTa00182W622y/Wax4MUmSWp90ypgE1AB8Y5h2/hmKAIKiDCoIuBDWugGK4vsq5rpmYGPKKg7vh%2Bc4LiAS4MCu64QBhKSNg8zZRosqF6h%2BhxfkQhyuGegFjiBAjgVhkFMNBsG4QhIBIShe5oSOBBAeO/61m4DHAaBLGhsREaiGMuajpJzGIfCSgGugCgchAdEALS/n0iyAqsgiiocNyHFwmBOpOdYdkKWm9juZiDu6r4gnyaFKSJtniQOTkEZmCAYtEkLoJu24DvxFHliQR4nvRF57le%2BYWlaNpaclj6WM%2BEr7pRVHfgZLT0d5TFgRBUEwXB66IagyFkdFQ4FfW84pEFIVhfhDYyeGAgto1OoCbK1E/nRMYScg5UseS7HVVxdUNa5BXedmvmTdNma9c28kEIpQmMVJqlgepAiadpenFQQRnjqZYoWVZNm5lOnaOX2UX5TK7meTFq0Ts9dnTgFJaLhi5IsBFTkfWhh7HoIKUAUlFgI%2BJaV3plyPZRYuWvp9H6jVdpUHcpFWsVVnHwYtfFDTFI4g1hYOQRDBFEX1DADeRw4FQT40AWVR1k3NFO1Tx9XU8NMp/aJ%2Bb8ypIDbXJZJ7acsuk8wJ2YBpmXYDpDDoIc%2BkzC0N0mQQZkPdZ632VGkW7nj30pjFFoEGsDCHEjnnih5PIcMstCcIkvB%2BBwWikOBHB5pY1gBtS6o7DwpAEJovvLAh4pcM86figAHNsUjZ2YToAJxcBoyT%2BxwkhB8nYecLw1waInyfLHAsAwIgWYsNi9BkBQEBAt3wz7IYwBxhojc0LQkHENcEDRDX0RhM0GKcAni/MMQGIAPLRNomtJ9wvBAmwghbzhK8h7wWBfMAbhiESq9X5gpJGOIl%2BkPgFoOHgCKYNc7%2BYFUJrAkmxQ4ClqDXWgeBQrLw8FgGuBBIQsEfssKgBhgAKAAGp4EwI8LeVpg4J34IIEQYh2BSBkIIRQKh1Dv10FwfQI8UBY30NA64kBlioBSPUf%2Bul0wxlMFHSwXBxQGy3mYXgiI4hhT/vAZYdh95ZBcHrSYfgGEhDCIMCowwGGFEyAIVReg9H1DmEMeIDCFHfwEL0CYnh2h6EsfUGx/RNHzB0bYP8hiLF/lMdo8x8jY7kL9gHau79w6HFUNnJ0uknSSA9AYMscZngaGSUeXAhA4rx0WLwA%2BWhFjLAQFBYYop9CcCrqQZBiRG7B1DuHeuIBG65N9qQVuHdswpAJOQSgA86BxAiKwTYkTomxPiSPSy8YUmh0wPgIgYU9DEOEKIcQFCFnULUDXehpBHgPHaofYJHBA6kBqZIzgW8CQdMFKgKgESokxLicPRJEzUkQA8F3XplYsk5Obqnfw8Zi4BC4FEsuRd05xlKZXXglTqk1zqbYBpTdL75NIAhMw2dniorMHGbOiQRHbA0NnIukgnTgu2KE2pdcEV5P2RIo5MKKVNKRb/GeSjJBAA%3D%3D) ]
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