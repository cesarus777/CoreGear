add_library(memory)
target_sources(memory
  PUBLIC
  FILE_SET CXX_MODULES
    FILES
      memory.mpp
)
target_include_directories(memory PRIVATE ${CG_INCLUDE_DIR})
target_link_libraries(memory utility common)
