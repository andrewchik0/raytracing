cmake_minimum_required(VERSION 3.28)
project(RayTracing LANGUAGES CXX)

find_package(OpenGL REQUIRED)

include(dependencies.cmake)

add_executable(ray-tracing
  src/main.cpp
  src/pch.h
  src/rt.cpp
  src/rt.h
  src/input.cpp
  src/input.h
  src/camera.cpp
  src/camera.h
  src/gui.cpp
  src/gui.h
  src/scene_serializer.cpp
  src/scene_serializer.h
  src/render/render.cpp
  src/render/render.h
  src/render/uniform_buffer.cpp
  src/render/uniform_buffer.h
  src/render/textures.cpp
  src/render/textures.h
  src/render/model.cpp
  src/render/model.h
  src/render/bounding_volume_builder.cpp
  src/render/bounding_volume_builder.h
  src/render/shader.cpp
  src/render/shader.h
  src/render/render_texture.cpp
  src/render/render_texture.h
  third_party/stb/stb_image_wrapper.cpp
  src/render/texture.cpp
  src/render/texture.h
  src/utils/math_helper.h
  src/utils/thread_pool.cpp
  src/utils/thread_pool.h
  src/utils/platform.h
  src/utils/rt_types.h
  src/utils/utils.h
  src/utils/assert.h
  src/utils/assert.cpp
)

target_precompile_headers(ray-tracing PUBLIC src/pch.h)
target_compile_features(ray-tracing PRIVATE cxx_std_20)

target_include_directories(ray-tracing PUBLIC
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/src
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

set_property(TARGET ray-tracing PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
