find_package(OpenGL REQUIRED)
if(FERSA_FETCH_DEPENDENCIES)
  include(FetchContent)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
  set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.4.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_Declare(glm
    URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  # The official release contains generated glew.c/glew.h; the Git tag alone does not.
  FetchContent_Declare(glew_source
    URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(glfw glm glew_source)
  add_library(fersa_glew STATIC "${glew_source_SOURCE_DIR}/src/glew.c")
  target_include_directories(fersa_glew PUBLIC "${glew_source_SOURCE_DIR}/include")
  target_compile_definitions(fersa_glew PUBLIC GLEW_STATIC GLEW_NO_GLU)
  target_link_libraries(fersa_glew PUBLIC OpenGL::GL)
  add_library(GLEW::GLEW ALIAS fersa_glew)
else()
  find_package(glfw3 3.3 REQUIRED)
  find_package(GLEW REQUIRED)
  find_package(glm CONFIG REQUIRED)
endif()
add_library(engine_dependencies INTERFACE)
target_link_libraries(engine_dependencies INTERFACE OpenGL::GL GLEW::GLEW glfw glm::glm)
