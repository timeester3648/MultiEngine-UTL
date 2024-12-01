# utl::predef

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**predef** module contains macros and constant expressions for detecting compilation details, while also providing several helper macros for codegen.

It uses known implementation-defined macros to deduce compilation details and abstracts them away behind a unified API.

> [!Note]
> There exists a very similar [boost library](https://www.boost.org/doc/libs/1_55_0/libs/predef/doc/html/index.html) with the same name, it supports more exotic platforms, but has a different API and doesn't provide some of the utilities defined in this module.

## Definitions

```cpp
// Compiler detection
#define UTL_PREDEF_COMPILER_IS_MSVC // only one of these macros will be defined
#define UTL_PREDEF_COMPILER_IS_GCC
#define UTL_PREDEF_COMPILER_IS_CLANG
#define UTL_PREDEF_COMPILER_IS_LLVM
#define UTL_PREDEF_COMPILER_IS_ICC
#define UTL_PREDEF_COMPILER_IS_PGI
#define UTL_PREDEF_COMPILER_IS_IBMCPP
#define UTL_PREDEF_COMPILER_IS_NVCC
#define UTL_PREDEF_COMPILER_IS_UNKNOWN

constexpr std::string_view compiler_name;
constexpr std::string_view compiler_full_name;

// Platform detection
#define UTL_PREDEF_PLATFORM_IS_WINDOWS_X64 // only one of these macros will be defined
#define UTL_PREDEF_PLATFORM_IS_WINDOWS_X32
#define UTL_PREDEF_PLATFORM_IS_CYGWIN
#define UTL_PREDEF_PLATFORM_IS_ANDROID
#define UTL_PREDEF_PLATFORM_IS_LINUX
#define UTL_PREDEF_PLATFORM_IS_UNIX
#define UTL_PREDEF_PLATFORM_IS_MACOS
#define UTL_PREDEF_PLATFORM_IS_UNKNOWN

constexpr std::string_view platform_name;

// Architecture detection
#define UTL_PREDEF_ARCHITECTURE_IS_X86_64 // only one of these macros will be defined
#define UTL_PREDEF_ARCHITECTURE_IS_X86_32
#define UTL_PREDEF_ARCHITECTURE_IS_ARM
#define UTL_PREDEF_ARCHITECTURE_IS_UNKNOWN

constexpr std::string_view architecture_name;

// Language standard detection
#define UTL_PREDEF_STANDARD_IS_23_PLUS // multiple of these macros can be defined
#define UTL_PREDEF_STANDARD_IS_20_PLUS
#define UTL_PREDEF_STANDARD_IS_17_PLUS
#define UTL_PREDEF_STANDARD_IS_14_PLUS
#define UTL_PREDEF_STANDARD_IS_11_PLUS
#define UTL_DEFINE_STANDARD_IS_UNKNOWN

constexpr std::string_view standard_name;

// Compilation mode detection
#define UTL_PREDEF_MODE_IS_DEBUG

constexpr bool debug;

// Other utils
std::string compilation_summary();

#define UTL_PREDEF_VA_ARGS_COUNT(...) // size of __VA_ARGS__ in variadic macros

#define UTL_PREDEF_ENUM_WITH_STRING_CONVERSION(enum_name, ...)

#define UTL_PREDEF_IS_FUNCTION_DEFINED_TRAIT(function_name, return_type, ...)
```

## Methods

### Compiler detection

> ```cpp
> // Compiler detection
> #define UTL_PREDEF_COMPILER_IS_MSVC
> #define UTL_PREDEF_COMPILER_IS_GCC
> #define UTL_PREDEF_COMPILER_IS_CLANG
> #define UTL_PREDEF_COMPILER_IS_LLVM
> #define UTL_PREDEF_COMPILER_IS_ICC
> #define UTL_PREDEF_COMPILER_IS_PGI
> #define UTL_PREDEF_COMPILER_IS_IBMCPP
> #define UTL_PREDEF_COMPILER_IS_NVCC
> #define UTL_PREDEF_COMPILER_IS_UNKNOWN
> ```

**Only one of these macros will be defined.** The macro that gets defined corresponds to the detected compiler. If no other option is suitable, unknown is used as a fallback.

This is useful for compiler-specific conditional compilation.

> ```cpp
> constexpr std::string_view compiler_name;
> ```

`constexpr` string that evaluates to the abbreviated name of the detected compiler.

Possible values: `MSVC`, `GCC`, `clang`, `LLVM`, `ICC`, `PGI`, `IBMCPP`, `NVCC`, `<unknown>`.

> ```cpp
> constexpr std::string_view compiler_full_name;
> ```

`constexpr` string that evaluates to the full name of the detected compiler.

Possible values: `Microsoft Visual C++ Compiler`, `GNU C/C++ Compiler`, `Clang Compiler`, `LLVM Compiler`, `Inter C/C++ Compiler`, `Portland Group C/C++ Compiler`, `IBM XL C/C++ Compiler`, `Nvidia Cuda Compiler Driver`, `<unknown>`.

### Platform detection

> ```cpp
> // Platform detection
> #define UTL_PREDEF_PLATFORM_IS_WINDOWS_X64
> #define UTL_PREDEF_PLATFORM_IS_WINDOWS_X32
> #define UTL_PREDEF_PLATFORM_IS_CYGWIN
> #define UTL_PREDEF_PLATFORM_IS_ANDROID
> #define UTL_PREDEF_PLATFORM_IS_LINUX
> #define UTL_PREDEF_PLATFORM_IS_UNIX
> #define UTL_PREDEF_PLATFORM_IS_MACOS
> #define UTL_PREDEF_PLATFORM_IS_UNKNOWN
> ```

**Only one of these macros will be defined.** The macro that gets defined corresponds to the detected platform. If no other option is suitable, unknown is used as a fallback.

This is useful for platform-specific conditional compilation.

> ```cpp
> constexpr std::string_view platform_name;
> ```

`constexpr` string that evaluates to the name of the detected platform.

Possible values: `Windows64`, `Windows32`, `Windows (CYGWIN)`, `Android`, `Linux`, `Unix-like OS`, `MacOS`, `<unknown>`. 

### Architecture detection

> ```cpp
> #define UTL_PREDEF_ARCHITECTURE_IS_X86_64
> #define UTL_PREDEF_ARCHITECTURE_IS_X86_32
> #define UTL_PREDEF_ARCHITECTURE_IS_ARM
> #define UTL_PREDEF_ARCHITECTURE_IS_UNKNOWN
> ```

**Only one of these macros will be defined.** The macro that gets defined corresponds to the detected CPU architecture. If no other option is suitable, unknown is used as a fallback.

This is useful for architecture-specific conditional compilation.

> ```cpp
> constexpr std::string_view architecture_name;
> ```

`constexpr` string that evaluates to the name of the detected CPU architecture.

Possible values: `x86-64`, `x86-32`, `ARM`, `<unknown>`

### Language standard detection

> ```cpp
> #define UTL_PREDEF_STANDARD_IS_23_PLUS
> #define UTL_PREDEF_STANDARD_IS_20_PLUS
> #define UTL_PREDEF_STANDARD_IS_17_PLUS
> #define UTL_PREDEF_STANDARD_IS_14_PLUS
> #define UTL_PREDEF_STANDARD_IS_11_PLUS
> #define UTL_DEFINE_STANDARD_IS_UNKNOWN
> ```

**Multiple of these macros can be defined.** Macro that get defined correspond to the available C++ standard. If no other option is suitable, unknown is used as a fallback.

This is useful for conditional compilation based on available standard.

> ```cpp
> constexpr std::string_view standard_name;
> ```

`constexpr` string that evaluates to the name of the detected C++ standard. Possible values: `C++23`, `C++20`, `C++17`, `C++14`, `C++11`, `<unknown>`.

**Note:** Considering that this is a C++17 library, the should be no feasible way to get values below `C++17`, however they are still provided for the sake of implementation completeness, shall the source code be copied directly.

### Compilation mode detection

> ```cpp
> #define UTL_PREDEF_MODE_IS_DEBUG
> ```

Defined when compiling in debug mode. Works as an alias for `_DEBUG`, provided for the sake of feature completeness.

> ```cpp
> constexpr bool debug;
> ```

`constexpr` bool that evaluates to `true` when compiling in debug mode.

This is useful for `if constexpr` conditional compilation of debug code.

### Other utils

> ```cpp
> UTL_PREDEF_VA_ARGS_COUNT(...)
> ```

Returns number of comma-separated arguments passed. Used mainly to deduce the size of `__VA_ARGS__` in variadic macros.

**Note**: Since macro evaluates arbitrary preprocessor text, language construct with additional comma should be surrounded by parentheses aka `(std::pair<int, int>{4, 5})`, since `std::pair<int, int>{4, 5}` from preprocessor perspective can be interpreted as 3 separate args `std::pair<int`, ` int>{4` and ` 5}`.

> ```cpp
> UTL_PREDEF_ENUM_WITH_STRING_CONVERSION(enum_name, ...)
> ```

Declares namespace `enum_name` that contains enumeration `enum_name` with members `...` and methods `enum_name::to_string()`, `enum_name::from_string()` that convert enum values to corresponding `std::string` and back.

If no valid convertion to enum exists for `arg`, `enum_name::from_string(arg)` returns `enum_name::_count`.

> ```cpp
> UTL_PREDEF_IS_FUNCTION_DEFINED_TRAIT(function_name, return_type, ...)
> ```

Declares [integral constant](https://en.cppreference.com/w/cpp/types/integral_constant) named `is_function_defined_##function_name`, which returns on compile-time whether function `function_name()` with return type `return_type` that accepts arguments with types `...` exists or not.

Used mainly to detect presence of platform- or library-specific functions, which allows `constexpr if` logic based on existing methods without resorting to macros.

**Note 1**: Like a regular template class, this macro should be declared outside of functions.

**Note 2**: The usefulness of this trait is rather dubious, considering it can't be used to conditionally compile platform-specific code with `if constexpr` since that would still require all branches to use existing identifiers, however since that somewhat arcane thing is already implemented there is little reason to take it out of the library, perhaps it might prove useful in some context later on.

## Examples

### Conditional compilation

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:4,positionColumn:1,positionLineNumber:4,selectionStartColumn:1,selectionStartLineNumber:4,startColumn:1,startLineNumber:4),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++//+Portable+thread-safe+!'localtime()!'+implementation%0A++++std::time_t+timer+%3D+std::time(nullptr)%3B%0A++++std::tm+++++time_moment%7B%7D%3B%0A++++%0A++++%23if+defined(UTL_PREDEF_COMPILER_IS_GCC)+%7C%7C+defined(UTL_PREDEF_COMPILER_IS_CLANG)%0A++++localtime_r(%26timer,+%26time_moment)%3B%0A++++%23elif+defined(UTL_PREDEF_COMPILER_IS_MSVC)%0A++++localtime_s(%26time_moment,+%26timer)%3B%0A++++%23else%0A++++static+std::mutex+mtx%3B%0A++++std::lock_guard%3Cstd::mutex%3E+lock(mtx)%3B%0A++++time_moment+%3D+*std::localtime(%26timer)%3B%0A++++%23endif%0A++++%0A++++//+GCC/clang/MSVC+will+usually+complain+when+using+!'std::localtime()!'+directly%0A++++//+and+recommend+a+thread-safe+platform-specific+version+of+it.%0A++++//+Here,+we+choose+appropriate+version+automatically+and+fallback+onto+a%0A++++//+mutex-guarded+implementation+otherwise%0A++++%0A++++//+Get+formatted+date+from+localtime%0A++++std::array%3Cchar,+sizeof(%22yyyy-mm-dd+HH:MM:SS%22)%3E+buffer%3B%0A++++std::strftime(buffer.data(),+buffer.size(),+%22%25Y-%25m-%25d+%25H:%25M:%25S%22,+%26time_moment)%3B%0A++++%0A++++std::cout+%3C%3C+%22Current+time+is:+%22%3C%3C+buffer.data()%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
// Portable thread-safe 'localtime()' implementation
std::time_t timer = std::time(nullptr);
std::tm     time_moment{};

#if defined(UTL_PREDEF_COMPILER_IS_GCC) || defined(UTL_PREDEF_COMPILER_IS_CLANG)
localtime_r(&timer, &time_moment);
#elif defined(UTL_PREDEF_COMPILER_IS_MSVC)
localtime_s(&time_moment, &timer);
#else
static std::mutex mtx;
std::lock_guard<std::mutex> lock(mtx);
time_moment = *std::localtime(&timer);
#endif

// GCC/clang/MSVC will usually complain when using 'std::localtime()' directly
// and recommend a thread-safe platform-specific version of it.
// Here, we choose appropriate version automatically and fallback onto a
// mutex-guarded implementation otherwise

// Get formatted date from localtime
std::array<char, sizeof("yyyy-mm-dd HH:MM:SS")> buffer;
std::strftime(buffer.data(), buffer.size(), "%Y-%m-%d %H:%M:%S", &time_moment);

std::cout << "Current time is: "<< buffer.data();
```

Output:
```
Current time is: 2024-12-01 03:39:46
```

### Compilation summary

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:53,endLineNumber:5,positionColumn:53,positionLineNumber:5,selectionStartColumn:53,selectionStartLineNumber:5,startColumn:53,startLineNumber:5),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++std::cout+%3C%3C+utl::predef::compilation_summary()%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
std::cout << utl::predef::compilation_summary();
```

Output:
```
Compiler:          GNU C/C++ Compiler
Platform:          Linux
Architecture:      x86-64
Compiled in DEBUG: false
Compiled under OS: true
Compilation date:  Dec  1 2024 03:47:20
```

### Counting Variadic Macro Args

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:2,endLineNumber:12,positionColumn:2,positionLineNumber:12,selectionStartColumn:2,selectionStartLineNumber:12,startColumn:2,startLineNumber:12),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++%23define+VARIADIC_MACRO(...)+UTL_PREDEF_VA_ARGS_COUNT(__VA_ARGS__)%0A%0A++++constexpr+int+args+%3D+VARIADIC_MACRO(1,+2.,+%223%22,+A,+B,+(std::pair%3Cint,+int%3E%7B4,+5%7D),+%2212,3%22)%3B%0A%0A++++std::cout+%3C%3C+%22Size+of+__VA_ARGS__:+%22+%3C%3C+args+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
#define VARIADIC_MACRO(...) UTL_PREDEF_VA_ARGS_COUNT(__VA_ARGS__)

