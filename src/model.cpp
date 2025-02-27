#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>

#include "assimp/pbrmaterial.h"
#include "glm/gtc/type_ptr.hpp"
#include "rt.h"

namespace raytracing
{
  void model::load_from_file(std::filesystem::path file)
  {
    Assimp::Importer importer;

    mBasePath = file.parent_path();

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
    uint32_t materialIndex = 0;
    if (mesh->mMaterialIndex >= 0)
    {
      aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];
      materialIndex = process_material(aiMat);
    }

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

      if (mesh->HasTangentsAndBitangents())
      {
        glm::vec4 tangent = glm::vec4(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 0.0f);
        glm::vec4 bitangent = glm::vec4(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z, 0.0f);
        vertex.tangent = glm::normalize(meshTransform * tangent);
        vertex.bitangent = glm::normalize(meshTransform * bitangent);
      }
      else
      {
        vertex.tangent = glm::vec4(1, 0, 0, 0);
        vertex.bitangent = glm::vec4(0, 1, 0, 0);
      }

      if (mesh->HasTextureCoords(0))
      {
        vertex.tangent.w = mesh->mTextureCoords[0][i].x; // Store U in tangent.w
        vertex.bitangent.w = mesh->mTextureCoords[0][i].y; // Store V in bitangent.w
      }
      else
      {
        vertex.tangent.w = 0.0f;
        vertex.bitangent.w = 0.0f;
      }

      mVertices.push_back(vertex);
    }

    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
      aiFace face = mesh->mFaces[i];
      TriangleObject triangle;
      triangle.indices[0] = face.mIndices[0] + vertexOffset;
      triangle.indices[1] = face.mIndices[1] + vertexOffset;
      triangle.indices[2] = face.mIndices[2] + vertexOffset;
      triangle.materialIndex = materialIndex;
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

  uint32_t model::process_material(const aiMaterial* material)
  {
    Material mat;

    aiColor3D diffuse(1.0f, 1.0f, 1.0f);
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
    {
      mat.albedo = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
    }

    aiColor3D emissivity(0, 0, 0);
    if (material->Get(AI_MATKEY_COLOR_EMISSIVE, diffuse) == AI_SUCCESS)
    {
      mat.emissivity = glm::vec3(emissivity.r, emissivity.g, emissivity.b);
    }

    std::string baseColorTexture = get_texture_path(material, aiTextureType_DIFFUSE);
    std::string metallicTexture = get_texture_path(material, aiTextureType_METALNESS);
    std::string normalTexture = get_texture_path(material, aiTextureType_NORMALS);
    std::string normalPBR = get_texture_path(material, aiTextureType_NORMAL_CAMERA);
    std::string heightTexture = get_texture_path(material, aiTextureType_HEIGHT);


    if (baseColorTexture.size() > 0)
      mat.textureIndex = rt::get()->mRender.mTextures.add_texture((mBasePath / baseColorTexture).string());
    if (metallicTexture.size() > 0)
      mat.metallicTextureIndex = rt::get()->mRender.mTextures.add_texture((mBasePath / metallicTexture).string());

    if (normalTexture.size() > 0)
      mat.normalTextureIndex = rt::get()->mRender.mTextures.add_texture((mBasePath / normalTexture).string());
    else if (normalPBR.size() > 0)
      mat.normalTextureIndex = rt::get()->mRender.mTextures.add_texture((mBasePath / normalPBR).string());
    else if (heightTexture.size() > 0)
      mat.normalTextureIndex = rt::get()->mRender.mTextures.add_texture((mBasePath / heightTexture).string());

    rt::get()->add_material(get_material_name(material), mat);
    return rt::get()->mRender.mMaterialsCount - 1;
  }

  std::string model::get_texture_path(const aiMaterial* mat, aiTextureType type)
  {
    if (mat->GetTextureCount(type) > 0)
    {
      aiString path;
      if (mat->GetTexture(type, 0, &path) == AI_SUCCESS)
      {
        return std::string(path.C_Str());
      }
    }
    return ""; // No texture found
  }

  std::string model::get_material_name(const aiMaterial* material)
  {
    aiString name;
    if (material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
    {
      return std::string(name.C_Str());
    }
    return "Unnamed Material " + std::to_string(rt::get()->mRender.mMaterialsCount);
  }
} // namespace raytracing
