# utl::config

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**config** contains function for simple export/import of JSON configs.

## Definitions

```cpp
template<typename... Types>
void export_json(std::string_view path, const Types... entries);

template<typename T>
std::tuple<std::string_view, T> entry(std::string_view key, T value);
```

## Methods

> ```cpp
> config::export_json(std::string_view path, const Types... entries);
> ```

Creates JSON at `path` with given `entries`. Entries can be specified using `config::entry(key, value)`. Corresponding to the JSON standard, `value` can be an **integer**, **float**, **string** or **bool** (or any of the similar types). Arrays and nested arrays are also supported as long as their types are homogenous.

> ```cpp
> std::tuple<std::string_view, T> config::entry(std::string_view key, T value);
> ```

Specifies a config entry.

Performs type resolution on `value` which helps other functions resolve a proper JSON datatype without being manually specified by user.

## Example 1 (saving JSON config)

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCBmAMykAA6oCoRODB7evnrJqY4CQSHhLFEx8baY9vkMQgRMxASZPn5cFVXptfUEhWGR0bEJCnUNTdmtQ109xaUDAJS2qF7EyOwc5nHByN5YANQmcW4IBASJCiAA9OfETADuAHTAhAheEV5Ky7KMBHdoLOcAIixCMQ8BZUMB0IZUAA3c6JYioIgEACeiWCwAA%2Bl5HLQFOcWEwhtFzgpFstMHCEUQsTiFHcEIlEvtsCYNABBVls4IEHYE4IQbk7erAZCkHbIBD1HYAKmlwuhsz2AHYrOyduqdm90TtmGwFIkmCtNQRaPtVWyNXt2ayAJxoBg0UAgTCqZINDHaUkMCCcy2W8xme2Ou6egQB0i%2Bv3qoN4J1fYjIn1mMyQgiYcNRzN7ZNmDRmSR3DQFvMBrNR%2BaRqMxuOCBNJsyONgYomnDOZgCsGg0ZZ7lorasz1ZAztriYDjcwGMS0Sc6Dbfq4d3zSo0NvTljMvb9/Ytg4EjuH8bHyaYXlUdDw9WRGOC/HnOwIxC8mC3PZ3WaHI8fx7MClEtHRDEqC8BgHHSBQMxMFVszMPAsEEQhkUg5NaHBLgNFLKD/g1d89wdWND1HetUESaoIOTMUeygiwdi4JUxUkdtlWw191Vwqt9wIr86wDNBMCocizEo3tqJ2DQ7i4Mx2zFcS4mYrd2L9T8j3rKgDGAQThKo6DH2fMVdMwMUqDEJR5L7CMBw4/Ca2/esCUfPBVHvLNRNE8T6J2ABaRc5KwsVRLiCT5Kw2ZKzLS4H1RTBPIQVAWHBRhFgUHYzH%2BMU4jSnZJGw%2BprmRZL6hfEzUB2GgQjC0K4nNMKIuUEFBHFYhMCYNN0FKugXyIHYhnQYc0GxMKeuHPABMfZqWHa%2Bh62rEMvQDSrzUtIaUEWHl9jcdbJswO5iHQV4qAgBbOTCpqCCWBgxLNY6lX%2BTkOHmWhOHbXg/A4LRSFQTgNssaxurJI11h4UgCE0e75gAaxAFc7hXSQbQANg0AAOOJ207G0pH0ThJBe0GPs4Xgzg0YHQfmOBYBgRAVpYNF6DICgIF%2BWn%2Bi2QxgC4RHiZoWg02IM4IAiPGImCK9OCB4XmATAB5CJtEwBwxd4X42EEKWGFoZE8awV5gDcMRcUV0gsAJIxxDe3h8CasDoUwM5zdIF15exVZ3u5So8YAiJrgTDwsDxhyWENm3iAiFJMH%2BTATeAACjFJvh1IUAA1PBMBuKXp1eoH%2BEEEQxHYTHs/kJQ1Dx3RWgMWPTB%2Byx9DwCIzkgeYSLIzhPJ6/Z/irqxLDorypbMXgYWiEEsAbw62nl6oXAYdxPGafwZ6mPoYlaXI0gEUYWiSFJ14YJeSn6cZKknjphkaOexgnsCBE6Bp95mcYz83vQJjv4JegPlf5lJJYVgkB6nq43tp9DgOxVBI3hp5eGkhxQV2ALReGhZCw7AgLgQgJBsxxC4LMXgINzazHmAgZqWAYjj0ehwHGpBA6dlIK9d6IDCYgGJngrQZNKYQCQKtRI2JyCUCZh1YgoRWCrHAZA6BsC2YIKQe9TA%2BAiAjz0IXXO4gC6yEUCodQ9sy6kBuNcRIisAEcGerQvGICpbYm4TyVAVAwEQKgTA1mRgpHiW7BADwNMBGYOwbg0mEMQCSEQfDG0kgzBcAgRodsNolRcA5ljChvBqHEzoYPAmtgmEk3wX4swNolw2k7HEDG0C4bI3hvDOJcQgH0NSSwsGcSB4mOAdU3xpBg6pGcJIIAA%3D%3D%3D) ]
```cpp
config::export_json(
    "config.json",
    config::entry("date",              "2024.04.02"              ),
    config::entry("time_steps",        500                       ),
    config::entry("time_period",       1.24709e+2                ),
    config::entry("auxiliary_info",    true                      ),
    config::entry("scaling_functions", { "identity", "log10" }   ),
    config::entry("options",           { 17, 45 }                ),
    config::entry("coefs",             { 0.125, 0.3 }            ),
    config::entry("flags",             { true, true, false }     ),
    config::entry("matrix",            { { 0.7, -1.3 }, { 3.1 } })
        // type-homogenous 2D, 3D, 4D arrays are also fine
);

// Print created file to std::cout
std::ifstream file("config.json");
std::cout << file.rdbuf();
```

Output:
```
{
    "date": "2024.04.02",
    "time_steps": 500,
    "time_period": 124.709,
    "auxiliary_info": true,
    "scaling_functions": [ "identity", "log10" ],
    "options": [ 17, 45 ],
    "coefs": [ 0.125, 0.3 ],
    "flags": [ true, true, false ],
    "matrix": [ [ 0.7, -1.3 ], [ 3.1 ] ]
}
```