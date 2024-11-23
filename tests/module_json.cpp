// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "module_json.hpp"

// _______________________ INCLUDES _______________________

#include <filesystem>

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

using namespace utl;
namespace fs = std::filesystem;

TEST_CASE("JSON validation test suite (accept)") {
    const fs::path test_suite_path = "tests/data/json_test_suite/should_accept/";
    
    std::cout << "Running test case from path: " << fs::current_path() << '\n' << std::flush;

    for (const auto& test_suite_entry : fs::directory_iterator(test_suite_path)) {
        int status = 0;
        
        std::cout << "Parsing file named: " << test_suite_entry.path() << '\n' << std::flush;

        try {
            json::from_file(test_suite_entry.path());
        } catch (...) { status = 1; }

        CHECK(status == 0);
    }
}

TEST_CASE("JSON validation test suite (reject)") {
    const fs::path test_suite_path = "tests/data/json_test_suite/should_reject/";
    
    std::cout << "Running test case from path: " << fs::current_path() << '\n' << std::flush;
    
    for (const auto& test_suite_entry : fs::directory_iterator(test_suite_path)) {
        int status = 0;
        
        std::cout << "Parsing file named: " << test_suite_entry.path() << '\n' << std::flush;

        try {
            json::from_file(test_suite_entry.path());
        } catch (...) { status = 1; }

        CHECK(status == 1);
    }
}
