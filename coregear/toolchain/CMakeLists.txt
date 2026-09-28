add_library(reflection_smoke)
target_sources(reflection_smoke PUBLIC FILE_SET CXX_MODULES FILES
                                       reflection.mpp)

if(CG_ENABLE_TESTING)
  add_executable(reflection_smoke_test reflection.test.cpp)
  target_link_libraries(reflection_smoke_test reflection_smoke)
  add_test(NAME reflection_smoke COMMAND reflection_smoke_test)
endif()
