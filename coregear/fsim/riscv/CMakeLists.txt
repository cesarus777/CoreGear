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
target_link_libraries(riscv utility common sim memory elfio::elfio)

add_executable(cg_riscv_sim main.cpp)
target_link_libraries(cg_riscv_sim riscv)

if (CG_ENABLE_TESTING)
  add_executable(riscv_test riscv.test.cpp)

  target_include_directories(riscv_test PRIVATE ${CG_INCLUDE_DIR})
  target_link_libraries(riscv_test riscv GTest::gtest GTest::gtest_main)

  include(GoogleTest)
  gtest_discover_tests(riscv_test)
endif()

