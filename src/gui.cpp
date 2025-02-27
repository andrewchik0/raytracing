#include "gui.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <IconsFontAwesome6.h>

#include "nfd.h"
#include "rt.h"

namespace raytracing
{
  bool gui::check(bool value)
  {
    if (value)
      rt::get()->mRender.reset_accumulation();
    return value;
  }

  bool gui::init()
  {
    bool result = ImGui::SFML::Init(rt::get()->mWindow);
    ImGuiIO& io = ImGui::GetIO();

    mFont = io.Fonts->AddFontFromFileTTF("assets/Roboto.ttf", 17.0f);

    static const ImWchar iconRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig config;
    config.MergeMode = true;  // Merge with default font
    config.PixelSnapH = true;
    mFAFont = io.Fonts->AddFontFromFileTTF("assets/fa-regular-400.ttf", 17.0f, &config, iconRanges);

    if (!ImGui::SFML::UpdateFontTexture())
      return false;

    setup_style();

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    return result;
  }

  void gui::update()
  {
    rt::get()->mElapsedTime = rt::get()->mClock.getElapsedTime();
    rt::get()->mTime += rt::get()->mElapsedTime.asSeconds();

    ImGui::SFML::Update(rt::get()->mWindow, rt::get()->mClock.restart());

    ImGui::PushFont(mFont);

    static bool open = true;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2({0.0f, 0.0f}));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Main", &open,
      ImGuiWindowFlags_MenuBar |
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground |
      ImGuiConfigFlags_NoMouse
    );
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiWindowFlags_NoBackground | ImGuiConfigFlags_NoMouse);

    ImGui::PopStyleVar();
    if (ImGui::BeginMainMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem(ICON_FA_FOLDER " Open..."))
          rt::get()->mSceneSerializer.load();
        if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK " Save"))
          rt::get()->mSceneSerializer.save(rt::get()->mSceneFilename);
        if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK " Save as..."))
        {
          nfdchar_t *outPath = nullptr;
          nfdfilteritem_t filterItem[2] = {{ "YAML Files", "yaml" }};
          nfdresult_t result = NFD_SaveDialog(
            &outPath, filterItem, 1, (std::filesystem::current_path() / "scenes").string().c_str(), "scene.yaml"
          );
          if (result == NFD_OKAY)
            rt::get()->mSceneSerializer.save(outPath);
          free(outPath);
        }
        if (ImGui::MenuItem(ICON_FA_XMARK " Exit"))
          exit(0);
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Help"))
      {
        if (ImGui::MenuItem(ICON_FA_INFO " About"))
          mShowAbout = true;
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

    ImGui::End();

    if (mShowAbout)
    {
      ImGui::Begin("About");
      push_font(2);
      ImGui::Text("Ray tracing App");
      pop_font();
      ImGui::Text("Built by andrewchik0");
      ImGui::Text("2025");
      if (ImGui::Button("Close"))
        mShowAbout = false;
      ImGui::End();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2({0.0f, 0.0f}));

    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiConfigFlags_NoMouse | ImGuiWindowFlags_NoBackground);
    mIsViewPortInFocus = ImGui::IsWindowFocused();
    mViewportSize = {ImGui::GetCurrentWindow()->Size.x, ImGui::GetCurrentWindow()->Size.y};
    mViewportPosition = {ImGui::GetCurrentWindow()->Pos.x, ImGui::GetCurrentWindow()->Pos.y};
    if (rt::get()->mLoaded)
      ImGui::Image(rt::get()->mRender.mFinalTexture);
    ImGui::End();

    ImGui::PopStyleVar();

    ImGui::Begin("Render");
    render_tab();
    ImGui::End();

    ImGui::Begin("Scene");
    scene_tab();
    ImGui::End();

    ImGui::Begin("General");
    general_tab();
    ImGui::End();

    ImGui::PopFont();
  }

  void gui::general_tab()
  {
    static float secondsCounter = 0.0;
    static int framesCounter = 0.0;
    static float fps = 60;
    secondsCounter += rt::get()->mElapsedTime.asSeconds();
    framesCounter++;
    ImGui::Text("FPS: %.1f", fps);
    if (secondsCounter > 0.25)
    {
      fps = framesCounter / secondsCounter;
      secondsCounter = 0.0;
      framesCounter = 0;
    }
    {
      static float values[256] = {};
      static int values_offset = 0;
      static double refresh_time = 0.0;
      if (refresh_time == 0.0)
        refresh_time = ImGui::GetTime();
      while (refresh_time < ImGui::GetTime())
      {
        static float phase = 0.0f;
        values[values_offset] = rt::get()->mElapsedTime.asSeconds() * 1000.0f;
        values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
        phase += 0.10f * values_offset;
        refresh_time += 1.0f / 60.0f;
      }
      ImGui::Text("Frame time: %.3f ms", rt::get()->mElapsedTime.asSeconds() * 1000.0f);
      float max = 0.0f;
      for (int n = 0; n < IM_ARRAYSIZE(values); n++)
        max = max > values[n] ? max : values[n];
      ImGui::PlotLines("###FrameTime", values, IM_ARRAYSIZE(values), values_offset, nullptr, 0, max * 1.2f, ImVec2(mGuiWidth - 15, 40.0f));
    }

    ImGui::Separator();
    check(ImGui::Checkbox("Render mode", &rt::get()->mRender.mRenderMode));
    check(ImGui::Checkbox("Interpolate normals", &rt::get()->mRender.mInterpolateNormals));
    check(ImGui::Checkbox("FXAA", &rt::get()->mRender.mUseFXAA));
    static bool vSync = false;
    if (check(ImGui::Checkbox("V-Sync", &vSync)))
      rt::get()->mWindow.setVerticalSyncEnabled(vSync);
    int minAccumulation = 1, minBounces = 2;
    check(ImGui::DragScalar("Bounces count", ImGuiDataType_U32, &rt::get()->mRender.mBouncesCount, 1, &minBounces));
    check(ImGui::DragScalar("Maximum accumulated frames", ImGuiDataType_U32, &rt::get()->mRender.mMaxAccumulation, 1, &minAccumulation));
    ImGui::DragFloat("Camera speed", &rt::get()->mCamera.mSpeed, 0.1, 0.001, 0, "%.2f");
    ImGui::DragFloat("Mouse sensitivity", &rt::get()->mCamera.mMouseSensitivity, 0.01, 0.01, 100, "%.2f");
    ImGui::Separator();
    if (check(ImGui::Button("Reload shaders")))
    {
      rt::get()->mRender.load_shaders();
      rt::get()->mRender.resize(rt::get()->mWindowWidth, rt::get()->mWindowHeight);
    }
    if (rt::get()->mRender.mShaderErrors.size() > 0)
      ImGui::Text(rt::get()->mRender.mShaderErrors.c_str());

    if (rt::get()->mBVHLoading || rt::get()->mTexturesLoading || rt::get()->mModelsLoading)
      ImGui::Separator();
    if (rt::get()->mBVHLoading)
      ImGui::Text("Building bounding volume hierarchies... %.1f%%",
        float(rt::get()->mRender.mBoundingVolumeBuilder.mBVHNodes.size()) /
        float(rt::get()->mRender.mTriangles.size() * 2) *
        100.0f
      );
    if (rt::get()->mTexturesLoading)
      ImGui::Text("Loading textures...");
    if (rt::get()->mModelsLoading)
      ImGui::Text("Loading 3D models...");

    ImVec2 windowSize = ImGui::GetWindowSize();
    ImGui::SetCursorPos(ImVec2(10, windowSize.y - 70));
    ImGui::Text("Left mouse button - Rotate");
    ImGui::Text("W, A, S, D - Move");
    ImGui::Text("Space / Shift - Up / Down");
  }

  void gui::scene_tab()
  {
    float headerFontScale = 1.0;

    push_font(headerFontScale);
    if (ImGui::TreeNode(ICON_FA_CAMERA " Camera options"))
    {
      pop_font();
      check(ImGui::DragFloat3("Camera position", &rt::get()->mCamera.mPosition.x, 0.01, 0, 0, "%.2f"));
      check(ImGui::DragFloat3("Camera direction", &rt::get()->mCamera.mDirection.x, 0.01, -1.0, 1.0, "%.2f"));
      rt::get()->mCamera.mDirection = normalize(rt::get()->mCamera.mDirection);
      check(ImGui::DragFloat("Gamma", &rt::get()->mRender.mGamma, 0.01, 0.01, 100, "%.2f"));
      check(ImGui::DragFloat("Exposure", &rt::get()->mRender.mExposure, 0.01, 0.01, 100, "%.2f"));
      check(ImGui::DragFloat("Blur size", &rt::get()->mRender.mBlurSize, 0.1, 0, 100, "%.1f"));
      check(ImGui::DragFloat("FOV", &rt::get()->mCamera.mFovY, 0.1, 30.0, 150.0, "%.1f"));
      ImGui::TreePop();
    }
    else
    {
      pop_font();
    }

    push_font(headerFontScale);
    if (ImGui::TreeNode(ICON_FA_OBJECT_GROUP " Scene objects"))
    {
      pop_font();
      objects_section();
      ImGui::TreePop();
    }
    else
      pop_font();

    push_font(headerFontScale);
    if (ImGui::TreeNode(ICON_FA_CIRCLE " Materials"))
    {
      pop_font();
      materials_section();
      ImGui::TreePop();
    }
    else
      pop_font();

    push_font(headerFontScale);
    if (ImGui::TreeNode(ICON_FA_IMAGE " Textures"))
    {
      pop_font();
      textures_section();
      ImGui::TreePop();
    }
    else
      pop_font();
  }

  void gui::objects_section()
  {
    std::string label;
    for (size_t i = 0; i < rt::get()->mRender.mSpheresCount; ++i)
    {
      label = rt::get()->mRender.mSpheresAdditional[i].name +"###Sphere" + std::to_string(i);
      if (ImGui::TreeNode(label.c_str()))
      {
        label = "Name###SphereName" + std::to_string(i);
        ImGui::InputText(label.c_str(), &rt::get()->mRender.mSpheresAdditional[i].name);
        label = "Position###SpherePosition" + std::to_string(i);
        check(ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mSpheres[i].center.x, 0.01f, -100.0f, 100.0f, "%.2f"));
        label = "Radius###SphereRadius" + std::to_string(i);
        check(ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mSpheres[i].radius, 0.01f, 0.01f, 100.0f, "%.2f"));
        label = "Material ID##SphereMaterialID" + std::to_string(i);
        check(ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &rt::get()->mRender.mSpheres[i].materialIndex));
        label = "Delete###SphereDelete" + std::to_string(i);
        if (check(ImGui::Button(label.c_str())))
          rt::get()->delete_sphere(i);
        ImGui::TreePop();
      }
    }
    for (size_t i = 0; i < rt::get()->mRender.mPlanesCount; ++i)
    {
      label = rt::get()->mRender.mPlanesAdditional[i].name + "###Plane" + std::to_string(i);
      if (ImGui::TreeNode(label.c_str()))
      {
        label = "Name###PlaneName" + std::to_string(i);
        ImGui::InputText(label.c_str(), &rt::get()->mRender.mPlanesAdditional[i].name);
        label = "Normal###PlaneNormal" + std::to_string(i);
        check(ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mPlanes[i].normal.x, 0.01f, -1.0f, 1.0f, "%.2f"));
        label = "Distance###PlaneDistance" + std::to_string(i);
        check(ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mPlanes[i].distance, 0.01f, 0, 0, "%.2f"));
        label = "Material ID##PlaneMaterialID" + std::to_string(i);
        check(ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &rt::get()->mRender.mPlanes[i].materialIndex));
        label = "Delete###PlaneDelete" + std::to_string(i);
        if (check(ImGui::Button(label.c_str())))
          rt::get()->delete_plane(i);
        ImGui::TreePop();
      }
    }
    for (size_t i = 0; i < rt::get()->mModelNames.size(); ++i)
    {
      label = rt::get()->mModelNames[i] + "###Model" + std::to_string(i);
      if (ImGui::TreeNode(label.c_str()))
      {
        ImGui::Text(rt::get()->mModelNames[i].c_str());
        ImGui::TreePop();
      }
    }

    if (ImGui::Button("+ Add object"))
      mAddItemOpened = !mAddItemOpened;

    if (mAddItemOpened)
      add_item_window();
  }

  void gui::add_item_window()
  {
    ImGui::Begin("Add object");

    const char* items[] = {"Sphere", "Plane"};
    static const char* currentItem = nullptr;

    if (ImGui::BeginCombo("Object Type##combo", currentItem))
    {
      for (auto& item : items)
      {
        bool is_selected = (currentItem == item);
        if (ImGui::Selectable(item, is_selected))
          currentItem = item;
        if (is_selected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    static SphereObject sphere{{0, 0, 0}, 1, {0, 0, 0}, 0};
    if (currentItem && strncmp(currentItem, "Sphere", 5) == 0)
    {

      std::string label = "Position###SphereNewPosition";
      ImGui::DragFloat3(label.c_str(), &sphere.center.x, 0.01f, -100.0f, 100.0f, "%.2f");
      label = "Radius###SphereNewRadius";
      ImGui::DragFloat(label.c_str(), &sphere.radius, 0.01f, 0.01f, 100.0f, "%.2f");
      label = "Material ID##SphereMaterialID";
      ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &sphere.materialIndex);
      ImGui::Separator();
    }

    static PlaneObject plane{{0, 1, 0}, 0, {0, 0, 0}, 0};
    if (currentItem && strncmp(currentItem, "Plane", 5) == 0)
    {
      std::string label = "Normal###PlaneNewNormal";
      ImGui::DragFloat3(label.c_str(), &plane.normal.x, 0.01f, -1.0f, 1.0f, "%.2f");
      label = "Distance###PlaneNewDistance";
      ImGui::DragFloat(label.c_str(), &plane.distance, 0.01f, -100.0f, 100.0f, "%.2f");
      label = "Material ID##PlaneMaterialID";
      ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &plane.materialIndex);
      ImGui::Separator();
    }

    if (ImGui::Button("Cancel###NewObjectCancel"))
      mAddItemOpened = false;
    ImGui::SameLine(0.0f);
    if (currentItem && ImGui::Button("Add###NewObjectAdd"))
    {
      mAddItemOpened = false;

      if (strncmp(currentItem, "Sphere", 5) == 0)
        rt::get()->add_sphere("Sphere", sphere);

      if (strncmp(currentItem, "Plane", 5) == 0)
        rt::get()->add_plane("Plane", plane);

      currentItem = nullptr;
      check(true);
    }
    ImGui::End();
  }

  void gui::materials_section()
  {
    std::string label;

    for (size_t i = 0; i < rt::get()->mRender.mMaterialsCount; ++i)
    {
      label = rt::get()->mRender.mMaterialsAdditional[i].name + " ID: " + std::to_string(i) + "###Material" + std::to_string(i);
      if (ImGui::TreeNode(label.c_str()))
      {
        label = "Name###MaterialName" + std::to_string(i);
        ImGui::InputText(label.c_str(), &rt::get()->mRender.mMaterialsAdditional[i].name);
        label = "Albedo###Albedo" + std::to_string(i);
        check(ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mMaterials[i].albedo.x, 0.01f, 0.0f, 1.0f, "%.2f"));
        label = "Roughness###Roughness" + std::to_string(i);
        check(ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mMaterials[i].roughness, 0.01f, 0.0f, 1.0f, "%.2f"));
        label = "Emissivity###Emissivity" + std::to_string(i);
        check(ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mMaterials[i].emissivity.x, 0.01f, 0.0f, 100.0f, "%.2f"));
        label = "Texture ID###TextureID" + std::to_string(i);
        check(ImGui::InputInt(label.c_str(), &rt::get()->mRender.mMaterials[i].textureIndex));
        label = "Normal Texture ID###NormalTextureID" + std::to_string(i);
        check(ImGui::InputInt(label.c_str(), &rt::get()->mRender.mMaterials[i].normalTextureIndex));
        label = "Metallic Texture ID###MetallicTextureID" + std::to_string(i);
        check(ImGui::InputInt(label.c_str(), &rt::get()->mRender.mMaterials[i].metallicTextureIndex));
        label = "Texture Coordinates Multiplier###TextureCoordinatesMultiplier" + std::to_string(i);
        check(ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mMaterials[i].textureCoordinatesMultiplier, 0.01f, 0.01f,
                         100.0f, "%.2f"));
        label = "Delete###MaterialDelete" + std::to_string(i);
        if (ImGui::Button(label.c_str()))
          rt::get()->delete_material(i);
        ImGui::TreePop();
      }
    }

    static Material material;
    if (ImGui::Button("+ Add Material"))
      rt::get()->add_material("Material", material);
  }

  void gui::textures_section()
  {
    std::string label;
    for (size_t i = 0; i < rt::get()->mRender.mTextures.mTextureFilenames.size(); ++i)
    {
      label = "[" + std::to_string(i) + "] " + rt::get()->mRender.mTextures.mTextureFilenames[i];
      ImGui::Text(label.c_str());
    }
    if (ImGui::Button("Add texture..."))
    {
      rt::get()->mRender.mTextures.load_from_filesystem();
      rt::get()->mRender.mTextures.reload();
    }
  }


  void gui::render_tab()
  {
    uint32_t minSamples = 1, minBounces = 1;
    ImGui::DragScalar("Samples", ImGuiDataType_U32, &rt::get()->mRenderOptions.samples, 1, &minSamples);
    ImGui::DragScalar("Bounces", ImGuiDataType_U32, &rt::get()->mRenderOptions.bounces, 1, &minBounces);
    ImGui::InputText("Filename", &rt::get()->mRenderOptions.filename);
    ImGui::SameLine();
    if (ImGui::Button("Render to file"))
      rt::get()->render_to_image();
  }

  void gui::push_font(float scale)
  {
    mOldFontSize = ImGui::GetFont()->Scale;
    ImGui::GetFont()->Scale *= scale;
    ImGui::PushFont(ImGui::GetFont());
  }

  void gui::pop_font()
  {
    ImGui::GetFont()->Scale = mOldFontSize;
    ImGui::PopFont();
  }

  void gui::setup_style()
    {
      ImVec4* colors = ImGui::GetStyle().Colors;
      colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
      colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
      colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
      colors[ImGuiCol_ChildBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
      colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
      colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
      colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
      colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
      colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
      colors[ImGuiCol_FrameBgActive]          = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
      colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
      colors[ImGuiCol_TitleBgActive]          = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
      colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
      colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
      colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
      colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
      colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
      colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
      colors[ImGuiCol_CheckMark]              = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
      colors[ImGuiCol_SliderGrab]             = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
      colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
      colors[ImGuiCol_Button]                 = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
      colors[ImGuiCol_ButtonHovered]          = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
      colors[ImGuiCol_ButtonActive]           = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
      colors[ImGuiCol_Header]                 = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
      colors[ImGuiCol_HeaderHovered]          = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
      colors[ImGuiCol_HeaderActive]           = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
      colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
      colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
      colors[ImGuiCol_SeparatorActive]        = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
      colors[ImGuiCol_ResizeGrip]             = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
      colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
      colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
      colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
      colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
      colors[ImGuiCol_PlotHistogram]          = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
      colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
      colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
      colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
      colors[ImGuiCol_NavHighlight]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
      colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
      colors[ImGuiCol_Tab]                    = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
      colors[ImGuiCol_TabActive]              = ImVec4(0.3f, 0.3f, 0.3f, 1.00f);
      colors[ImGuiCol_TabDimmed]              = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
      colors[ImGuiCol_TabUnfocused]           = ImVec4(0.2f, 0.2f, 0.2f, 1.00f);
      colors[ImGuiCol_TabSelected]            = ImVec4(0.3f, 0.3f, 0.3f, 1.00f);
      colors[ImGuiCol_TabHovered]             = ImVec4(0.4f, 0.4f, 0.4f, 1.00f);
    }

} // namespace raytracing
