# utl::sleep

[<- to README.md](..)

[<- to implementation.hpp](../include/UTL/sleep.hpp)

**sleep** implements 3 variations of sleep() function with a goal of providing improved precision.

## Definitions

```cpp
void system(double ms);   // imprecise, uses CPU   0% of time, expected error ~0.1-5 ms
void spinlock(double ms); //   precise, uses CPU 100% of time, expected error ~0.01 ms
void hybrid(double ms);   //   precise, uses CPU  ~5% of time, expected error ~0.01 ms
```

## Methods

> ```cpp
> sleep::system(double ms);
> ```

System sleep is an alias of [std::this_thread::sleep_for()](https://en.cppreference.com/w/cpp/thread/sleep_for), as such it's subjected to OS scheduler which wakes up approximately every ~3ms (usually, **true value depends on the system and may vary significantly**), with delays even more inconsistent for larger sleep times, which deems it unfit for such applications as real-time rendering.

> ```cpp
> sleep::spinlock(double ms);
> ```

Spinlock sleep is a common way of implementing thread locks, it is based in a looped time check, which ends the loop once timer tuns out. While such approach allows for a much greater precision it has a downside of constantly using corresponding CPU thread.

> ```cpp
> sleep::hybrid(double ms);
> ```

Hybrid version loops a short system sleep, estimating it's error mean and variance on the fly ([Welford's algorithm](https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance)), and once remaining time gets below the mean plus a standard deviation switches to spinlock. This results in a precision almost as good as that of a pure spinlock, while keeping the CPU thread mostly free.

## Examples

### Comparing sleep precision

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCAArKQADqgKhE4MHt6%2BekkpjgJBIeEsUTHxdpgOaUIETMQEGT5%2BXLaY9nkM1bUEBWGR0XG2NXUNWc0KQ93BvcX9sQCUtqhexMjsHOYAzMHI3lgA1CYbbggEBAkKIAD0l8RMAO4AdMCECF4RXkorsowED2gslwAIixCMQ8BZUMB0IZUAA3S5yAAqAUuLCY42ilxSRnoAH1trtMAjkQ8EAkEodsCYNABBak04IEPZo4IQOYHADsVlpe15ew%2BwWAe2YbAUCSYq35BFoh25dJ5fLQDAxqgSxD2jL2xEwCUwTAICgOG0BewAbLL6Xy9kqVWq9uglhF6HsFPQdbj0Mt9WlcSxDYcTVxTQ9TRyLbTLXzxugQCglkzDm5EwczGYhG6EoK9vx1eYzEak0cXRmPV72r7/Udk3nmQoHiZYm4GHnw/KaVbo7G0F4E1Wiy3G8qM7GxcFaKhkABrNkgBtNlsbOVWnN7CCavBGk0aWUagtanV6g076zWPDskxcyNW3lMHuoF0TTcPmMoBDEASoEcEPXoACeuJ2CdJ1jBhUDuNlWyva9XUwHUR0zBhxynCAYPdT1bnLP05kghVrz2W8iD2Vwn07V931Ar8f3/QCpxAsCIMXKCOwIF9u17Qs3GfLs3w/WN0O9AQAPRAhE1I5AeIokBmFApQlXQBRKQgYiAFoHy6OY/iWQQ2T2S49i4TBTQLatU1rOdm1TVsrQvQF6SgsT42M/tLMHVCEljBBfwiMF0BncyFyXPkVzXQRdwDPZt0XMKi21XV9QUqKT0sM9OUCvCCPvcYuhIljuPIz8QAxJg/wApDgKk%2BjsMYiNcOg4cQE87y8F8tzSwwn0sJw9s8Pwu8iIYdActYiSCqKkqaPK0DwKqtLoNyuMeyczixJGviyx9URxlE%2BbxPykCYVkgR5MUlS1LqDTu209k9IMozExM/M/X8yzqu63kbLs2qHMW%2B7nLMcy3JHX8MRYPzBwCqDgvXJ9IosaLONiw8ErhpKLBSi9ZqtDKzt7E0Vr2wrv2K6iyro6aup64tYPcwrge/UHWv4zCFBmz63uvbHiPC/HeMJqjSqAsmGMxqMdsc37lp21aQCZjbhO24aCek5IKiOhLsCUga9lUrLzs0rwrt0/TDKWlNHoU8GXtmj6Iw5WzaQ4BZaE4WJeD8DgtFIT8OCTSxrBdJYVkwFMNh4UgCE0R2FmAjkuAeWOOQADlDyRE7MU0AE4uA0eJnY4SQ3cjr3OF4C4NHDyOFjgWAYEQOMWEzegyAoCB/kb/odkMYAgw0cuaFob9iAuCAIiLiJglqX9ODD8fmGIX8AHkIm0CoI%2B4Xh/jYQQF8QqePd4LB3mANwxFoC519ILA0RxNZPfwbVKlhTBz89zBVAqHtb94RlWiL2g8G8pPDwWAi4EDBCwaevAn7EAiCrQEmBr7AH/kYSufADDAAUAANTwJgO4C9dTuzDvwQQIgxDsCkDIQQigVDqH3qQXQzQDAoNMH7Sw%2BgAEXEgAsVACR2jn2UtGAMLCrCWC4BybWC8zC8DhNEHyz94ALHKJUZwmt3CeEaP4AaPQiglGyMkVIAgRhNESPo9o2i%2BgxDGK0VeVQJhGL0Eo9onQ6jmJmJYwYXR7FjAmK43RXBFGB1WBIJ2LtC50O9nsVQidTTKVNJIa0TChRBgeBoFJq5cCEBICHfxvA15aDmAsBAP5%2Bhsn0JwAupAIGxHLu7T23tS4gHLnkx2pBq513jAkHs5BKBtzoNEUIrA1hRJiXEhJXd9LBlSa/fARAfJ6GIcIUQ4gKELOoWoIuDDSB3FuO5deISOCu1ILU6RnAF49k6UyVAVBInRNifEzuRgJkpLSRADwDc%2Bm5jMKHOYuTK7RxAJIYMmdJBmC4NEnOGdY5BjKfnXgVSalF3qbYRpFd94FNIMBMwicHhYtBaaROsQxEbA0InDOgKYUbDCXUkuqL8n7KkUcxFNLmnougSkZwkggA%3D%3D%3D) ]
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