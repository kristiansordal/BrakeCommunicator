cmake_minimum_required(VERSION 3.15)

project(mat-vec-mult
        VERSION 1.0
        DESCRIPTION "Matrix-Vector Multiplication"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED
             COMPONENTS
                 mpi
                 program_options
             REQUIRED)

add_executable(mat_vec_mult main.cpp)
target_include_directories(mat_vec_mult PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(mat_vec_mult PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(mat_vec_mult PRIVATE ${Boost_LIBRARIES})
target_compile_options(mat_vec_mult PRIVATE #[[ -Wall -Wextra -Wpedantic -Werror ]])

set(CMAKE_COLOR_DIAGNOSTICS On)
