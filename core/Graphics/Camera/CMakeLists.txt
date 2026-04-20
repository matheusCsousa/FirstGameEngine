add_library(camera
  Camera.cpp
)

target_include_directories(camera PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)
