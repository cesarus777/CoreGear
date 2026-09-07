cmake_minimum_required(VERSION 4.2.3)

if(CMAKE_VERSION VERSION_EQUAL "4.2.3")
  # Activating UUID for 4.2.3 cmake
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
elseif(CMAKE_VERSION VERSION_GREATER_EQUAL "4.3")
  # Activating UUID for 4.3.* cmake
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_MODULE_STD ON)

if(DEFINED CG_CXX_STDLIB_MODULES_JSON AND NOT "${CG_CXX_STDLIB_MODULES_JSON}"
                                          STREQUAL "")
  set(CMAKE_CXX_STDLIB_MODULES_JSON "${CG_CXX_STDLIB_MODULES_JSON}")
  message(STATUS "Using configured libc++.modules.json")
endif()

if(DEFINED CG_CXX_STDLIB_INCLUDE_DIR AND NOT "${CG_CXX_STDLIB_INCLUDE_DIR}"
                                         STREQUAL "")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -nostdinc++ -isystem${CG_CXX_STDLIB_INCLUDE_DIR}")
  message(STATUS "Using configured libc++ headers")
endif()

if(DEFINED CG_CXX_STDLIB_LIBRARY_DIR AND NOT "${CG_CXX_STDLIB_LIBRARY_DIR}"
                                         STREQUAL "")
  string(APPEND CMAKE_CXX_STANDARD_LIBRARIES
         " -L${CG_CXX_STDLIB_LIBRARY_DIR} -Wl,-rpath,"
         "${CG_CXX_STDLIB_LIBRARY_DIR} -lc++ -lc++abi")
  message(STATUS "Using configured libc++ libraries")
endif()

project(CoreGear VERSION 0.1)

find_package(elfio REQUIRED)

set(CG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

if(CG_ENABLE_TESTING)
  find_package(GTest REQUIRED)
  include(CTest)
  add_subdirectory(test)
endif()

add_subdirectory(coregear)
