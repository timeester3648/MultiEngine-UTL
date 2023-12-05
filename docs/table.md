

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

[ [Run this code](GODBOLT_LINK) ]
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