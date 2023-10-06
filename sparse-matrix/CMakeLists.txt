cmake_minimum_required(VERSION 3.15)

project(mat-vec-mult
        VERSION 1.0
        DESCRIPTION "Sparse-Matrix multiplication for meshes"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED
             COMPONENTS
                 mpi
                 program_options
             REQUIRED)


add_executable(sparse_matrix sparse.cpp)

target_include_directories(sparse_matrix PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(sparse_matrix PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(sparse_matrix PRIVATE ${Boost_LIBRARIES})
target_compile_options(sparse_matrix PRIVATE  -Wall -Wextra -Wpedantic -Werror )

set(CMAKE_COLOR_DIAGNOSTICS On)
