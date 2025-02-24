#include "shader.h"

#include <fstream>
#include <iostream>

namespace raytracing
{
  status shader::load(const std::string& vertexPath, const std::string& fragmentPath)
  {
    auto
     vertexIncluded = std::set<std::string>(),
     fragmentIncluded = std::set<std::string>();
    auto vertex = parse_shader_from_file(vertexPath, vertexIncluded);
    auto fragment = parse_shader_from_file(fragmentPath, fragmentIncluded);

    if (!this->loadFromMemory(vertex, fragment))
      return status::error;

    return status::success;
  }

  std::string shader::read_shader_file(const std::string& path)
  {
    std::ifstream file(path);

    if (!file.is_open())
      std::cerr << "Failed to open file: " << path << '\n';

    std::string content((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());

    return content;
  }

  std::string shader::parse_shader_from_file(const std::string& path, std::set<std::string>& includedFiles)
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
}