#include "uniform_buffer.h"

#include <GL/glew.h>

namespace raytracing
{
  void uniform_buffer::create(const uint32_t bindingPoint, const size_t size, const char* name, const int32_t shaderID)
  {
    mBindingPoint = bindingPoint;
    mSize = size;
    glGenBuffers(1, &mBuffer);

    glBindBuffer(GL_UNIFORM_BUFFER, mBuffer);
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    uint32_t bindIndex = glGetUniformBlockIndex(shaderID, name);
    glUniformBlockBinding(shaderID, bindIndex, mBindingPoint);

    glBindBufferRange(GL_UNIFORM_BUFFER, mBindingPoint, mBuffer, 0, size);
  }

  void uniform_buffer::bind_to_shader(const char* name, const uint32_t shaderID) const
  {
    uint32_t bindIndex = glGetUniformBlockIndex(shaderID, name);
    glUniformBlockBinding(shaderID, bindIndex, mBindingPoint);
  }

  void uniform_buffer::set(const void* data) const
  {
    glBindBufferBase(GL_UNIFORM_BUFFER, mBindingPoint, mBuffer);
    glBufferData(GL_UNIFORM_BUFFER, mSize, data, GL_STATIC_READ);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }
}