#pragma once

#include "pch.h"

#include "shaders/uniforms.h"

#include <assimp/matrix4x4.h>

enum aiTextureType : int;
struct aiMesh;
struct aiNode;
struct aiScene;
struct aiMaterial;

namespace raytracing
{
  class model
  {
  public:
    model() = default;

    [[nodiscard]] status load_from_file(const std::filesystem::path& file);
    [[nodiscard]] status load();

    std::string mFilename;
    glm::vec3 mMin, mMax;
    std::vector<glm::ivec4> mTriangles;
    std::vector<Vertex> mVertices;
    std::filesystem::path mBasePath;
    std::vector<uint32_t> mMaterialIndices;
    std::vector<uint32_t> mTextureIndices;
    glm::vec3
      mScale = glm::vec3(1.0),
      mTranslate = glm::vec3(0.0),
      mRotation = glm::vec3(0.0);
    size_t index = 0;
  private:

    uint32_t process_material(const aiMaterial* material);
    void process_node(aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform);
    void process_mesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4& transform);
    void process_lights(const aiScene* scene);
    static aiNode* find_node(const aiNode* root, const std::string& name);
    static aiMatrix4x4 get_node_transform(const aiNode* node);
    static std::string get_material_name(const aiMaterial* material);
    static std::string get_texture_path(const aiMaterial* mat, aiTextureType type);

    std::vector<PointLight> mPointLights;
  };
}
