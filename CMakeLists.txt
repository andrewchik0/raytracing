cmake_minimum_required(VERSION 3.28)
project(RayTracing LANGUAGES CXX)

find_package(OpenGL REQUIRED)

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

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG 0.8.0
)
FetchContent_MakeAvailable(yaml-cpp)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG        v5.4.3
)
FetchContent_MakeAvailable(assimp)

FetchContent_Declare(
  nativefiledialog
  GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git
  GIT_TAG        v1.2.1
)
FetchContent_MakeAvailable(nativefiledialog)

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
  src/uniform_buffer.h
  src/imgui.cpp
  src/imgui.h
  src/scene_serializer.cpp
  src/scene_serializer.h
  src/textures.cpp
  src/textures.h
  src/model.cpp
  src/model.h
  src/bounding_volume_builder.cpp
  src/bounding_volume_builder.h
  src/shader.cpp
  src/shader.h
  src/pch.h)

target_precompile_headers(main PUBLIC src/pch.h)
target_compile_features(main PRIVATE cxx_std_20)

add_subdirectory(third_party/glm)

target_include_directories(main PUBLIC
  third_party/glm
  third_party/stb
  ${OPENGL_INCLUDE_DIRS}
)

target_link_libraries(main PUBLIC
  SFML::Graphics
  ImGui-SFML::ImGui-SFML
  OpenGL::GL
  libglew_static
  yaml-cpp::yaml-cpp
  nfd
  assimp
)
