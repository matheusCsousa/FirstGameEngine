add_library(entity
  Entity.cpp
)

target_include_directories(entity PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(entity
  PUBLIC
  graphics
)
