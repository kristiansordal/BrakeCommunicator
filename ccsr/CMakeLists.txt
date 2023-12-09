cmake_minimum_required(VERSION 3.15)

project(sparse_matrix
        VERSION 1.0
        DESCRIPTION "Compressed Sparse Row matrix multiplication"
        LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -ffast-math")

find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED COMPONENTS C)


file(GLOB TASK_A_SRC "task_a/*.c")
add_executable(csr ${TASK_A_SRC})

target_include_directories(csr PRIVATE ${MPI_INCLUDE_PATH})  # Include MPI headers

target_link_libraries(csr
    PRIVATE ${MPI_C_LIBRARIES} 
    PRIVATE OpenMP::OpenMP_C)
