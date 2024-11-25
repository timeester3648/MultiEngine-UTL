# utl::profiler

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**utl::profiler** module contains macros for quick scope profiling and micro-benchmarking with x86 intrinsics (GCC/clang only).

## Definitions

```cpp
UTL_PROFILER(label);

UTL_PROFILER_EXCLUSIVE(label);
    
UTL_PROFILER_BEGIN(segment_label, label);
UTL_PROFILER_END(segment_label);

UTL_PROFILER_EXCLUSIVE_BEGIN(segment_label, label);
UTL_PROFILER_EXCLUSIVE_END(segment_label);

using clock; // alias for 'std::chrono::steady_clock' or a custom implementetion, depending on macro- options
using duration   = clock::duration;
using time_point = clock::time_point;
```

## Methods

> ```cpp
> UTL_PROFILER(label);
> ```

Profiles the following scope or expression. If profiled scope was entered at any point of the program, upon exiting `main()` the table with profiling results will be printed. Profiling results include:

- Total program runtime
- Total runtime of each profiled scope
- % of total runtime taken by each profiled scope
- Profiler **labels**
- Profiler call-sites: file, function, line

**Note:** Multiple profilers can exist at the same time. Profiled scopes can be nested. Profiler overhead corresponds to entering & exiting the profiled scope, while insignificant in most applications, it may affect runtime in a tight loop.

> ```cpp
> UTL_PROFILER_EXCLUSIVE(label);
> ```

Similar to a `UTL_PROFILER`, but unlike a regular case only one `UTL_PROFILER_EXCLUSIVE` can exit at the same time. This is useful for profiling recursive functions, see [recursion profiling example](#profiling-recursion) and [why recursion is a rather non-trivial thing to measure](#why-recursion-is-a-rather-non-trivial-thing-to-measure).

> ```cpp
> UTL_PROFILER_BEGIN(segment_label, label);
> UTL_PROFILER_END(segment_label);
> 
> UTL_PROFILER_EXCLUSIVE_BEGIN(segment_label, label);
> UTL_PROFILER_EXCLUSIVE_END(segment_label);
> ```

Same thing as `UTL_PROFILER(label)`, except instead of measuring the time inside a following scope, the measurement happens between the pair of `UTL_PROFILER_BEGIN` & `UTL_PROFILER_END` macros with the same `segment_label`.

Same thing for `EXCLUSIVE` versions.

```cpp
using clock;
using duration   = clock::duration;
using time_point = clock::time_point;
```

Alias for the underlying clock implementation. By default `clock` is [`std::chrono::steady_clock`](https://en.cppreference.com/w/cpp/chrono/steady_clock), however when the use of intrinsics is enabled with `#define UTL_PROFILER_OPTION_USE_x86_INTRINSICS_FOR_FREQUENCY <user_cpu_frequency_hz>` (see [example](micro-benchmarking-with-x86-intrinsics)) it switches to a custom implementation using `rdstc` ASM instruction, which tends to have a much lower overhead than the portable implementations, thus making profilers suitable for a more precise benchmarking on a hot path.

`clock` is compatible with all [`<chrono>`](https://en.cppreference.com/w/cpp/chrono)  functionality and works like any other `std::chrono::` clock, providing a user with a way of leveraging fast time measurements of `rdstc` intrinsic by simply replacing the clock type inside a regular C++ code.

## Examples

### Profiling a Scope

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:8,positionColumn:1,positionLineNumber:8,selectionStartColumn:1,selectionStartLineNumber:8,startColumn:1,startLineNumber:8),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Avoid+computation_1()+%7B+std::this_thread::sleep_for(std::chrono::milliseconds(300))%3B+%7D%0Avoid+computation_2()+%7B+std::this_thread::sleep_for(std::chrono::milliseconds(200))%3B+%7D%0Avoid+computation_3()+%7B+std::this_thread::sleep_for(std::chrono::milliseconds(400))%3B+%7D%0Avoid+computation_4()+%7B+std::this_thread::sleep_for(std::chrono::milliseconds(600))%3B+%7D%0Avoid+computation_5()+%7B+std::this_thread::sleep_for(std::chrono::milliseconds(100))%3B+%7D%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++//+Profile+a+scope%0A++++UTL_PROFILER(%22Computation+1+%26+2%22)+%7B%0A++++++++computation_1()%3B%0A++++++++computation_2()%3B%0A++++%7D%0A%0A++++//+Profile+a+single+statement%0A++++UTL_PROFILER(%22Computation+3%22)+computation_3()%3B%0A%0A++++//+Profile+a+code+segment%0A++++UTL_PROFILER_BEGIN(segment_label,+%22Computation+4+%26+5%22)%3B%0A++++computation_4()%3B%0A++++computation_5()%3B%0A++++UTL_PROFILER_END(segment_label)%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:12,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

> [!Note]
> Online compiler explorer may be a little weird when it comes to sleep & time measurement precision.

```cpp
void computation_1() { std::this_thread::sleep_for(std::chrono::milliseconds(300)); }
void computation_2() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
void computation_3() { std::this_thread::sleep_for(std::chrono::milliseconds(400)); }
void computation_4() { std::this_thread::sleep_for(std::chrono::milliseconds(600)); }
void computation_5() { std::this_thread::sleep_for(std::chrono::milliseconds(100)); }

// ...

// Profile a scope
UTL_PROFILER("Computation 1 & 2") {
    computation_1();
    computation_2();
}

// Profile a single statement
UTL_PROFILER("Computation 3") computation_3();

// Profile a code segment
UTL_PROFILER_BEGIN(segment_label, "Computation 4 & 5");
computation_4();
computation_5();
UTL_PROFILER_END(segment_label);
```

Output:
```
--------------------- UTL PROFILING RESULTS ----------------------

 Total runtime -> 1.60 sec

 |              Call Site |             Label |   Time | Time % |
 |------------------------|-------------------|--------|--------|
 | example.cpp:21, main() | Computation 4 & 5 | 0.70 s |  43.8% |
 | example.cpp:12, main() | Computation 1 & 2 | 0.50 s |  31.2% |
 | example.cpp:18, main() |     Computation 3 | 0.40 s |  25.0% |
```

### Nested Profilers & Loops

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:6,endLineNumber:12,positionColumn:6,positionLineNumber:12,selectionStartColumn:6,selectionStartLineNumber:12,startColumn:6,startLineNumber:12),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Avoid+some_function()+%7B+std::this_thread::sleep_for(std::chrono::milliseconds(200))%3B+%7D%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++//+Profile+how+much+of+a+loop+runtime+is+spent+inside+!'some_function()!'%0A++++UTL_PROFILER(%22whole+loop%22)%0A++++for+(int+i+%3D+0%3B+i+%3C+5%3B+%2B%2Bi)+%7B%0A++++++++std::this_thread::sleep_for(std::chrono::milliseconds(200))%3B%0A%0A++++++++UTL_PROFILER(%22some_function()%22)+some_function()%3B%0A++++%7D%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:12,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
void some_function() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }

