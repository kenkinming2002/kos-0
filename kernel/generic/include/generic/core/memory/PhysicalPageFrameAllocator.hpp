#pragma once

#include <stdint.h>
#include <stddef.h>

#include <optional>

#include <generic/core/memory/PageFrame.hpp>

#include <containers/List.hpp>
#include <allocators/CAllocator.hpp>

namespace core::memory
{
  class PhysicalPageFrameAllocator
  {
  public:
    PhysicalPageFrameAllocator(struct multiboot_mmap_entry* mmap_entries, size_t length);

    PhysicalPageFrameAllocator(const PhysicalPageFrameAllocator&) = delete;
    PhysicalPageFrameAllocator(PhysicalPageFrameAllocator&&) = delete;

    ~PhysicalPageFrameAllocator();

  public:
    std::optional<PhysicalPageFrameRange> allocate(size_t count = 1u);
    void deallocate(PhysicalPageFrameRange physicalPageFrameRange);

  private:
    containers::ForwardList<PhysicalPageFrameRange, allocators::CAllocator<PhysicalPageFrameRange>> m_physicalPageFrameRanges;
  };
}

