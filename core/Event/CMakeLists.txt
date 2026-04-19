add_library(event
  Event.cpp
)

target_include_directories(event PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)
