include(CTest)
enable_testing()

# Macro for defining a test with proper doctest args
# NOTE: Adding '--force-colors' to 'add_test()' allows doctest coloring in terminal but messe up CTest logs formatting
macro(doctest modulename)
    add_executable(${modulename} ${modulename}.cpp)
    add_test(NAME ${modulename} COMMAND ${modulename} --no-intro --no-path-filenames --force-colors)
endmacro()

# Tests
doctest(module_storage)
doctest(module_stre)
doctest(module_random)
doctest(MACRO_LOG)