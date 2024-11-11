[<img src ="docs/images/icon_cpp_std_17.svg">](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[<img src ="docs/images/icon_license_mit.svg">](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/LICENSE.md)
[<img src ="docs/images/icon_header_only.svg">](https://en.wikipedia.org/wiki/Header-only)

# Prototyping utils

This is a collection of various utilities that aim to provide a set of concise "helpers" that allow prototyping with minimal boilerplate. Most of the following modules were created during my work in gamedev, HPC and math research projects.

For the ease of integration, this library is distributed in a form of a **single header**, which can be found [here](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/include/proto_utils.hpp).

## Design goals

Implementation of this library sets following design goals:

* **Header only**. Adding library to the project should be as simple as adding a header.
* **Concise**. Methods must require minimal boilerplate on user side.
* **Platform agnostic**. Implementation should be based around C++ standard and whenever OS-specific methods are unavoidable select appropriate implementation automatically.
* **Fully modular**. Every module should be independent of others and reside in a separate namespace. Modules together with their STL dependencies should be toggleable through a `#define`.

Secondary design goals also include:

* **Modern C++ style**. Usage of modern C++ idioms is preferred.
* **Usage examples**. Documentation should include usage examples for all major methods.
* **Test & benchmark coverage.** All modules should have appropriate unit test coverage, performance-focused implementations should be benchmarked against alternative approaches.

## Modules & documentation

| Module | Short description |
| - | - |
| [**utl::json**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_json.md) | Lightweight JSON parsing and serializing |
| [**utl::math**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_math.md) | Math-related utilities |
| [**utl::mvl**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_mvl.md) | Flexible API for vector and matrix operations |
| [**utl::progressbar**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_progressbar.md) | Configurable progress bars for console apps |
| [**utl::random**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_random.md) | Convenient random functions |
| [**utl::shell**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_shell.md) | Command-line related utilities |
| [**utl::sleep**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_sleep.md) | Precise sleep implementations |
| [**utl::stre**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_stre.md) | String conversion extensions |
| [**utl::table**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_table.md) | ASCII Table rendering tools |
| [**utl::timer**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_timer.md) | Timing methods |
| [**utl::voidstream**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_voidstream.md) | `std::ostream` API silencing method |

| Macro-Module | Short description |
| - | - |
| [**UTL_DEFINE**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACRO_DEFINE.md) | Automatic codegen macros |
| [**UTL_LOG**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACRO_LOG.md) | Simple debug/release logging macros |
| [**UTL_PROFILER**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACRO_PROFILER.md) | Quick scope & expression profiling macros |

## See also

* [How to include only specific modules](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/guide_selecting_modules.md)

* [Names reserved for implementation](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/guide_reserved_names.md)

* [Building tests & examples](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/guide_building_project.md)



## Requirements

* Requires **C++17** support
* Some modules require POSIX-compliant system (Linux) or Windows

## Third-party tools & libraries

While the library itself consists of a single header, it was built and tested using a number of third-party tools and libraries, some of which are embedded in a repo.

| Tool | Version | Used for |
| - | - | - |
| [clang-format](https://clang.llvm.org/docs/ClangFormat.html) | **v.14.0.0** | Automatic code formatting |
| [clangd](https://clangd.llvm.org) | **v.15.0.7** | Language server functionality |
| [CMake](https://cmake.org) | **v.3.2.11** | Build and test automation |
| [valgrind](https://valgrind.org) | **v.3.18.1** | Memory leak detection |

| Library | Version | Used for | Embedded in repo |
| - | - | - | - |
| [doctest](https://github.com/doctest/doctest) | **v.2.4.11** | Unit testing | ✔ |
| [nanobench](https://github.com/martinus/nanobench) | **v.4.3.11** | Benchmarking | ✔ |

## Work in progress

* Finalizing `utl::json` module;
* `utl::json` documentation;
* `utl::mvl` Godbolt links;
* Vector API in `utl::mvl`;
* `utl::font` module;
* Directory selection for temp file creation in `utl::shell`;
* `timer::benchmark()`;
* `utl::bitflag` module with shortcuts for various enum bitflag operations;
* `stre::centered()` stream modifier;
* Coordinate transformations in `utl::math`;
* More type traits in `utl::math`;
* Potential `utl::async` module with threadpool implementation and parallel execution methods;
* `stre::to_str()` implementation for `std::stack` and `std::queue`;

## License

This project is licensed under the MIT License - see the [LICENSE.md](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/LICENSE.md) for details
