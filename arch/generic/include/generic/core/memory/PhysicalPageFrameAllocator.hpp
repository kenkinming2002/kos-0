#pragma once

#include <stdint.h>
#include <stddef.h>

#include <optional>

#include <generic/core/memory/PageFrame.hpp>

#include <generic/utils/List.hpp>
#include <generic/utils/StaticVector.hpp>

namespace core::memory
{
  template<template<typename> typename Container>
  class PhysicalPageFrameAllocator
  {
  public:
    PhysicalPageFrameAllocator(struct multiboot_mmap_entry* mmap_entries, size_t length);

  public:
    std::optional<PhysicalPageFrameRange> allocate(size_t count = 1u);
    void deallocate(PhysicalPageFrameRange physicalPageFrameRange);

  private:
    Container<PhysicalPageFrameRange> m_physicalPageFrameRange;
  };

  template<typename T>
  using StaticVector = utils::StaticVector<T, 64>; // 256 byte
  using StaticPhysicalPageFrameAllocator = PhysicalPageFrameAllocator<StaticVector>;
}

