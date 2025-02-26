#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

#include "glm/gtc/type_ptr.hpp"

namespace raytracing
{
  void model::load_from_file(std::filesystem::path file)
  {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(file.string(),
      aiProcess_CalcTangentSpace |
      aiProcess_JoinIdenticalVertices |
      aiProcess_Triangulate |
      aiProcess_GenSmoothNormals |
      aiProcess_ValidateDataStructure |
      aiProcess_RemoveRedundantMaterials |
      aiProcess_GenUVCoords |
      aiProcess_GenBoundingBoxes
    );

    if (!scene || !scene->mRootNode)
    {
      std::cerr << "Failed to load object: " << file << std::endl;
    }

    process_node(scene->mRootNode, scene);
  }

  void model::process_mesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4& transform)
  {
    size_t vertexOffset = mVertices.size();

    mVertices.reserve(mVertices.size() + mesh->mNumVertices);
    mTriangles.reserve(mTriangles.size() + mesh->mNumFaces);

    glm::mat4 meshTransform = glm::transpose(glm::make_mat4(&transform.a1)); // Assimp uses row-major order

    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
      Vertex vertex;
      glm::vec4 pos = glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f);
      glm::vec4 normal = glm::vec4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0.0f);

      vertex.position = meshTransform * pos;
      vertex.normal = glm::normalize(meshTransform * normal);
      mVertices.push_back(vertex);
    }

    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
      aiFace face = mesh->mFaces[i];
      TriangleObject triangle;
      triangle.indices[0] = face.mIndices[0] + vertexOffset;
      triangle.indices[1] = face.mIndices[1] + vertexOffset;
      triangle.indices[2] = face.mIndices[2] + vertexOffset;
      mTriangles.push_back(triangle);
    }
  }


  void model::process_node(aiNode* node, const aiScene* scene)
  {
    aiMatrix4x4 nodeTransform = node->mTransformation;

    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      process_mesh(mesh, scene, nodeTransform);
    }
    for (size_t i = 0; i < node->mNumChildren; ++i)
    {
      process_node(node->mChildren[i], scene);
    }
  }
} // namespace raytracing
