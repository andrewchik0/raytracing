#include "rt.h"

#include <filesystem>
#include <fstream>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>

namespace raytracing
{
  rt* rt::sInstance = nullptr;

  void rt::init(const init_options& options)
  {
    sInstance = this;
    if (!std::filesystem::exists("shaders/main.frag"))
      return;

    mWindowWidth = options.width;
    mWindowHeight = options.height;

    mWindow = sf::RenderWindow(sf::VideoMode({mWindowWidth, mWindowHeight}), options.title);
    if (!ImGui::SFML::Init(mWindow))
      return;

    mRender.init();

    resize();
  }

  void rt::run()
  {
    mRender.push_scene();

    while (mWindow.isOpen())
    {
      mWindow.clear();
      mRender.clear();
      mInput.clear();

      handle_messages();
      imgui_update();

      mCamera.update(mElapsedTime.asSeconds());
      mRender.draw(&mWindow);

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
    mRender.resize(mWindowWidth, mWindowHeight);
  }

  void rt::imgui_update()
  {
    mElapsedTime = mClock.getElapsedTime();

    ImGui::SFML::Update(mWindow, mClock.restart());

    ImGui::Begin("Stats");
    ImGui::Text("FrameTime: %.3f ms", static_cast<float>(mElapsedTime.asMicroseconds()) / 1000.0);
    ImGui::Text("FPS: %.1f", 1.0 / mElapsedTime.asSeconds());
    ImGui::Separator();
    ImGui::Checkbox("Use FXAA", &mRender.mUseFXAA);
    ImGui::Separator();
    ImGui::DragFloat3("Light Direction", &mRender.mLightDirection.x, 0.01f, -1.0f, 1.0f, "%.2f");
    ImGui::Separator();
    if (ImGui::Button("Reload shaders"))
    {
      mRender.load_shaders();
    }
    ImGui::End();
  }

  void rt::add_sphere(const SphereObject& object)
  {
    if (mRender.mSpheresCount >= MAX_SPHERES) return;
    mRender.mSpheres[mRender.mSpheresCount++] = std::move(object);
  }
  void rt::add_plane(const PlaneObject& object)
  {
    if (mRender.mSpheresCount >= MAX_PLANES) return;
    mRender.mPlanes[mRender.mPlanesCount++] = std::move(object);
  }
}
