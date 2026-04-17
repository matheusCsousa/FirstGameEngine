add_executable(${PROJECT_NAME}
  main.cpp
  GameLogic.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  core
)
