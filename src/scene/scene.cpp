#include "scene.h"

#include <glm/ext/matrix_transform.hpp>

#include "rt.h"

namespace raytracing
{
  
  scene::scene()
    : mSpheres(), mPlanes(), mMaterials(), mMandelbulb()
  {
    mWater.isShown = false;
    mWater.animationTime = 0.0f;
    mWater.albedo = glm::vec3(0.3, 0.6, 0.7);
    mWater.amplitude = 5.49;
    mWater.roughness = 0;
    mWater.speed = 1;
    mWater.samples = 256;
    mWater.size = 0.05;
  }

  void scene::load_models()
  {
    std::vector<std::future<void>> modelFutures;

    for (auto& m : mModels)
    {
      modelFutures.push_back(rt::get()->thread_pool().enqueue([&]
      {
        if (m.load() == status::success)
        {
          m.index = mTriangles.size();
          mTriangles.emplace_back(std::move(m.mTriangles));
          mVertices.emplace_back(std::move(m.mVertices));
          mModelMaterials.insert(mModelMaterials.end(), m.mMaterialIndices.begin(), m.mMaterialIndices.end());
          mModelTextures.insert(mModelTextures.end(), m.mTextureIndices.begin(), m.mTextureIndices.end());
        }
      }));
    }

    for (auto& model : modelFutures)
      model.wait();
  }

  void scene::update(const float deltaTime)
  {
    mCamera.update(deltaTime);
    for (auto& m : mModels)
    {
      glm::mat4 model = glm::mat4(1.0f);

      model = glm::translate(model, m.mTranslate);

      model = glm::rotate(model, glm::radians(m.mRotation.x), glm::vec3(1, 0, 0));
      model = glm::rotate(model, glm::radians(m.mRotation.y), glm::vec3(0, 1, 0));
      model = glm::rotate(model, glm::radians(m.mRotation.z), glm::vec3(0, 0, 1));

      model = glm::scale(model, m.mScale);

      mBVHEntries[m.index].transform = model;
    }
  }

  void scene::add_sphere(const std::string& name, const SphereObject& object)
  {
    if (mSpheresCount >= MAX_SPHERES)
      return;
    mSpheresAdditional[mSpheresCount].name = name;
    mSpheres[mSpheresCount++] = object;
  }
  void scene::add_plane(const std::string& name, const PlaneObject& object)
  {
    if (mPlanesCount >= MAX_PLANES)
      return;
    mPlanesAdditional[mPlanesCount].name = name;
    mPlanes[mPlanesCount++] = object;
  }
  void scene::add_material(const std::string& name, const Material& material)
  {
    if (mMaterialsCount >= MAX_MATERIALS)
      return;
    mMaterialsAdditional[mMaterialsCount].name = name;
    mMaterials[mMaterialsCount++] = material;
  }
  void scene::add_point_light(const PointLight& light)
  {
    mPointLights.emplace_back(light);
  }
  void scene::delete_sphere(size_t index)
  {
    for (size_t i = index; i < mSpheresCount; ++i)
      mSpheres[i] = mSpheres[i + 1];
    mSpheresCount--;
  }
  void scene::delete_plane(size_t index)
  {
    for (size_t i = index; i < mPlanesCount; ++i)
      mPlanes[i] = mPlanes[i + 1];
    mPlanesCount--;
  }
  void scene::delete_material(size_t index)
  {
    for (size_t i = index; i < mMaterialsCount; ++i)
      mMaterials[i] = mMaterials[i + 1];
    mMaterialsCount--;
  }
  void scene::delete_point_light(size_t index)
  {
    for (size_t i = index; i < mPointLights.size(); ++i)
      mPointLights[i] = mPointLights[i + 1];
    mPointLights.erase(mPointLights.end() - 1);
  }

  void scene::add_model(const std::string& filename)
  {
    model m;
    m.mFilename = filename;
    mModels.push_back(std::move(m));
  }

  void scene::add_grid(grid&& grid)
  {
    mTriangles.emplace_back(std::move(grid.mTriangles));
    mVertices.emplace_back(std::move(grid.mVertices));
    mGrids.emplace_back(std::move(grid));
  }

} // namespace raytracing
