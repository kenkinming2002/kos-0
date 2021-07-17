#pragma once

#include <stddef.h>

namespace rt::hooks
{
  void log(const char* str, size_t length);
  void* malloc(size_t size);
  void free(void* ptr);
  bool validAddress(void* ptr);
  [[noreturn]] void abort();
}
