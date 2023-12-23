
# utl::timer

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**timer** contains a number of methods for time measurement. Intended mainly for measuring code execution time without [std::chrono](https://en.cppreference.com/w/cpp/chrono) boilerplate. Outputs time as a string, prints formatted local time and date.

## Definitions
```cpp
void start() // start measurement

// Elapsed time as double
double elapsed_ms();
double elapsed_sec();
double elapsed_min();
double elapsed_hours();

// Elapsed time as string
std::string elapsed_string_ms();
std::string elapsed_string_sec();
std::string elapsed_string_min();
std::string elapsed_string_hours();

std::string elapsed_string_fullform(); // format "%H hours %M min %S sec %MS ms"

// Local date and time
std::string datetime_string();    // format "%y-%m-%d %H:%M:%S"
std::string datetime_string_id(); // format "%y-%m-%d-%H-%M-%S", works in filenames
```

## Methods
> ```cpp
> timer::start()
> ```

Sets internal start timepoint for elapsed measurements.

> ```cpp
> double timer::elapsed_ms();
> double timer::elapsed_sec();
> double timer::elapsed_min();
> double timer::elapsed_hours()
> ```

Returns elapsed time as `double`. Internally time is measured in nanoseconds.

> ```cpp
> std::string timer::elapsed_string_ms();
> std::string timer::elapsed_string_sec();
> std::string timer::elapsed_string_min();
> std::string timer::elapsed_string_hours();
> ```

Returns elapsed time as `std::string` with units.

> ```cpp
> std::string timer::elapsed_string_fullform();
> ```

Returns elapsed time in format `%H hours %M min %S sec %MS ms`.

> ```cpp
> std::string timer::datetime_string();
> std::string timer::datetime_string_id();
> ```

Returns current local date and time in format `%y-%m-%d %H:%M:%S` or `%y-%m-%d-%H-%M-%S`. Since first format is contains characters illegal in filenames, second format can be used instead.

## Example 1 (measuring time)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:7,positionColumn:1,positionLineNumber:7,selectionStartColumn:1,selectionStartLineNumber:7,startColumn:1,startLineNumber:7),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++timer::start()%3B%0A%0A++++std::this_thread::sleep_for(std::chrono::milliseconds(3700))%3B%0A%0A++++std::cout%0A++++++++%3C%3C+%22Time+elapsed+during+sleep_for(3700+ms):%5Cn%22%0A++++++++%3C%3C+timer::elapsed_string_sec()+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+timer::elapsed_string_fullform()+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
using namespace utl;

timer::start();

std::this_thread::sleep_for(std::chrono::milliseconds(3700));

std::cout
	<< "Time elapsed during sleep_for(3700 ms):\n"
	<< timer::elapsed_string_sec() << "\n"
	<< timer::elapsed_string_fullform() << "\n";
```

Output:
```
Time elapsed during sleep_for(3700 ms):
3.711095 sec
0 hours 0 min 3 sec 712 ms
```

## Example 2 (local date and time)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:5,endLineNumber:6,positionColumn:5,positionLineNumber:6,selectionStartColumn:5,selectionStartLineNumber:6,startColumn:5,startLineNumber:6),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++std::cout+%3C%3C+%22Current+time+is:+%22+%3C%3C+timer::datetime_string()+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
using namespace utl;

std::cout << "Current time is: " << timer::datetime_string() << "\n";
```

Output:
```
Current time is: 2023-12-05 02:11:34
```