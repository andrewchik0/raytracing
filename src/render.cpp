#include "render.h"

#include <fstream>
#include <iostream>
#include <GL/glew.h>

#include "rt.h"

namespace raytracing
{
  typedef struct SceneBufferStruct SceneBuffer;

  sf::Glsl::Vec3 glm_to_sfml(glm::vec3 v)
  {
    return { v.x, v.y, v.z };
  }

  sf::Glsl::Vec2 glm_to_sfml(glm::vec2 v)
  {
    return { v.x, v.y };
  }

  void render::init()
  {
    if (load_shaders() != status::success)
      return;

    glewInit();

    mRenderQuad = sf::RectangleShape({static_cast<float>(rt::get()->mWindowWidth), static_cast<float>(rt::get()->mWindowHeight)});
    mRenderQuad.setFillColor(sf::Color::Red);
    mSceneBuffer.create(SCENE_BINDING, sizeof(SceneBuffer), "SceneBuffer", mShader.getNativeHandle());

    mSkyTexture = sf::Texture("assets/sky.hdr");
  }

  void render::clear()
  {
    mTexture.clear();
  }

  void render::draw(sf::RenderTarget* window)
  {
    set_uniforms();
    push_scene();
    mTextures.push();

    mTexture.draw(mRenderQuad, &mShader);
    mTexture.display();

    mPostShader.setUniform("renderedTexture", mTexture.getTexture());
    window->draw(mRenderQuad, &mPostShader);
  }


  void render::resize(uint32_t width, uint32_t height)
  {
    if (!mTexture.resize({ width, height}))
    {
      std::cerr << "Failed to resize texture\n";
    }
    mShader.setUniform("windowSize", sf::Vector2f(static_cast<float>(width), static_cast<float>(height)));
    mShader.setUniform("halfHeight", rt::get()->mCamera.mHalfHeight);
    mShader.setUniform("halfWidth", rt::get()->mCamera.mHalfWidth);
    mPostShader.setUniform("windowSize", sf::Vector2f(static_cast<float>(width), static_cast<float>(height)));
  }

  void render::push_scene()
  {
    SceneBuffer buffer = {};
    for (size_t i = 0; i < mPlanesCount; i++)
    {
      mPlanes[i].normal = glm::normalize(mPlanes[i].normal);
    }
    memcpy(buffer.planes, mPlanes.data(), sizeof(PlaneObject) * MAX_PLANES);
    memcpy(buffer.spheres, mSpheres.data(), sizeof(SphereObject) * MAX_SPHERES);
    memcpy(buffer.materials, mMaterials.data(), sizeof(Material) * MAX_MATERIALS);
    buffer.planesCount = mPlanesCount;
    buffer.spheresCount = mSpheresCount;
    mSceneBuffer.set(&buffer);
  }

  std::string render::read_shader_file(const std::string& path)
  {
    std::ifstream file(path);

    if (!file.is_open())
      std::cerr << "Failed to open file: " << path << '\n';

    std::string content((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());

    return content;
  }

  std::string render::parse_shader_from_file(const std::string& path, std::set<std::string>& includedFiles)
  {
    std::filesystem::path fpath(path);
    std::string text = read_shader_file(path);
    std::string result;
    size_t pos = 0;

    for (auto it = text.begin(); it < text.end(); ++it)
    {
      if (text.size() > pos + 18 && *it == '#' && std::string(it + 1, it + 18) == "ifdef __cplusplus")
      {
        while (*it != '#' || std::string(it + 1, it + 6) != "endif")
          ++it, pos++;
        it += 6, pos += 6;
      }

      if (*it == '#' && std::string(it + 1, it + 8) == "include")
      {
        it += 8, pos += 8;
        while(*it++ != '\"') pos++;
        auto start = it;
        while(*it++ != '\"') pos++;
        auto end = it - 1;

        auto filename =
          fpath.parent_path().string() +
          '/' +
          std::string(start, end);

        if (includedFiles.find(filename) == includedFiles.end())
        {
          includedFiles.insert(filename);
          result += parse_shader_from_file(filename, includedFiles);
        }
      }
      result += *it;
    }
    return result;
  }

  status render::load_shaders()
  {
    if (!sf::Shader::isAvailable())
    {
      return status::error;
    }

    load_shader(&mShader, "./shaders/quad.vert", "./shaders/main.frag");
    load_shader(&mPostShader, "./shaders/quad.vert", "./shaders/post.frag");

    return status::success;
  }

  status render::load_shader(sf::Shader* shader, const std::string& vertexPath, const std::string& fragmentPath)
  {
    auto
     vertexIncluded = std::set<std::string>(),
     fragmentIncluded = std::set<std::string>();
    auto vertex = parse_shader_from_file(vertexPath, vertexIncluded);
    auto fragment = parse_shader_from_file(fragmentPath, fragmentIncluded);

    if (!shader->loadFromMemory(vertex, fragment))
      return status::error;

    return status::success;
  }


  void render::set_uniforms()
  {
    mLightDirection = glm::normalize(mLightDirection);
    mShader.setUniform("cameraPosition", glm_to_sfml(rt::get()->mCamera.mPosition));
    mShader.setUniform("cameraDirection", glm_to_sfml(rt::get()->mCamera.mDirection));
    mShader.setUniform("cameraRight", glm_to_sfml(rt::get()->mCamera.mRight));
    mShader.setUniform("cameraUp", glm_to_sfml(rt::get()->mCamera.mUp));
    mShader.setUniform("lightDirection", glm_to_sfml(mLightDirection));
    mShader.setUniform("time", rt::get()->mTime);
    mShader.setUniform("samples", static_cast<int>(mSamplesCount));
    mShader.setUniform("bounces", static_cast<int>(mBouncesCount));
    mShader.setUniform("sky", mSkyTexture);
    mPostShader.setUniform("useFXAA", mUseFXAA);
  }
}
