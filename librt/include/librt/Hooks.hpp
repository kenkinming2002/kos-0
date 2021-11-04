#pragma once

#include <stddef.h>

namespace rt::hooks
{
  void* allocPages(size_t count);
  int freePages(void* ptr, size_t count);

  void log(const char* str, size_t length);
  bool validAddress(void* ptr);
  [[noreturn]] void abort();

  int lockAllocator();
  int unlockAllocator();
}
