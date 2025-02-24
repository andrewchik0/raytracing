#pragma once

#include "pch.h"

namespace raytracing
{
  class uniform_buffer
  {
  public:
    uniform_buffer() = default;

    uniform_buffer(const uint32_t bindingPoint, const size_t size, const char* name, const int32_t shaderID)
    { create(bindingPoint, size, name, shaderID); }

    void create(const uint32_t bindingPoint, const size_t size, const char* name, const int32_t shaderID);
    void bind_to_shader(const char* name, const uint32_t shaderID) const;
    void set(const void* data) const;

  private:
    uint32_t mBuffer = 0;
    size_t mSize = 0;
    int32_t mBindingPoint = -1;
  };

}
