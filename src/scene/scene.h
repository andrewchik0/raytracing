#pragma once

#include "bounding_volume_builder.h"
#include "camera.h"
#include "render/grid.h"
#include "render/model.h"

namespace raytracing
{
  struct object_additional
  {
    std::string name;
  };

  class scene
  {
  public:
    scene();
    ~scene() = default;

    std::string mSceneFilename = "";
    std::string mSkyFilename;

    camera mCamera;
    std::vector<model> mModels;
    std::vector<grid> mGrids;

    void load_models();

    void update(float deltaTime);

    void add_model(const std::string& filename, const glm::mat4& matrix = glm::mat4(1.0f));
    void add_grid(grid&& grid);

    void add_sphere(const std::string& name, const SphereObject& object);
    void add_plane(const std::string& name, const PlaneObject& object);
    void add_material(const std::string& name, const Material& material);
    void delete_sphere(size_t index);
    void delete_plane(size_t index);
    void delete_material(size_t index);

    size_t
      mSpheresCount = 0,
      mPlanesCount = 0,
      mMaterialsCount = 0;
  private:

    bounding_volume_builder mBoundingVolumeBuilder;

    std::array<SphereObject, MAX_SPHERES> mSpheres;
    std::array<PlaneObject, MAX_PLANES> mPlanes;
    std::array<Material, MAX_MATERIALS> mMaterials;

    std::vector<std::vector<Vertex>> mVertices;
    std::vector<std::vector<ivec4>> mTriangles;

    Water mWater;
    Mandelbulb mMandelbulb;

    struct
    {
      int count = 0;
      std::vector<int> indices;
    } mBVHEntries;
    std::vector<BoundingVolume> mBoundingVolumes;

    // Additional data stored separately in order to easily pass main data to the shader
    std::array<object_additional, MAX_SPHERES> mSpheresAdditional;
    std::array<object_additional, MAX_PLANES> mPlanesAdditional;
    std::array<object_additional, MAX_MATERIALS> mMaterialsAdditional;

    friend class render;
    friend class rt;
    friend class bounding_volume_builder;
    friend class gui;
    friend class serializer;
    friend class final_render;
  };
}