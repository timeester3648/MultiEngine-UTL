



# UTL_PROFILER

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**URL_PROFILER** module contains macros for quick scope profiling.

## Definitions

```cpp
UTL_PROFILE           // assigns default label to profiler
UTL_PROFILE_LABELED() // assigns custom  label to profiler
```

## Methods

> ```cpp
> UTL_PROFILE
> UTL_PROFILE_LABELED(label)
> ```

Profiles the following scope or expression. If profiled scope was entered at any point of the program, upon exiting `main()` the table with profiling results will be printed. Profiling results include:

- Total program runtime
- Total runtime of each profiled scope
- % of total runtime taken by each profiled scope
- Profiler **labels** (if using `UTL_PROFILE_LABELED()`, otherwise the label is set to `<NONE>`)
- Profiler locations: file, function, line

Miltiple profilers can exist at the same time. Profiled scopes can be nested. Profiler overhead corresponds to entering & exiting the profiled scope, while insignificant in most applications, it may affect runtime in a tight loop.

## Example 1 (profiling a function called inside a loop)

[ [Run this code](link) ]
```cpp
code
```

Output:
```
print
```

## Example 2 (nesting profilers)

[ [Run this code](link) ]
```cpp
code
```

Output:
```
print
```