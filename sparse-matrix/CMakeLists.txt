cmake_minimum_required(VERSION 3.15)

project(sparse_matrix
        VERSION 1.0
        DESCRIPTION "Sparse-Matrix multiplication for meshes"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED
             COMPONENTS
                 mpi
                 program_options
             REQUIRED)

# Find SFML
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

add_executable(sparse_matrix main.cpp)
add_executable(visualizer visualizers/seq/main.cpp)
add_executable(p_visualizer1 visualizers/par_a/main.cpp)

target_include_directories(sparse_matrix PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(sparse_matrix PRIVATE ${Boost_INCLUDE_DIRS} ${SFML_INCLUDE_DIR})
target_link_libraries(sparse_matrix PRIVATE ${Boost_LIBRARIES} sfml-system sfml-window sfml-graphics)
target_compile_options(sparse_matrix PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_include_directories(visualizer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(visualizer PRIVATE ${Boost_INCLUDE_DIRS} ${SFML_INCLUDE_DIR})
target_link_libraries(visualizer PRIVATE ${Boost_LIBRARIES} sfml-system sfml-window sfml-graphics)
target_compile_options(visualizer PRIVATE -Wall -Wextra -Wpedantic -Werror)


target_include_directories(p_visualizer1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(p_visualizer1 PRIVATE ${Boost_INCLUDE_DIRS} ${SFML_INCLUDE_DIR})
target_link_libraries(p_visualizer1 PRIVATE ${Boost_LIBRARIES} sfml-system sfml-window sfml-graphics)
target_compile_options(p_visualizer1 PRIVATE -Wall -Wextra -Wpedantic -Werror)
set(CMAKE_COLOR_DIAGNOSTICS On)
