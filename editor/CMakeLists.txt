include(FetchContent)
set(FERSA_IMGUI_SOURCE_DIR "" CACHE PATH "Existing Dear ImGui docking source checkout (optional)")
if(FERSA_IMGUI_SOURCE_DIR)
  set(imgui_SOURCE_DIR "${FERSA_IMGUI_SOURCE_DIR}")
else()
  FetchContent_Declare(imgui URL https://github.com/ocornut/imgui/archive/refs/tags/v1.92.0-docking.tar.gz DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(imgui)
endif()
add_library(editor_ui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(editor_ui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends ${imgui_SOURCE_DIR}/misc/cpp)
target_link_libraries(editor_ui PUBLIC engine_dependencies)
add_executable(fersa_editor main.cpp Editor.cpp)
target_link_libraries(fersa_editor PRIVATE core editor_ui)
foreach(asset_directory models shader textures)
  add_custom_command(TARGET fersa_editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/gameApp/${asset_directory}"
      "$<TARGET_FILE_DIR:fersa_editor>/assets/${asset_directory}" VERBATIM)
endforeach()
