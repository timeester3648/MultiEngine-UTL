include(CTest)
enable_testing()

# Macro for defining a test with proper doctest args
# Note: Adding '--force-colors' to 'add_test()' allows doctest coloring in terminal but messes up CTest logs formatting
macro(add_utl_test filename)
    add_executable(${filename} ${filename}.cpp)
    target_compile_features(${filename} PRIVATE cxx_std_17)
    target_compile_options(${filename} PRIVATE -O2 -Wall -Wextra -Wpedantic -Werror -fmax-errors=0)
    add_test(
        NAME ${filename}
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${filename} --no-intro --no-path-filenames --force-colors
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        # we want relative path to start at the top-level project directory '/',
        # by default CTest sets working directory to ${CMAKE_CURRENT_BINARY_DIR} which is '/build/tests/'
    )
endmacro()

# Tests
add_utl_test(test_json)
add_utl_test(test_log)
add_utl_test(test_math)
add_utl_test(test_mvl)
add_utl_test(test_random)
add_utl_test(test_stre)