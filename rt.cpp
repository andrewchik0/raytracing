#include "rt.h"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace raytracing
{
  sf::Glsl::Vec3 glm_to_sfml(glm::vec3 v)
  {
    return { v.x, v.y, v.z };
  }

  sf::Glsl::Vec2 glm_to_sfml(glm::vec2 v)
  {
    return { v.x, v.y };
  }

  void rt::run(const init_options& options)
  {
    if (!std::filesystem::exists(mVertexShaderPath) || !std::filesystem::exists(mFragmentShaderPath))
      return;

    mWindowWidth = options.width;
    mWindowHeight = options.height;

    mWindow = sf::RenderWindow(sf::VideoMode({mWindowWidth, mWindowHeight}), options.title);

    if (load_shaders() != status::success)
      return;

    mRenderQuad = sf::RectangleShape({static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight)});
    mRenderQuad.setFillColor(sf::Color::Green);

    resize();

    while (mWindow.isOpen())
    {
      handle_messages();
      mWindow.clear();
      mTexture.clear();

      set_uniforms();

      mTexture.draw(mRenderQuad, &mShader);
      mTexture.display();

      mPostShader.setUniform("renderedTexture", mTexture.getTexture());
      mWindow.draw(mRenderQuad, &mPostShader);

      mWindow.display();
    }
  }

  void rt::handle_messages()
  {
    while (const std::optional event = mWindow.pollEvent())
    {
      mInput.handle(event);

      if (event->is<sf::Event::Closed>())
      {
        mWindow.close();
      }
      if (event->is<sf::Event::Resized>())
      {
        mWindowWidth = mWindow.getSize().x;
        mWindowHeight = mWindow.getSize().y;
        resize();
      }
    }
  }

  void rt::resize()
  {
    if (!mTexture.resize({ mWindowWidth, mWindowHeight}))
    {
      std::cerr << "Failed to resize texture\n";
    }
    mShader.setUniform("windowSize", sf::Vector2f(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight)));
    mPostShader.setUniform("windowSize", sf::Vector2f(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight)));
  }


  std::string rt::read_shader_file(const std::string& path)
  {
    std::ifstream file(path);

    if (!file.is_open())
      std::cerr << "Failed to open file: " << path << '\n';

    std::string content((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());

    return content;
  }

  std::string rt::parse_shader_from_file(const std::string& path, std::set<std::string>& includedFiles)
  {
    std::filesystem::path fpath(path);
    std::string text = read_shader_file(path);
    std::string result;

    for (auto it = text.begin(); it < text.end(); ++it)
    {
      if (*it == '#' && std::string(it + 1, it + 8) == "include")
      {
        it += 8;
        while(*it++ != '\"') {}
        auto start = it;
        while(*it++ != '\"') {}
        auto end = it - 1;

        auto filename =
          fpath.parent_path().string() +
          '/' +
          std::string(start, end);

        if (includedFiles.find(filename) == includedFiles.end())
        {
          includedFiles.insert(filename);
          result += parse_shader_from_file(filename, includedFiles);
        }
      }
      result += *it;
    }
    return result;
  }

  status rt::load_shaders()
  {
    if (!sf::Shader::isAvailable())
    {
      return status::error;
    }

    load_shader(&mShader, mVertexShaderPath, mFragmentShaderPath);
    load_shader(&mPostShader, mVertexShaderPath, "./shaders/post.frag");

    return status::success;
  }

  status rt::load_shader(sf::Shader* shader, const std::string& vertexPath, const std::string& fragmentPath)
  {
    auto
     vertexIncluded = std::set<std::string>(),
     fragmentIncluded = std::set<std::string>();
    auto vertex = parse_shader_from_file(vertexPath, vertexIncluded);
    auto fragment = parse_shader_from_file(fragmentPath, fragmentIncluded);

    if (!shader->loadFromMemory(vertex, fragment))
      return status::error;

    return status::success;
  }


  void rt::set_uniforms()
  {
    mShader.setUniform("cameraPosition", glm_to_sfml(mCamera.position));
    mShader.setUniform("cameraDirection", glm_to_sfml(mCamera.direction));
  }

}
