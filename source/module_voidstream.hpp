// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::voidstream
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_voidstream.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_VOIDSTREAM)
#ifndef UTLHEADERGUARD_VOIDSTREAM
#define UTLHEADERGUARD_VOIDSTREAM

// _______________________ INCLUDES _______________________

#include <ostream>   // ostream
#include <streambuf> // streambuf

// ____________________ DEVELOPER DOCS ____________________

// TODO:
// Expand this module into a proper "streams" module. Being as small as it currently is
// it has rather little reason for existence. Combined with what is probably a rather
// unperformant way of discaring '::vout' inputs, it's quite justified for a rewrite.
// Streams that could be implemented:
//    - Multisink Stream (forwards inputs to multiple std::ostream's)
//    - Appending Stream (not sure if it's even implementable or useful)
//
// "voidstream" that functions like std::ostream with no output.
// Can be passed to interfaces that use streams to silence their output.
//
// # ::vstreambuf #
// Stream buffer that overflows with no output, usage example:
//   > std::ofstream output_stream(&vstreambuf);
//   > output_stream << VALUE; // produces nothing
//
// # ::vout #
// Output stream that produces no output, usage example:
//   > vout << VALUE; // produces nothing

// ____________________ IMPLEMENTATION ____________________

namespace utl::voidstream {

// ===================
// --- Void Stream ---
// ===================

class VoidStreamBuf : public std::streambuf {
public:
    inline int overflow(int c) { return c; }
};

class VoidStream : public std::ostream {
public:
    inline VoidStream() : std::ostream(&buffer) {}

private:
    VoidStreamBuf buffer;
};

inline VoidStreamBuf vstreambuf;
inline VoidStream    vout;

} // namespace utl::voidstream

#endif
#endif // module utl::voidstream
