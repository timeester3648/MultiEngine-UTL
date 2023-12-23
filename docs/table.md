

# utl::table

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**table** implements LaTeX-like table drawing methods. Useful in benchmarks and prototyping of algorithms to represent results as a table.

## Definitions
```cpp
// Table setup
void create(std::initializer_list<_uint> &&widths);

// (optional)
void set_formats(std::initializer_list<ColumnFormat> &&formats);
void set_ostream(std::ostream &new_ostream);

// Drawing
template<typename T, typename... Types>
void cell(T value, const Types... other_values); // draw cells with values

void hline(); // draw horizontal line

// Format flags
ColumnFormat NONE;                          // default
ColumnFormat FIXED(uint decimals = 3);      // fixed      floats with given precision
ColumnFormat DEFAULT(uint decimals = 6);    // default    floats with given precision
ColumnFormat SCIENTIFIC(uint decimals = 3); // scientific floats with given precision
ColumnFormat BOOL;                          // bools as text
```

## Methods
> ```cpp
> table::create(std::initializer_list<_uint> &&widths);
> ```

Sets up table with given column widths. Similar to LaTeX `|c{1cm}|c{1cm}|c{1cm}|` syntax.

> ```cpp
> table::set_formats(std::initializer_list<ColumnFormat> &&formats);
> ```

Sets up column [std::ios](https://en.cppreference.com/w/cpp/io/ios_base/flags) flags. Mainly used with build-in `table::` flags to change float formatting.

> ```cpp
> table::set_ostream(std::ostream &new_ostream);
> ```

Redirects output to given `std::ostream`. By default `std::cout` is used.

> ```cpp
> table::cell(T value, const Types... other_values);
> ```

Draws cells with given values, accepts any number of arguments and can be used to draw entire rows in a single line (see examples). Similar to LaTeX `val1 & val2 & val3 \\` except line breaks are placed automatically.

> ```cpp
> table::hline();
> ```

Draws a horizontal line. Similar to LaTeX `\hline`.

> ```cpp
> table::NONE;
> table::FIXED(uint decimals = 3);
> table::DEFAULT(uint decimals = 6);
> table::SCIENTIFIC(uint decimals = 3);
> table::BOOL;
> ```

Predefined format flags. `NONE` sets no flags. `FIXED(n)`, `DEFAULT(n)` and `SCIENTIFIC(n)` set corresponding float representations and precision `n`. `BOOL` makes bools render as `true`, `false`.

## Example (drawing a table)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:14,endLineNumber:18,positionColumn:14,positionLineNumber:18,selectionStartColumn:14,selectionStartLineNumber:18,startColumn:14,startLineNumber:18),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++table::create(%7B+16,+16,+16,+16,+20+%7D)%3B%0A++++table::set_formats(%7B+table::NONE,+table::DEFAULT(),+table::FIXED(2),table::SCIENTIFIC(3),+table::BOOL+%7D)%3B%0A%0A++++table::hline()%3B%0A++++table::cell(%22Method%22,+%22Threads%22,+%22Speedup%22,+%22Error%22,+%22Err.+within+range%22)%3B%0A++++table::hline()%3B%0A++++table::cell(%22Gauss%22,+16,+11.845236,+1.96e-4,+false)%3B%0A++++table::cell(%22Jacobi%22,+16,+15.512512,+1.37e-5,+false)%3B%0A++++table::cell(%22Seidel%22,+16,+13.412321,+1.74e-6,+true)%3B%0A++++table::cell(%22Relaxation%22,+16,+13.926783,+1.17e-6,+true)%3B%0A++++table::hline()%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
using namespace utl;

table::create({ 16, 16, 16, 16, 20 });
table::set_formats({ table::NONE, table::DEFAULT(), table::FIXED(2),table::SCIENTIFIC(3), table::BOOL });

table::hline();
table::cell("Method", "Threads", "Speedup", "Error", "Err. within range");
table::hline();
table::cell("Gauss", 16, 11.845236, 1.96e-4, false);
table::cell("Jacobi", 16, 15.512512, 1.37e-5, false);
table::cell("Seidel", 16, 13.412321, 1.74e-6, true);
table::cell("Relaxation", 16, 13.926783, 1.17e-6, true);
table::hline();
```

Output:
```
|----------------|----------------|----------------|----------------|--------------------|
|          Method|         Threads|         Speedup|           Error|   Err. within range|
|----------------|----------------|----------------|----------------|--------------------|
|           Gauss|              16|           11.85|       1.960e-04|               false|
|          Jacobi|              16|           15.51|       1.370e-05|               false|
|          Seidel|              16|           13.41|       1.740e-06|                true|
|      Relaxation|              16|           13.93|       1.170e-06|                true|
|----------------|----------------|----------------|----------------|--------------------|
```