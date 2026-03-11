add_library(riscv riscv.cpp)
target_sources(riscv
  PUBLIC
  FILE_SET CXX_MODULES
    FILES
      extensions.mpp
      regs.mpp
      riscv.mpp
)
target_include_directories(riscv PRIVATE ${CG_INCLUDE_DIR})
target_link_libraries(riscv utility common sim memory elfio)

add_executable(cg_riscv_sim main.cpp)
target_link_libraries(cg_riscv_sim riscv)

