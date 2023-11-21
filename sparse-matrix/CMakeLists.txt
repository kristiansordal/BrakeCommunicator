cmake_minimum_required(VERSION 3.15)

project(sparse_matrix
        VERSION 1.0
        DESCRIPTION "Sparse-Matrix multiplication for meshes"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math")

find_package(Boost REQUIRED
             COMPONENTS
                 mpi
                 program_options
             REQUIRED)

find_package(OpenMP REQUIRED)
find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

add_executable(par_a par_a/main.cpp)
add_executable(par_d par_d/main.cpp)
add_executable(visualizer visualizers/seq/main.cpp)
add_executable(p_visualizer1 visualizers/task_a/main.cpp)
# add_executable(p_visualizer2 visualizers/task_b/main.cpp)
add_executable(p_visualizer3 visualizers/task_d/main.cpp)

target_include_directories(par_a PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(par_a PRIVATE ${Boost_INCLUDE_DIRS} )#${SFML_INCLUDE_DIR})
target_link_libraries(par_a PRIVATE ${Boost_LIBRARIES} )#sfml-system sfml-window sfml-graphics)
target_compile_options(par_a PRIVATE )#-Wall -Wextra -Wpedantic -Werror)

target_include_directories(par_d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(par_d PRIVATE ${Boost_INCLUDE_DIRS} #[[ ${SFML_INCLUDE_DIR} ]])
target_link_libraries(par_d PRIVATE ${Boost_LIBRARIES} #[[ sfml-system sfml-window sfml-graphics ]])
target_compile_options(par_d PRIVATE )#-Wall -Wextra -Wpedantic -Werror)
# target_include_directories(sparse_matrix PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# target_include_directories(sparse_matrix PRIVATE ${Boost_INCLUDE_DIRS} ${SFML_INCLUDE_DIR})
# target_link_libraries(sparse_matrix PRIVATE ${Boost_LIBRARIES} sfml-system sfml-window sfml-graphics)
# target_compile_options(sparse_matrix PRIVATE )#-Wall -Wextra -Wpedantic -Werror)

target_include_directories(visualizer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(visualizer PRIVATE ${Boost_INCLUDE_DIRS} ${SFML_INCLUDE_DIR})
target_link_libraries(visualizer PRIVATE ${Boost_LIBRARIES} sfml-system sfml-window sfml-graphics)
target_compile_options(visualizer PRIVATE )#-Wall -Wextra -Wpedantic -Werror)


target_include_directories(p_visualizer1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(p_visualizer1 PRIVATE ${Boost_INCLUDE_DIRS} ${SFML_INCLUDE_DIR})
target_link_libraries(p_visualizer1 PRIVATE ${Boost_LIBRARIES} sfml-system sfml-window sfml-graphics)
target_compile_options(p_visualizer1 PRIVATE )#-Wall -Wextra -Wpedantic -Werror)


# target_include_directories(p_visualizer2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
# target_include_directories(p_visualizer2 PRIVATE ${Boost_INCLUDE_DIRS} ${SFML_INCLUDE_DIR})
# target_link_libraries(p_visualizer2 PRIVATE ${Boost_LIBRARIES} sfml-system sfml-window sfml-graphics)
# target_compile_options(p_visualizer2 PRIVATE )#-Wall -Wextra -Wpedantic -Werror)

target_include_directories(p_visualizer3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(p_visualizer3 PRIVATE ${Boost_INCLUDE_DIRS} ${SFML_INCLUDE_DIR})
target_link_libraries(p_visualizer3 PRIVATE ${Boost_LIBRARIES} sfml-system sfml-window sfml-graphics)
target_compile_options(p_visualizer3 PRIVATE )#-Wall -Wextra -Wpedantic -Werror)

set(CMAKE_COLOR_DIAGNOSTICS On)
