cmake_minimum_required(VERSION 3.8)
set(gtest_SOURCE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/googletest/googletest")
message(STATUS "Using googletest in: ${gtest_SOURCE_DIR}")
set(SOURCES 
	test.cpp
)

#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/tests/bin)

include_directories(${gtest_SOURCE_DIR}/include)
include_directories(${gtest_SOURCE_DIR}/src)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
add_executable(dragonfly_test ${SOURCES})
target_link_libraries(dragonfly_test PUBLIC gtest_main ${llvm_libs})

add_test(NAME dragonfly_test COMMAND dragonfly_test )