# ---------------------------------- CONTENTS -----------------------------------
#   
#   This script contains shortcuts for building, running
#   and testing the project. All action keywords can be
#   chained which causes them to be executed one after another.
#
#   See "docs/guide_building_project.md" for the whole building guide.
#   
# ------------------------------------ GUIDE ------------------------------------
#   
#   Usage format:
#     > bash actions.sh [ACTIONS]
#   
#   Actions:
#     clear  - Clears "build/" folder
#     config - Configures CMake with appropriate args
#     build  - Builds the project (requires configured CMake)
#     run    - Runs main executable (requires successful build)
#     test   - Runs CTest tests
#   
#   Usage example:
#     > bash actions.sh clear config build run
#   
# -------------------------------------------------------------------------------

# -----------------------
# ---- Configuration ----
# -----------------------
directory_source="source/"
directory_build="build/"
directory_tests="${directory_build}tests/"

path_executable="${directory_build}benchmarks/benchmark_json"

compiler="g++" # clang++-11
test_flags="--rerun-failed --output-on-failure --timeout 60"
build_jobs="6"
header_merger_script="scripts/create_single_header.sh"

# -----------------------
# ------ Functions ------
# -----------------------
check_command_exists() {
    if ! command -v $1 &> /dev/null
    then
        echo "Command [ $1 ] could not be found."
        exit 1
    fi
}

clear_files() {
    if [ -d "$directory_build" ]; then
        rm --recursive $directory_build
        echo "Cleared directory [ $directory_build ]."
    else
        echo "Directory [ $directory_build ] is clear."
    fi
}

cmake_config() {
    check_command_exists "cmake"
    check_command_exists "$compiler"
    cmake -D CMAKE_CXX_COMPILER=$compiler -B $directory_build -S .
}

cmake_build() {
    check_command_exists "cmake"
    cmake --build $directory_build --parallel $build_jobs
}

cmake_test() {
    check_command_exists "ctest"
    cd $directory_tests
    ctest $test_flags
    cd ..
}

executable_run() {
    ./$path_executable
}

executable_profile() {
    check_command_exists "valgrind"
    check_command_exists "callgrind_annotate"
    check_command_exists "kcachegrind"
    valgrind --tool=callgrind --dump-line=yes --callgrind-out-file="${directory_temp}callgrind.latest" ./$path_executable
    callgrind_annotate --auto=yes --include="source/" "${directory_temp}callgrind.latest" > "${directory_temp}callgrind.annotate.txt"
    kcachegrind "./${directory_temp}callgrind.latest"
}

# -----------------------
# --- Action selector ---
# -----------------------
valid_command=false

for var in "$@"
do
    valid_command=false
    
    if [ "$var" = "clear" ]; then
        echo "# Action: Clear Files"
        clear_files
        valid_command=true
    fi

    if [ "$var" = "config" ]; then
        echo "# Action: CMake Configure"
        cmake_config
        valid_command=true
    fi

    if [ "$var" = "build" ]; then
        echo "# Action: CMake Build"
        if [ -f "$header_merger_script" ]; then
            echo "Merging single header include..."
            bash "$header_merger_script"
            echo "Merge complete."
        else
            echo "# Error: Could not find \"$header_merger_script\"."
        fi
        cmake_build
        valid_command=true
    fi
    
    if [ "$var" = "test" ]; then
        echo "# Action: CMake Test"
        cmake_test
        valid_command=true
    fi

    if [ "$var" = "run" ]; then
        echo "# Action: run"
        executable_run
        valid_command=true
    fi
    
    if [ "$var" = "profile" ]; then
        echo "# Action: profile"
        executable_profile
        valid_command=true
    fi
    
    if [ $valid_command = false ]; then
        echo "# Error: Invalid action name -> ${var}"
        break
    fi

done