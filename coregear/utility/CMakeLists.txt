add_library(utility)
target_sources(
  utility
  PUBLIC FILE_SET
         CXX_MODULES
         FILES
         assert.mpp
         logger.mpp
         unreachable.mpp
         utility.mpp)
target_include_directories(utility PRIVATE ${CG_INCLUDE_DIR})
target_link_libraries(utility common)
