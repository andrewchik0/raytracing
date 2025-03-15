#include "shader.h"

#include <fstream>
#include <regex>

#include <glm/gtc/type_ptr.hpp>

#include "rt.h"

namespace raytracing
{
  shader::~shader()
  {
    glUseProgram(0);
    glDeleteProgram(mShaderHandle);
  }

  status shader::load(const std::string& computePath)
  {
    info shaders[1];
    shaders[0].path = computePath;
    shaders[0].type = GL_COMPUTE_SHADER;
    mIsCompute = true;
    return load(shaders, 1);
  }

  status shader::load(const std::string& vertexPath, const std::string& fragmentPath)
  {
    constexpr int shaderStagesCount = 2;

    info shaders[shaderStagesCount];

    shaders[0].path = vertexPath;
    shaders[0].type = GL_VERTEX_SHADER;

    shaders[1].path = fragmentPath;
    shaders[1].type = GL_FRAGMENT_SHADER;

    return load(shaders, shaderStagesCount);
  }

  status shader::load(const std::string& vertexPath, const std::string& geometryPath, const std::string& fragmentPath)
  {
    constexpr int shaderStagesCount = 3;

    info shaders[shaderStagesCount];

    shaders[0].path = vertexPath;
    shaders[0].type = GL_VERTEX_SHADER;

    shaders[1].path = geometryPath;
    shaders[1].type = GL_GEOMETRY_SHADER;

    shaders[2].path = fragmentPath;
    shaders[2].type = GL_FRAGMENT_SHADER;

    return load(shaders, shaderStagesCount);
  }

  std::string shader::parse_shader_error(const std::set<std::string>& includedFiles, std::string& fullShaderCode,
                                         const std::vector<byte>& shaderErrors, const info& shader)
  {
    //
    // Not the best method to handle errors, but enough for debugging
    //
    std::string parsedError;
    std::istringstream log(shaderErrors.data());
    std::string line;

    // Parse every error produced by glsl compiler
    while (std::getline(log, line)) // Reads up to '\n'
    {
      std::regex numPattern(R"(\d+)");
      std::sregex_iterator it(line.begin(), line.end(), numPattern);
      std::sregex_iterator end;
      int32_t originalLineNumber = -1;

      int count = 0;
      while (it != end)
      {
        count++;
        if (count == 2)
          originalLineNumber = std::stoi(it->str());
        ++it;
      }
      if (originalLineNumber == -1)
        continue;

      // Find the original file where error occurred
      std::stringstream fullShaderCodeStream(fullShaderCode);
      std::string lineOfError;
      size_t counter = 0;
      while (std::getline(fullShaderCodeStream, lineOfError))
      {
        counter++;
        if (counter == originalLineNumber)
          break;
      }
      if (counter != originalLineNumber)
        continue;

      std::string fileNameOfError;
      size_t foundLineOfError = -1;
      std::set<std::string> fullIncludedFiles = includedFiles;
      fullIncludedFiles.emplace(shader.path);
      for (auto file : fullIncludedFiles)
      {
        std::ifstream fileStream(file);
        std::string lineToFind;
        size_t counter = 0;
        bool found = false;
        while (std::getline(fileStream, lineToFind))
        {
          counter++;
          if (lineToFind == lineOfError)
          {
            found = true;
            break;
          }
        }
        if (found)
        {
          foundLineOfError = counter;
          fileNameOfError = file;
          break;
        }
      }
      if (foundLineOfError != -1)
      {
        std::sregex_iterator replaceIt(line.begin(), line.end(), numPattern);
        std::sregex_iterator replaceEnd;

        int replaceCount = 0;
        std::string result = line;

        while (replaceIt != replaceEnd)
        {
          replaceCount++;
          if (replaceCount == 2)
          {
            result.replace(replaceIt->position(), replaceIt->length(), std::to_string(foundLineOfError));
            break;
          }
          ++replaceIt;
        }
        if (replaceCount < 2)
          break;

        fileNameOfError.replace(0, 10, "");
        parsedError += fileNameOfError + ": " + result + '\n';
      }
    }
    return parsedError;
  }

  status shader::load(info* shaders, const size_t infoCount)
  {
    if (glIsProgram(mShaderHandle))
    {
      glUseProgram(0);
      glDeleteProgram(mShaderHandle);
    }
    mTextures.clear();
    mLocations.clear();
    mShaderHandle = glCreateProgram();

    int32_t success;
    std::vector<byte> infoLog(1024);
    for (uint32_t i = 0; i < infoCount; ++i)
    {
      std::set<std::string> includedFiles;
      std::string text;
      auto status = parse_shader_from_file(shaders[i].path, includedFiles, text);
      if (status != status::success)
        return status;
      const char* buf = text.c_str();

      shaders[i].shader = glCreateShader(shaders[i].type);
      glShaderSource(shaders[i].shader, 1, &buf, nullptr);
      glCompileShader(shaders[i].shader);
      glGetShaderiv(shaders[i].shader, GL_COMPILE_STATUS, &success);
      if (success != GL_TRUE)
      {
        glGetShaderInfoLog(shaders[i].shader, infoLog.size(), nullptr, infoLog.data());
        std::string shaderKind;
        switch (shaders[i].type)
        {
        case GL_VERTEX_SHADER:
          shaderKind = "vertex";
          break;
        case GL_COMPUTE_SHADER:
          shaderKind = "compute";
          break;
        case GL_FRAGMENT_SHADER:
          shaderKind = "fragment";
          break;
        default:
          shaderKind = "unknown";
        }
        rt::get()->mRender.mShaderErrors +=
          "ERROR: Failed to compile " + shaderKind + " shader\n" +
          parse_shader_error(includedFiles, text, infoLog, shaders[i]);
      }

      glAttachShader(mShaderHandle, shaders[i].shader);
    }

    glLinkProgram(mShaderHandle);
    glGetProgramiv(mShaderHandle, GL_LINK_STATUS, &success);

    for (uint32_t i = 0; i < infoCount; ++i)
    {
      glDetachShader(mShaderHandle, shaders[i].shader);
      glDeleteShader(shaders[i].shader);
    }
    return success ? status::success : status::error;
  }

