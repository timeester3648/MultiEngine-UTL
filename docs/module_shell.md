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
std::string_view get_exe_path(char** argv);
std::vector<std::string_view> get_command_line_args(int argc, char** argv);

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

Deletes all temporary files created with `generate_temp_file()` during current runtime. **Called automatically when exiting the program.**

**Note:** "Exiting" happens when program returns from `main()` or calls `std::exit()`.

> ```cpp
> shell::erase_temp_file(const std::string &file);
> ```

Deletes a single temporary file. Used to make methods that can clean up internal temporary files without affecting the global state.

> ```cpp
> std::string_view shell::get_exe_path(char** argv);
> ```

Parses program executable path from `argv` as `std::string_view`.

`argc == 1` is a reasonable assumption since the only way to achieve such launch is to run executable through a null `execv()`, most command-line programs assume such scenario to be either impossible or an error on user side.

> ```cpp
> std::vector<std::string_view> shell::get_command_line_args(int argc, char** argv);
> ```

Parses program command line arguments from `argv` as `std::string_view`.

> ```cpp
> CommandResult shell::run_command(const std::string &command);
> ```

Runs console command using a default system shell (*cmd* for Windows, *bash* for Linux). Return *status* code, *stdout* and *stderr* (see [standard streams](https://en.wikipedia.org/wiki/Standard_streams)) piped from the process.

## Example 1 (creating temporary file)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:14,endLineNumber:16,positionColumn:14,positionLineNumber:16,selectionStartColumn:14,selectionStartLineNumber:16,startColumn:14,startLineNumber:16),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++//+Create+temp.+file+and+fill+it+with+random+text%0A++++const+auto+temp_file_path+%3D+shell::generate_temp_file()%3B%0A++++const+auto+temp_file_text+%3D+%22~~~%22+%2B+shell::random_ascii_string(20)+%2B+%22~~~%22%3B%0A++++std::ofstream(temp_file_path)+%3C%3C+temp_file_text%3B%0A%0A++++std::cout+%3C%3C+%22Temp.+file+path:+%22+%3C%3C+temp_file_path+%3C%3C+%22%5Cn%22%3B%0A++++std::cout+%3C%3C+%22Temp.+file+text:+%22+%3C%3C+temp_file_text+%3C%3C+%22%5Cn%22%3B%0A%0A++++shell::clear_temp_files()%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
using namespace utl;

// Create temp. file and fill it with random text
const auto temp_file_path = shell::generate_temp_file();
const auto temp_file_text = "~~~" + shell::random_ascii_string(20) + "~~~";
std::ofstream(temp_file_path) << temp_file_text;

std::cout << "Temp. file path: " << temp_file_path << "\n";
std::cout << "Temp. file text: " << temp_file_text << "\n";

// We can clear files manually, but otherwise they will be automatically cleared upon exit
// ('exit' is triggered by calling std::exit() or returning from main())
// shell::clear_temp_files();
```

Output:
```
Temp. file path: gebknywycgeaospotmfjsskpcflasr.txt
Temp. file text: ~~~fjocozcfeyajpjrekjml~~~
```

## Example 2 (getting .exe path and args)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:14,endLineNumber:12,positionColumn:14,positionLineNumber:12,selectionStartColumn:14,selectionStartLineNumber:12,startColumn:14,startLineNumber:12),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++std::cout+%3C%3C+%22Exe+path:%5Cn%22+%3C%3C+shell::get_exe_path(argv)+%3C%3C+%22%5Cn%5Cn%22%3B%0A%0A++++std::cout+%3C%3C+%22Command+line+arguments+(if+present):%5Cn%22%3B%0A++++for+(const+auto+%26arg+:+shell::get_command_line_args(argc,+argv))%0A++++++++std::cout+%3C%3C+arg+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:53,endLineNumber:9,positionColumn:53,positionLineNumber:9,selectionStartColumn:53,selectionStartLineNumber:9,startColumn:53,startLineNumber:9),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main(int+argc,+char+**argv)+%7B%0A++++using+namespace+utl%3B%0A%0A++++//+Create+temp.+file+and+fill+it+with+text%0A++++const+auto+temp_file_path+%3D+shell::generate_temp_file()%3B%0A++++const+auto+temp_file_text+%3D+%22~~~FILE+CONTENTS~~~%22%3B%0A++++std::ofstream(temp_file_path)+%3C%3C+temp_file_text%3B%0A%0A++++//+Run+command+to+show+file+contents+(does+not+work+in+godbolt)%0A++++const+auto+command+%3D+%22cat+%22+%2B+temp_file_path%3B%0A++++const+auto+command_result+%3D+shell::run_command(command)%3B%0A%0A++++std::cout%0A++++++++%3C%3C+%22shell::run_command(%22+%3C%3C+command+%3C%3C+%22):%5Cn%22%0A++++++++%3C%3C+%22command_result.status+%3D+%22+%3C%3C+command_result.status+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22command_result.stdout_output+%3D+%22+%3C%3C+command_result.stdout_output+%3C%3C+%22%5Cn%22%0A++++++++%3C%3C+%22command_result.stderr_output+%3D+%22+%3C%3C+command_result.stderr_output+%3C%3C+%22%5Cn%22%3B%0A%0A++++shell::clear_temp_files()%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ] (std::system might not work with Godbolt)
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
