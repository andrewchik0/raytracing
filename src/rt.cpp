#include "rt.h"

#include <filesystem>
#include <imgui-SFML.h>
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

    mLoading = true;
    mSceneSerializer.load(options.scene_filename);
  }

  void rt::run()
  {
    if (!mLoading) return;

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
          mRender.mTextures.load_to_gpu();
          mLoaded = true;
        }

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
      else
      {
        mWindow.clear();
        handle_messages();

        mElapsedTime = mClock.getElapsedTime();
        mClock.restart();
        loadingSprite.rotate(sf::degrees(mElapsedTime.asSeconds() * 200.0f));
        mWindow.draw(loadingSprite);
        mWindow.display();
      }
    }
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
    if (mRender.mPlanesCount >= MAX_PLANES)
      return;
    mRender.mPlanes[mRender.mPlanesCount++] = object;
  }
  void rt::add_material(const Material& material)
  {
    if (mRender.mMaterialsCount >= MAX_MATERIALS)
      return;
    mRender.mMaterials[mRender.mMaterialsCount++] = material;
  }
  void rt::add_triangle(const TriangleObject& object)
  {
    if (mRender.mTrianglesCount >= MAX_TRIANGLES)
      return;
    mRender.mTriangles[mRender.mTrianglesCount++] = object;
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
  void rt::delete_triangle(size_t index)
  {
    for (size_t i = index; i < mRender.mTrianglesCount; ++i)
      mRender.mTriangles[i] = mRender.mTriangles[i + 1];
    mRender.mTrianglesCount--;
  }
} // namespace raytracing
