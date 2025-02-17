#pragma once

#include <set>

#include <SFML/Graphics.hpp>

#include "../shaders/uniforms.h"

namespace raytracing
{
  enum class status
  {
    success = 0,
    error = 1,
    file_not_found = 2,
  };

  class render
  {
  public:
    void init();
    void clear();
    void draw(sf::RenderWindow* window);
    void resize(uint32_t width, uint32_t height);

  private:
    sf::Shader
      mShader, mPostShader;
    sf::RectangleShape mRenderQuad;
    sf::RenderTexture mTexture;

    std::vector<SphereObject> mSpheres;
    std::vector<PlaneObject> mPlanes;

    void set_uniforms();

    status load_shaders();

    static status load_shader(sf::Shader* shader, const std::string& vertexPath, const std::string& fragmentPath);

    static std::string read_shader_file(const std::string& path);
    static std::string parse_shader_from_file(const std::string& path, std::set<std::string>& includedFiles);

    friend class rt;
  };
}
