add_library(game
  Game.cpp
)

target_include_directories(game PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(game PRIVATE
  graphics
  window
  input
)
