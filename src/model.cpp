#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

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

  void model::process_mesh(aiMesh* mesh, const aiScene* scene)
  {
    mTriangles.reserve(mesh->mNumFaces * 3);
    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
      aiFace face = mesh->mFaces[i];
      TriangleObject triangle;
      triangle.indices[0] = face.mIndices[0];
      triangle.indices[1] = face.mIndices[1];
      triangle.indices[2] = face.mIndices[2];
      mTriangles.push_back(triangle);
    }

    mVertices.reserve(mesh->mNumVertices);
    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
      Vertex vertex;
      vertex.position = glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0);
      vertex.normal = glm::vec4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 1.0);
      mVertices.push_back(vertex);
    }

    mMax.x = mesh->mAABB.mMax.x;
    mMax.y = mesh->mAABB.mMax.y;
    mMax.z = mesh->mAABB.mMax.z;
    mMin.x = mesh->mAABB.mMin.x;
    mMin.y = mesh->mAABB.mMin.y;
    mMin.z = mesh->mAABB.mMin.z;
  }

  void model::process_node(aiNode* node, const aiScene* scene)
  {
    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      process_mesh(mesh, scene);
    }
    for (size_t i = 0; i < node->mNumChildren; ++i)
    {
      process_node(node->mChildren[i], scene);
    }
  }
} // namespace raytracing
