add_library(tbl)
target_sources(tbl PUBLIC FILE_SET CXX_MODULES FILES tbl.mpp)

if(CG_ENABLE_TESTING)
  add_executable(tbl_test tbl.test.cpp)
  target_link_libraries(tbl_test tbl)
  add_test(NAME tbl_fields COMMAND tbl_test)

  add_executable(tbl_records_test records.test.cpp)
  target_link_libraries(tbl_records_test tbl)
  add_test(NAME tbl_records COMMAND tbl_records_test)

  add_executable(tbl_expressions_test expressions.test.cpp)
  target_link_libraries(tbl_expressions_test tbl)
  add_test(NAME tbl_expressions COMMAND tbl_expressions_test)

  set(TEST_OBJECT_DIR "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/tbl_test.dir")
  set(TEST_MAPPER "${TEST_OBJECT_DIR}/tbl.test.cpp.o.modmap")
  foreach(
    case IN
    ITEMS unsupported_annotation
          contradictory_annotations
          invalid_member_type
          incompatible_inheritance
          expression_cycle
          unresolved_reference
          expression_type_mismatch
          record_expression_cycle
          unselected_expression_cycle
          invalid_record_reference
          unresolved_record_field
          record_reference_type_mismatch
          negative_shift
          invalid_list_index
          invalid_bit_slice
          unmarked_override
          orphan_override
          override_type_mismatch
          override_final
          transitive_override_final
          conflicting_final_override
          branch_override_conflict)
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
