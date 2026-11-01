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

if(FERSA_BUILD_EDITOR)
  add_executable(scene_document_tests scene_document_tests.cpp)
  target_link_libraries(scene_document_tests PRIVATE scene_document)
  add_test(NAME scene_documents COMMAND scene_document_tests)
  add_executable(editor_controls_tests editor_controls_tests.cpp)
  target_link_libraries(editor_controls_tests PRIVATE editor_widgets)
  add_test(NAME editor_controls COMMAND editor_controls_tests)
  set_tests_properties(scene_documents editor_controls PROPERTIES TIMEOUT 30)
endif()

if(FERSA_BUILD_EDITOR)
  add_executable(play_session_tests play_session_tests.cpp)
  target_link_libraries(play_session_tests PRIVATE editor_play scene_document)
  add_test(NAME play_session COMMAND play_session_tests)
  set_tests_properties(play_session PROPERTIES TIMEOUT 30)
endif()

if(FERSA_BUILD_EDITOR)
  add_executable(editor_authoring_tests editor_authoring_tests.cpp)
  target_link_libraries(editor_authoring_tests PRIVATE editor_authoring)
  add_test(NAME editor_authoring COMMAND editor_authoring_tests)
  set_tests_properties(editor_authoring PROPERTIES TIMEOUT 30)
endif()
