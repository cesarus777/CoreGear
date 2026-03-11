add_custom_command(
  OUTPUT hello-min-rv32i.x
  COMMAND riscv64-elf-gcc ARGS "${CMAKE_CURRENT_SOURCE_DIR}/hello-min.S" -march=rv32i -mabi=ilp32 -nostdlib -o hello-min-rv32i.x
  MAIN_DEPENDENCY hello-min.S
  COMMENT "Building rv32i minimal hello test"
)

add_custom_target(CoreGearE2ETests ALL DEPENDS hello-min-rv32i.x)

add_test(
  NAME TestRV32I_MinimalHello
  COMMAND cg_riscv_sim i "${CMAKE_CURRENT_BINARY_DIR}/hello-min-rv32i.x"
)

set_tests_properties(TestRV32I_MinimalHello PROPERTIES DEPENDS CoreGearE2ETests)
