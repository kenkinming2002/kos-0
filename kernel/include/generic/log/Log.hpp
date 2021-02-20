#pragma once

#include <stddef.h>

namespace core::log
{
  using Logger = void(*)(const char* str, size_t length);

  void initialize();
  void registerLogger(Logger logger);
}
