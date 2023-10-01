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
find_package(OpenMP REQUIRED)

add_executable(row_mult rowmult.cpp)
add_executable(col_mult colmult.cpp)
add_executable(grid_mult gridmult.cpp)

add_executable(row_mult_total rowmult_total.cpp)
add_executable(col_mult_total colmult_total.cpp)
add_executable(grid_mult_total gridmult_total.cpp)

add_executable(row_mult_pragma rowmult_pragma.cpp)
add_executable(col_mult_pragma colmult_pragma.cpp)
add_executable(grid_mult_pragma gridmult_pragma.cpp)

target_include_directories(grid_mult PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(grid_mult PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(grid_mult PRIVATE ${Boost_LIBRARIES})
target_compile_options(grid_mult PRIVATE  -Wall -Wextra -Wpedantic -Werror )

target_include_directories(row_mult PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(row_mult PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(row_mult PRIVATE ${Boost_LIBRARIES})
target_compile_options(row_mult PRIVATE  -Wall -Wextra -Wpedantic -Werror )

target_include_directories(col_mult PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(col_mult PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(col_mult PRIVATE ${Boost_LIBRARIES})
target_compile_options(col_mult PRIVATE  -Wall -Wextra -Wpedantic -Werror )


target_include_directories(grid_mult_total PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(grid_mult_total PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(grid_mult_total PRIVATE ${Boost_LIBRARIES})
target_compile_options(grid_mult_total PRIVATE  -Wall -Wextra -Wpedantic -Werror )

target_include_directories(row_mult_total PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(row_mult_total PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(row_mult_total PRIVATE ${Boost_LIBRARIES})
target_compile_options(row_mult_total PRIVATE  -Wall -Wextra -Wpedantic -Werror )

target_include_directories(col_mult_total PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(col_mult_total PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(col_mult_total PRIVATE ${Boost_LIBRARIES})
target_compile_options(col_mult_total PRIVATE  -Wall -Wextra -Wpedantic -Werror )

target_include_directories(grid_mult_pragma PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(grid_mult_pragma PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(grid_mult_pragma PRIVATE ${Boost_LIBRARIES})
target_compile_options(grid_mult_pragma PRIVATE  -Wall -Wextra -Wpedantic -Werror )
target_link_libraries(grid_mult_pragma PRIVATE OpenMP::OpenMP_CXX)

target_include_directories(row_mult_pragma PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(row_mult_pragma PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(row_mult_pragma PRIVATE ${Boost_LIBRARIES})
target_compile_options(row_mult_pragma PRIVATE  -Wall -Wextra -Wpedantic -Werror )
target_link_libraries(row_mult_pragma PRIVATE OpenMP::OpenMP_CXX)

target_include_directories(col_mult_pragma PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(col_mult_pragma PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(col_mult_pragma PRIVATE ${Boost_LIBRARIES})
target_compile_options(col_mult_pragma PRIVATE  -Wall -Wextra -Wpedantic -Werror )
target_link_libraries(col_mult_pragma PRIVATE OpenMP::OpenMP_CXX)
set(CMAKE_COLOR_DIAGNOSTICS On)