  status shader::read_shader_file(const std::string& path, std::string& out)
  {
    std::ifstream file(path);

    if (!file.is_open())
      return status::file_not_found;

    out = std::string(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());

    return status::success;
  }

  status shader::parse_shader_from_file(const std::string& path, std::set<std::string>& includedFiles, std::string& out)
  {
    std::filesystem::path fpath(path);
    size_t pos = 0;
    std::string text;
    auto status = read_shader_file(path, text);

    if (status != status::success)
    {
      return status;
    }

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
          std::string parseOut;
          if (auto parseStatus = parse_shader_from_file(filename, includedFiles, parseOut); parseStatus != status::success)
            return parseStatus;
          out += parseOut;
        }
      }
      out += *it;
    }
    return status::success;
  }

  int32_t shader::get_uniform_location(const std::string& name)
  {
    if (!mLocations.contains(name))
    {
      int32_t location = glGetUniformLocation(mShaderHandle, name.c_str());
      if (location != -1)
        mLocations.insert(std::pair(name, location));
      return location;
    }
    return mLocations.at(name);
  }

  void shader::use() const { glUseProgram(mShaderHandle); }

  uint32_t shader::get_handle() const { return mShaderHandle; }

  bool shader::is_compute() const
  {
    return mIsCompute;
  }

  void shader::set_uniform(const std::string& name, int value)
  {
    use();
    if (int32_t location = get_uniform_location(name); location != -1)
      glUniform1i(location, value);
  }

  void shader::set_uniform(const std::string& name, float value)
  {
    use();
    if (int32_t location = get_uniform_location(name); location != -1)
      glUniform1f(location, value);
  }

  void shader::set_uniform(const std::string& name, glm::vec2 value)
  {
    use();
    if (int32_t location = get_uniform_location(name); location != -1)
      glUniform2fv(location, 1, glm::value_ptr(value));
  }

  void shader::set_uniform(const std::string& name, glm::vec3 value)
  {
    use();
    if (int32_t location = get_uniform_location(name); location != -1)
      glUniform3fv(location, 1, glm::value_ptr(value));
  }

  void shader::set_uniform(const std::string& name, glm::vec4 value)
  {
    use();
    if (int32_t location = get_uniform_location(name); location != -1)
      glUniform4fv(location, 1, glm::value_ptr(value));
  }

  void shader::set_uniform(const std::string& name, glm::ivec2 value)
  {
    use();
    if (int32_t location = get_uniform_location(name); location != -1)
      glUniform2iv(location, 1, glm::value_ptr(value));
  }

  void shader::set_uniform(const std::string& name, glm::ivec3 value)
  {
    use();
    if (int32_t location = get_uniform_location(name); location != -1)
      glUniform3iv(location, 1, glm::value_ptr(value));
  }

  void shader::set_uniform(const std::string& name, glm::ivec4 value)
  {
    use();
    if (int32_t location = get_uniform_location(name); location != -1)
      glUniform4iv(location, 1, glm::value_ptr(value));
  }

  void shader::set_uniform(const std::string& name, const glm::mat3& value)
  {
    use();
    if (int32_t location = get_uniform_location(name); location != -1)
      glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
  }

  void shader::set_uniform(const std::string& name, const glm::mat4& value)
  {
    use();
    if (int32_t location = get_uniform_location(name); location != -1)
      glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
  }

  void shader::set_uniform(const std::string& name, const texture& value)
  {
    use();

    bool bound = false;
    for (auto& texture : mTextures)
    {
      if (texture.handle == value.get_texture_handle())
        bound = true;
    }

    if (!bound)
    {
      mTextures.push_back({value.get_texture_handle(), name});
    }
  }

  size_t shader::get_free_texture_index()
  {
    return mTextures.size();
  }

  void shader::bind_textures()
  {
    use();
    for (size_t i = 0; i < mTextures.size(); i++)
    {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, mTextures[i].handle);
      set_uniform(mTextures[i].name, i);
    }
  }

  void shader::dispatch_compute(const render_texture& buffer)
  {
    rt_assert(mIsCompute, "Trying to dispatch compute on non-compute shader!");

    bind_textures();
    glBindImageTexture(0, buffer.get_texture_handle(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    uint32_t numGroupsX = (buffer.width() + sWorkGroupSizeX - 1) / sWorkGroupSizeX;
    uint32_t numGroupsY = (buffer.height() + sWorkGroupSizeY - 1) / sWorkGroupSizeY;
    glDispatchCompute(numGroupsX, numGroupsY, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
  }

} // namespace raytracing
