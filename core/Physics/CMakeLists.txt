add_library(physics
  Physics.cpp
)

target_include_directories(physics PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(physics
  PUBLIC
  entity
  collision
  glm
)
