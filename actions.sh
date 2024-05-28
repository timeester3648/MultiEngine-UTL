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

path_executable="${directory_build}source/run"

compiler="g++"
test_flags="--rerun-failed --output-on-failure --timeout 60"

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
    cmake --build $directory_build
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
    
    if [ $valid_command = false ]; then
        echo "# Error: Invalid action name -> ${var}"
        break
    fi

done