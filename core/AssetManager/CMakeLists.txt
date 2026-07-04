add_library(asset_manager
  AssetManager.cpp
)

target_include_directories(asset_manager PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)
