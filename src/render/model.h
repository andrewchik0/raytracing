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
    model(const model&) = delete;

    [[nodiscard]] status load_from_file(std::filesystem::path file, glm::mat4 modelMatrix = glm::mat4(1.0f));

    glm::vec3 mMin, mMax;
    std::vector<glm::ivec4> mTriangles;
    std::vector<Vertex> mVertices;
    std::filesystem::path mBasePath;
    glm::mat4 mModelMatrix;
  private:

    uint32_t process_material(const aiMaterial* material);
    void process_node(aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform);
    void process_mesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4& transform);
    static std::string get_material_name(const aiMaterial* material);
    static std::string get_texture_path(const aiMaterial* mat, aiTextureType type);
  };
}
