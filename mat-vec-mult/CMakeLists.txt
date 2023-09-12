cmake_minimum_required(VERSION 3.15)

project(cpp-mpi
        VERSION 1.0
        DESCRIPTION "Matrix Vector Multiplier"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED
             COMPONENTS
                 program_options
                 mpi
             REQUIRED)

add_executable(cpp_mpi main.cpp)
target_include_directories(cpp_mpi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(cpp_mpi PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(cpp_mpi PRIVATE ${Boost_LIBRARIES})
target_compile_options(cpp_mpi PRIVATE -Wall -Wextra -Wpedantic -Werror)

set(CMAKE_COLOR_DIAGNOSTICS On)
