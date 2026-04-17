find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_library(graphics INTERFACE)

target_link_libraries(graphics INTERFACE
    OpenGL::GL
    GLEW::GLEW
    glfw
)
