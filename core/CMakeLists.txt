add_subdirectory(Window)
add_subdirectory(Input)
add_subdirectory(Graphics)
add_subdirectory(Game)
add_subdirectory(Event)

add_library(core INTERFACE)

target_link_libraries(core INTERFACE
  window
  input
  graphics
  event
  game
)
