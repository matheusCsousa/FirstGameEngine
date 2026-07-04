add_library(renderer
  Renderer.cpp
  VAO/VAO.cpp
  VBO/VBO.cpp
  EBO/EBO.cpp
  Shader/Shader.cpp
  Texture/Texture.cpp
)

target_include_directories(renderer PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/VAO
  ${CMAKE_CURRENT_SOURCE_DIR}/VBO
  ${CMAKE_CURRENT_SOURCE_DIR}/EBO
  ${CMAKE_CURRENT_SOURCE_DIR}/Shader
  ${CMAKE_CURRENT_SOURCE_DIR}/Texture
)
