cmake_minimum_required(VERSION 3.4.3)
project(llvm-explore)

set(CMAKE_C_COMPILER "/usr/bin/clang-9")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-9")

set(LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")

list(APPEND CMAKE_PREFIX_PATH "${LLVM_INSTALL_DIR}/lib/cmake/llvm/")

find_package(LLVM 11.0.0 REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 14 CACHE STRING "")

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

add_library(RedundantLoadPass SHARED RedundantLoadPass.cpp)
add_library(MemoryTransferPass SHARED MemoryTransferPass.cpp)
add_library(UnusedStorePass SHARED UnusedStorePass.cpp)
add_library(PrimitiveBranchPass SHARED PrimitiveBranchPass.cpp)
