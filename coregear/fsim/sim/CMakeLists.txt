add_library(sim)
target_sources(sim
  PUBLIC
  FILE_SET CXX_MODULES
    FILES
      sim.mpp
)
target_include_directories(sim PRIVATE ${CG_INCLUDE_DIR})
target_link_libraries(sim utility)
