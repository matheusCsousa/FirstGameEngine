add_subdirectory(Window)
add_subdirectory(Input)
add_subdirectory(Graphics)
add_subdirectory(Entity)
add_subdirectory(Game)
add_subdirectory(Event)
add_subdirectory(Scene)
add_subdirectory(AssetManager)
add_subdirectory(Collision)
add_subdirectory(Physics)

add_library(core INTERFACE)

target_link_libraries(core INTERFACE
  window
  input
  graphics
  entity
  scene
  event
  game
  asset_manager
  collision
  physics
)

