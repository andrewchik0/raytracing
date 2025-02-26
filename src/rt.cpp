#include "rt.h"

#include <filesystem>
#include <imgui-SFML.h>
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
    {
      std::cerr << "Filed to load shaders, folder does not exist!\n";
      return;
    }

    NFD_Init();

    if (options.width == 0 || options.height == 0)
    {
      mWindowWidth = sf::VideoMode().getDesktopMode().size.x - 20;
      mWindowHeight = sf::VideoMode().getDesktopMode().size.y - 160;
    }
    else
    {
      mWindowWidth = options.width;
      mWindowHeight = options.height;
    }
    mWindow = sf::RenderWindow(sf::VideoMode({ mWindowWidth, mWindowHeight }), options.title);

    mGui.init();
    mRender.init();

    resize();

    mLoading = true;
    mSceneSerializer.load(options.scene_filename);
  }

  void rt::run()
  {
    if (!mLoading)
      return;

    sf::Texture loadingTexture("assets/loading.png");
    sf::Sprite loadingSprite(loadingTexture);
    loadingSprite.setOrigin((sf::Vector2f)loadingTexture.getSize() / 2.f);
    loadingSprite.setPosition((sf::Vector2f)mWindow.getSize() / 2.f);

    while (mWindow.isOpen())
    {
      if (!mLoading)
      {
        if (!mLoaded)
        {
          mRender.post_init();
          mLoaded = true;
        }

        mWindow.clear();
        mRender.clear();
        mInput.clear();

        if (!handle_messages())
          break;
        mGui.update();

        mCamera.update(mElapsedTime.asSeconds());
        mRender.draw(&mWindow);

        ImGui::SFML::Render(mWindow);

        mWindow.display();
      }
      else
      {
        mWindow.clear();
        handle_messages();

        mElapsedTime = mClock.getElapsedTime();
        mClock.restart();
        loadingSprite.rotate(sf::degrees(mElapsedTime.asSeconds() * 200.0f));
        mWindow.draw(loadingSprite);
        mGui.update();
        ImGui::SFML::Render(mWindow);
        mWindow.display();
      }
    }
  }

  void rt::add_model(const std::string& filename)
  {
    model m;
    m.load_from_file(filename.c_str());

    mRender.mTriangles.insert(mRender.mTriangles.end(), m.mTriangles.begin(), m.mTriangles.end());
    mRender.mVertices.insert(mRender.mVertices.end(), m.mVertices.begin(), m.mVertices.end());
  }

  void rt::render_to_image(const std::string& image_path)
  {
    uint32_t renderWidth = 1920, renderHeight = 1080;
    sf::RenderTexture rt({ renderWidth, renderHeight });
    uint32_t samples = mRender.mSamplesCount;
    uint32_t bounces = mRender.mBouncesCount;
    uint32_t width = mWindowWidth, height = mWindowHeight;
    mWindowWidth = renderWidth; mWindowHeight = renderHeight;
    resize();
    mRender.mSamplesCount = 256;
    mRender.mBouncesCount = 12;
    mRender.clear();
    mRender.draw(&rt);
    rt.display();
    if (!rt.getTexture().copyToImage().saveToFile(image_path))
      std::cerr << "Failed to save image" << std::endl;
    mRender.mSamplesCount = samples;
    mRender.mBouncesCount = bounces;
    mWindowWidth = width; mWindowHeight = height;
    resize();
  }


  bool rt::handle_messages()
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
        return false;
      }
      if (event->is<sf::Event::Resized>())
      {
        mWindowWidth = mWindow.getSize().x;
        mWindowHeight = mWindow.getSize().y;
        resize();
      }
    }
    return true;
  }

  void rt::resize()
  {
    mCamera.resize(mWindowWidth, mWindowHeight);
    mRender.resize(mWindowWidth, mWindowHeight);
  }

  void rt::add_sphere(const std::string& name, const SphereObject& object)
  {
    if (mRender.mSpheresCount >= MAX_SPHERES)
      return;
    mRender.mSpheresAdditional[mRender.mSpheresCount].name = name;
    mRender.mSpheres[mRender.mSpheresCount++] = object;
  }
  void rt::add_plane(const std::string& name, const PlaneObject& object)
  {
    if (mRender.mPlanesCount >= MAX_PLANES)
      return;
    mRender.mPlanesAdditional[mRender.mPlanesCount].name = name;
    mRender.mPlanes[mRender.mPlanesCount++] = object;
  }
  void rt::add_material(const std::string& name, const Material& material)
  {
    if (mRender.mMaterialsCount >= MAX_MATERIALS)
      return;
    mRender.mMaterialsAdditional[mRender.mMaterialsCount].name = name;
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
