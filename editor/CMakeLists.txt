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
FetchContent_Declare(imguizmo
  URL https://github.com/CedricGuillemet/ImGuizmo/archive/105e4fde45e4e74aae8973d98750820ef7aca7e2.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SOURCE_SUBDIR fersa-source-only)
FetchContent_MakeAvailable(imguizmo)
add_library(editor_gizmo STATIC ${imguizmo_SOURCE_DIR}/src/ImGuizmo.cpp)
target_include_directories(editor_gizmo PUBLIC ${imguizmo_SOURCE_DIR}/src)
target_link_libraries(editor_gizmo PUBLIC editor_ui)
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(nlohmann_json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(nlohmann_json)
add_library(scene_document STATIC SceneDocument.cpp)
target_include_directories(scene_document PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(scene_document PUBLIC core nlohmann_json::nlohmann_json)
add_library(editor_widgets STATIC TransformControls.cpp)
target_include_directories(editor_widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(editor_widgets PUBLIC editor_ui core)
FetchContent_Declare(lua_source URL https://www.lua.org/ftp/lua-5.4.9.tar.gz
  URL_HASH SHA256=2335b6c582a52654f94612bf10d2f4672805d05329aa6568b1d8cd9e5c6fb8e6
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(lua_source)
file(GLOB lua_sources CONFIGURE_DEPENDS ${lua_source_SOURCE_DIR}/src/*.c)
list(FILTER lua_sources EXCLUDE REGEX "/(lua|luac)\\.c$")
add_library(editor_lua STATIC ${lua_sources})
target_include_directories(editor_lua PUBLIC ${lua_source_SOURCE_DIR}/src)
if(UNIX)
  target_link_libraries(editor_lua PUBLIC m)
endif()
add_library(editor_scripts STATIC ScriptRuntime.cpp)
target_include_directories(editor_scripts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(editor_scripts PUBLIC core editor_lua)
add_library(editor_play STATIC PlaySession.cpp)
target_include_directories(editor_play PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(editor_play PUBLIC core editor_scripts)
add_library(editor_authoring STATIC EditorMath.cpp EditHistory.cpp)
target_include_directories(editor_authoring PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(editor_authoring PUBLIC scene_document)
add_library(editor_assets STATIC AssetLibrary.cpp NativeFiles.cpp)
target_include_directories(editor_assets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(editor_assets PUBLIC scene_document editor_scripts)
if(WIN32)
  target_link_libraries(editor_assets PRIVATE comdlg32)
endif()
add_executable(fersa_editor main.cpp Editor.cpp)
target_link_libraries(fersa_editor PRIVATE core editor_ui scene_document editor_widgets editor_play editor_gizmo editor_authoring editor_assets)
foreach(asset_directory models shader textures)
  add_custom_command(TARGET fersa_editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/gameApp/${asset_directory}"
      "$<TARGET_FILE_DIR:fersa_editor>/assets/${asset_directory}" VERBATIM)
endforeach()

add_custom_command(TARGET fersa_editor POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:fersa_editor>/licenses"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/licenses/Lua.txt" "$<TARGET_FILE_DIR:fersa_editor>/licenses/Lua.txt"
  VERBATIM)
