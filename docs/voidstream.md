
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

[ [Run this code](GODBOLT_LINK) ]
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