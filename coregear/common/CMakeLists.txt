add_library(common)
target_sources(common PUBLIC FILE_SET CXX_MODULES FILES char_bits.mpp
                             common.mpp)
target_include_directories(common PRIVATE ${CG_INCLUDE_DIR})
target_link_libraries(common PRIVATE utility)
