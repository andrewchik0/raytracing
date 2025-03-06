#pragma once

#include "pch.h"

namespace raytracing
{
  class render_texture;
  class texture;

  class shader
  {
  public:

    shader() = default;
    shader(const shader&) = delete;

    ~shader();

    [[nodiscard]] status load(const std::string& computePath);
    [[nodiscard]] status load(const std::string& vertexPath, const std::string& fragmentPath);
    [[nodiscard]] status load(const std::string& vertexPath, const std::string& geometryPath, const std::string& fragmentPath);

    void set_uniform(const std::string& name, const size_t value) { set_uniform(name, int(value)); }
    void set_uniform(const std::string& name, const uint32_t value) { set_uniform(name, int(value)); }
    void set_uniform(const std::string& name, int value);
    void set_uniform(const std::string& name, float value);
    void set_uniform(const std::string& name, const double value) { set_uniform(name, float(value)); }
    void set_uniform(const std::string& name, glm::vec2 value);
    void set_uniform(const std::string& name, glm::vec3 value);
    void set_uniform(const std::string& name, glm::vec4 value);
    void set_uniform(const std::string& name, glm::ivec2 value);
    void set_uniform(const std::string& name, glm::ivec3 value);
    void set_uniform(const std::string& name, glm::ivec4 value);
    void set_uniform(const std::string& name, const glm::mat3& value);
    void set_uniform(const std::string& name, const glm::mat4& value);
    void set_uniform(const std::string& name, const texture& value);

    [[nodiscard]] size_t get_free_texture_index();

    void bind_textures();

    void dispatch_compute(const render_texture& buffer);

    void use() const;

    [[nodiscard]] uint32_t get_handle() const;
    [[nodiscard]] bool is_compute() const;

  private:

    static constexpr GLuint sWorkGroupSizeX = 16;
    static constexpr GLuint sWorkGroupSizeY = 16;

    bool mIsCompute = false;

    struct info
    {
      std::string path;
      GLenum type = 0;
      uint32_t shader = 0;
    };

    uint32_t mShaderHandle = 0;
    std::unordered_map<std::string, int32_t> mLocations;

    struct texture_data
    {
      uint32_t handle;
      std::string name;
    };
    std::vector<texture_data> mTextures;

    status load(info* shaders, size_t infoCount);

    int32_t get_uniform_location(const std::string& name);

    static status read_shader_file(const std::string& path, std::string& out);
    static status parse_shader_from_file(const std::string& path, std::set<std::string>& includedFiles, std::string& out);
  };
}
