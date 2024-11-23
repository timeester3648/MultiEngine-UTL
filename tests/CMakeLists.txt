include(CTest)
enable_testing()

# Macro for defining a test with proper doctest args
# Note: Adding '--force-colors' to 'add_test()' allows doctest coloring in terminal but messes up CTest logs formatting
macro(add_utl_test filename)
    add_executable(${filename} ${filename}.cpp)
    add_test(
        NAME ${filename}
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${filename} --no-intro --no-path-filenames --force-colors
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        # we want relative path to start at the top-level project directory '/',
        # by default CTest sets working directory to ${CMAKE_CURRENT_BINARY_DIR} which is '/build/tests/'
    )
endmacro()

# Tests
#add_utl_test(MACRO_LOG)
add_utl_test(module_json)
add_utl_test(module_math)
add_utl_test(module_mvl)
add_utl_test(module_random)
add_utl_test(module_stre)