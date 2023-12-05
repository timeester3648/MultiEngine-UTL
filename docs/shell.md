

# utl::shell

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**shell** contains convenience functions for command line and system related operations.

## Definitions
```cpp
// Temporary file operations
std::string random_ascii_string(size_t length);

std::string generate_temp_file();
void clear_temp_files();
void erase_temp_file(const std::string &file);

// Argc/argv parsers
std::string get_exe_path(char** argv);
std::string_view get_exe_path_view(char** argv);

std::vector<std::string> get_command_line_args(int argc, char** argv);
std::vector<std::string_view> get_command_line_args_view(int argc, char** argv);

// Command line utils
struct CommandResult {
	int status;
	std::string stdout_output;
	std::string stderr_output;
};

CommandResult run_command(const std::string &command);
```

## Methods
> ```cpp
> std::string shell::random_ascii_string(size_t length);
> ```

Generates a string of random ASCII characters in ['a', 'z'] range. Uses [std::rand](https://en.cppreference.com/w/cpp/numeric/random/rand) internally.

> ```cpp
> std::string shell::generate_temp_file();
> ```

Generates a temporary file with random unique name in the current folder (using relative path). Returns filename.

Internally module keeps track of all created temporary files so they can be later deleted with `clear_temp_files()`.

> ```cpp
> shell::clear_temp_files();
> ```

Deletes all temporary files created with `generate_temp_file()` during current runtime.

> ```cpp
> shell::erase_temp_file(const std::string &file);
> ```

Deletes a single temporary file. Used to make methods that can clean up internal temporary files without affecting the global state.

> ```cpp
> std::string shell::get_exe_path(char** argv);
> std::string_view shell::get_exe_path_view(char** argv);
> ```

Parses program executable path from `argv` as `std::string` or `std::string_view`. Views have lower overhead, but keep pointers to original data.

`argc == 1` is a reasonable assumption since the only way to achieve such launch is to run executable through a null `execv()`, most command-line programs assume such scenario to be either impossible or an error on user side.

> ```cpp
> std::vector<std::string> shell::get_command_line_args(int argc, char** argv);
> std::vector<std::string_view> shell::get_command_line_args_view(int argc, char** argv);
> ```

Parses program command line arguments from `argv` as `std::string` or `std::string_view`. Views have lower overhead, but keep pointers to original data.

> ```cpp
> CommandResult shell::run_command(const std::string &command);
> ```

Runs console command using a default system shell (*cmd* for Windows, *bash* for Linux). Return *status* code, *stdout* and *stderr* (see [standard streams](https://en.wikipedia.org/wiki/Standard_streams)) piped from the process.

## Example 1 (creating temporary file)

[ [Run this code](GODBOLT_LINK) ]
```cpp
using namespace utl;

// Create temp. file and fill it with random text
const auto temp_file_path = shell::generate_temp_file();
const auto temp_file_text = "~~~" + shell::random_ascii_string(20) + "~~~";
std::ofstream(temp_file_path) << temp_file_text;

std::cout << "Temp. file path: " << temp_file_path << "\n";
std::cout << "Temp. file text: " << temp_file_text << "\n";

shell::clear_temp_files();
```

Output:
```
Temp. file path: gebknywycgeaospotmfjsskpcflasr.txt
Temp. file text: ~~~fjocozcfeyajpjrekjml~~~
```

## Example 2 (getting .exe path and args)

[ [Run this code](GODBOLT_LINK) ]
```cpp
using namespace utl;

std::cout << "Exe path:\n" << shell::get_exe_path(argv) << "\n\n";

std::cout << "Command line arguments (if present):\n";
for (const auto &arg : shell::get_command_line_args(argc, argv))
	std::cout << arg << "\n";
```

Output:
```
Exe path:
C:\GENERAL_FOLDER\Files\C++\proto_utils\Release\proto_utils.exe

Command line arguments (if present):

```

## Example 3 (running console command)

[ [Run this code](GODBOLT_LINK) ]
```cpp
using namespace utl;

// Create temp. file and fill it with text
const auto temp_file_path = shell::generate_temp_file();
const auto temp_file_text = "~~~FILE CONTENTS~~~";
std::ofstream(temp_file_path) << temp_file_text;

// Run command to show file contents (windows-specific command in this example)
const auto command = "type " + temp_file_path;
const auto command_result = shell::run_command(command);

std::cout
	<< "shell::run_command(" << command << "):\n"
	<< "command_result.status = " << command_result.status << "\n"
	<< "command_result.stdout_output = " << command_result.stdout_output << "\n"
	<< "command_result.stderr_output = " << command_result.stderr_output << "\n";

shell::clear_temp_files();
```

Output:
```
shell::run_command(type lvswemcevuvhoysbbvjqokhhbpudfx.txt):
command_result.status = 0
command_result.stdout_output = ~~~FILE CONTENTS~~~
command_result.stderr_output =
```