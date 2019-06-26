
cmake_minimum_required(VERSION 3.5)
project(comz)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

add_subdirectory(external/coreutils)
add_subdirectory(external/sqlite3)
add_subdirectory(external/linenoise-ng)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE external/asio/asio/include)

set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "")
set(FLATBUFFERS_INSTALL OFF CACHE BOOL "")
add_subdirectory(external/flatbuffers)

set(FBS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/comz.fbs)
set(FLATBUF_BIN ${CMAKE_CURRENT_BINARY_DIR}/external/flatbuffers)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/comz_generated.h
                   COMMAND ${FLATBUF_BIN}/flatc
                           --cpp -o ${CMAKE_CURRENT_BINARY_DIR} ${FBS_FILE}
                   DEPENDS ${FBS_FILE}
                   DEPENDS
                     ${CMAKE_CURRENT_BINARY_DIR}/external/flatbuffers/flatc
                   COMMENT "Running flatc")

add_custom_target(comz_header
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/comz_generated.h)

add_library(comz_fbs INTERFACE)
target_include_directories(comz_fbs INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(comz_fbs comz_header)

add_executable(comz src/comz.cpp src/messageboard.cpp)
target_link_libraries(
  comz PRIVATE comz_fbs coreutils sqlite3 flatbuffers asio pthread)

add_executable(client src/client.cpp)
target_link_libraries(
  client PRIVATE comz_fbs coreutils linenoise flatbuffers asio pthread)
