
cmake_minimum_required (VERSION 3.8)
project(dragonfly)

if(${LLVM_DIR} MATCHES "LLVM_DIR-NOTFOUND" )
	message("Using LLVM_DIR From cmake/FindLLVM.cmake")
	include("${PROJECT_SOURCE_DIR}/cmake/FindLLVM.cmake")
endif(${LLVM_DIR} MATCHES "LLVM_DIR-NOTFOUND" )

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader bitwriter)


include_directories(

        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
)
 
file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/*.c"
        )

		 
add_executable (${PROJECT_NAME} ${all_SRCS})
target_link_libraries(${PROJECT_NAME} ${llvm_libs})

#install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
include(CPack)

enable_testing()
add_subdirectory(tests)