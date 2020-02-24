#pragma once

#include <generic/core/memory/PageFrameAllocator.hpp>

namespace core::memory
{
  void init();

  void* malloc(size_t size);
  void free(void* ptr);

  std::optional<MemoryRegion> allocatePhysicalMemoryRegion(size_t count);
  void deallocatePhysicalMemoryRegion(MemoryRegion physicalMemoryRegion);

  std::pair<void*, phyaddr_t> allocateHeapPages(size_t count);
  std::pair<void*, phyaddr_t> deallocateHeapPages(size_t count);
}
