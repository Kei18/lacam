cmake_minimum_required(VERSION 3.16)
project(fast-mapf CXX)

add_subdirectory(./fast_mapf)
add_subdirectory(./third_party/argparse)
add_subdirectory(./third_party/googletest)

add_executable(main main.cpp)
target_compile_features(main PUBLIC cxx_std_17)
target_link_libraries(main fast_mapf argparse)

# test
set(TEST_MAIN_FUNC ./third_party/googletest/googletest/src/gtest_main.cc)
set(TEST_ALL_SRC ${TEST_MAIN_FUNC})
macro(add_test name target)
  add_executable(${name} ${target} ${TEST_MAIN_FUNC})
  target_link_libraries(${name} fast_mapf gtest)
  list(APPEND TEST_ALL_SRC ${target})
endmacro(add_test)

add_test(test_graph ./tests/test_graph.cpp)
add_test(test_instance ./tests/test_instance.cpp)
add_test(test_dist_table ./tests/test_dist_table.cpp)
add_test(test_planner ./tests/test_planner.cpp)

add_executable(test_all ${TEST_ALL_SRC})
target_link_libraries(test_all fast_mapf gtest)
