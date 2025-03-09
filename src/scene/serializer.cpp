#include "serializer.h"

#include <nfd.h>
#include <yaml-cpp/yaml.h>
#include <fstream>

#include "../rt.h"

namespace YAML
{
  template<>
  struct convert<glm::vec3>
  {
    static Node encode(const glm::vec3& v)
    {
      Node node;
      node.push_back(v.x);
      node.push_back(v.y);
      node.push_back(v.z);
      return node;
    }

    static bool decode(const Node& node, glm::vec3& v)
    {
      if(!node.IsSequence() || node.size() != 3)
      {
        return false;
      }

      v.x = node[0].as<float>();
      v.y = node[1].as<float>();
      v.z = node[2].as<float>();
      return true;
    }
  };

  template<>
  struct convert<glm::mat4>
  {
    static Node encode(const glm::mat4& m)
    {
      Node node;
      node.push_back(m[0][0]);
      node.push_back(m[0][1]);
      node.push_back(m[0][2]);
      node.push_back(m[0][3]);
      node.push_back(m[1][0]);
      node.push_back(m[1][1]);
      node.push_back(m[1][2]);
      node.push_back(m[1][3]);
      node.push_back(m[2][0]);
      node.push_back(m[2][1]);
      node.push_back(m[2][2]);
      node.push_back(m[2][3]);
      node.push_back(m[3][0]);
      node.push_back(m[3][1]);
      node.push_back(m[3][2]);
      node.push_back(m[3][3]);
      return node;
    }

    static bool decode(const Node& node, glm::mat4& m)
    {
      if(!node.IsSequence() || node.size() != 16)
      {
        return false;
      }

      m[0][0] = node[0].as<float>();
      m[0][1] = node[1].as<float>();
      m[0][2] = node[2].as<float>();
      m[0][3] = node[3].as<float>();

      m[1][0] = node[4].as<float>();
      m[1][1] = node[5].as<float>();
      m[1][2] = node[6].as<float>();
      m[1][3] = node[7].as<float>();

      m[2][0] = node[8].as<float>();
      m[2][1] = node[9].as<float>();
      m[2][2] = node[10].as<float>();
      m[2][3] = node[11].as<float>();

      m[3][0] = node[12].as<float>();
      m[3][1] = node[13].as<float>();
      m[3][2] = node[14].as<float>();
      m[3][3] = node[15].as<float>();
      return true;
    }
  };

  Emitter& operator<<(Emitter& out, const glm::vec3& v)
  {
    out << Flow;
    out << BeginSeq << v.x << v.y << v.z << EndSeq;
    return out;
  }

  Emitter& operator<<(Emitter& out, const glm::mat4& m)
  {
    out << Flow;
    out << BeginSeq
      << m[0][0]
      << m[0][1]
      << m[0][2]
      << m[0][3]
      << m[1][0]
      << m[1][1]
      << m[1][2]
      << m[1][3]
      << m[2][0]
      << m[2][1]
      << m[2][2]
      << m[2][3]
      << m[3][0]
      << m[3][1]
      << m[3][2]
      << m[3][3]
      << EndSeq;
    return out;
  }
}

