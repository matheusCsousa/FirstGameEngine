execute_process(COMMAND "${APP}" --assets "${CMAKE_CURRENT_BINARY_DIR}/no-such-assets"
  RESULT_VARIABLE result ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "Missing sample asset")
  message(FATAL_ERROR "Missing assets were not reported cleanly: ${result}: ${error}")
endif()
