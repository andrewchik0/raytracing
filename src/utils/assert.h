#pragma once

#include <string>
void __engine_assert(bool bExpr, const char* message, const char* file, uint32_t line);
void __engine_assert(bool bExpr, const std::string& message, const char* file, uint32_t line);
#ifndef NDEBUG
  #define rt_assert(condition, message) (__engine_assert((condition), (message), __FILE__, __LINE__));
#else
  #define rt_assert(condition, message)
#endif
