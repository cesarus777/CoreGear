add_executable(coregear coregear.cpp)
target_link_libraries(coregear riscv_sim)
target_include_directories(coregear PRIVATE ${PUB_INCLUDE_DIR})
