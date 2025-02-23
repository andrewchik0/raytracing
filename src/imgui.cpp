#include "imgui.h"

#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include "rt.h"

namespace raytracing
{
  bool imgui::init()
  {
    bool result = ImGui::SFML::Init(rt::get()->mWindow);
    ImGuiIO& io = ImGui::GetIO();
    mFont = io.Fonts->AddFontFromFileTTF("assets/Roboto.ttf", 16.0f);
    ImGui::SFML::UpdateFontTexture();
    SetupImGuiStyle(true, 0.75f);
    return result;
  }

  void imgui::update()
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

  void imgui::general_tab()
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
      ImGui::Text("Frame time:");
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
      float average = 0.0f;
      float max = 0.0f;
      for (int n = 0; n < IM_ARRAYSIZE(values); n++)
        average += values[n], max = max > values[n] ? max : values[n];
      average /= (float)IM_ARRAYSIZE(values);
      char overlay[32];
      sprintf(overlay, "average: %f", average);
      ImGui::PlotLines("###FrameTime", values, IM_ARRAYSIZE(values), values_offset, overlay, 0, max * 1.2f, ImVec2(mGuiWidth - 15, 40.0f));
    }

    ImGui::Separator();
    ImGui::Checkbox("FXAA", &rt::get()->mRender.mUseFXAA);
    static bool vSync = false;
    if (ImGui::Checkbox("V-Sync", &vSync))
      rt::get()->mWindow.setVerticalSyncEnabled(vSync);
    int min = 1;
    ImGui::DragScalar("Samples Count", ImGuiDataType_U32, &rt::get()->mRender.mSamplesCount, 1, &min);
    ImGui::DragScalar("Bounces Count", ImGuiDataType_U32, &rt::get()->mRender.mBouncesCount, 1, &min);
    ImGui::Separator();
    if (ImGui::Button("Reload shaders"))
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

  void imgui::scene_tab()
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
      ImGui::DragFloat3("Camera position", &rt::get()->mCamera.mPosition.x, 0.01, 0, 0, "%.2f");
      ImGui::DragFloat3("Camera direction", &rt::get()->mCamera.mDirection.x, 0.01, -1.0, 1.0, "%.2f");
      rt::get()->mCamera.mDirection = normalize(rt::get()->mCamera.mDirection);
      ImGui::DragFloat("Gamma", &rt::get()->mRender.mGamma, 0.01, 0.01, 100, "%.2f");
      ImGui::DragFloat("Exposure", &rt::get()->mRender.mExposure, 0.01, 0.01, 100, "%.2f");
      ImGui::DragFloat("Blur size", &rt::get()->mRender.mBlurSize, 0.1, 0, 100, "%.1f");
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

  void imgui::objects_section()
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
        ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mSpheres[i].center.x, 0.01f, -100.0f, 100.0f, "%.2f");
        label = "Radius###SphereRadius" + std::to_string(i);
        ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mSpheres[i].radius, 0.01f, 0.01f, 100.0f, "%.2f");
        label = "Material ID##SphereMaterialID" + std::to_string(i);
        ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &rt::get()->mRender.mSpheres[i].materialIndex);
        label = "Delete###SphereDelete" + std::to_string(i);
        if (ImGui::Button(label.c_str()))
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
        ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mPlanes[i].normal.x, 0.01f, -1.0f, 1.0f, "%.2f");
        label = "Distance###PlaneDistance" + std::to_string(i);
        ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mPlanes[i].distance, 0.01f, -100.0f, 100.0f, "%.2f");
        label = "Material ID##PlaneMaterialID" + std::to_string(i);
        ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &rt::get()->mRender.mPlanes[i].materialIndex);
        label = "Delete###PlaneDelete" + std::to_string(i);
        if (ImGui::Button(label.c_str()))
          rt::get()->delete_plane(i);
        ImGui::TreePop();
      }
    }
    for (size_t i = 0; i < rt::get()->mRender.mTrianglesCount; ++i)
    {
      label = rt::get()->mRender.mTrianglesAdditional[i].name + "###Triangle" + std::to_string(i);
      if (ImGui::TreeNode(label.c_str()))
      {
        label = "Name###TriangleName" + std::to_string(i);
        ImGui::InputText(label.c_str(), &rt::get()->mRender.mTrianglesAdditional[i].name);
        label = "Vertex 1###TriangleVertex1" + std::to_string(i);
        ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mTriangles[i].a.x, 0.01f);
        label = "Vertex 2###TriangleVertex2" + std::to_string(i);
        ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mTriangles[i].b.x, 0.01f);
        label = "Vertex 3###TriangleVertex3" + std::to_string(i);
        ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mTriangles[i].c.x, 0.01f);
        label = "Material ID##TriangleMaterialID" + std::to_string(i);
        ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &rt::get()->mRender.mTriangles[i].materialIndex);
        label = "Delete###TriangleDelete" + std::to_string(i);
        if (ImGui::Button(label.c_str()))
          rt::get()->delete_triangle(i);
        ImGui::TreePop();
      }
    }

    if (ImGui::Button("+ Add object"))
      mAddItemOpened = !mAddItemOpened;

    if (mAddItemOpened)
      add_item_window();
  }

  void imgui::add_item_window()
  {
    ImGui::Begin("Add object");

    const char* items[] = {"Sphere", "Plane", "Triangle"};
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

    static TriangleObject triangle{};
    if (currentItem && strncmp(currentItem, "Triangle", 8) == 0)
    {
      std::string label = "Vertex 1###TriangleVertex1New";
      ImGui::DragFloat3(label.c_str(), &triangle.a.x, 0.01f);
      label = "Vertex 2###TriangleVertex2New";
      ImGui::DragFloat3(label.c_str(), &triangle.b.x, 0.01f);
      label = "Vertex 3###TriangleVertex3New";
      ImGui::DragFloat3(label.c_str(), &triangle.c.x, 0.01f);
      label = "Material ID##TriangleMaterialID";
      ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &triangle.materialIndex);
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

      if (strncmp(currentItem, "Triangle", 8) == 0)
        rt::get()->add_triangle("Triangle", triangle);

      currentItem = nullptr;
    }
    ImGui::End();
  }

  void imgui::materials_section()
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
        ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mMaterials[i].albedo.x, 0.01f, 0.0f, 1.0f, "%.2f");
        label = "Roughness###Roughness" + std::to_string(i);
        ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mMaterials[i].roughness, 0.01f, 0.0f, 1.0f, "%.2f");
        label = "Emissivity###Emissivity" + std::to_string(i);
        ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mMaterials[i].emissivity.x, 0.01f, 0.0f, 100.0f, "%.2f");
        label = "Texture ID###TextureID" + std::to_string(i);
        ImGui::InputInt(label.c_str(), &rt::get()->mRender.mMaterials[i].textureIndex);
        label = "Normal Texture ID###NormalTextureID" + std::to_string(i);
        ImGui::InputInt(label.c_str(), &rt::get()->mRender.mMaterials[i].normalTextureIndex);
        label = "Metallic Texture ID###MetallicTextureID" + std::to_string(i);
        ImGui::InputInt(label.c_str(), &rt::get()->mRender.mMaterials[i].metallicTextureIndex);
        label = "Texture Coordinates Multiplier###TextureCoordinatesMultiplier" + std::to_string(i);
        ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mMaterials[i].textureCoordinatesMultiplier, 0.01f, 0.01f,
                         100.0f, "%.2f");
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

  void imgui::textures_section()
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


  void imgui::render_tab()
  {
    static char filename[256] = "screenshot.png";
    ImGui::InputText("###RenderToFileName", filename, 256);
    ImGui::SameLine();
    if (ImGui::Button("Render to file"))
      rt::get()->render_to_image(filename);
  }

  void imgui::push_font(float scale)
  {
    mOldFontSize = ImGui::GetFont()->Scale;
    ImGui::GetFont()->Scale *= scale;
    ImGui::PushFont(ImGui::GetFont());
  }

  void imgui::pop_font()
  {
    ImGui::GetFont()->Scale = mOldFontSize;
    ImGui::PopFont();
  }

} // namespace raytracing