// ...

// Profile how much of a loop runtime is spent inside 'some_function()'
UTL_PROFILER("whole loop")
for (int i = 0; i < 5; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    UTL_PROFILER("some_function()") some_function();
}

// we expect to see that 'some_function()' will measure ~half the time of the 'whole loop'
```

Output:
```
-------------------- UTL PROFILING RESULTS ---------------------

 Total runtime -> 2.00 sec

 |              Call Site |           Label |   Time | Time % |
 |------------------------|-----------------|--------|--------|
 |  example.cpp:8, main() |      whole loop | 2.00 s | 100.0% |
 | example.cpp:12, main() | some_function() | 1.00 s |  50.0% |
```

### Profiling Recursion

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:22,positionColumn:1,positionLineNumber:22,selectionStartColumn:1,selectionStartLineNumber:22,startColumn:1,startLineNumber:22),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Adouble+some_computation()+%7B%0A++++utl::sleep::spinlock(1)%3B%0A++++return+utl::random::rand_double()%3B%0A%7D%0A%0Adouble+recursive_function(int+recursion)+%7B%0A++++if+(recursion+%3E+5)+return+some_computation()%3B%0A++++%0A++++UTL_PROFILER_EXCLUSIVE_BEGIN(segment_1,+%221st+recursion+branch%22)%3B%0A++++const+double+s1+%3D+recursive_function(recursion+%2B+1)%3B%0A++++UTL_PROFILER_EXCLUSIVE_END(segment_1)%3B%0A++++%0A++++UTL_PROFILER_EXCLUSIVE_BEGIN(segment_2,+%222nd+recursion+branch%22)%3B%0A++++const+double+s2+%3D+recursive_function(recursion+%2B+1)%3B%0A++++const+double+s3+%3D+recursive_function(recursion+%2B+1)%3B%0A++++UTL_PROFILER_EXCLUSIVE_END(segment_2)%3B%0A++++%0A++++return+s1+%2B+s2+%2B+s3%3B%0A%7D%0A%0Aint+main()+%7B%0A%0A++++std::cout+%3C%3C+%22SUM+%3D+%22+%3C%3C+recursive_function(0)+%3C%3C+!'%5Cn!'%3B%0A++++%0A++++//+we+expect+that+!'1st+recursion+branch!'+will+measure+~33%25+and%0A++++//+!'2nd+recursion+branch!'+will+measure+~66%25%0A%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:12,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]

```cpp
double some_computation() {
    utl::sleep::spinlock(1);
    return utl::random::rand_double();
}

double recursive_function(int recursion) {
    if (recursion > 5) return some_computation();
    
    UTL_PROFILER_EXCLUSIVE_BEGIN(segment_1, "1st recursion branch");
    const double s1 = recursive_function(recursion + 1);
    UTL_PROFILER_EXCLUSIVE_END(segment_1);
    
    UTL_PROFILER_EXCLUSIVE_BEGIN(segment_2, "2nd recursion branch");
    const double s2 = recursive_function(recursion + 1);
    const double s3 = recursive_function(recursion + 1);
    UTL_PROFILER_EXCLUSIVE_END(segment_2);
    
    return s1 + s2 + s3;
}

// ...

