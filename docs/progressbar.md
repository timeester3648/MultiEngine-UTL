

# utl::progressbar

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**progressbar** adds configurable progress bars for terminal applications.

## Definitions

```cpp
// Configuration
set_ostream(std::ostream &new_ostream);

// 'Percentage' progressbar
//    Proper progress bar, uses carriage return escape
//    sequence (\r) to render new states in the same spot
class Percentage {
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
//    Primitive & lightweight progress bar, usefull when
//    terminal has no proper support for escape sequences
class Ruler{
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

Start, update & finish progress bar display. Percentage is a value in $[0, 1]$ range, corresponding to a porting of total progress.

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

Start, update & finish progress bar display. Percentage is a value in $[0, 1]$ range, corresponding to a porting of total progress.



## Example 1 ('Percentage' progress bar)

[ [Run this code]() ]
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

[ [Run this code]() ]
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
