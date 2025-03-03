#include "rt.h"

#include <filesystem>
#include <iostream>
#include <thread>
#include <optional>

#include <nfd.h>

#include <imgui-SFML.h>

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

    mRender.init();
    mGui.init();

    mSceneSerializer.load(options.scene_filename);
  }

  void rt::run()
  {
    while (mWindow.isOpen())
    {
      if (!mTexturesLoading && !mModelsLoading && !mBVHLoading)
      {
        if (!mLoaded)
        {
          mRender.post_init();
          mWindow.setVerticalSyncEnabled(mVSyncEnabled);
          mLoaded = true;
        }

        mWindow.clear();
        mRender.clear();
        mInput.clear();

        mGui.update();
        if (!handle_messages())
          break;

        mCamera.update(mElapsedTime.asSeconds());
        set_viewport();
        mRender.draw(nullptr);

        ImGui::SFML::Render(mWindow);

        mWindow.display();
      }
      else
      {
        mWindow.clear();
        handle_messages();
        mGui.update();
        ImGui::SFML::Render(mWindow);
        mWindow.display();
      }
    }
  }

  void rt::add_model(const std::string& filename)
  {
    mModelNames.push_back(filename);
  }

  void rt::render_to_image()
  {
    sf::RenderTexture rt({ mRenderOptions.width, mRenderOptions.height });

    // Store data
    uint32_t bounces = mRender.mBouncesCount;
    size_t accumulatingFrameIndex = mRender.mAccumulatingFrameIndex;
    size_t maxAccumulation = mRender.mMaxAccumulation;
    int renderMode = mRender.mRenderMode;

    mRender.mRenderMode = true;
    mRender.mBouncesCount = mRenderOptions.bounces;
    mRender.mAccumulatingFrameIndex = 0;
    mRender.mMaxAccumulation = mRenderOptions.samples;
    set_viewport(mRenderOptions.width, mRenderOptions.height);

    size_t sampleCounter = 0;

    while (sampleCounter++ < mRenderOptions.samples)
    {
      mRender.clear();
      // mRender.draw(&rt);
      mElapsedTime = mClock.getElapsedTime();
      mTime += mElapsedTime.asSeconds();
      mClock.restart();
    }

    rt.display();
    if (!rt.getTexture().copyToImage().saveToFile(mRenderOptions.filename))
      std::cerr << "Failed to save image" << std::endl;

    // Restore data
    mRender.mBouncesCount = bounces;
    mRender.mRenderMode = renderMode;
    mRender.mMaxAccumulation = maxAccumulation;
    mRender.mAccumulatingFrameIndex = accumulatingFrameIndex;
    set_viewport();
  }

  void rt::render_to_video()
  {
    // Store data
    uint32_t bounces = mRender.mBouncesCount;
    size_t accumulatingFrameIndex = mRender.mAccumulatingFrameIndex;
    size_t maxAccumulation = mRender.mMaxAccumulation;
    int renderMode = mRender.mRenderMode;

    mRender.mRenderMode = true;
    mRender.mBouncesCount = mRenderOptions.bounces;
    mRender.reset_accumulation();
    mRender.mMaxAccumulation = mRenderOptions.samples;

    set_viewport(mRenderOptions.width, mRenderOptions.height);

    for (size_t i = 0; i < mRenderOptions.duration * mRenderOptions.framerate; i++)
    {
      sf::RenderTexture rt({mRenderOptions.width, mRenderOptions.height});
      size_t sampleCounter = 0;
      mCamera.move_right(0.1);

      while (sampleCounter++ < mRenderOptions.samples)
      {
        mRender.clear();
        // mRender.draw(&rt);
        mElapsedTime = mClock.getElapsedTime();
        mTime += mElapsedTime.asSeconds();
        mClock.restart();
      }

      mRender.reset_accumulation();

      rt.display();
      if (!rt.getTexture().copyToImage().saveToFile(mRenderOptions.video_filename_base + std::to_string(i) + ".png"))
        std::cerr << "Failed to save image" << std::endl;
    }

    // Restore data
    mRender.mBouncesCount = bounces;
    mRender.mRenderMode = renderMode;
    mRender.mMaxAccumulation = maxAccumulation;
    mRender.mAccumulatingFrameIndex = accumulatingFrameIndex;
    set_viewport();
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
      }
    }
    return true;
  }

  void rt::set_viewport()
  {
    set_viewport(mGui.mViewportSize.x, mGui.mViewportSize.y);
  }
  void rt::set_viewport(const uint32_t width, const uint32_t height)
  {
    mRender.resize(width, height);
    mCamera.resize(width, height);
  }

  void rt::load_async()
  {
    mWindow.setVerticalSyncEnabled(true);
    mModelsLoading = true;
    std::thread([&]
    {
      std::vector<std::thread> modelThreads;

      for (auto& modelName : mModelNames)
      {
        modelThreads.emplace_back([&]
        {
          model m;
          m.load_from_file(modelName);

          mRender.mTriangles.insert(mRender.mTriangles.end(), m.mTriangles.begin(), m.mTriangles.end());
          mRender.mVertices.insert(mRender.mVertices.end(), m.mVertices.begin(), m.mVertices.end());
        });
      }

      for (auto& thread : modelThreads)
        thread.join();

      mTexturesLoading = true;
      mModelsLoading = false;

      mRender.mTextures.reload();

      mBVHLoading = true;
      mTexturesLoading = false;

      mRender.mBoundingVolumeBuilder.build();

      mBVHLoading = false;
    }).detach();
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
