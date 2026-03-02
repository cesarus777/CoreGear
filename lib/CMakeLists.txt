set(SIM_SOURCES riscv_sim.cpp extensions.cpp)
add_library(riscv_sim ${SIM_SOURCES})

target_include_directories(riscv_sim PRIVATE ${PUB_INCLUDE_DIR})
