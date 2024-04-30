

[![Language](https://img.shields.io/badge/C++-std=17-blue.svg?style=flat&logo=cplusplus)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/LICENSE.md)

# Prototyping utils

This is a collection of various utilities that aim to provide a set of concise "helpers" that allow prototying with minimal boilerplate. Most of the following modules were created during my work in gamedev and university projects.

## Modules

> [**utl::config**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_config.md)

Simple export/import of JSON configs.

> [**utl::math**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_math.md)

Math-related utilities.

> [**utl::progressbar**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_progressbar.md)

Configurable progress bars for console appls.

> [**utl::random**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_random.md)

Convenient random functions.

> [**utl::shell**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_shell.md)

Command-line related utilities.

> [**utl::sleep**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_sleep.md)

Precise sleep implementations.

> [**utl::stre**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_stre.md)

String conversion extensions.

> [**utl::table**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_table.md)

Table rendering tools.

> [**utl::timer**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_timer.md)

Timing methods.

> [**utl::voidstream**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_voidstream.md)

`std::ostream` API silencing method.

## Macro-Modules

> [**UTL_DEFINE**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACRO_DEFINE.md)

Macros for automatic code generation of commonly needed concepts.

> [**UTL_LOG**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACRO_LOG.md)

Macros for simple debug/release logging.

> [**UTL_PROFILER**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACRO_PROFILER.md)

Macros for quick scope & expression profiling.

## See also

* [How to include only specific modules](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/guide_selecting_modules.md)

* [Names reserved for implementation](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/guide_reserved_names.md)

## Design goals

Implementation of this library sets following design goals:

* **Header only**. Adding library to the project should be as simple as adding a header.
* **Minimal boilerplate code**. Methods must be concise and require minimal prerequisites.
* **Platform agnostic**. Implementation should be based around C++ standard and whenever OS-specific methods are unavoidable select appropriate implementation automatically.
* **Toggleable modules**. Every module should be independent and reside in a separate namespace. Modules together with their STL dependencies should be toggleable through a `#define`.

Secondary design goals also include:

* **Modern C++ style**. Usage of modern C++ idioms is prefered.
* **Usage examples**. Documentation should include usage examples for all major methods.

## Work in progress

* `utl::stre` docs for `stre::repeat_symbol()`, `stre::repeat_string()` and `stre::pad_with_zeroes()`.
* `UTL_PROFILER_SET_OSTREAM()` macro;
* Method for selecting folder for temp file creation in `utl::shell`;
* `timer::benchmark()` function;
* `UTL_DEFINE_BITFLAG_ENUM()` macro;
* `UTL_DEFINE_BITFLAG_COMPAT_CHECK()` macro;
* `utl::bitflag` module with shortcuts for various enum bitflag operations;
* `config::import_json()` function;
* `stre::centered()` stream modifier;
* Potentially add `utl::config()` support for TOML and JSON5 with comments declared through `config:comment()` entries;
* Coordinate transformations in `utl::math`;
* More type traits in `utl::math`;
* Async module with functions like `async::run_task()` and `async::await(task)`;
* `stre::to_str()` implementation for `std::stack` and `std::queue`;
* Colored text module or color support for log macros that does not involve including `<Windows.h>`, perhaps a full-on `utl::font` module;


## Requirements

* Requires **C++17** support;
* Some modules require POSIX-compliant system (Linux) or Windows.


## License

This project is licensed under the MIT License - see the [LICENSE.md](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/LICENSE.md) for details
