add_library(mesh
  Mesh.cpp
)

target_include_directories(mesh PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)
