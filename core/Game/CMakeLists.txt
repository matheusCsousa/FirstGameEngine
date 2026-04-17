add_library(game
  Game.cpp
  Game.hpp
)

target_link_libraries(game PRIVATE
  graphics
)
