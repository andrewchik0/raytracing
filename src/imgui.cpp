#include "imgui.h"

#include <imgui-SFML.h>
#include <imgui.h>

#include "rt.h"

namespace raytracing
{
  bool imgui::init() { return ImGui::SFML::Init(rt::get()->mWindow); }

  void imgui::update()
  {
    rt::get()->mElapsedTime = rt::get()->mClock.getElapsedTime();
    rt::get()->mTime += rt::get()->mElapsedTime.asSeconds();

    ImGui::SFML::Update(rt::get()->mWindow, rt::get()->mClock.restart());

    ImGui::Begin("Stats");
    ImGui::Text("Frame time: %.3f ms", static_cast<float>(rt::get()->mElapsedTime.asMicroseconds()) / 1000.0);

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

    ImGui::Separator();
    ImGui::Checkbox("FXAA", &rt::get()->mRender.mUseFXAA);
    static bool vSync = false;
    if (ImGui::Checkbox("V-Sync", &vSync))
      rt::get()->mWindow.setVerticalSyncEnabled(vSync);
    ImGui::DragFloat("Gamma", &rt::get()->mRender.mGamma, 0.01, 0.01, 100, "%.2f");
    ImGui::DragFloat("Exposure", &rt::get()->mRender.mExposure, 0.01, 0.01, 100, "%.2f");
    ImGui::DragFloat("Blur size", &rt::get()->mRender.mBlurSize, 0.1, 0, 100, "%.1f");
    int min = 1;
    ImGui::DragScalar("Samples Count", ImGuiDataType_U32, &rt::get()->mRender.mSamplesCount, 1, &min);
    ImGui::DragScalar("Bounces Count", ImGuiDataType_U32, &rt::get()->mRender.mBouncesCount, 1, &min);
    static char filename[256] = "screenshot.png";
    ImGui::Separator();
    ImGui::InputText("###RenderToFileName", filename, 256);
    ImGui::SameLine();
    if (ImGui::Button("Render to file"))
      rt::get()->render_to_image(filename);
    ImGui::Separator();
    if (ImGui::Button("Reload shaders"))
    {
      rt::get()->mRender.load_shaders();
      rt::get()->mRender.resize(rt::get()->mWindowWidth, rt::get()->mWindowHeight);
    }

    if (ImGui::Button("Edit scene..."))
      mSceneOpened = !mSceneOpened;

    if (mSceneOpened)
      scene_window();

    ImGui::End();
  }

  void imgui::scene_window()
  {
    ImGui::Begin("Scene");
    std::string label;

    for (size_t i = 0; i < rt::get()->mRender.mSpheresCount; ++i)
    {
      label = "Sphere###Sphere" + std::to_string(i);
      if (ImGui::CollapsingHeader(label.c_str()))
      {
        label = "Position###SpherePosition" + std::to_string(i);
        ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mSpheres[i].center.x, 0.01f, -100.0f, 100.0f, "%.2f");
        label = "Radius###SphereRadius" + std::to_string(i);
        ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mSpheres[i].radius, 0.01f, 0.01f, 100.0f, "%.2f");
        label = "Material ID##SphereMaterialID" + std::to_string(i);
        ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &rt::get()->mRender.mSpheres[i].materialIndex);
        label = "Delete###SphereDelete" + std::to_string(i);
        if (ImGui::Button(label.c_str()))
          rt::get()->delete_sphere(i);
      }
      ImGui::Separator();
    }
    for (size_t i = 0; i < rt::get()->mRender.mPlanesCount; ++i)
    {
      label = "Plane###Plane" + std::to_string(i);
      if (ImGui::CollapsingHeader(label.c_str()))
      {
        label = "Normal###PlaneNormal" + std::to_string(i);
        ImGui::DragFloat3(label.c_str(), &rt::get()->mRender.mPlanes[i].normal.x, 0.01f, -1.0f, 1.0f, "%.2f");
        label = "Distance###PlaneDistance" + std::to_string(i);
        ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mPlanes[i].distance, 0.01f, -100.0f, 100.0f, "%.2f");
        label = "Material ID##PlaneMaterialID" + std::to_string(i);
        ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &rt::get()->mRender.mPlanes[i].materialIndex);
        label = "Delete###PlaneDelete" + std::to_string(i);
        if (ImGui::Button(label.c_str()))
          rt::get()->delete_plane(i);
      }
      ImGui::Separator();
    }
    for (size_t i = 0; i < rt::get()->mRender.mTrianglesCount; ++i)
    {
      label = "Triangle###Triangle" + std::to_string(i);
      if (ImGui::CollapsingHeader(label.c_str()))
      {
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
      }
      ImGui::Separator();
    }

    ImGui::Separator();
    if (ImGui::Button("+ Add object"))
      mAddItemOpened = !mAddItemOpened;

    ImGui::Separator();

    if (ImGui::Button("Edit materials..."))
      mMaterialsOpened = !mMaterialsOpened;

    static char filename[256] = "scenes/default.yaml";
    ImGui::Separator();
    ImGui::InputText("###SaveSceneFileName", filename, 256);
    ImGui::SameLine();
    if (ImGui::Button("Save scene"))
      rt::get()->mSceneSerializer.save(filename);
    if (ImGui::Button("Load scene..."))
      rt::get()->mSceneSerializer.load();

    if (mMaterialsOpened)
      materials_window();

    ImGui::Separator();

    if (mAddItemOpened)
      add_item_window();

    if (ImGui::Button("Close"))
      mSceneOpened = false;
    ImGui::End();
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
        rt::get()->add_sphere(sphere);

      if (strncmp(currentItem, "Plane", 5) == 0)
        rt::get()->add_plane(plane);

      if (strncmp(currentItem, "Triangle", 8) == 0)
        rt::get()->add_triangle(triangle);

      currentItem = nullptr;
    }
    ImGui::End();
  }

  void imgui::materials_window()
  {
    ImGui::Begin("Materials");

    std::string label;

    for (size_t i = 0; i < rt::get()->mRender.mMaterialsCount; ++i)
    {
      label = "Material ID: " + std::to_string(i);
      if (ImGui::CollapsingHeader(label.c_str()))
      {
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
        ImGui::DragFloat(label.c_str(), &rt::get()->mRender.mMaterials[i].textureCoordinatesMultiplier, 0.01f, 0.01f, 100.0f, "%.2f");
        label = "Delete###MaterialDelete" + std::to_string(i);
        if (ImGui::Button(label.c_str()))
          rt::get()->delete_material(i);
      }
      ImGui::Separator();
    }

    static Material material;
    if (ImGui::Button("+ Add Material"))
      rt::get()->add_material(material);

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Textures"))
    {
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

    ImGui::End();
  }

} // namespace raytracing
