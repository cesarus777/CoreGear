set(SIM_SOURCES sim.cpp extensions.cpp)
add_library(sim ${SIM_SOURCES})

target_include_directories(sim PRIVATE ${PUB_INCLUDE_DIR})
