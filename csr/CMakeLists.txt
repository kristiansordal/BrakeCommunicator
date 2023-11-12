cmake_minimum_required(VERSION 3.15)

project(sparse_matrix
        VERSION 1.0
        DESCRIPTION "Compressed Sparse Row matrix multiplication"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")

find_package(Boost REQUIRED
             COMPONENTS
                 mpi
                 program_options
             REQUIRED)

find_package(OpenMP REQUIRED)
add_subdirectory(include/METIS)
# add_subdirectory(include/fast_matrix_market)

include(FetchContent)
FetchContent_Declare(
        fast_matrix_market
        GIT_REPOSITORY https://github.com/alugowski/fast_matrix_market
        GIT_TAG main
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(fast_matrix_market)

file(GLOB TASK_A_SRC "task_a/*.cpp")
add_executable(csr ${TASK_A_SRC})

target_include_directories(csr
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${Boost_INCLUDE_DIRS}
    PRIVATE ${METIS_INCLUDE_DIR})

target_link_libraries(csr
    PRIVATE ${Boost_LIBRARIES}
    PRIVATE ${METIS_LIBRARIES}
    PRIVATE fast_matrix_market::fast_matrix_market)
