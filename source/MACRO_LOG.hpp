// // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// //
// // Macro-Module:  UTL_LOG
// // Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACRO_LOG.md
// // Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
// //
// // This project is licensed under the MIT License
// //
// // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// #if !defined(UTL_PICK_MODULES) || defined(UTLMACRO_LOG)
// #ifndef UTLHEADERGUARD_LOG
// #define UTLHEADERGUARD_LOG

// // _______________________ INCLUDES _______________________

// #include <iostream>    // cout
// #include <ostream>     // ostream
// #include <string_view> // string_view

// // ____________________ DEVELOPER DOCS ____________________

// // A very minimalistic logger with source file info. Mainly used for debugging.
// // NOTE: Perhaps can be expanded to something faster and more production-usable.
// //
// // # UTL_LOG_SET_OUTPUT() #
// // Select ostream used by LOG macros.
// //
// // # UTL_LOG(), UTL_LOG_DEBUG() #
// // Print message to selected ostream prefixed with [<filename>:<line> (<function>)].
// // Accepts multiple args (with defined operator <<) that get concatenated into a single message.
// // DEBUG version compiles to nothing in release.

// // ____________________ IMPLEMENTATION ____________________

// // ======================
// // --- Logging Macros ---
// // ======================

// inline std::ostream* _utl_log_ostream = &std::cout;

// #define UTL_LOG_SET_OUTPUT(new_stream_) _utl_log_ostream = &new_stream_;

// template <typename... Args>
// inline void _utl_log_print(std::string_view file, int line, std::string_view func, const Args&... args) {
//     const std::string_view filename = file.substr(file.find_last_of("/\\") + 1);

//     ///(*_utl_log_ostream) << "\033[31;1m"; // Supported by Linux and Windows10+, but prints to files, figure out a fix
//     (*_utl_log_ostream) << "[" << filename << ":" << line << ", " << func << "()]";
//     ///(*_utl_log_ostream) << "\033[0m";

//     (*_utl_log_ostream) << " ";
//     ((*_utl_log_ostream) << ... << args);
//     (*_utl_log_ostream) << '\n';
// }

// #define UTL_LOG(...) _utl_log_print(__FILE__, __LINE__, __func__, __VA_ARGS__)

// #ifdef _DEBUG
// #define UTL_LOG_DEBUG(...) _utl_log_print(__FILE__, __LINE__, __func__, __VA_ARGS__)
// #else
// #define UTL_LOG_DEBUG(...)
// #endif

// #endif
// #endif // macro-module UTL_LOG
