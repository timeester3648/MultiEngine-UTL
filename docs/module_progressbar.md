# utl::progressbar

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**progressbar** adds configurable progress bars for terminal applications.

## Definitions

```cpp
// Configuration
void set_ostream(std::ostream &new_ostream);

// 'Percentage' progressbar
// > Proper progress bar, uses carriage return escape
// > sequence (\r) to render new states in the same spot
class Percentage {
public:
    Percentage(
        char done_char = '#',
        char not_done_char = '.',
        size_t bar_length = 30,
        double update_rate = 1e-2,
        bool show_time_estimate = true
    );
    
    start();
    set_progress(double percentage);
    finish();
};

// 'Ruler' progressbar
// > Primitive & lightweight progress bar, useful when
// > terminal has no proper support for escape sequences
class Ruler {
public:
    Percentage(
        char done_char = '#'
    );
    
    start();
    set_progress(double percentage);
    finish();
};
```

## Methods

> ```cpp
> progressbar::set_ostream()
> ```

Redirects output to given `std::ostream`. By default `std::cout` is used.

> ```cpp
> progressbar::Percentage::Percentage(
>      char done_char = '#',
>      char not_done_char = '.',
>      size_t bar_length = 30,
>      double update_rate = 1e-2,
>      bool show_time_estimate = true
> )
> ```

Construct progress bar object with following options:

- `done_char` - character used for "filled" part of the bar;
- `not_done_char ` - character used for "empty" part of the bar;
- `bar_length` - bar width in characters;
- `update_rate` - how often should the bar update its displayed percentage, `1e-2` corresponds to a single percent;
- `show_time_estimate` - whether to show remaining time estimate (estimated through linear extrapolation);

**Note 1:** for terminals that do not support carriage return `\r`  a less advanced `progressbar::Ruler` should be used.

**Note 2:** for terminals that cannot fit progress bar into a single line & don't properly handle carriage return for wrapped lines, a less advanced `progressbar::Ruler` can be used. Such case is a rarity and depends on terminal implementation.

> ```cpp
> progressbar::Percentage::start();
> progressbar::Percentage::set_progress(double percentage);
> progressbar::Percentage::finish();
> ```

Start, update & finish progress bar display. Percentage is a value in $[0, 1]$ range, corresponding to a portion of total progress.

> ```cpp
> progressbar::Ruler::Ruler(
>      char done_char = '#'
> )
> ```

Construct progress bar object with following options:

- `done_char` - character used for "filled" part of the bar;

> ```cpp
> progressbar::Ruler::start();
> progressbar::Ruler::set_progress(double percentage);
> progressbar::Ruler::finish();
> ```

Start, update & finish progress bar display. Percentage is a value in $[0, 1]$ range, corresponding to a portion of total progress.



## Example 1 ('Percentage' progress bar)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:78,endLineNumber:11,positionColumn:5,positionLineNumber:9,selectionStartColumn:78,selectionStartLineNumber:11,startColumn:5,startLineNumber:9),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++using+namespace+utl%3B%0A++++using+ms+%3D+std::chrono::milliseconds%3B%0A%0A++++//+Due+to+%22Godbolt.org%22+limitation+on+execution+time+and+nonfunctional+carriage+return,%0A++++//+in+this+example+we+use+short+runtime+and+a+rather+rough+update+rate.%0A++++//+Real-time+progress+display+may+also+be+skipped+by+the+online+compiler.%0A++++constexpr+ms+time(5!'000)%3B%0A++++constexpr+ms+tau(700)%3B%0A%0A++++//+Create+progress+bar+with+style+!'%5B%23%23%23%23%23...%5D+xx.xx%25!'+and+width+50%0A++++//+that+updates+every+0.05%25%0A++++auto+bar+%3D+progressbar::Percentage(!'%23!',+!'.!',+20,+0.05+*+1e-2,+true)%3B%0A%0A++++std::cout+%3C%3C+%22%5Cn-+progressbar::Percentage+-%22%3B%0A%0A++++bar.start()%3B%0A++++for+(ms+t(0)%3B+t+%3C%3D+time%3B+t+%2B%3D+tau)+%7B%0A++++++++std::this_thread::sleep_for(tau)%3B+//+simulate+some+work%0A++++++++const+double+percentage+%3D+double(t.count())+/+time.count()%3B%0A%0A++++++++bar.set_progress(percentage)%3B%0A++++%7D%0A++++bar.finish()%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:12,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
using namespace utl;
using ms = std::chrono::milliseconds;

constexpr ms time(15'000);
constexpr ms tau(10);

// Create progress bar with style '[#####...] xx.xx%' and width 50
// that updates every 0.05%
auto bar = progressbar::Percentage('#', '.', 50, 0.05 * 1e-2, true);

std::cout << "\n- progressbar::Percentage -";

bar.start();
for (ms t(0); t <= time; t += tau) {
    std::this_thread::sleep_for(tau); // simulate some work
    const double percentage = double(t.count()) / time.count();

    bar.set_progress(percentage);
}
bar.finish();
```

Output (at some point in time):
```
- progressbar::Percentage -
[#################################.................] 67.45% (remaining: 7 sec)
```

## Example 2 ('Ruler' progress bar)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:6,endLineNumber:21,positionColumn:6,positionLineNumber:21,selectionStartColumn:6,selectionStartLineNumber:21,startColumn:6,startLineNumber:21),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A++++using+ms+%3D+std::chrono::milliseconds%3B%0A%0A++++//+Due+to+%22Godbolt.org%22+limitation+on+execution+time,%0A++++//+in+this+example+we+use+short+runtime+and+a+rather+rough+update+rate.%0A++++//+Real-time+progress+display+may+also+be+skipped+by+the+online+compiler.%0A++++constexpr+ms+time(1!'000)%3B%0A++++constexpr+ms+tau(10)%3B%0A%0A++++auto+ruler+%3D+progressbar::Ruler(!'%23!')%3B%0A%0A++++ruler.start()%3B%0A++++for+(ms+t(0)%3B+t+%3C%3D+time%3B+t+%2B%3D+tau)+%7B%0A++++++++std::this_thread::sleep_for(tau)%3B+//+simulate+some+work%0A++++++++const+double+percentage+%3D+double(t.count())+/+time.count()%3B%0A%0A++++++++ruler.set_progress(percentage)%3B%0A++++%7D%0A++++ruler.finish()%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:12,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
using namespace utl;
using ms = std::chrono::milliseconds;

constexpr ms time(15'000);
constexpr ms tau(10);

// Create a primitive progress bar with ruler-like style
auto ruler = progressbar::Ruler('#');

ruler.start();
for (ms t(0); t <= time; t += tau) {
    std::this_thread::sleep_for(tau); // simulate some work
    const double percentage = double(t.count()) / time.count();

    ruler.set_progress(percentage);
}
ruler.finish();
```

Output (at some point in time):
```
- progressbar::Ruler -
 0    10   20   30   40   50   60   70   80   90   100%
 |----|----|----|----|----|----|----|----|----|----|
 ###################################
```
