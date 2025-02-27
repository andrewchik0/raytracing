cmake_minimum_required(VERSION 3.28)
project(RayTracing LANGUAGES CXX)

find_package(OpenGL REQUIRED)

include(dependencies.cmake)

add_executable(ray-tracing
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
  src/gui.cpp
  src/gui.h
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
  src/pch.h
  third_party/stb/stb_image_wrapper.cpp
)

target_precompile_headers(ray-tracing PUBLIC src/pch.h)
target_compile_features(ray-tracing PRIVATE cxx_std_20)

target_include_directories(ray-tracing PUBLIC
  third_party/stb
  third_party/FontAwesome
  ${glm_SOURCE_DIR}
  ${OPENGL_INCLUDE_DIRS}
)

target_link_libraries(ray-tracing PUBLIC
  SFML::Graphics
  ImGui-SFML::ImGui-SFML
  OpenGL::GL
  libglew_static
  yaml-cpp::yaml-cpp
  nfd
  assimp
)
