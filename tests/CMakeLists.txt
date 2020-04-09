cmake_minimum_required(VERSION 3.8)

set(SOURCES 
	test.cpp
)
set(gtest_SOURCE_DIR ${PROJECT_SOURCE_DIR}/thirdparty/googletest/googletest)
message(STATUS "Using googletest in: ${gtest_SOURCE_DIR}")
include_directories(${gtest_SOURCE_DIR}/include)
add_executable(dragonfly_test ${SOURCES})
target_link_libraries(dragonfly_test PUBLIC gtest_main)
add_test(NAME dragonfly_test COMMAND dragonfly_test)