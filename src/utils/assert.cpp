#include "assert.h"

#include <iostream>

void __engine_assert(const bool bExpr, const char* message, const char* file, const uint32_t line)
{
  if (!bExpr)
  {
    std::cerr
      << "Assertion failed:\t" << message << "\n"
      << file << ", line " << line << "\n";
    abort();
  }
}

void __engine_assert(const bool bExpr, const std::string& message, const char* file, const uint32_t line)
{
  __engine_assert(bExpr, message.c_str(), file, line);
}