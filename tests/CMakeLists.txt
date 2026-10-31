add_executable(runtime_tests runtime_tests.cpp)
target_link_libraries(runtime_tests PRIVATE core)
target_compile_definitions(runtime_tests PRIVATE FERSA_TEST_ASSETS="${PROJECT_SOURCE_DIR}/gameApp")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/outside-working-directory")
add_test(NAME runtime_lifecycle COMMAND runtime_tests)
add_test(NAME sample_smoke COMMAND $<TARGET_FILE:fersa> --smoke-test)
set_tests_properties(runtime_lifecycle sample_smoke PROPERTIES TIMEOUT 30
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/outside-working-directory")
add_test(NAME missing_assets COMMAND ${CMAKE_COMMAND}
  -DAPP=$<TARGET_FILE:fersa> -P "${CMAKE_CURRENT_SOURCE_DIR}/missing_assets.cmake")
if(UNIX AND NOT APPLE)
  add_test(NAME no_display COMMAND ${CMAKE_COMMAND}
    -DAPP=$<TARGET_FILE:fersa> -P "${CMAKE_CURRENT_SOURCE_DIR}/no_display.cmake")
endif()

if(FERSA_BUILD_EDITOR)
  add_test(NAME editor_smoke COMMAND $<TARGET_FILE:fersa_editor> --smoke-test)
  set_tests_properties(editor_smoke PROPERTIES TIMEOUT 30
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/outside-working-directory")
endif()
