add_executable(${PROJECT_NAME}
  main.cpp
  GameLogic.cpp
  Camera/freeCam.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Camera
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  core
)
