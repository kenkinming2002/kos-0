#pragma once

#include <generic/core/memory/PageFrameAllocator.hpp>

namespace core::memory
{
  void* malloc(size_t size);
  void free(void* ptr);

  void* mallocPages(size_t count);
  void freePages(void* pages, size_t count);
}
