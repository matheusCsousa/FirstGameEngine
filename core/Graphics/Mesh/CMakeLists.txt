add_library(mesh
  Mesh.cpp
  Cube.cpp
)

target_include_directories(mesh PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mesh PUBLIC renderer engine_dependencies)
