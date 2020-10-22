
cmake_minimum_required (VERSION 3.8)
project(dragonfly)

#LLVM
if(${LLVM_DIR} MATCHES "LLVM_DIR-NOTFOUND" )
	message("Using LLVM_DIR From cmake/FindLLVM.cmake")
	include("${PROJECT_SOURCE_DIR}/cmake/FindLLVM.cmake")
endif(${LLVM_DIR} MATCHES "LLVM_DIR-NOTFOUND" )
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVM_DIR: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs support core irreader bitwriter)
message(STATUS "Using LLVM_Include: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Using LLVM_Lib: ${llvm_libs}")


#CUDA
find_package(CUDA 10.1 REQUIRED)
message(STATUS "Found CUDA ${CUDA_VERSION_STRING} at ${CUDA_TOOLKIT_ROOT_DIR}")

if (DEFINED ENV{CUDA_HOME})
  set(CUDA_HOME "$ENV{CUDA_HOME}")
  set(CUDA_HOME "$ENV{CUDA_HOME}" CACHE PATH "Path to CUDA Toolkit.")
elseif (NOT DEFINED CUDA_HOME)
  # There was no CUDA_HOME explicitly given, so try to determine it from
  # locating nvcc in PATH.
  find_program(NVCC nvcc)
  if (NOT NVCC)
    message(FATAL_ERROR "CUDA_HOME not set, and could not find nvcc in "
                        "your PATH.  Please either set CUDA_HOME to a "
                        "installation path for the CUDA Toolkit or "
                        "add nvcc to your PATH.")
  endif()
  get_filename_component(CUDA_HOME "${NVCC}" PATH)
  get_filename_component(CUDA_HOME "${NVVM_LIB}" PATH)
  get_filename_component(CUDA_HOME "${CUDA_HOME}/.." ABSOLUTE)
  set(CUDA_HOME "${CUDA_HOME}" CACHE PATH "Path to CUDA Toolkit.")
endif()
message(STATUS "Using CUDA_HOME: ${CUDA_HOME}")
 set(CUDA_LIB ${CUDA_HOME}/lib/x64/cuda.lib)
link_directories(${CUDA_HOME}/lib/x64)


# libNVVM
if(NOT DEFINED ENV{LIBNVVM_HOME})
  set(LIBNVVM_HOME "${CUDA_HOME}/nvvm" )
else()
  set(LIBNVVM_HOME "$ENV{LIBNVVM_HOME}" CACHE PATH "Path to NVVM.")
endif()
message(STATUS "Using LIBNVVM_HOME: ${LIBNVVM_HOME}")
# The directory structure for nvvm is a bit complex. 
# On Windows:
#   32-bit -- nvvm/lib/Win32
#   64-bit -- nvvm/lib/x64
# On Linux:
#   32-bit -- nvvm/lib
#   64-bit -- nvvm/lib64
# On Mac:
#   Universal -- nvvm/lib
if (CMAKE_SIZEOF_VOID_P STREQUAL "8")
  if (WIN32)
    set (LIB_ARCH_SUFFIX "/x64")
  elseif (APPLE)
    set (LIB_ARCH_SUFFIX "")
  else ()
    set (LIB_ARCH_SUFFIX "64")
  endif()
else()
  if (WIN32)
    set (LIB_ARCH_SUFFIX "/Win32")
  else()
    set (LIB_ARCH_SUFFIX "")
  endif()
endif()
find_library(NVVM_LIB nvvm PATHS "${LIBNVVM_HOME}/lib${LIB_ARCH_SUFFIX}")
find_file(NVVM_H nvvm.h PATHS "${LIBNVVM_HOME}/include")
if(NVVM_LIB AND NVVM_H)
  get_filename_component(NVVM_INCLUDE_DIR ${NVVM_H} PATH)
else()
  message(FATAL_ERROR "Unable to find libNVVM")
endif()
include_directories()
message(STATUS "Using libnvvm header path: ${NVVM_INCLUDE_DIR}")
message(STATUS "Using libnvvm library: ${NVVM_LIB}")




#MAIN
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(
	    ${NVVM_INCLUDE_DIR}
		${CUDA_HOME}/include

	    ${PROJECT_SOURCE_DIR}/src
	    ${PROJECT_SOURCE_DIR}/thirdparty/json
)
 
file(GLOB_RECURSE all_SRCS
	    "${PROJECT_SOURCE_DIR}/src/*.cpp" 
        )

add_executable (${PROJECT_NAME} ${all_SRCS})
message(STATUS "Using cuda library: ${CUDA_LIB}")
target_link_libraries(${PROJECT_NAME} ${llvm_libs} ${NVVM_LIB} ${CUDA_LIB} )

#install(TARGETS "${PROJECT_NAME}" DESTINATION bin)
include(CPack) 

enable_testing()
add_subdirectory(tests)