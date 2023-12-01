# Prototyping utils

This is a collection of various utilities that aim to provide a set of concise "helpers" that allow prototying with minimal boilerplate. Most of the following modules were created during my work in gamedev and university projects.

## Design principles

* Library should be header only;
* Each module should be self-contained and reside in a separate namespace;
* Each module should toggleable through a define;
* Boilerplate code on user side should be minimal;
* Modern C++ idioms should be prefered both externally and internally;
* Header should contain a duplicate set of documentation;
* Usage examples should be provided for all major functions.

## Modules


## Requirements

* Requires C++17 support;
* Requires POSIX-compliant system or Windows.

## Version history

* 00.01
    * Uploaded initial draft with "voidstream", "argcv", "table", "timer", "sleep", "random" and "math" modules.

## License

This project is licensed under the MIT License - see the LICENSE.md file for details