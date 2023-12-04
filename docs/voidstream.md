# utl::voidstream

**voidstream** is specification of [std::ostream](CPPREF_LINK) that does nothing. This paradoxically inert class can be passed to API's that use streams to silence their output, avoiding the boilerplate of getting and then discarding undesirable output.

## Methods

'''cpp
vstreambuf; // streambuf that discards overflow
VoidStream vout; // ostream that discards input
'''

## Types

TYPE_TABLE

## Example

[ [Run this code](GODBOLT_LINK) ]
'''cpp
using namespace utl;

std::cout << "std::cout will print:\n";
std::cout << "<hello there!>\n\n";

std::cout << "voidstream::vout will not:\n";
voidstream::vout << "<hello there!>\n\n";
'''

Output:
'''
'''