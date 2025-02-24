#pragma once

#include "pch.h"

#include "SFML/Graphics/Shader.hpp"

namespace raytracing
{
  class shader : public sf::Shader
  {
  public:
    status load(const std::string& vertexPath, const std::string& fragmentPath);
  private:

    static std::string read_shader_file(const std::string& path);
    static std::string parse_shader_from_file(const std::string& path, std::set<std::string>& includedFiles);
  };
}
