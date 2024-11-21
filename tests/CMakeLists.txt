include(CTest)
enable_testing()

# Macro for defining a test with proper doctest args
# Note: Adding '--force-colors' to 'add_test()' allows doctest coloring in terminal but messes up CTest logs formatting
macro(add_utl_test filename)
    add_executable(${filename} ${filename}.cpp)
    add_test(NAME ${filename} COMMAND ${filename} --no-intro --no-path-filenames --force-colors)
endmacro()

# Tests
#add_utl_test(MACRO_LOG)
add_utl_test(module_math)
add_utl_test(module_mvl)
add_utl_test(module_random)
add_utl_test(module_stre)