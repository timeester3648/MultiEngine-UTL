# UTL_PROFILER

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**URL_PROFILER** module contains macros for quick scope profiling.

## Definitions

```cpp
UTL_PROFILER           // assigns default label to profiler
UTL_PROFILER_LABELED() // assigns custom  label to profiler

UTL_PROFILER_REROUTE_TO_FILE(filepath)
```

## Methods

> ```cpp
> UTL_PROFILER
> UTL_PROFILER_LABELED(label)
> ```

Profiles the following scope or expression. If profiled scope was entered at any point of the program, upon exiting `main()` the table with profiling results will be printed. Profiling results include:

- Total program runtime
- Total runtime of each profiled scope
- % of total runtime taken by each profiled scope
- Profiler **labels** (if using `UTL_PROFILE_LABELED()`, otherwise the label is set to `<NONE>`)
- Profiler locations: file, function, line

Miltiple profilers can exist at the same time. Profiled scopes can be nested. Profiler overhead corresponds to entering & exiting the profiled scope, while insignificant in most applications, it may affect runtime in a tight loop.

> ```cpp
> UTL_PROFILER_REROUTE_TO_FILE(filepath)
> ```

Reroutes profiler summary from `std::cout` to a file with a given name.

## Example 1 (profiling a function called inside a loop)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:34,endLineNumber:11,positionColumn:34,positionLineNumber:11,selectionStartColumn:34,selectionStartLineNumber:11,startColumn:34,startLineNumber:11),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0A%23include+%3Cthread%3E%0A%0A%0Avoid+some_function()+%7B%0A++++std::this_thread::sleep_for(std::chrono::milliseconds(200))%3B%0A%7D%0A%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++%0A++++//+Profile+how+much+of+a+loop+runtime+is+spent+inside+!'some_function()!'%0A%09UTL_PROFILER_LABELED(%22whole+loop%22)%0A%09for+(int+i+%3D+0%3B+i+%3C+5%3B+%2B%2Bi)+%7B%0A%09%09std::this_thread::sleep_for(std::chrono::milliseconds(200))%3B%0A%0A%09%09UTL_PROFILER_LABELED(%22some_function()%22)%0A%09%09some_function()%3B%0A%09%7D%0A++++%0A%09//+we+expect+to+see+that+!'some_function()!'+will+measure+about+half+the+time+of+the+!'whole+loop!'%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:70.70496083550914,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:29.295039164490866,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
void some_function() {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

// ...

// Profile how much of a loop runtime is spent inside 'some_function()'
UTL_PROFILER_LABELED("whole loop")
for (int i = 0; i < 5; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    UTL_PROFILER_LABELED("some_function()")
    some_function();
}

// we expect to see that 'some_function()' will measure about half the time of the 'whole loop'
```

Output:
```
-------------------- UTL PROFILING RESULTS ---------------------

 Total runtime -> 2.00 sec

 |              Call Site |           Label |   Time | Time % |
 |------------------------|-----------------|--------|--------|
 | example.cpp:14, main() |      whole loop | 2.00 s | 100.0% |
 | example.cpp:18, main() | some_function() | 1.00 s |  50.0% |
```

## Example 2 (profiling a scope)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:6,endLineNumber:23,positionColumn:2,positionLineNumber:13,selectionStartColumn:6,selectionStartLineNumber:23,startColumn:2,startLineNumber:13),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0A%23include+%3Cthread%3E%0A%0A%0Avoid+computation_1()+%7B+std::this_thread::sleep_for(std::chrono::milliseconds(200))%3B+%7D%0Avoid+computation_2()+%7B+std::this_thread::sleep_for(std::chrono::milliseconds(700))%3B+%7D%0Avoid+computation_3()+%7B+std::this_thread::sleep_for(std::chrono::milliseconds(100))%3B+%7D%0A%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++%0A%09UTL_PROFILER+%7B%0A++++++++computation_1()%3B%0A++++%7D%0A%0A++++UTL_PROFILER+%7B%0A++++++++computation_2()%3B%0A++++%7D%0A%0A++++UTL_PROFILER+%7B%0A++++++++computation_3()%3B%0A++++%7D%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:70.70496083550914,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:29.295039164490866,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
void computation_1() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
void computation_2() { std::this_thread::sleep_for(std::chrono::milliseconds(700)); }
void computation_3() { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }

// ...

UTL_PROFILER {
    computation_1();
}

UTL_PROFILER {
    computation_2();
}

UTL_PROFILER {
    computation_3();
}
```

Output:
```
---------------- UTL PROFILING RESULTS ----------------

 Total runtime -> 1.00 sec

 |              Call Site |  Label |   Time | Time % |
 |------------------------|--------|--------|--------|
 | example.cpp:21, main() | <NONE> | 0.10 s |  10.0% |
 | example.cpp:17, main() | <NONE> | 0.70 s |  70.0% |
 | example.cpp:13, main() | <NONE> | 0.20 s |  20.0% |
```