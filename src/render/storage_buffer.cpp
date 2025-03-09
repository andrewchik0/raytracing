#include "storage_buffer.h"

namespace raytracing
{
  storage_buffer::~storage_buffer()
  {
    if (glIsBuffer(mBufferHandle))
    {
      glDeleteBuffers(1, &mBufferHandle);
    }
  }

  void storage_buffer::create(uint32_t bindingPoint)
  {
    glGenBuffers(1, &mBufferHandle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferHandle);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, mBufferHandle);
  }

  void storage_buffer::set(const void* data, const uint32_t size)
  {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferHandle);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size, data);
  }
}