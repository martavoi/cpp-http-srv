cmake_minimum_required(VERSION 3.28)

project(cpp-http-srv)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Boost configuration
find_package(Boost 1.84 REQUIRED COMPONENTS system log)

include_directories(${Boost_INCLUDE_DIRS})

# Diagnostic information
message(STATUS "=== Build Configuration Diagnostics ===")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Compiler Path: ${CMAKE_CXX_COMPILER}")

message(STATUS "CXX Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Binary Output Directory: ${CMAKE_BINARY_DIR}")
message(STATUS "Boost Version: ${Boost_VERSION}")
message(STATUS "Boost Include Dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost Libraries: ${Boost_LIBRARIES}")
message(STATUS "=======================================")

# Set output directory for binaries (modern CMake convention)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(cpp-http-srv main.cpp)

target_compile_features(cpp-http-srv PRIVATE cxx_std_23)
target_link_libraries(cpp-http-srv PRIVATE ${Boost_LIBRARIES})
