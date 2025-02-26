#include "gui.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

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
    mFont = io.Fonts->AddFontFromFileTTF("assets/Roboto.ttf", 16.0f);
    if (!ImGui::SFML::UpdateFontTexture())
      return false;
    setup_style(true, 0.75f);
    return result;
  }

  void gui::update()
  {
    rt::get()->mElapsedTime = rt::get()->mClock.getElapsedTime();
    rt::get()->mTime += rt::get()->mElapsedTime.asSeconds();

    ImGui::SFML::Update(rt::get()->mWindow, rt::get()->mClock.restart());

    ImGui::PushFont(mFont);
    ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
    ImGui::BeginViewportSideBar("main", viewport, ImGuiDir_Left, mGuiWidth, false);

    if (ImGui::BeginTabBar("main_tab_bar", false))
    {
      if (ImGui::BeginTabItem("General"))
      {
        general_tab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Scene"))
      {
        scene_tab();
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("Render"))
      {
        render_tab();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
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
    check(ImGui::Checkbox("FXAA", &rt::get()->mRender.mUseFXAA));
    static bool vSync = false;
    if (check(ImGui::Checkbox("V-Sync", &vSync)))
      rt::get()->mWindow.setVerticalSyncEnabled(vSync);
    int min = 1;
    check(ImGui::DragScalar("Samples Count", ImGuiDataType_U32, &rt::get()->mRender.mSamplesCount, 1, &min));
    check(ImGui::DragScalar("Bounces Count", ImGuiDataType_U32, &rt::get()->mRender.mBouncesCount, 1, &min));
    ImGui::Separator();
    if (check(ImGui::Button("Reload shaders")))
    {
      rt::get()->mRender.load_shaders();
      rt::get()->mRender.resize(rt::get()->mWindowWidth, rt::get()->mWindowHeight);
    }

    ImVec2 windowSize = ImGui::GetWindowSize();
    ImGui::SetCursorPos(ImVec2(10, windowSize.y - 65));
    ImGui::Text("Left mouse button - Rotate");
    ImGui::Text("W, A, S, D - Move");
    ImGui::Text("Space / Shift - Up / Down");
  }

  void gui::scene_tab()
  {
    float headerFontScale = 1.0;
    push_font(headerFontScale);
    if (ImGui::TreeNode("Save/Load"))
    {
      pop_font();
      ImGui::Text("Filename:");
      ImGui::InputText("###SaveSceneFileName", rt::get()->mSceneFilename.data(), 256);
      ImGui::SameLine();
      if (ImGui::Button("Save scene"))
        rt::get()->mSceneSerializer.save(rt::get()->mSceneFilename);
      ImGui::Separator();
      if (ImGui::Button("Load scene..."))
        rt::get()->mSceneSerializer.load();
      ImGui::TreePop();
    }
    else
      pop_font();

    push_font(headerFontScale);
    if (ImGui::TreeNode("Camera options"))
    {
      pop_font();
      check(ImGui::DragFloat3("Camera position", &rt::get()->mCamera.mPosition.x, 0.01, 0, 0, "%.2f"));
      check(ImGui::DragFloat3("Camera direction", &rt::get()->mCamera.mDirection.x, 0.01, -1.0, 1.0, "%.2f"));
      rt::get()->mCamera.mDirection = normalize(rt::get()->mCamera.mDirection);
      check(ImGui::DragFloat("Gamma", &rt::get()->mRender.mGamma, 0.01, 0.01, 100, "%.2f"));
      check(ImGui::DragFloat("Exposure", &rt::get()->mRender.mExposure, 0.01, 0.01, 100, "%.2f"));
      check(ImGui::DragFloat("Blur size", &rt::get()->mRender.mBlurSize, 0.1, 0, 100, "%.1f"));
      ImGui::TreePop();
    }
    else
      pop_font();

    push_font(headerFontScale);
    if (ImGui::TreeNode("Scene objects"))
    {
      pop_font();
      objects_section();
      ImGui::TreePop();
    }
    else
      pop_font();

    push_font(headerFontScale);
    if (ImGui::TreeNode("Materials"))
    {
      pop_font();
      materials_section();
      ImGui::TreePop();
    }
    else
      pop_font();

    push_font(headerFontScale);
    if (ImGui::TreeNode("Textures"))
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
        check(ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mPlanes[i].distance, 0.01f, -100.0f, 100.0f, "%.2f"));
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
    static char filename[256] = "screenshot.png";
    ImGui::InputText("###RenderToFileName", filename, 256);
    ImGui::SameLine();
    if (ImGui::Button("Render to file"))
      rt::get()->render_to_image(filename);
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

  void gui::setup_style(bool bStyleDark, float alpha)
    {
      ImGuiStyle& style = ImGui::GetStyle();

      // light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
      style.Alpha = 1.0f;
      style.FrameRounding = 3.0f;
      style.Colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
      style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
      style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
      style.Colors[ImGuiCol_PopupBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
      style.Colors[ImGuiCol_Border]                = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
      style.Colors[ImGuiCol_BorderShadow]          = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
      style.Colors[ImGuiCol_FrameBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
      style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
      style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
      style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
      style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
      style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
      style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
      style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
      style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
      style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
      style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
      style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
      style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
      style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
      style.Colors[ImGuiCol_Button]                = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
      style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
      style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
      style.Colors[ImGuiCol_Header]                = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
      style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
      style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
      style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
      style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
      style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
      style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
      style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
      style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
      style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
      style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);

      if (bStyleDark)
      {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
          ImVec4& col = style.Colors[i];
          float H, S, V;
          ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

          if (S < 0.1f)
          {
            V = 1.0f - V;
          }
          ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
          if (col.w < 1.00f)
          {
            col.w *= alpha;
          }
        }
      }
      else
      {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
          ImVec4& col = style.Colors[i];
          if (col.w < 1.00f)
          {
            col.x *= alpha;
            col.y *= alpha;
            col.z *= alpha;
            col.w *= alpha;
          }
        }
      }
    }

} // namespace raytracing