std::cout << "SUM = " << recursive_function(0) << '\n';

// we expect that '1st recursion branch' will measure ~33% and
// '2nd recursion branch' will measure ~66%
```

Output:
```
SUM = 359.147

------------------------------ UTL PROFILING RESULTS ------------------------------

 Total runtime -> 0.73 sec

 |                            Call Site |                Label |   Time | Time % |
 |--------------------------------------|----------------------|--------|--------|
 | example.cpp:15, recursive_function() | 2nd recursion branch | 0.49 s |  66.7% |
 | example.cpp:11, recursive_function() | 1st recursion branch | 0.24 s |  33.3% |
```

## Why Recursion is a Rather Non-trivial Thing to Measure

Let's imagine we have a recursive function `f()` that calls 2 instances of itself recursively. Let's limit recursion depth to **2** and define following variables:

- $t$ —  time spent on a single recursion tail-call;
- $T$ — total time spent inside the recursive function `f()`;
- $T_1$ — total time spent inside the **1st** branch of recursion;
- $T_2$ — total time spent inside the **2nd** branch of recursion.

The function will end up with a following call graph:

<img src ="images/profiler_recursion_call_graph.svg">

If we profile  **1st** and **2nd** branches independently, we will measure following parts of the call graph:

<img src ="images/profiler_recursion_independent_profiling_branch_1.svg">

<img src ="images/profiler_recursion_independent_profiling_branch_2.svg">

Which means $T_1 + T_2 \neq T$, which goes against the logic of what were actually trying to measure. In essense, when profiling recursion, different profilers should be aware of each other and not invoke measurement while inside the call graph of a another already existing profiler.

This is exactly the problem solved by `UTL_PROFILER_EXCLUSIVE()`, as it guarantees no other profiler will be invoked under an already existing one. We will end up with a following call graph:

<img src ="images/profiler_recursion_exclusive_profiling_branch_1.svg">

<img src ="images/profiler_recursion_exclusive_profiling_branch_2.svg">

which corresponds to the parts we were trying to measure and satisfied $T_1 + T_2 = T$. The same logic can be generalized to an arbitrary recursion with $N$ different profilers.

> [!Note]
> There are some other non-trivial questions such as "how to automatically detect recursion and avoid double-counting the time spent inside the profiler that is nested under itself in the call graph", but those fall under the "implementation details" umbrella.

## Micro-benchmarking With x86 Intrinsics

To enable the use of intrinsics add folowing `#define` before including the `proto_utils` header:

```cpp
#define UTL_PROFILER_OPTION_USE_x86_INTRINSICS_FOR_FREQUENCY 3'300'000'000
// I have 3.3 GHz CPU in this example (AMD Ryzen 5 5600H)

#include "proto_utils.hpp"
```

This will switch `profiler::clock` from `std::chrono::steady_clock` to a custom [`<chrono>`](https://en.cppreference.com/w/cpp/chrono)-compatible implementation using GCC/clang [RDTSC x86 intrinsic](https://en.wikipedia.org/wiki/Time_Stamp_Counter) that is likely to be DRASTICALLY faster at getting time that std-lib solutions like `std::chrono::steady_clock` or `ctime()`.

This is exceedingly helpful when benchmarking code on a hot path, however it comes at a price of producing a non-portable executable. Below are a few [benchmarks](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/benchmarks/benchmark_profiler.cpp) showcasing the difference on a particular hardware:

```
======= USING std::chrono ========

| relative |               ms/op |                op/s |    err% |     total | benchmark
|---------:|--------------------:|--------------------:|--------:|----------:|:----------
|   100.0% |                5.15 |              194.16 |    1.2% |      0.62 | `UTL_PROFILE()`
|   102.5% |                5.02 |              199.04 |    0.3% |      0.61 | `Theoretical best std::chrono profiler`
|   209.8% |                2.45 |              407.38 |    0.2% |      0.30 | `Theoretical best __rdtsc() profiler`
|   233.8% |                2.20 |              453.91 |    1.4% |      0.26 | `Runtime without profiling`

// light workload is difficult to time and sensitive to overhead, here profiled code is ~2x slower then
// the non-profiled workload, measurement is going to be inaccurate and affect other segments of the code

====== USING x86 INTRINSICS ======

| relative |               ms/op |                op/s |    err% |     total | benchmark
|---------:|--------------------:|--------------------:|--------:|----------:|:----------
|   100.0% |                2.53 |              394.91 |    0.4% |      0.33 | `UTL_PROFILE()`
|    49.9% |                5.07 |              197.09 |    0.2% |      0.61 | `Theoretical best std::chrono profiler`
|   102.8% |                2.46 |              405.79 |    0.3% |      0.30 | `Theoretical best __rdtsc() profiler`
|   116.8% |                2.17 |              461.13 |    0.5% |      0.26 | `Runtime without profiling`

// while profiling overhead is still present, it is much less significant,
// this is more-or-less reliable despite measuring a rather lightweight operation
```

> [!Note]
> Here *"theoretical best"* refers to a hypothetical profiler that requires zero operations aside from measuring the time at two points  — before and after entering the code segment.