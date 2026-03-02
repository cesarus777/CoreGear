add_executable(prs prs.cpp)
target_link_libraries(prs riscv_sim)
target_include_directories(prs PRIVATE ${PUB_INCLUDE_DIR})
