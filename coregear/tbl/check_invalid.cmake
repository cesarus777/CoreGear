execute_process(
  COMMAND ${TEST_COMPILER} -std=gnu++26 -freflection -fmodules-ts
          -fmodule-mapper=${TEST_MAPPER} -fsyntax-only ${TEST_SOURCE}
  RESULT_VARIABLE compile_result
  OUTPUT_VARIABLE compile_output
  ERROR_VARIABLE compile_error)

if(compile_result EQUAL 0)
  message(FATAL_ERROR "Invalid record compiled successfully: ${TEST_SOURCE}")
endif()

if(TEST_EXPECTED STREQUAL "unsupported_annotation")
  set(expected_text "unsupported field annotation")
elseif(TEST_EXPECTED STREQUAL "contradictory_annotations")
  set(expected_text "contradictory field modification annotations")
elseif(TEST_EXPECTED STREQUAL "invalid_member_type")
  set(expected_text "record data members must have tbl::field<T> type")
elseif(TEST_EXPECTED STREQUAL "incompatible_inheritance")
  set(expected_text "incompatible inherited field types")
else()
  message(FATAL_ERROR "Unknown negative test: ${TEST_EXPECTED}")
endif()

if(NOT compile_error MATCHES "${expected_text}")
  message(
    FATAL_ERROR "Expected ${TEST_EXPECTED} diagnostic, got:\n${compile_error}")
endif()
