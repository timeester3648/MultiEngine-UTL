include(CTest)
enable_testing()

# Macro for defining a test with proper doctest args
macro(doctest modulename)
    add_executable(${modulename} ${modulename}.cpp)
    add_test(NAME ${modulename} COMMAND ${modulename} --no-intro --no-path-filenames)
endmacro()

# Tests
doctest(module_stre)
doctest(module_random)
doctest(MACRO_LOG)