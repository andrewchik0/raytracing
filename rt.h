#pragma once

#include <SFML/Graphics.hpp>
#include <set>

#include "input.h"
#include "camera.h"

namespace raytracing
{
  enum class status
  {
    success = 0,
    error = 1,
    file_not_found = 2,
  };

  struct init_options
  {
    std::string title = "Ray Tracing";
    uint32_t width = 800, height = 600;
  };

  class rt
  {
  public:
    void run(const init_options &options);

  private:
    sf::RenderWindow mWindow;
    sf::Shader
      mShader, mPostShader;
    sf::RectangleShape mRenderQuad;
    sf::RenderTexture mTexture;

    uint32_t mWindowWidth = 0, mWindowHeight = 0;

    input mInput;
    camera mCamera;

    const std::string mVertexShaderPath = "./shaders/rt.vert";
    const std::string mFragmentShaderPath = "./shaders/rt.frag";

    void handle_messages();
    void set_uniforms();
    void resize();

    status load_shaders();

    static status load_shader(sf::Shader* shader, const std::string& vertexPath, const std::string& fragmentPath);

    static std::string read_shader_file(const std::string& path);
    static std::string parse_shader_from_file(const std::string& path, std::set<std::string>& includedFiles);
  };
}
