


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
#define UTL_DECLARE_IS_FUNCTION_PRESENT(function_name, return_type, ...)

// Convenience macros
#define UTL_REPEAT(repeats) // repeat loop
#define UTL_CURRENT_OS
```

All macros in this module are prefixed with `UTL_` since namespaces do not affect the scope of preprocessor directives.

## Methods

> ```cpp
> UTL_LOG_SET_OUTPUT(ostream)
> ```

Sets [std::ostream](https://en.cppreference.com/w/cpp/io/basic_ostream) used by other logging macros. `std::cout` is set by default.

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

> ```cpp
> UTL_DECLARE_IS_FUNCTION_PRESENT(function_name, return_type, ...)
> ```

Declares [integral constant](https://en.cppreference.com/w/cpp/types/integral_constant) named `is_function_present_##function_name`, which returns on compile-time whether function `function_name()` with return type `return_type` that accepts arguments with types `...` exists or not.

Used mainly to detect presence of platform- or library-specific functions, which allows `constexpr if` logic based on existing methods without resorting to macros.

**NOTE**: Like a regular template class, this macro should be declared outside of functions.

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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:5,endLineNumber:4,positionColumn:5,positionLineNumber:4,selectionStartColumn:5,selectionStartLineNumber:4,startColumn:5,startLineNumber:4),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++%0A++++UTL_LOG_SET_OUTPUT(std::cerr)%3B%0A%0A++++//+LOG+compiles+always%0A++++UTL_LOG(%22Block+%22,+17,+%22:+responce+in+order.+Proceeding+with+code+%22,+0,+%22.%22)%3B%0A%0A++++//+LOG_DEBUG+compiles+only+in+Debug+(no+runtime+cost+in+Release)%0A++++UTL_LOG_DEBUG(%22Texture+with+ID+%22,+15037,+%22+seems+to+be+corrupted.%22)%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:6,positionColumn:1,positionLineNumber:6,selectionStartColumn:1,selectionStartLineNumber:6,startColumn:1,startLineNumber:6),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++%23define+VARIADIC_MACRO(...)+UTL_VA_ARGS_COUNT(__VA_ARGS__)%0A%0A++++constexpr+int+args+%3D+VARIADIC_MACRO(1,+2.,+%223%22,+A,+B,+(std::pair%3Cint,+int%3E%7B4,+5%7D),+%2212,3%22)%3B%0A%0A++++std::cout+%3C%3C+%22Size+of+__VA_ARGS__:+%22+%3C%3C+args+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:6,positionColumn:1,positionLineNumber:6,selectionStartColumn:1,selectionStartLineNumber:6,startColumn:1,startLineNumber:6),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0AUTL_DECLARE_ENUM_WITH_STRING_CONVERSION(Sides,+LEFT,+RIGHT,+TOP,+BOTTOM)%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++std::cout%0A++++++++%3C%3C+%22(enum+-%3E+string)+conversion:%5Cn%22%0A++++++++%3C%3C+Sides::BOTTOM+%3C%3C+%22+-%3E+%22+%3C%3C+Sides::to_string(Sides::BOTTOM)+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22(string+-%3E+enum)+conversion:%5Cn%22%0A++++++++%3C%3C+%22BOTTOM%22+%3C%3C+%22+-%3E+%22+%3C%3C+Sides::from_string(%22BOTTOM%22)+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
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

## Example 4 (detect existence of a function with SFINAE)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:21,endLineNumber:14,positionColumn:21,positionLineNumber:14,selectionStartColumn:21,selectionStartLineNumber:14,startColumn:21,startLineNumber:14),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0AUTL_DECLARE_IS_FUNCTION_PRESENT(localtime_s,+int,+tm*,+const+time_t*)%0AUTL_DECLARE_IS_FUNCTION_PRESENT(localtime_r,+tm*,+const+time_t*,+tm*)%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++//+Check+if+function+exists%0A++++constexpr+bool+exists_localtime_s+%3D+is_function_present_localtime_s::value%3B%0A++++constexpr+bool+exists_localtime_r+%3D+is_function_present_localtime_r::value%3B%0A%0A++++std::cout%0A++++++++%3C%3C+%22Windows+localtime_s()+present:+%22+%3C%3C+exists_localtime_s+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22Linux+++localtime_r()+present:+%22+%3C%3C+exists_localtime_r+%3C%3C+%22%5Cn%22%3B%0A%0A++++//+Do+some+specific+logic+based+on+existing+function%0A%09if+constexpr+(exists_localtime_s)+%7B%0A%09%09std::cout+%3C%3C+%22%5Cn~+Some+localtime_s()-specific+logic+~%22%3B%0A%09%7D%0A%09if+constexpr+(exists_localtime_r)+%7B%0A%09%09std::cout+%3C%3C+%22%5Cn~+Some+localtime_r()-specific+logic+~%22%3B%0A%09%7D%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
// Outside of function
UTL_DECLARE_IS_FUNCTION_PRESENT(localtime_s, int, tm*,  const time_t*)
UTL_DECLARE_IS_FUNCTION_PRESENT(localtime_r, tm*,  const time_t*, tm*)

// Inside function
// Check if function exists
constexpr bool exists_localtime_s = is_function_present_localtime_s::value;
constexpr bool exists_localtime_r = is_function_present_localtime_r::value;

std::cout
	<< "Windows 'localtime_s()' present: " << exists_localtime_s << "\n"
	<< "Linux   'localtime_r()' present: " << exists_localtime_r << "\n";

// Do some specific logic based on existing function
if constexpr (exists_localtime_s) {
	std::cout << "\n~ Some localtime_s()-specific logic ~";
}
if constexpr (exists_localtime_r) {
	std::cout << "\n~ Some localtime_r()-specific logic ~";
}
```

Output:
```
Windows 'localtime_s()' present: true
Linux   'localtime_r()' present: false

~ Some localtime_s()-specific logic ~
```

## Example 5 (getting compilation platform name)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:108,endLineNumber:1,positionColumn:108,positionLineNumber:1,selectionStartColumn:108,selectionStartLineNumber:1,startColumn:108,startLineNumber:1),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++std::cout+%3C%3C+%22Current+platform:+%22+%3C%3C+UTL_CURRENT_OS+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
std::cout << "Current platform: " << UTL_CURRENT_OS << "\n";
```

Output:
```
Current platform: Windows64
```
