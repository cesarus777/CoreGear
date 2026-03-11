cmake_minimum_required(VERSION 3.28)

# Activating UUID for 4.2.3 cmake
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_MODULE_STD ON)

project (CoreGear VERSION 0.1)

include(FetchContent)

FetchContent_Declare(
  elfio
  GIT_REPOSITORY https://github.com/serge1/ELFIO
  # It's recommended to use a specific tag or commit hash for stability
  GIT_TAG        Release_3.12
)

FetchContent_MakeAvailable(elfio)

set(CG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(coregear)

if (CG_ENABLE_TESTING)
  include(CTest)
  add_subdirectory(test)
endif()
