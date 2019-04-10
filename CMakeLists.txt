
cmake_minimum_required(VERSION 3.5)
project(comz)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

add_executable(comz comz.cpp)
target_link_libraries(comz PRIVATE pthread)
target_include_directories(comz PRIVATE external/asio/asio/include)
