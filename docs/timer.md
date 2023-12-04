# utl::timer

*utl::timer* contains a number of methods for time measurement. Intended mainly for measuring code execution time without [std::chrono](CPPREF_LINK) boilerplate. Outputs time as a string, prints formatted local time and date.

## Methods

'''cpp
void start() // start measurement

// Elapsed time as double
double elapsed_ms();
double elapsed_sec();
double elapsed_min();
double elapsed_hours();

// Elapsed time as string
std::string elapsed_string_ms();
std::string elapsed_string_sec();
std::string elapsed_string_min();
std::string elapsed_string_hours();

std::string elapsed_string_fullform(); // format "%H hours %M min %S sec %MS ms"

// Local date and time
std::string datetime_string();    // format "%y-%m-%d %H:%M:%S"
std::string datetime_string_id(); // format "%y-%m-%d-%H-%M-%S", works in filenames
'''

## Example (measuring time)

[ [Run this code](GODBOLT_LINK) ]
'''cpp
using namespace utl;

std::cout << "Current time is: " << timer::datetime_string() << "\n\n";

timer::start();
std::this_thread::sleep_for(std::chrono::milliseconds(3700));
std::cout << "Time elapsed:\n" << timer::elapsed_string_fullform() << "\n";
'''

Output:

'''

'''