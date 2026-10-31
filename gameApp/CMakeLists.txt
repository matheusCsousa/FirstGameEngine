add_executable(${PROJECT_NAME} main.cpp GameLogic.cpp Camera/freeCam.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Camera)
target_link_libraries(${PROJECT_NAME} PRIVATE core)
foreach(asset_directory models shader textures)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/${asset_directory}"
      "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/${asset_directory}"
    VERBATIM)
endforeach()
