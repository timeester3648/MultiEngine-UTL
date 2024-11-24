
# Macro for defining a benchmark with proper compile options
macro(add_utl_bencmark filename)
    add_executable(${filename} ${filename}.cpp)
    target_compile_features(${filename} PRIVATE cxx_std_17)
    target_compile_options(${filename} PRIVATE -O3 -Wall -Wextra -Wpedantic -Werror -fmax-errors=1)
endmacro()

add_utl_bencmark(benchmark_json)
add_utl_bencmark(benchmark_log)
add_utl_bencmark(benchmark_profiler)