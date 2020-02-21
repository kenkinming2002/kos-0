#pragma once

#include <stdint.h>
#include <stddef.h>

#include <optional>

#include <i686/boot/boot.hpp>

namespace core::memory
{
  template<typename Impl>
  class PhysicalMemoryRegionAllocator : public Impl
  {
  public:
    PhysicalMemoryRegionAllocator(BootInformation::MemoryMapEntry* memoryMapEntries, size_t length);

  public:
    using Impl::allocate;
    using Impl::deallocate;
  };
}

