# utl::voidstream

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**voidstream** is specification of [std::ostream](https://en.cppreference.com/w/cpp/io/basic_ostream) that does nothing. This paradoxically inert class can be passed to API's that use streams to silence their output, avoiding the boilerplate of getting and then discarding undesirable output.

## Definitions

```cpp
VoidStreamBuf vstreambuf; // streambuf that discards overflow
VoidStream    vout;       // ostream that discards input
```

## Methods

> ```cpp
> voidstream::vstreambuf;
> ```

Regular stream buffers use `.overflow()`  to output buffered characters. This buffer simply discards them. 

> ```cpp
> voidstream::vout;
> ```

Pass this object to API's that use `std::ostream` to discard their corresponding output.

## Example

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:13,endLineNumber:9,positionColumn:13,positionLineNumber:9,selectionStartColumn:13,selectionStartLineNumber:9,startColumn:13,startLineNumber:9),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/include/proto_utils.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++std::cout+%3C%3C+%22std::cout+will+print:%5Cn%22%3B%0A++++std::cout+%3C%3C+%22%3Chello+there!!%3E%5Cn%5Cn%22%3B%0A%0A++++std::cout+%3C%3C+%22voidstream::vout+will+not:%5Cn%22%3B%0A++++voidstream::vout+%3C%3C+%22%3Chello+there!!%3E%5Cn%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
using namespace utl;

std::cout << "std::cout will print:\n";
std::cout << "<hello there!>\n\n";

std::cout << "voidstream::vout will not:\n";
voidstream::vout << "<hello there!>\n\n";
```

Output:
```
std::cout will print:
<hello there!>

voidstream::vout will not:
```