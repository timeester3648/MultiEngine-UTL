# Building tests & examples

[<- back to README.md](https://github.com/DmitriBogdanov/UTL/tree/master)

This project uses [CMake](https://cmake.org) build system.

Unit testing for modules is implemented using [doctest](https://github.com/doctest/doctest) together with **CMake** testing facilities (see [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html)). All tests can be found in [`tests/`](https://github.com/DmitriBogdanov/UTL/tree/master/tests) split by per-module basis.

Benchmarks are implemented using the [nanobench](https://github.com/martinus/nanobench) functionality and can be found in [`benchmarks/`](https://github.com/DmitriBogdanov/UTL/tree/master/benchmarks) split by per-module basis.

All tests and benchmarks compile with `-Wall -Wextra -Wpedantic -Werror` flags.

## Building with a script

Clone the repo:

```bash
git clone https://github.com/DmitriBogdanov/UTL.git &&
cd "UTL/"
```

Configure & build the project:

```bash
bash actions.sh clear config build
```

Run all tests:

```bash
bash actions.sh test
```

Run benchmark:

```bash
./build/benchmarks/<benchmark_name>
```

## Configuring build

**Compiler** and **CTest** flags can be selected in `actions.sh` script configuration. To do so, edit following lines at the top of the script:
```bash
compiler="g++"
test_flags="--rerun-failed --output-on-failure --timeout 60"
```

**Compilation flags** can be changed in `source/CMakeLists.txt`, find following lines and enter appropriate values if necessary:

```cmake
target_compile_features(run PRIVATE cxx_std_17)
target_compile_options(run PRIVATE -O2 -Wall -Wextra -Wpedantic -Werror)
```

## Building manually

Clone the repo:

```bash
git clone https://github.com/DmitriBogdanov/UTL.git &&
cd "UTL/"
```

Configure **CMake**:

```bash
cmake -D CMAKE_CXX_COMPILER=g++ -B "build/" -S .
```

Build the project:

```bash
bash "scripts/create_single_header.sh"
cmake --build "build/"
```

Run tests:

```bash
cd "build/tests/" &&
ctest --rerun-failed --output-on-failure --timeout 60 &&
cd ..
```

Run benchmark:

```bash
./build/benchmarks/<benchmark_name>
```