namespace raytracing
{
  void serializer::load(const std::filesystem::path& filename)
  {
    rt::get()->mThreadPool.restart();
    rt::get()->mModelsLoading = false;
    rt::get()->mTexturesLoading = false;
    rt::get()->mBVHLoading = false;
    rt::get()->mRender.reset_accumulation();
    rt::get()->mRender.clear();
    rt::get()->mScene.mModels.clear();
    rt::get()->mScene.mSceneFilename = filename.string();
    rt::get()->mScene.mBoundingVolumes.clear();
    rt::get()->mScene.mTriangles.clear();
    rt::get()->mScene.mVertices.clear();
    rt::get()->mScene.mSceneFilename.resize(256, 0);
    rt::get()->mScene.mSpheresCount = 0;
    rt::get()->mScene.mPlanesCount = 0;
    rt::get()->mScene.mMaterialsCount = 0;
    rt::get()->mRender.mTextures.mTextureFilenames.clear();

    YAML::Node scene = YAML::LoadFile(filename.string());

    {
      if (scene["sky_filename"]) rt::get()->mScene.mSkyFilename = scene["sky_filename"].as<std::string>();
      if (scene["gamma"]) rt::get()->mRender.mGamma = scene["gamma"].as<float>();
      if (scene["exposure"]) rt::get()->mRender.mExposure = scene["exposure"].as<float>();
      if (scene["blur_radius"]) rt::get()->mRender.mBlurSize = scene["blur_radius"].as<float>();

      if (scene["camera"])
      {
        auto camera = scene["camera"].as<YAML::Node>();
        if (camera["fov"]) rt::get()->mScene.mCamera.mFovY = camera["fov"].as<float>();
        if (camera["position"]) rt::get()->mScene.mCamera.mPosition = camera["position"].as<glm::vec3>();
        if (camera["direction"]) rt::get()->mScene.mCamera.mDirection = camera["direction"].as<glm::vec3>();
      }

      if (scene["objects"])
      {
        auto objects = scene["objects"].as<YAML::Node>();
        size_t i = 0;
        for(YAML::const_iterator it = objects.begin(); it != objects.end(); ++it, ++i)
        {
          auto object = it->as<YAML::Node>();

          if (!object["type"]) continue;

          if (strcmp(object["type"].as<std::string>().c_str(), "sphere") == 0)
          {
            SphereObject sphere {};
            if (object["position"]) sphere.center = object["position"].as<glm::vec3>();
            if (object["radius"]) sphere.radius = object["radius"].as<float>();
            if (object["materialIndex"]) sphere.materialIndex = object["materialIndex"].as<int>();
            std::string name = "Sphere " + std::to_string(rt::get()->mScene.mSpheresCount + 1);
            if (object["name"]) name = object["name"].as<std::string>();
            rt::get()->mScene.add_sphere(name, sphere);
          }
          if (strcmp(object["type"].as<std::string>().c_str(), "plane") == 0)
          {
            PlaneObject plane {};
            if (object["normal"]) plane.normal = object["normal"].as<glm::vec3>();
            if (object["distance"]) plane.distance = object["distance"].as<float>();
            if (object["materialIndex"]) plane.materialIndex = object["materialIndex"].as<int>();
            std::string name = "Plane " + std::to_string(rt::get()->mScene.mPlanesCount + 1);
            if (object["name"]) name = object["name"].as<std::string>();
            rt::get()->mScene.add_plane(name, plane);
          }
          if (strcmp(object["type"].as<std::string>().c_str(), "model") == 0 && object["filename"])
          {
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            if (object["matrix"]) modelMatrix = object["matrix"].as<glm::mat4>();
            rt::get()->mScene.add_model(object["filename"].as<std::string>(), modelMatrix);
          }
        }
      }

      if (scene["materials"])
      {
        auto materials = scene["materials"].as<YAML::Node>();
        size_t i = 0;
        for(YAML::const_iterator it = materials.begin(); it != materials.end(); ++it, ++i)
        {
          auto materialNode = it->as<YAML::Node>();

          Material material {};
          if (materialNode["albedo"]) material.albedo = materialNode["albedo"].as<glm::vec3>();
          if (materialNode["emissivity"]) material.emissivity = materialNode["emissivity"].as<glm::vec3>();
          if (materialNode["roughness"]) material.roughness = materialNode["roughness"].as<float>();
          if (materialNode["texture_id"]) material.textureIndex = materialNode["texture_id"].as<int>();
          if (materialNode["normal_texture_id"]) material.normalTextureIndex = materialNode["normal_texture_id"].as<int>();
          if (materialNode["metallic_texture_id"]) material.metallicTextureIndex = materialNode["metallic_texture_id"].as<int>();
          if (materialNode["texture_coordinates_multiplier"]) material.textureCoordinatesMultiplier = materialNode["texture_coordinates_multiplier"].as<float>();
          std::string name = "Material " + std::to_string(rt::get()->mScene.mMaterialsCount + 1);
          if (materialNode["name"]) name = materialNode["name"].as<std::string>();
          rt::get()->mScene.add_material(name, material);
        }
      }

      if (scene["textures"])
      {
        auto textures = scene["textures"].as<YAML::Node>();
        for(YAML::const_iterator it = textures.begin(); it != textures.end(); ++it)
        {
          auto texture = it->as<std::string>();
          rt::get()->mRender.mTextures.add_texture(texture);
        }
      }
    }
    rt::get()->load_async();
  }


  void serializer::load()
  {
    nfdu8char_t* outPath;
    const nfdu8filteritem_t filters[1] = {{"YAML Files", "yaml,yml"}};
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = 1;
    auto defaultPath = (std::filesystem::current_path() / "scenes").string();
    args.defaultPath = defaultPath.c_str();
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
    if (result != NFD_OKAY)
      return;

    rt::get()->mLoaded = false;
    load(outPath);
    NFD_FreePathU8(outPath);
  }

