#pragma once

#include "pch.h"

#include "../shaders/uniforms.h"
#include "assimp/matrix4x4.h"

struct aiMesh;
struct aiNode;
struct aiScene;

namespace raytracing
{
  class model
  {
  public:
    void load_from_file(std::filesystem::path file);

    glm::vec3 mMin, mMax;
    std::vector<TriangleObject> mTriangles;
    std::vector<Vertex> mVertices;
  private:

    void process_node(aiNode* node, const aiScene* scene);
    void process_mesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4& transform);
  };
}
