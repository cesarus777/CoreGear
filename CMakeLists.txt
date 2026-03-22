cmake_minimum_required(VERSION 4.2.3)

# Activating UUID for 4.2.3 cmake
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_MODULE_STD ON)

if(CLANG_STDLIB_MODULES_JSON_WA)
  set(CMAKE_CXX_STDLIB_MODULES_JSON "/usr/lib/llvm-21/lib/libc++.modules.json")
  message(STATUS "clang libc++.modules.json WA enabled")
endif()

project (CoreGear VERSION 0.1)

find_package(elfio REQUIRED)

set(CG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if (CG_ENABLE_TESTING)
  find_package(GTest REQUIRED)
  include(CTest)
  add_subdirectory(test)
endif()

add_subdirectory(coregear)
