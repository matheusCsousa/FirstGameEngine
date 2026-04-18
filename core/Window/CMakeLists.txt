add_library(window STATIC
  Window.cpp
)

target_include_directories(window PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(window
  PUBLIC
  graphics
)
