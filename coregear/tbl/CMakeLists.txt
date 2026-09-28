add_library(tbl)
target_sources(tbl PUBLIC FILE_SET CXX_MODULES FILES tbl.mpp)

if(CG_ENABLE_TESTING)
  add_executable(tbl_test tbl.test.cpp)
  target_link_libraries(tbl_test tbl)
  add_test(NAME tbl_fields COMMAND tbl_test)

  set(TEST_OBJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/tbl_test.dir")
  set(TEST_MAPPER "${TEST_OBJECT_DIR}/tbl.test.cpp.o.modmap")
  foreach(case IN ITEMS unsupported_annotation contradictory_annotations
                        invalid_member_type)
    add_test(
      NAME tbl_${case}
      COMMAND
        ${CMAKE_COMMAND} -DTEST_COMPILER=${CMAKE_CXX_COMPILER}
        -DTEST_MAPPER=${TEST_MAPPER}
        -DTEST_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${case}.invalid.cpp
        -DTEST_EXPECTED=${case} -P
        ${CMAKE_CURRENT_SOURCE_DIR}/check_invalid.cmake)
    set_tests_properties(tbl_${case} PROPERTIES WORKING_DIRECTORY
                                                ${CMAKE_BINARY_DIR})
  endforeach()
endif()
