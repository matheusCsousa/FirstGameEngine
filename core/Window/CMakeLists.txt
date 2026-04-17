add_library(window
  Window.cpp
  Window.hpp
)

link_libraries(window PRIVATE
  core
)
