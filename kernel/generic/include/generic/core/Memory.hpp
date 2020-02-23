#pragma once

#include <generic/core/memory/PageFrameAllocator.hpp>

namespace core::memory
{
  void init();

  void* malloc(size_t size);
  void free(void* ptr);

  std::pair<void*, phyaddr_t> mallocMappedPages(size_t count);
  void freeMappedPages(void* pages, size_t count);

  std::optional<MemoryRegion> mallocPhysicalPages(size_t count);
  void freePhysicalPages(MemoryRegion physicalMemoryRegion);
}
