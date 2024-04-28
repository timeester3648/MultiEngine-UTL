

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

> [**UTL_PROFILER**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACROS.md)

Macros for quick scope & expression profiling.

> [**UTL_MACROS**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACROS.md)

Macros for automatic code generation, logging and general convenience

## Design goals

Implementation of this library sets following design goals:

* **Header only**. Adding library to the project should be as simple as adding a header.
* **Minimal boilerplate code**. Methods must be concise and require minimal prerequisites.
* **Platform agnostic**. Implementation should be based around C++ standard and avoid any OS-specific behaviour.
* **Toggleable modules**. Every module should be independent and reside in a separate namespace. Modules together with their STL dependencies should be toggleable through a `#define`.

Secondary design goals also include:

* **Modern C++ style**. Usage of modern C++ idioms is prefered.
* **Usage examples**. Documentation should include usage examples for all major methods.

## Work in progress

* Coordinate transformations in **utl::math**;
* More type traits in **utl::math**;
* Async module with functions like `async::run_task()` and `async::await(task)`;
* `stre::to_str()` implementation for `std::stack` and `std::queue`;
* Colored text module or color support for log macros that does not involve including '<Windows.h>';
* `UTL_PROFILE` module docs (godbold);
* `UTL_DEFINE` module docs;
* `utl::config` module docs (godbold).


## Requirements

* Requires **C++17** support;
* Some modules require POSIX-compliant system (Linux) or Windows.


## License

This project is licensed under the MIT License - see the [LICENSE.md](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/LICENSE.md) for details
