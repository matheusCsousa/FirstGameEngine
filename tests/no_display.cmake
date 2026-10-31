execute_process(COMMAND "${CMAKE_COMMAND}" -E env --unset=DISPLAY --unset=WAYLAND_DISPLAY
  "${APP}" --smoke-test RESULT_VARIABLE result ERROR_VARIABLE error)
if(result EQUAL 0 OR NOT error MATCHES "Failed to initialize GLFW")
  message(FATAL_ERROR "GLFW initialization failure was not propagated: ${result}: ${error}")
endif()
