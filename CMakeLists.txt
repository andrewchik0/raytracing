cmake_minimum_required(VERSION 3.28)
project(RayTracing LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 3.0.0
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
  SYSTEM)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(ImGui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG v1.91.1
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
  SYSTEM)
FetchContent_MakeAvailable(ImGui)
FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)

set(IMGUI_SFML_FIND_SFML OFF)
FetchContent_Declare(ImGui-SFML
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml
  GIT_TAG v3.0
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
  SYSTEM)
FetchContent_MakeAvailable(ImGui-SFML)

FetchContent_Declare(
  GLEW
  GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
  GIT_TAG glew-cmake-2.2.0
)
FetchContent_MakeAvailable(GLEW)

add_executable(main
  src/main.cpp
  src/rt.cpp
  src/rt.h
  src/input.cpp
  src/input.h
  src/camera.cpp
  src/camera.h
  src/render.cpp
  src/render.h
  src/uniform_buffer.cpp
  src/uniform_buffer.h)

find_package(OpenGL REQUIRED)

target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE SFML::Graphics ImGui-SFML::ImGui-SFML OpenGL::GL libglew_static)

add_subdirectory(third_party/glm)
target_include_directories(main PUBLIC third_party/glm/ ${OPENGL_INCLUDE_DIRS})

add_custom_target(
  run-ray-tracing
  COMMAND main
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  VERBATIM
)
add_dependencies(run-ray-tracing main)