  void serializer::save(const std::filesystem::path& filename)
  {
    YAML::Emitter out;
    out << YAML::BeginMap;

    out << YAML::Key << "exposure" << YAML::Value << rt::get()->mRender.mExposure;
    out << YAML::Key << "gamma" << YAML::Value << rt::get()->mRender.mGamma;
    out << YAML::Key << "blur_radius" << YAML::Value << rt::get()->mRender.mBlurSize;
    out << YAML::Key << "sky_filename" << YAML::Value << rt::get()->mScene.mSkyFilename;

    {
      out << YAML::Key << "camera";

      out << YAML::BeginMap;
      out << YAML::Key << "position" << YAML::Value << rt::get()->mScene.mCamera.mPosition;
      out << YAML::Key << "direction" << YAML::Value << rt::get()->mScene.mCamera.mDirection;
      out << YAML::Key << "fov" << YAML::Value << rt::get()->mScene.mCamera.mFovY;
      out << YAML::EndMap;
    }

    {
      out << YAML::Key << "objects";

      out << YAML::BeginSeq;
      for (size_t i = 0; i < rt::get()->mScene.mSpheresCount; i++)
      {
        out << YAML::BeginMap;
        out << YAML::Key << "type" << YAML::Value << "sphere";
        out << YAML::Key << "name" << YAML::Value << rt::get()->mScene.mSpheresAdditional[i].name;
        out << YAML::Key << "position" << YAML::Value << rt::get()->mScene.mSpheres[i].center;
        out << YAML::Key << "radius" << YAML::Value << rt::get()->mScene.mSpheres[i].radius;
        out << YAML::Key << "materialIndex" << YAML::Value << rt::get()->mScene.mSpheres[i].materialIndex;
        out << YAML::EndMap;
      }
      for (size_t i = 0; i < rt::get()->mScene.mPlanesCount; i++)
      {
        out << YAML::BeginMap;
        out << YAML::Key << "type" << YAML::Value << "plane";
        out << YAML::Key << "name" << YAML::Value << rt::get()->mScene.mPlanesAdditional[i].name;
        out << YAML::Key << "normal" << YAML::Value << rt::get()->mScene.mPlanes[i].normal;
        out << YAML::Key << "distance" << YAML::Value << rt::get()->mScene.mPlanes[i].distance;
        out << YAML::Key << "materialIndex" << YAML::Value << rt::get()->mScene.mPlanes[i].materialIndex;
        out << YAML::EndMap;
      }
      for (auto it = rt::get()->mScene.mModels.begin(); it != rt::get()->mScene.mModels.end(); ++it)
      {
        out << YAML::BeginMap;
        out << YAML::Key << "type" << YAML::Value << "model";
        out << YAML::Key << "filename" << YAML::Value << it->mFilename.c_str();
        out << YAML::Key << "matrix" << YAML::Value << it->mModelMatrix;
        out << YAML::EndMap;
      }
      out << YAML::EndSeq;
    }

    {
      out << YAML::Key << "materials";

      out << YAML::BeginSeq;
      for (size_t i = 0; i < rt::get()->mScene.mMaterialsCount; i++)
      {
        out << YAML::BeginMap;
        out << YAML::Key << "name" << YAML::Value << rt::get()->mScene.mMaterialsAdditional[i].name;
        out << YAML::Key << "albedo" << YAML::Value << rt::get()->mScene.mMaterials[i].albedo;
        out << YAML::Key << "emissivity" << YAML::Value << rt::get()->mScene.mMaterials[i].emissivity;
        out << YAML::Key << "roughness" << YAML::Value << rt::get()->mScene.mMaterials[i].roughness;
        out << YAML::Key << "texture_coordinates_multiplier" << YAML::Value << rt::get()->mScene.mMaterials[i].textureCoordinatesMultiplier;
        out << YAML::Key << "texture_id" << YAML::Value << rt::get()->mScene.mMaterials[i].textureIndex;
        out << YAML::Key << "metallic_texture_id" << YAML::Value << rt::get()->mScene.mMaterials[i].metallicTextureIndex;
        out << YAML::Key << "normal_texture_id" << YAML::Value << rt::get()->mScene.mMaterials[i].normalTextureIndex;
        out << YAML::EndMap;
      }
      out << YAML::EndSeq;
    }

    {
      out << YAML::Key << "textures";

      out << YAML::BeginSeq << YAML::Flow;
      for (size_t i = 0; i < rt::get()->mRender.mTextures.mTextureFilenames.size(); i++)
      {
        out << rt::get()->mRender.mTextures.mTextureFilenames[i];
      }
      out << YAML::EndSeq;
    }

    {
      std::fstream file(filename, std::ios::out | std::ofstream::trunc);
      file.write(out.c_str(), out.size());
    }
  }

} // namespace raytracing
