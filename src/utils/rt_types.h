#pragma once

namespace raytracing
{
  using uchar = unsigned char;

  enum class status
  {
    success = 0,
    error = 1,
    file_not_found = 2,
  };
}
