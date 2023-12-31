cmake_minimum_required(VERSION 3.15)

project(sparse_matrix
        VERSION 1.0
        DESCRIPTION "Compressed Sparse Row matrix multiplication"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Boost REQUIRED
             COMPONENTS
                 mpi
                 program_options
             REQUIRED)

find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)
find_package(METIS REQUIRED)

include(FetchContent)
FetchContent_Declare(
        fast_matrix_market
        GIT_REPOSITORY https://github.com/alugowski/fast_matrix_market
        GIT_TAG main
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(fast_matrix_market)

file(GLOB TASK_A_SRC "task_a/*.cpp")
file(GLOB TASK_C_SRC "task_c/*.cpp")
add_executable(csr ${TASK_A_SRC})
add_executable(test ${TASK_C_SRC})


target_include_directories(csr
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(csr
    PRIVATE ${Boost_LIBRARIES}
    PRIVATE OpenMP::OpenMP_CXX
    PRIVATE fast_matrix_market::fast_matrix_market)

include_directories(SYSTEM ${MPI_INCLUDE_PATH})

target_include_directories(test
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_link_libraries(test
    PRIVATE OpenMP::OpenMP_CXX
    PRIVATE ${MPI_CXX_LIBRARIES}
    PRIVATE fast_matrix_market::fast_matrix_market)
