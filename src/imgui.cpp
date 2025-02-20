#include "imgui.h"

#include <imgui-SFML.h>
#include <imgui.h>

#include "rt.h"

namespace raytracing
{
  bool imgui::init()
  {
    return ImGui::SFML::Init(rt::get()->mWindow);
  }

  void imgui::update()
  {
    rt::get()->mElapsedTime = rt::get()->mClock.getElapsedTime();
    rt::get()->mTime += rt::get()->mElapsedTime.asSeconds();

    ImGui::SFML::Update(rt::get()->mWindow, rt::get()->mClock.restart());

    ImGui::Begin("Stats");
    ImGui::Text("Frame time: %.3f ms", static_cast<float>(rt::get()->mElapsedTime.asMicroseconds()) / 1000.0);
    ImGui::Text("FPS: %.1f", 1.0 / rt::get()->mElapsedTime.asSeconds());
    ImGui::Separator();
    ImGui::Checkbox("Enable FXAA", &rt::get()->mRender.mUseFXAA);
    ImGui::DragScalar("Samples Count", ImGuiDataType_U32, &rt::get()->mRender.mSamplesCount);
    ImGui::DragScalar("Bounces Count", ImGuiDataType_U32, &rt::get()->mRender.mBouncesCount);
    ImGui::Separator();
    ImGui::DragFloat3("Light Direction", &rt::get()->mRender.mLightDirection.x, 0.01f, -1.0f, 1.0f, "%.2f");
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

    ImGui::Separator();
    if (ImGui::Button("+ Add object"))
      mAddItemOpened = !mAddItemOpened;

    ImGui::Separator();

    if (ImGui::Button("Edit materials..."))
      mMaterialsOpened = !mMaterialsOpened;

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

    const char* items[] = { "Sphere", "Plane" };
    static const char* currentItem = nullptr;

    if (ImGui::BeginCombo("Object Type##combo", currentItem))
    {
      for (auto & item : items)
      {
        bool is_selected = (currentItem == item);
        if (ImGui::Selectable(item, is_selected))
          currentItem = item;
        if (is_selected)
        ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    static SphereObject sphere { { 0, 0, 0 }, 1, { 0, 0, 0 }, 0 };
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

    static PlaneObject plane { { 0, 1, 0 }, 0, { 0, 0, 0 }, 0 };
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
        rt::get()->add_sphere(sphere);

      if (strncmp(currentItem, "Plane", 5) == 0)
        rt::get()->add_plane(plane);

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
        label = "Delete###MaterialDelete" + std::to_string(i);
        if (ImGui::Button(label.c_str()))
          rt::get()->delete_material(i);
      }
      ImGui::Separator();
    }

    if (ImGui::Button("+ Add Material"))
      mAddMaterialOpened = !mAddMaterialOpened;
    if (mAddMaterialOpened)
      add_material_window();

    ImGui::End();
  }

  void imgui::add_material_window()
  {
    ImGui::Begin("Add material");

    static Material material;
    ImGui::DragFloat3("Albedo###NewMaterialAlbedo", &material.albedo.x, 0.01f, 0.0f, 1.0f, "%.2f");
    ImGui::DragFloat("Roughness###NewMaterialRoughness", &material.roughness, 0.01f, 0.0f, 1.0f, "%.2f");

    if (ImGui::Button("Cancel###NewMaterialCancel"))
      mAddMaterialOpened = false;
    ImGui::SameLine(0.0f);
    if (ImGui::Button("Add###NewMaterialAdd"))
    {
      rt::get()->add_material(material);
      mAddMaterialOpened = false;
    }
    ImGui::End();
  }

}
