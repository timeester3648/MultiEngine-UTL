// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::shell
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_shell.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_SHELL)
#ifndef UTLHEADERGUARD_SHELL
#define UTLHEADERGUARD_SHELL

// _______________________ INCLUDES _______________________

#include <cstddef>       // size_t
#include <cstdlib>       // atexit(), system()
#include <filesystem>    // filesystem::remove(), filesystem::path, filesystem::exists()
#include <fstream>       // ofstream, ifstream
#include <sstream>       // ostringstream
#include <string>        // string
#include <string_view>   // string_view
#include <unordered_set> // unordered_set<>
#include <vector>        // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Command line utils that allow simple creation of temporary files and command line
// calls with stdout and stderr piping (a task surprisingly untrivial in standard C++).
//
// # ::random_ascii_string() #
// Creates random ASCII string of given length.
// Uses chars in ['a', 'z'] range.
//
// # ::generate_temp_file() #
// Generates temporary .txt file with a random unique name, and returns it's filepath.
// Files generated during current runtime can be deleted with '::clear_temp_files()'.
// If '::clear_temp_files()' wasn't called manually, it gets called automatically upon exiting 'main()'.
// Uses relative path internally.
//
// # ::clear_temp_files() #
// Clears temporary files generated during current runtime.
//
// # ::erase_temp_file() #
// Clears a single temporary file with given filepath.
//
// # ::run_command() #
// Runs a command using the default system shell.
// Returns piped status (error code), stdout and stderr.
//
// # ::exe_path() #
// Parses executable path from argcv as std::string_view.
//
// # ::command_line_args() #
// Parses command line arguments from argcv as std::string_view.

// ____________________ IMPLEMENTATION ____________________

namespace utl::shell {

// =================================
// --- Temporary File Generation ---
// =================================

[[nodiscard]] inline std::string random_ascii_string(std::size_t length) {
    constexpr char min_char = 'a';
    constexpr char max_char = 'z';

    std::string result(length, '0');
    for (std::size_t i = 0; i < length; ++i)
        result[i] = static_cast<char>(min_char + rand() % (max_char - min_char + 1));
    return result;
}

inline std::unordered_set<std::string> _temp_files; // currently existing temp files
inline bool                            _temp_files_cleanup_registered = false;

inline void clear_temp_files() {
    for (const auto& file : _temp_files) std::filesystem::remove(file);
    _temp_files.clear();
}

inline void erase_temp_file(const std::string& file) {
    // we take 'file' as 'std::string&' instead of 'std::string_view' because it is
    // used to call '.erase()' on the map of 'std::string', which does not take string_view
    std::filesystem::remove(file);
    _temp_files.erase(file);
}

inline std::string generate_temp_file() {
    // No '[[nodiscard]]' since the function could still be used to generate files without
    // actually accessing them (through the returned path) in the same program.
    
    constexpr std::size_t MAX_ATTEMPTS = 500; // shouldn't realistically be encountered but still
    constexpr std::size_t NAME_LENGTH  = 30;

    // Register std::atexit() if not already registered
    if (!_temp_files_cleanup_registered) {
        const bool success             = (std::atexit(clear_temp_files) == 0);
        _temp_files_cleanup_registered = success;
    }

    // Try creating files until unique name is found
    for (std::size_t i = 0; i < MAX_ATTEMPTS; ++i) {
        const std::filesystem::path temp_path(random_ascii_string(NAME_LENGTH) + ".txt");

        if (std::filesystem::exists(temp_path)) continue;

        const std::ofstream temp_file(temp_path);

        if (temp_file.good()) {
            _temp_files.insert(temp_path.string());
            return temp_path.string();
        } else {
            return std::string();
        }
    }

    return std::string();
}

// ===================
// --- Shell Utils ---
// ===================

struct CommandResult {
    int         status; // aka error code
    std::string stdout_output;
    std::string stderr_output;
};

inline CommandResult run_command(const std::string& command) {
    // Note 1:
    // we take 'std::string&' instead of 'std::string_view' because there
    // has to be a guarantee that contained string is null-terminated

    // Note 2:
    // Creating temporary files doesn't seem to be ideal, but I'd yet to find
    // a way to pipe BOTH stdout and stderr directly into the program without
    // relying on platform-specific API like Unix forks and Windows processes

    // Note 3:
    // Usage of std::system() is often discouraged due to security reasons,
    // but it doesn't seem there is a portable way to do better (aka going
    // back to previous note about platform-specific APIs)

    const auto stdout_file = utl::shell::generate_temp_file();
    const auto stderr_file = utl::shell::generate_temp_file();

    // Redirect stdout and stderr of the command to temporary files
    std::ostringstream ss;
    ss << command.c_str() << " >" << stdout_file << " 2>" << stderr_file;
    const std::string modified_command = ss.str();

    // Call command
    const auto status = std::system(modified_command.c_str());

    // Read stdout and stderr from temp files and remove them
    std::ostringstream stdout_stream;
    std::ostringstream stderr_stream;
    stdout_stream << std::ifstream(stdout_file).rdbuf();
    stderr_stream << std::ifstream(stderr_file).rdbuf();
    utl::shell::erase_temp_file(stdout_file);
    utl::shell::erase_temp_file(stderr_file);

    // Return
    CommandResult result = {status, stdout_stream.str(), stderr_stream.str()};

    return result;
}

// =========================
// --- Argc/Argv parsing ---
// =========================

// This is just "C to C++ string conversion" for argc/argv
//
// Perhaps it could be expanded to proper parsing of standard "CLI options" format
// (like ordered/unordered flags prefixed with '--', shortcuts prefixed with '-' and etc.)

[[nodiscard]] inline std::string_view get_exe_path(char** argv) {
    // argc == 1 is a reasonable assumption since the only way to achieve such launch
    // is to run executable through a null-execv, most command-line programs assume
    // such scenario to be either impossible or an error on user side
    return std::string_view(argv[0]);
}

[[nodiscard]] inline std::vector<std::string_view> get_command_line_args(int argc, char** argv) {
    std::vector<std::string_view> arguments(argc - 1);
    for (std::size_t i = 0; i < arguments.size(); ++i) arguments.emplace_back(argv[i]);
    return arguments;
}

} // namespace utl::shell

#endif
#endif // module utl::shell
