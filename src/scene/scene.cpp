#include "scene.h"

#include "rt.h"

namespace raytracing
{
  void scene::load_models()
  {
    std::vector<std::future<void>> modelFutures;

    for (auto& m : mModels)
    {
      modelFutures.push_back(rt::get()->thread_pool().enqueue([&]
      {
        if (m.load() == status::success)
        {
          mTriangles.emplace_back(std::move(m.mTriangles));
          mVertices.emplace_back(std::move(m.mVertices));
        }
      }));
    }

    for (auto& model : modelFutures)
      model.wait();
  }

  void scene::update(float deltaTime)
  {
    mCamera.update(deltaTime);
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

  void scene::add_model(const std::string& filename, const glm::mat4& matrix)
  {
    model m;
    m.mFilename = filename;
    m.mModelMatrix = matrix;
    mModels.push_back(std::move(m));
  }

  void scene::add_grid(grid&& grid)
  {
    mTriangles.emplace_back(std::move(grid.mTriangles));
    mVertices.emplace_back(std::move(grid.mVertices));
    mGrids.emplace_back(std::move(grid));
  }

} // namespace raytracing
