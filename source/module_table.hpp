// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::table
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_table.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_TABLE)
#ifndef UTLHEADERGUARD_TABLE
#define UTLHEADERGUARD_TABLE

// _______________________ INCLUDES _______________________

#include <cstddef>          // size_t
#include <initializer_list> // initializer_list<>
#include <iomanip>          // resetiosflags(), setw()
#include <ios>              // streamsize, ios_base::fmtflags, ios
#include <iostream>         // cout
#include <ostream>          // ostream
#include <string>           // string
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Functions used to build and render simple ASCII table in console.
//
// # ::create() #
// Sets up table with given number of columns and their widths.
//
// # ::set_formats() #
// (optional) Sets up column formats for better display
//
// # ::set_ostream() #
// (optional) Select 'std::ostream' to which all output gets forwarded. By default 'std::cout' is selected.
//
// # ::NONE, ::FIXED(), ::DEFAULT(), ::SCIENTIFIC(), ::BOOL() #
// Format flags with following effects:
// > NONE          - Use default C++ formats
// > FIXED(N)      - Display floats in fixed form with N decimals, no argument assumes N = 3
// > DEFAULT(N)    - Display floats in default form with N decimals, no argument assumes N = 6
// > SCIENTIFIC(N) - Display floats in scientific form with N decimals, no argument assumes N = 3
// > BOOL          - Display booleans as text
//
// # ::cell() #
// Draws a single table cell, if multiple arguments are passed, draws each one in a new cell.
// Accepts any type with a defined "<<" ostream operator.
//
// # ::hline() #
// Draws horizontal line in a table. Similar to LaTeX '\hline'.

// ____________________ IMPLEMENTATION ____________________

namespace utl::table {

// =====================
// --- Column Format ---
// =====================

using uint       = std::streamsize;
using _ios_flags = std::ios_base::fmtflags;

struct ColumnFormat {
    _ios_flags flags;
    uint       precision;
};

struct _Column {
    uint         width;
    ColumnFormat col_format;
};

// --- Predefined Formats ---
// --------------------------

constexpr ColumnFormat NONE = {std::ios::showpoint, 6};

constexpr ColumnFormat FIXED(uint decimals = 3) { return {std::ios::fixed, decimals}; }
constexpr ColumnFormat DEFAULT(uint decimals = 6) { return {std::ios::showpoint, decimals}; }
constexpr ColumnFormat SCIENTIFIC(uint decimals = 3) { return {std::ios::scientific, decimals}; }

constexpr ColumnFormat BOOL = {std::ios::boolalpha, 3};

// --- Internal Table State ---
// ----------------------------

inline std::vector<_Column> _columns;
inline std::size_t          _current_column = 0;
inline std::ostream*        _output_stream  = &std::cout;

// ===================
// --- Table Setup ---
// ===================

inline void create(std::initializer_list<uint>&& widths) {
    _columns.resize(widths.size());
    for (std::size_t i = 0; i < _columns.size(); ++i) {
        _columns[i].width      = widths.begin()[i];
        _columns[i].col_format = DEFAULT();
    }
}

inline void set_formats(std::initializer_list<ColumnFormat>&& formats) {
    for (std::size_t i = 0; i < _columns.size(); ++i) _columns[i].col_format = formats.begin()[i];
}

inline void set_ostream(std::ostream& new_ostream) { _output_stream = &new_ostream; }

// =======================
// --- Table Rendering ---
// =======================

inline void cell(){};

template <typename T, typename... Types>
void cell(T value, const Types... other_values) {
    const std::string left_cline      = (_current_column == 0) ? "|" : "";
    const std::string right_cline     = (_current_column == _columns.size() - 1) ? "|\n" : "|";
    const _ios_flags  format          = _columns[_current_column].col_format.flags;
    const uint        float_precision = _columns[_current_column].col_format.precision;

    // Save old stream state
    std::ios old_state(nullptr);
    old_state.copyfmt(*_output_stream);

    // Set table formatting
    (*_output_stream) << std::resetiosflags((*_output_stream).flags());
    (*_output_stream).flags(format);
    (*_output_stream).precision(float_precision);

    // Print
    (*_output_stream) << left_cline << std::setw(_columns[_current_column].width) << value << right_cline;

    // Return old stream state
    (*_output_stream).copyfmt(old_state);

    // Advance column counter
    _current_column = (_current_column == _columns.size() - 1) ? 0 : _current_column + 1;

    cell(other_values...);
}

inline void hline() {
    (*_output_stream) << "|";
    for (const auto& col : _columns) (*_output_stream) << std::string(static_cast<std::size_t>(col.width), '-') << "|";
    (*_output_stream) << "\n";
}

} // namespace utl::table

#endif
#endif // module utl::table