constexpr int args = VARIADIC_MACRO(1, 2., "3", A, B, (std::pair<int, int>{4, 5}), "12,3");

std::cout << "Size of __VA_ARGS__: " << args << "\n";
```

Output:
```
Size of __VA_ARGS__: 7
```

### Declaring enum with string conversion

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:14,endLineNumber:13,positionColumn:14,positionLineNumber:13,selectionStartColumn:14,selectionStartLineNumber:13,startColumn:14,startLineNumber:13),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0AUTL_PREDEF_ENUM_WITH_STRING_CONVERSION(Sides,+LEFT,+RIGHT,+TOP,+BOTTOM)%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++std::cout%0A++++++++%3C%3C+%22(enum+-%3E+string)+conversion:%5Cn%22%0A++++++++%3C%3C+Sides::BOTTOM+%3C%3C+%22+-%3E+%22+%3C%3C+Sides::to_string(Sides::BOTTOM)+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22(string+-%3E+enum)+conversion:%5Cn%22%0A++++++++%3C%3C+%22BOTTOM%22+%3C%3C+%22+-%3E+%22+%3C%3C+Sides::from_string(%22BOTTOM%22)+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
// Outside of function
UTL_PREDEF_ENUM_WITH_STRING_CONVERSION(Sides, LEFT, RIGHT, TOP, BOTTOM)

// Inside the function
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

### Detecting existence of a function signature

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:34,endLineNumber:6,positionColumn:34,positionLineNumber:6,selectionStartColumn:34,selectionStartLineNumber:6,startColumn:34,startLineNumber:6),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0AUTL_PREDEF_IS_FUNCTION_DEFINED(localtime_s,+int,+tm*,++const+time_t*)%0AUTL_PREDEF_IS_FUNCTION_DEFINED(localtime_r,+tm*,++const+time_t*,+tm*)%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++constexpr+bool+localtime_s_is_defined+%3D+is_function_defined_localtime_s::value%3B%0A++++constexpr+bool+localtime_r_is_defined+%3D+is_function_defined_localtime_r::value%3B%0A%0A++++std::cout%0A++++++++%3C%3C+std::boolalpha%0A++++++++%3C%3C+%22Windows+localtime_s()+present:+%22+%3C%3C+localtime_s_is_defined+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22Linux+++localtime_r()+present:+%22+%3C%3C+localtime_r_is_defined+%3C%3C+%22%5Cn%22%3B%0A%0A++++//+Do+some+specific+logic+based+on+existing+function%0A++++if+constexpr+(localtime_s_is_defined)+%7B%0A++++++++std::cout+%3C%3C+%22%5Cn~+Some+localtime_s()-specific+logic+~%22%3B%0A++++%7D%0A++++if+constexpr+(localtime_r_is_defined)+%7B%0A++++++++std::cout+%3C%3C+%22%5Cn~+Some+localtime_r()-specific+logic+~%22%3B%0A++++%7D%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
// Outside the function
UTL_PREDEF_IS_FUNCTION_DEFINED(localtime_s, int, tm*,  const time_t*)
UTL_PREDEF_IS_FUNCTION_DEFINED(localtime_r, tm*,  const time_t*, tm*)

// ...

// Inside the function
constexpr bool localtime_s_is_defined = is_function_defined_localtime_s::value;
constexpr bool localtime_r_is_defined = is_function_defined_localtime_r::value;

std::cout
    << std::boolalpha
    << "Windows localtime_s() present: " << localtime_s_is_defined << "\n"
    << "Linux   localtime_r() present: " << localtime_r_is_defined << "\n";

// Do some specific logic based on existing function
if constexpr (localtime_s_is_defined) {
    std::cout << "\n~ Some localtime_s()-specific logic ~";
}
if constexpr (localtime_r_is_defined) {
    std::cout << "\n~ Some localtime_r()-specific logic ~";
}
```

Output:
```
Windows localtime_s() present: false
Linux   localtime_r() present: true

~ Some localtime_r()-specific logic ~
```