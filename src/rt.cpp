#include "rt.h"

#include <filesystem>
#include <fstream>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>
#include <nfd.h>

namespace raytracing
{
  rt* rt::sInstance = nullptr;

  rt::~rt()
  {
    NFD_Quit();
  }

  void rt::init(const init_options& options)
  {
    sInstance = this;
    if (!std::filesystem::exists("shaders/main.frag"))
      return;

    mWindowWidth = options.width;
    mWindowHeight = options.height;

    NFD_Init();

    mWindow = sf::RenderWindow(sf::VideoMode({mWindowWidth, mWindowHeight}), options.title);

    mGui.init();
    mRender.init();

    resize();

    mSceneSerializer.load(options.scene_filename);
  }

  void rt::run()
  {
    while (mWindow.isOpen())
    {
      mWindow.clear();
      mRender.clear();
      mInput.clear();

      handle_messages();
      mGui.update();

      mCamera.update(mElapsedTime.asSeconds());
      mRender.draw(&mWindow);

      ImGui::SFML::Render(mWindow);

      mWindow.display();
    }
  }

  void rt::render_to_image(const std::string& image_path)
  {
    sf::RenderTexture rt({ 3840, 2160 });
    uint32_t samples = mRender.mSamplesCount;
    uint32_t bounces = mRender.mBouncesCount;
    uint32_t width = mWindowWidth, height = mWindowHeight;
    mWindowWidth = 3840; mWindowHeight = 2160;
    resize();
    mRender.mSamplesCount = 1024;
    mRender.mBouncesCount = 64;
    mRender.clear();
    mRender.draw(&rt);
    rt.display();
    rt.getTexture().copyToImage().saveToFile(image_path);
    mRender.mSamplesCount = samples;
    mRender.mBouncesCount = bounces;
    mWindowWidth = width; mWindowHeight = height;
    resize();
  }


  void rt::handle_messages()
  {
    while (const std::optional event = mWindow.pollEvent())
    {
      ImGui::SFML::ProcessEvent(mWindow, *event);
      mInput.handle(event);

      if (event->is<sf::Event::KeyPressed>())
      {
        if (event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::R)
        {
          mRender.load_shaders();
          mRender.resize(mWindowWidth, mWindowHeight);
        }
      }

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

  void rt::add_sphere(const SphereObject& object)
  {
    if (mRender.mSpheresCount >= MAX_SPHERES)
      return;
    mRender.mSpheres[mRender.mSpheresCount++] = object;
  }
  void rt::add_plane(const PlaneObject& object)
  {
    if (mRender.mSpheresCount >= MAX_PLANES)
      return;
    mRender.mPlanes[mRender.mPlanesCount++] = object;
  }
  void rt::add_material(const Material& material)
  {
    if (mRender.mMaterialsCount >= MAX_MATERIALS)
      return;
    mRender.mMaterials[mRender.mMaterialsCount++] = material;
  }
  void rt::delete_sphere(size_t index)
  {
    for (size_t i = index; i < mRender.mSpheresCount; ++i)
      mRender.mSpheres[i] = mRender.mSpheres[i + 1];
    mRender.mSpheresCount--;
  }
  void rt::delete_plane(size_t index)
  {
    for (size_t i = index; i < mRender.mPlanesCount; ++i)
      mRender.mPlanes[i] = mRender.mPlanes[i + 1];
    mRender.mPlanesCount--;
  }
  void rt::delete_material(size_t index)
  {
    for (size_t i = index; i < mRender.mMaterialsCount; ++i)
      mRender.mMaterials[i] = mRender.mMaterials[i + 1];
    mRender.mMaterialsCount--;
  }
} // namespace raytracing
