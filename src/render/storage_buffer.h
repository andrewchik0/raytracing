#pragma once

namespace raytracing
{
  class storage_buffer
  {
  public:
    storage_buffer() = default;
    storage_buffer(const storage_buffer&) = delete;
    explicit storage_buffer(const uint32_t bindingPoint) { create(bindingPoint); }
    ~storage_buffer();

    void create(uint32_t bindingPoint);
    void set(const void* data, uint32_t size);

  private:

    uint32_t mBufferHandle;
  };
}