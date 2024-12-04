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
| [**utl::json**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_json.md) | JSON parsing and serializing |
| [**utl::log**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_log.md) | Logging library |
| [**utl::math**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_math.md) | Math-related utilities |
| [**utl::mvl**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_mvl.md) | Flexible API for vector and matrix operations |
| [**utl::predef**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_predef.md) | Detection of architectures, compilers, platforms and etc. |
| [**utl::profiler**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_profiler.md) | Quick scope & expression profiling macros |
| [**utl::progressbar**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_progressbar.md) | Progress bars for CLI apps |
| [**utl::random**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_random.md) | Sensible random functions |
| [**utl::shell**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_shell.md) | Shell commands and temporary files |
| [**utl::sleep**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_sleep.md) | Precise sleep implementations |
| [**utl::stre**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_stre.md) | Efficient implementations of common string utils |
| [**utl::table**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_table.md) | ASCII table rendering tools |
| [**utl::timer**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_timer.md) | Timing methods |
| [**utl::voidstream**](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_voidstream.md) | `std::ostream` API silencing method |

## See also

* [How to include only specific modules](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/guide_selecting_modules.md)

* [Names reserved for implementation](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/guide_reserved_names.md)

* [Building tests & examples](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/guide_building_project.md)



## Requirements

* Requires **C++17** support
* Some modules require POSIX-compliant system (Linux) or Windows

## Third-party tools & libraries

While the library itself consists of a single header with no embedded dependencies, it was built and tested using a number of third-party tools and libraries, some of which are embedded in a repo.

| Tool | Version | Used for |
| - | - | - |
| [clang-format](https://clang.llvm.org/docs/ClangFormat.html) | **v.14.0.0** | Automatic code formatting |
| [clangd](https://clangd.llvm.org) | **v.15.0.7** | Language server functionality |
| [CMake](https://cmake.org) | **v.3.2.11** | Build and test automation |
| [valgrind](https://valgrind.org) | **v.3.18.1** | Memory leak detection |

| Library | Version | License | Used for | Embedded in repo |
| - | - | - | - | - |
| [doctest](https://github.com/doctest/doctest) | **v.2.4.11** | [MIT](https://github.com/doctest/doctest/blob/master/LICENSE.txt) | Unit testing | ✔ |
| [nanobench](https://github.com/martinus/nanobench) | **v.4.3.11** | [MIT](https://github.com/martinus/nanobench/blob/master/LICENSE) | Benchmarking | ✔ |
| [nlohmann json](https://github.com/nlohmann/json) | **v.3.11.3** | [MIT](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT) | Benchmark comparison | ✔ |
| [PicoJSON](https://github.com/kazuho/picojson) | **v.1.3.0** | [BSD-2](https://github.com/kazuho/picojson/blob/master/LICENSE) | Benchmark comparison | ✔ |
| [RapidJSON](https://github.com/Tencent/rapidjson) | **v.1.1.0** | [MIT, BSD, JSON](https://github.com/Tencent/rapidjson/blob/master/license.txt) | Benchmark comparison | ✔ |
| [JSONTestSuite](https://github.com/nst/JSONTestSuite/) | **commit 1ef36fa** | [MIT](https://github.com/nst/JSONTestSuite/blob/master/LICENSE) | JSON Validation test suite | ✔ |

## Work in progress

* Deprecating/refactoring following modules: `utl::voidstream`.
* Finishing following modules: `utl::log`, `utl::mvl`.
* Struct reflection in `utl::json` module;
* `utl::font` module;
* Directory selection for temp file creation in `utl::shell`;
* `utl::bitflag` module with shortcuts for various enum bitflag operations;
* `stre::centered()` stream modifier;
* Coordinate transformations in `utl::math`;
* More type traits in `utl::math`;
* Potential `utl::async` module with threadpool implementation and parallel execution methods;
* `log::stringify()` implementation for `std::stack` and `std::queue`.

## License

This project is licensed under the MIT License - see the [LICENSE.md](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/LICENSE.md) for details.
