

# UTL_MACROS

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**MACROS** module contains macros for automatic code generation, logging and general convenience.

## Definitions

```cpp
// Logger
#define UTL_LOG_SET_OUTPUT(ostream) // cout by default
#define UTL_LOG(...)
#define UTL_LOG_DEBUG(...)

// Automatic code generation
#define UTL_VA_ARGS_COUNT(...) // size of __VA_ARGS__ in variadic macros
#define UTL_DECLARE_ENUM_WITH_STRING_CONVERSION(enum_name, ...)

// Convenience macros
#define UTL_REPEAT(repeats) // repeat loop
#define UTL_CURRENT_OS
```

All macros in this module are prefixed with `UTL_` since namespaces do not affect the scope of preprocessor directives.

## Methods

> ```cpp
> UTL_LOG_SET_OUTPUT(ostream)
> ```

Sets [std::ostream](https://en.cppreference.com/w/cpp/io/basic_ostream) used by other logging macros.

> ```cpp
> UTL_LOG(...)
> UTL_LOG_DEBUG(...)
> ```

Prints `[<filename>:<line> (<function>)] <message>` to currently selected `std::ostream`. Multiple arguments can be passed, as long as they have a defined `operator<<` all arguments get concatenated into `<message>`.

Debug version of the macro only prints in debug mode, compiling to nothing otherwise.

> ```cpp
> UTL_VA_ARGS_COUNT(...)
> ```

Returns number of comma-separated arguments passed. Used mainly to deduce the size of `__VA_ARGS__` in variadic macros.

**NOTE**: Since macro evaluates arbitrary preprocessor text, language construct with additional comma should be surrounded by parentheses aka `(std::pair<int, int>{4, 5})`, since `std::pair<int, int>{4, 5}` from preprocessor perspective can be interpreted as 3 separate args `std::pair<int`, ` int>{4` and ` 5}`.

> ```cpp
> UTL_DECLARE_ENUM_WITH_STRING_CONVERSION(enum_name, ...)
> ```

Declares namespace `enum_name` that contains enumeration `enum_name` with members `...` and methods `enum_name::to_string()`, `enum_name::from_string()` that convert enum values to corresponding `std::string` and back.

**NOTE**: Like a regular namespace definition, this macro should be declared outside of functions.

> ```cpp
> UTL_REPEAT(repeats)
> ```

Loop that repeats fixed number of times, equivalent to `for (int i = 0; i < repeats; ++i)`.

> ```cpp
> UTL_CURRENT_OS
> ```

Evaluates to string that contains the name of compilation platform.

Possible values: `"Windows64"`, `"Windows32"`, `"Windows (CYGWIN)"`, `"Android"`, `"Linux"`, `"Unix-like OS"`, `"MacOS"`, `""` (if none of the platforms were detected).


## Example 1 (logging)

[ [Run this code](link) ]
```cpp
UTL_LOG_SET_OUTPUT(std::cerr);

// LOG compiles always
UTL_LOG("Block ", 17, ": responce in order. Proceeding with code ", 0, ".");

// LOG_DEBUG compiles only in Debug (no runtime cost in Release)
UTL_LOG_DEBUG("Texture with ID ", 15037, " seems to be corrupted.");
```

Output:
```
[examples.cpp:224, main()] Block 17: responce in order. Proceeding with code 0.
[examples.cpp:227, main()] Texture with ID 15037 seems to be corrupted.
```

## Example 2 (getting the size of `__VA_ARGS__`)

[ [Run this code](link) ]
```cpp
#define VARIADIC_MACRO(...) UTL_VA_ARGS_COUNT(__VA_ARGS__)

constexpr int args = VARIADIC_MACRO(1, 2., "3", A, B, (std::pair<int, int>{4, 5}), "12,3");

std::cout << "Size of __VA_ARGS__: " << args << "\n";
```

Output:
```
Size of __VA_ARGS__: 7
```

## Example 3 (declaring enum with string conversion)

[ [Run this code](link) ]
```cpp
// Outside of function
UTL_DECLARE_ENUM_WITH_STRING_CONVERSION(Sides, LEFT, RIGHT, TOP, BOTTOM)

// Inside function
std::cout
		<< "(enum -> string) conversion:\n"
		<< Sides::BOTTOM << " -> " << Sides::to_string(Sides::BOTTOM) << "\n"
		<< "(string -> enum) conversion:\n"
		<< "BOTTOM" << " -> " << Sides::from_string("BOTTOM") << "\n";
```

Output:
```
(enum -> string) conversion:
3 -> BOTTOM
(string -> enum) conversion:
BOTTOM -> 3
```

## Example 4 (getting compilation platform name)

[ [Run this code](link) ]
```cpp
std::cout << "Current platform: " << UTL_CURRENT_OS << "\n";
```

Output:
```
Current platform: Windows64
```
