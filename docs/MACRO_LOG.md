# UTL_LOG

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**URL_PROFILER** module contains macros for simple debug/release logging that uses less boilerplate than standard streams and automatically formats info about the call site.

## Definitions

```cpp
#define UTL_LOG_SET_OUTPUT(ostream) // std::cout by default
#define UTL_LOG(...)
#define UTL_LOG_DEBUG(...)
```

## Methods

> ```cpp
> UTL_LOG_SET_OUTPUT(ostream)
> ```

Sets [std::ostream](https://en.cppreference.com/w/cpp/io/basic_ostream) used by other logging macros. `std::cout` is set by default.

> ```cpp
> UTL_LOG(...)
> UTL_LOG_DEBUG(...)
> ```

Prints `[<filename>:<line> (<function>)] <message>` to currently selected `std::ostream`. Multiple arguments can be passed, as long as they have a defined `operator<<` all arguments get concatenated into a `<message>`.

Debug version of the macro only prints in debug mode, compiling to nothing otherwise.

## Example (logging to `std::cerr`)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:5,endLineNumber:4,positionColumn:5,positionLineNumber:4,selectionStartColumn:5,selectionStartLineNumber:4,startColumn:5,startLineNumber:4),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++%0A++++UTL_LOG_SET_OUTPUT(std::cerr)%3B%0A%0A++++//+LOG+compiles+always%0A++++UTL_LOG(%22Block+%22,+17,+%22:+responce+in+order.+Proceeding+with+code+%22,+0,+%22.%22)%3B%0A%0A++++//+LOG_DEBUG+compiles+only+in+Debug+(no+runtime+cost+in+Release)%0A++++UTL_LOG_DEBUG(%22Texture+with+ID+%22,+15037,+%22+seems+to+be+corrupted.%22)%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
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