#include <chrono>     

using milliseconds = std::chrono::milliseconds;
using system_clock = std::chrono::system_clock;
using time_point = std::chrono::time_point<system_clock>;
using duration = std::chrono::duration<double>;

time_point start, end; 