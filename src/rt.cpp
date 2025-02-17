#include "rt.h"

#include <filesystem>
#include <fstream>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>

namespace raytracing
{
  rt* rt::sInstance = nullptr;

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
    sInstance = this;
    if (!std::filesystem::exists("shaders/main.frag"))
      return;

    mWindowWidth = options.width;
    mWindowHeight = options.height;

    mWindow = sf::RenderWindow(sf::VideoMode({mWindowWidth, mWindowHeight}), options.title);
    if (!ImGui::SFML::Init(mWindow))
      return;

    if (load_shaders() != status::success)
      return;

    mRenderQuad = sf::RectangleShape({static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight)});
    mRenderQuad.setFillColor(sf::Color::Green);

    resize();

    while (mWindow.isOpen())
    {
      mWindow.clear();
      mTexture.clear();
      mInput.clear();

      handle_messages();

      imgui_update();

      mCamera.update(mElapsedTime.asSeconds());

      set_uniforms();

      mTexture.draw(mRenderQuad, &mShader);
      mTexture.display();

      mPostShader.setUniform("renderedTexture", mTexture.getTexture());
      mWindow.draw(mRenderQuad, &mPostShader);
      ImGui::SFML::Render(mWindow);

      mWindow.display();
    }
  }

  void rt::handle_messages()
  {
    while (const std::optional event = mWindow.pollEvent())
    {
      ImGui::SFML::ProcessEvent(mWindow, *event);
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
    mCamera.resize(mWindowWidth, mWindowHeight);
    if (!mTexture.resize({ mWindowWidth, mWindowHeight}))
    {
      std::cerr << "Failed to resize texture\n";
    }
    mShader.setUniform("windowSize", sf::Vector2f(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight)));
    mShader.setUniform("halfHeight", mCamera.mHalfHeight);
    mShader.setUniform("halfWidth", mCamera.mHalfWidth);
    mPostShader.setUniform("windowSize", sf::Vector2f(static_cast<float>(mWindowWidth), static_cast<float>(mWindowHeight)));
  }

  void rt::imgui_update()
  {
    mElapsedTime = mClock.getElapsedTime();

    ImGui::SFML::Update(mWindow, mClock.restart());

    ImGui::Begin("Stats");
    ImGui::Text("FrameTime: %.3f ms", static_cast<float>(mElapsedTime.asMicroseconds()) / 1000.0);
    ImGui::Text("FPS: %.1f", 1.0 / mElapsedTime.asSeconds());
    ImGui::End();
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

    load_shader(&mShader, "./shaders/quad.vert", "./shaders/main.frag");
    load_shader(&mPostShader, "./shaders/quad.vert", "./shaders/post.frag");

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
    mShader.setUniform("cameraPosition", glm_to_sfml(mCamera.mPosition));
    mShader.setUniform("cameraDirection", glm_to_sfml(mCamera.mDirection));
    mShader.setUniform("cameraRight", glm_to_sfml(mCamera.mRight));
    mShader.setUniform("cameraUp", glm_to_sfml(mCamera.mUp));
  }

}
