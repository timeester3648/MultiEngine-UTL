# utl::sleep

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

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

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCAArKQADqgKhE4MHt6%2BekkpjgJBIeEsUTHxdpgOaUIETMQEGT5%2BXLaY9nkM1bUEBWGR0XG2NXUNWc0KQ93BvcX9sQCUtqhexMjsHOYAzMHI3lgA1CYbbggEBAkKIAD0l8RMAO4AdMCECF4RXkorsowED2gslwAIixCMQ8BZUMB0IZUAA3S4JYioIgEACeCWCwAA%2Bl5HLQFJcWExxtFLttdpgEUiiDi8QoHggEglDtgTBoAIJs9nBAh7InBCBzA4AdisHL2Er2H0xe2YbAUCSYqylBFohzFnPFkrQDBJqkRex5e2ImASmCYBAUBw2gL2ADZ1VzJXsdXqDeglhF6HsFPRTVj0MsLWksSwrYdbVw7Q87cLHRynZLxugQCglrzDm5MwczGYhH6MUY9vxiDmzNas0cfQWA0H2qHw0ds%2BZy2GHiZYm4GC345r2c7k6m0LiK83cx2u77MKbUwrgrRUMgANaCkAT7vjjYa50lvYQI14a22jTqw0V42m82W0/Wax4IUmUWJ50Spi41A%2BiZHz8plAIJEMKgs4EOa6ColiOyLkuqaAXcgq9s%2BL5TjOIBzgwC7LhAyEJLWtz1mGcwIVqL57G%2BRB7K436Dn%2BAFAahIFMGBEEYdBICwfBW6IQOBC/sOGZNlW1HIP%2BAh0YGeEhqI4yZkJImATBMJKDq6AKCyECUQAtJ%2BXRzH8SyCIKeyXHsXCYHao5Vi2fKqZ2G5mL2zqPoCXKIUJ6YWW4ZbrthqYIKiERgugq7rj2nHEcWJB7ge34nluZ7ZiaZoWqpcW3pY94ituJGke%2B2l1FRPFDnJdEkox4GQcuMGoHBhFhX22XVtOCS%2Bf5gVYTW4nBgIDa1RqXGSmRH6URGP5FbRwGgeVLFVTVDnZW5I6ZtmsnjSAnX1lJBAyYVNGiQpgFKQIKlqZpeUELpw4GUKxmmeZS2Wbm1khZuWUSk5LnhQt/GVp5Pa2T5qGoiSLDBbZoWvRFpb7oI8U2nssUWLDnmJVeKWI2lFgZY%2BEPOoNZ0FbxxUTWVzFQTNHF9eFA4FrOQMgSD2G4V1DA9UR/bZXjw1wyte30ZNpOVWx1UU/1EpfR5o27fJa11pJxJbUcPPS8wB0VEdKXYOpDDoHsWnjDpeleFdRkmWZEtWWGz32XVjnCs5CbhSaBDLAw8MIXbXIcAstCcLEvB%2BBwWikEBHBZpY1g%2BksKyYDmGw8KQBCaF7CzQcKXAPGnwoABxx5IWdmHaACcXAaPEPscJI/tJ8HnC8BcGgJ0nCxwLAMCIGmLAYvQZAUBA/xd/0OyGMAUYaA3NC0CBxAXBAETVxEwS1KinDxwvzDEKiADyETaBUifcLw/xsIIm/ocvge8Fg7zAG4Yj4ivl%2BYESRjiBfpD4CalSwpgFxv5gqgVFxGsIOPJWjV1oHgAKS8PBYGrgQMELAH6kG/sQCIyRMCAifsPCBRgm58AMMABQAA1PAmA7ibzNAHeO/BBAiDEOwKQMhBCKBUOoN%2BuhmgGFwaYcOlh9CQIuJABYqAEjtF/hpZMEYeFWEsFwYUutN5mF4HCaIgUf7wAWOUSozgtbuE8I0fw2sehFBKNkZIqQBAjCaIkcx7RjF9BiGMVoe8qgTCsXoLR7ROh1HsTMRxgwujuLGBMXxpiuCaKjqsCQ3tfZVzfiHPYqgs52g0naSQLouHABMtGDQDwNB7lwIQSKmxwm8H3loOYCwECgX6IKfQnBK6kEQbEBuAcg4hzriABu5SvakBbu3dMCRcTkEoP3Og0RQisDWEklJaSMnD2yXkvJvBMD4CIIFPQNDhCiHEIwrZLC1DVw4aQO4txmoHxiRwP2pA2nKM4JvXEQzeSoCoIk5JqT0lDyLFGJZ%2BSIAeE7uM0sJS5hlKbinEAkhoxF0kGYLgyTS6FzTlGepFdeDNNadXDptgumNwvpU0g0EzBZweMSuFdos6xDkRsDQWdC5QtRRsOJ7Ta54oqZcpRNysWsp6QSlBKRnCSCAA%3D%3D) ]
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