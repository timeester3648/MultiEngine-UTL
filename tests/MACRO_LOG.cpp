// // __________ TEST FRAMEWORK & LIBRARY  __________

// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
// #define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
// #include "thirdparty/doctest.h"

// #define UTL_PICK_MODULES
// #define UTLMACRO_LOG
// #include "proto_utils.hpp"

// // ________________ TEST INCLUDES ________________

// #include <sstream>

// // _____________ TEST IMPLEMENTATION _____________

// TEST_CASE("UTL_LOG() produces correct format") {
//     std::stringstream ss;
//     UTL_LOG_SET_OUTPUT(ss);
//     UTL_LOG("Block ", 17, ": responce in order. Proceeding with code ", 0, ".");
    
//     CHECK(ss.str().data() == doctest::Contains("Block 17: responce in order. Proceeding with code 0.\n"));
// }