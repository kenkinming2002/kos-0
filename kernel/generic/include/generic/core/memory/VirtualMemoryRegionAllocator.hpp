#pragma once

#include <optional>
#include <cstddef>

namespace core::memory
{
  template<typename Impl>
  class VirtualMemoryRegionAllocator : public Impl
  {
  public:
    VirtualMemoryRegionAllocator(std::byte* begin, std::byte* end);

  public:
    using Impl::allocate;
    using Impl::deallocate;
  };
}


