#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

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

    process_node(scene->mRootNode, scene, aiMatrix4x4());
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

    glm::mat4 meshTransform = glm::transpose(glm::make_mat4(&transform.a1));

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
      glm::ivec4 triangle;
      triangle[0] = face.mIndices[0] + vertexOffset;
      triangle[1] = face.mIndices[1] + vertexOffset;
      triangle[2] = face.mIndices[2] + vertexOffset;
      triangle[3] = materialIndex;
      mTriangles.push_back(triangle);
    }
  }


  void model::process_node(aiNode* node, const aiScene* scene, const aiMatrix4x4& parentTransform)
  {
    aiMatrix4x4 nodeTransform = parentTransform * node->mTransformation;

    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      process_mesh(mesh, scene, nodeTransform);
    }
    for (size_t i = 0; i < node->mNumChildren; ++i)
    {
      process_node(node->mChildren[i], scene, nodeTransform);
    }
  }

  uint32_t model::process_material(const aiMaterial* material)
  {
    Material mat;

    aiColor3D diffuse(0.0f, 0.0f, 0.0f);
    aiColor3D ambient(0.0f, 0.0f, 0.0f);
    aiColor3D specular(0.0f, 0.0f, 0.0f);
    aiColor3D emissive(0.0f, 0.0f, 0.0f);
    aiColor3D transparent(0.0f, 0.0f, 0.0f);
    aiColor3D reflective(0.0f, 0.0f, 0.0f);
    float shininess = 0.0f;
    float shininessStrength = 0.0f;
    float opacity = 1.0f;
    float reflectivity = 0.0f;
    float refractiveIndex = 1.0f;

    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
    material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
    material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);
    material->Get(AI_MATKEY_COLOR_TRANSPARENT, transparent);
    material->Get(AI_MATKEY_COLOR_REFLECTIVE, reflective);
    material->Get(AI_MATKEY_SHININESS, shininess);
    material->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength);
    material->Get(AI_MATKEY_OPACITY, opacity);
    material->Get(AI_MATKEY_REFLECTIVITY, reflectivity);
    material->Get(AI_MATKEY_REFRACTI, refractiveIndex);

    float metallic = (reflectivity > 0.5f && specular.r > 0.5f) ? 1.0f : 0.0f;
    float roughness = 1.0f - std::sqrt(std::max(0.0f, std::min(shininess / 100.0f, 1.0f)));

    std::string baseColorTexture = get_texture_path(material, aiTextureType_DIFFUSE);
    std::string metallicTexture = get_texture_path(material, aiTextureType_METALNESS);
    std::string roughnessTexture = get_texture_path(material, aiTextureType_DIFFUSE_ROUGHNESS);
    std::string ambientOcclusionTexture = get_texture_path(material, aiTextureType_AMBIENT_OCCLUSION);
    std::string normalTexture = get_texture_path(material, aiTextureType_NORMALS);
    std::string normalPBR = get_texture_path(material, aiTextureType_NORMAL_CAMERA);
    std::string emissiveTexture = get_texture_path(material, aiTextureType_EMISSIVE);
    std::string opacityTexture = get_texture_path(material, aiTextureType_OPACITY);
    std::string displacementTexture = get_texture_path(material, aiTextureType_DISPLACEMENT);
    std::string heightTexture = get_texture_path(material, aiTextureType_HEIGHT);
    std::string specularTexture = get_texture_path(material, aiTextureType_SPECULAR);
    std::string shininessTexture = get_texture_path(material, aiTextureType_SHININESS);
    std::string reflectionTexture = get_texture_path(material, aiTextureType_REFLECTION);
    std::string lightmapTexture = get_texture_path(material, aiTextureType_LIGHTMAP);
    std::string sheensTexture = get_texture_path(material, aiTextureType_SHEEN);
    std::string clearcoatTexture = get_texture_path(material, aiTextureType_CLEARCOAT);
    std::string transmissionTexture = get_texture_path(material, aiTextureType_TRANSMISSION);


    mat.albedo = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
    mat.emissivity = glm::vec3(emissive.r, emissive.g, emissive.b);
    mat.roughness = roughness;

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
