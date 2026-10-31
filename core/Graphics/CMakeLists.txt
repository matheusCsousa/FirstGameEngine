add_subdirectory(Renderer)
add_subdirectory(Mesh)
# Renderer::Draw uses Mesh, while Mesh owns renderer buffer objects. Declare the
# static-library cycle so CMake repeats the archives for single-pass Unix linkers.
target_link_libraries(renderer PUBLIC mesh)
add_subdirectory(Camera)
add_library(graphics INTERFACE)
target_link_libraries(graphics INTERFACE mesh renderer camera engine_dependencies)
