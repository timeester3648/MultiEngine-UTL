# Building tests & examples

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

This project uses [CMake](https://cmake.org) build system.

Unit testing for modules is implemented using [doctest](https://github.com/doctest/doctest) together with **CMake** testing facilities (see [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html)). All tests can be found in [`tests/`](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/tests) split by per-module basis.

A rundown of usage examples can be found at [`source/examples.cpp`](https://github.com/DmitriBogdanov/prototyping_utils/blob/master/source/examples.cpp), by default it's compiled with **g++** `-Wall -Wextra -Wpedantic -Werror` to ensure stricter error checking.

## Building with a script

Clone the repo:

```bash
git clone https://github.com/DmitriBogdanov/prototyping_utils.git &&
cd "prototyping_utils/"
```

Build the project:

```bash
bash actions.sh clear config build
```

Run all tests:

```bash
bash actions.sh test
```

Run examples:

```bash
bash actions.sh run
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
git clone https://github.com/DmitriBogdanov/prototyping_utils.git &&
cd "prototyping_utils/"
```

Configure **CMake**:

```bash
cmake -D CMAKE_CXX_COMPILER=g++ -B "build/" -S .
```

Build the project:

```bash
cmake --build "build/"
```

Run target (`examples.cpp`):

```bash
./build/source/run
```

Run tests:

```bash
cd "build/tests/" &&
ctest --rerun-failed --output-on-failure --timeout 60 &&
cd ..
```

