
# Macro for defining a benchmark with proper compile options
macro(add_utl_bencmark filename)
    add_executable(${filename} ${filename}.cpp)
    target_compile_features(${filename} PRIVATE cxx_std_17)
    target_compile_options(${filename} PRIVATE -O2 -Wall -Wextra -Wpedantic -Werror -Wchar-subscripts -fmax-errors=1)
endmacro()

add_utl_bencmark(benchmark_json)
add_utl_bencmark(benchmark_log)
add_utl_bencmark(benchmark_mvl)
add_utl_bencmark(benchmark_parallel)
add_utl_bencmark(benchmark_profiler)

# Link OpenMP if doing benchmarks with it.
# Don't forget to add '-fopenmp' to 'target_compile_options' 
#find_package(OpenMP REQUIRED)
# target_link_libraries(benchmark_parallel PRIVATE OpenMP::OpenMP_CXX)