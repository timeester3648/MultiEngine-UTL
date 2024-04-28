# UTL_DEFINE

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**UTL_DEFINE** module contains macros for automatic codegen, such as codegen for type traits, loops, operators, enum bitflags, enum <=> string conversions and etc.

## Definitions

```cpp
// Auto codegen: Type traits
#define UTL_DEFINE_IS_FUNCTION_PRESENT(function_name, return_type, ...)

// Auto codegen: Enum <=> string conversion
#define UTL_DEFINE_ENUM_WITH_STRING_CONVERSION(enum_name, ...)

// Auto codegen: Utilities
#define UTL_DEFINE_VA_ARGS_COUNT(...) // size of __VA_ARGS__ in variadic macros
#define UTL_DEFINE_CURRENT_OS_STRING
#define UTL_DEFINE_IS_DEBUG
#define UTL_DEFINE_REPEAT(repeats) // repeat loop
#define UTL_DEFINE_EXIT(message, code) // omitted args get replaced with default values
```

## Methods

> ```cpp
> UTL_DEFINE_IS_FUNCTION_PRESENT(function_name, return_type, ...)
> ```

Declares [integral constant](https://en.cppreference.com/w/cpp/types/integral_constant) named `is_function_present_##function_name`, which returns on compile-time whether function `function_name()` with return type `return_type` that accepts arguments with types `...` exists or not.

Used mainly to detect presence of platform- or library-specific functions, which allows `constexpr if` logic based on existing methods without resorting to macros.

**NOTE**: Like a regular template class, this macro should be declared outside of functions.

> ```cpp
> UTL_DEFINE_ENUM_WITH_STRING_CONVERSION(enum_name, ...)
> ```

Declares namespace `enum_name` that contains enumeration `enum_name` with members `...` and methods `enum_name::to_string()`, `enum_name::from_string()` that convert enum values to corresponding `std::string` and back.

> ```cpp
> UTL_DEFINE_VA_ARGS_COUNT(...)
> ```

Returns number of comma-separated arguments passed. Used mainly to deduce the size of `__VA_ARGS__` in variadic macros.

**NOTE**: Since macro evaluates arbitrary preprocessor text, language construct with additional comma should be surrounded by parentheses aka `(std::pair<int, int>{4, 5})`, since `std::pair<int, int>{4, 5}` from preprocessor perspective can be interpreted as 3 separate args `std::pair<int`, ` int>{4` and ` 5}`.

> ```cpp
> UTL_DEFINE_CURRENT_OS_STRING
> ```

Evaluates to string that contains the name of compilation platform.

Possible values: `"Windows64"`, `"Windows32"`, `"Windows (CYGWIN)"`, `"Android"`, `"Linux"`, `"Unix-like OS"`, `"MacOS"`, `""` (if none of the platforms were detected).

> ```cpp
> UTL_DEFINE_IS_DEBUG
> ```

Evaluated as `true`  for a debug build, otherwise `false`.

> ```cpp
> UTL_DEFINE_REPEAT(repeats)
> ```

Loop that repeats fixed number of times, equivalent to `for (int i = 0; i < repeats; ++i)`.

> ```cpp
> UTL_DEFINE_EXIT(message, code)
> ```

Prints call site & message to  `std::cerr` and calls `std::exit()` with a given code. Omitted arguments take default values: `message = "<NO MESSAGE>"` and `code = 1`.

## Example 1 (declaring enum with string conversion)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:4,positionColumn:1,positionLineNumber:4,selectionStartColumn:1,selectionStartLineNumber:4,startColumn:1,startLineNumber:4),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0AUTL_DEFINE_ENUM_WITH_STRING_CONVERSION(Sides,+LEFT,+RIGHT,+TOP,+BOTTOM)%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++std::cout%0A++++++++%3C%3C+%22(enum+-%3E+string)+conversion:%5Cn%22%0A++++++++%3C%3C+Sides::BOTTOM+%3C%3C+%22+-%3E+%22+%3C%3C+Sides::to_string(Sides::BOTTOM)+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22(string+-%3E+enum)+conversion:%5Cn%22%0A++++++++%3C%3C+%22BOTTOM%22+%3C%3C+%22+-%3E+%22+%3C%3C+Sides::from_string(%22BOTTOM%22)+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
// Outside of function
UTL_DEFINE_ENUM_WITH_STRING_CONVERSION(Sides, LEFT, RIGHT, TOP, BOTTOM)

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

## Example 2 (detect existence of a function with SFINAE)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:32,endLineNumber:8,positionColumn:32,positionLineNumber:8,selectionStartColumn:32,selectionStartLineNumber:8,startColumn:32,startLineNumber:8),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0AUTL_DEFINE_IS_FUNCTION_PRESENT(localtime_s,+int,+tm*,+const+time_t*)%0AUTL_DEFINE_IS_FUNCTION_PRESENT(localtime_r,+tm*,+const+time_t*,+tm*)%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++//+Check+if+function+exists%0A++++constexpr+bool+exists_localtime_s+%3D+is_function_present_localtime_s::value%3B%0A++++constexpr+bool+exists_localtime_r+%3D+is_function_present_localtime_r::value%3B%0A%0A++++std::cout%0A++++++++%3C%3C+std::boolalpha%0A++++++++%3C%3C+%22Windows+localtime_s()+present:+%22+%3C%3C+exists_localtime_s+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22Linux+++localtime_r()+present:+%22+%3C%3C+exists_localtime_r+%3C%3C+%22%5Cn%22%3B%0A%0A++++//+Do+some+specific+logic+based+on+existing+function%0A%09if+constexpr+(exists_localtime_s)+%7B%0A%09%09std::cout+%3C%3C+%22%5Cn~+Some+localtime_s()-specific+logic+~%22%3B%0A%09%7D%0A%09if+constexpr+(exists_localtime_r)+%7B%0A%09%09std::cout+%3C%3C+%22%5Cn~+Some+localtime_r()-specific+logic+~%22%3B%0A%09%7D%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
// Outside of function
UTL_DEFINE_IS_FUNCTION_PRESENT(localtime_s, int, tm*,  const time_t*)
UTL_DEFINE_IS_FUNCTION_PRESENT(localtime_r, tm*,  const time_t*, tm*)

// Inside function
// Check if function exists
constexpr bool exists_localtime_s = is_function_present_localtime_s::value;
constexpr bool exists_localtime_r = is_function_present_localtime_r::value;

std::cout
	<< std::boolalpha
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
Windows 'localtime_s()' present: false
Linux   'localtime_r()' present: true

~ Some localtime_s()-specific logic ~
```

## Example 3 (getting the size of `__VA_ARGS__`)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:57,endLineNumber:5,positionColumn:57,positionLineNumber:5,selectionStartColumn:33,selectionStartLineNumber:5,startColumn:33,startLineNumber:5),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++%23define+VARIADIC_MACRO(...)+UTL_DEFINE_VA_ARGS_COUNT(__VA_ARGS__)%0A%0A++++constexpr+int+args+%3D+VARIADIC_MACRO(1,+2.,+%223%22,+A,+B,+(std::pair%3Cint,+int%3E%7B4,+5%7D),+%2212,3%22)%3B%0A%0A++++std::cout+%3C%3C+%22Size+of+__VA_ARGS__:+%22+%3C%3C+args+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
#define VARIADIC_MACRO(...) UTL_DEFINE_VA_ARGS_COUNT(__VA_ARGS__)

constexpr int args = VARIADIC_MACRO(1, 2., "3", A, B, (std::pair<int, int>{4, 5}), "12,3");

std::cout << "Size of __VA_ARGS__: " << args << "\n";
```

Output:
```
Size of __VA_ARGS__: 7
```

## Example 4 (getting compilation platform name)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:66,endLineNumber:6,positionColumn:66,positionLineNumber:6,selectionStartColumn:66,selectionStartLineNumber:6,startColumn:66,startLineNumber:6),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++std::cout%0A%09%09%3C%3C+%22Current+platform:+%22+%3C%3C+UTL_DEFINE_CURRENT_OS_STRING+%3C%3C+%22%5Cn%22%0A%09%09%3C%3C+%22Compilation+mode:+%22+%3C%3C+(UTL_DEFINE_IS_DEBUG+%3F+%22Debug%22+:+%22Release%22)+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
std::cout
		<< "Current platform: " << UTL_DEFINE_CURRENT_OS_STRING << "\n"
		<< "Compilation mode: " << (UTL_DEFINE_IS_DEBUG ? "Debug" : "Release") << "\n";
```

Output:
```
Current platform: Linux
Compilation mode: Release
```

## Example 5 (exiting)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:16,endLineNumber:5,positionColumn:16,positionLineNumber:5,selectionStartColumn:16,selectionStartLineNumber:5,startColumn:16,startLineNumber:5),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A%0A++++UTL_DEFINE_EXIT(%22Some+condition+failed%22,+3)%3B%0A%0A%09//+Code+below+will+not+be+reached%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
UTL_DEFINE_EXIT("Some condition failed",  3);

// Code below will not be reached
```

Output:
```
--------------------------------------------------
Exit triggered on [examples.cpp:312, main()] with:
Message => Some condition failed
Code    => 3
--------------------------------------------------
```
