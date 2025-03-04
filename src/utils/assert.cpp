#include "assert.h"

#include <iostream>

void __engine_assert(bool bExpr, const char* message, const char* file, uint32_t line)
{
  if (!bExpr)
  {
    std::cerr
      << "Assertion failed:\t" << message << "\n"
      << file << ", line " << line << "\n";
    abort();
  }
}

void __engine_assert(bool bExpr, const std::string& message, const char* file, uint32_t line)
{
  __engine_assert(bExpr, message.c_str(), file, line);
}