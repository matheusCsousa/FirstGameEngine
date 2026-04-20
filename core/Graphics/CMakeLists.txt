add_subdirectory(Renderer)
add_subdirectory(Mesh)
add_subdirectory(Camera)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_library(graphics INTERFACE)

target_link_libraries(graphics INTERFACE
  mesh
  renderer
  camera
  OpenGL::GL
  GLEW::GLEW
  glfw
  glm
)
