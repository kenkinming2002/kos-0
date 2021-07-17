#pragma once

#include <common/i686/memory/Paging.hpp>

#include <librt/Assert.hpp>

#include <stdint.h>

namespace core::memory
{
  using common::memory::PAGE_SIZE;
  using common::memory::LARGE_PAGE_SIZE;

  using uintptr_t = uintptr_t;
  using physaddr_t = uintptr_t;

  // We will tweak that later, but prefer not to clobber mapping using existing
  // recursive mapping mechanism first
  constexpr physaddr_t PHYSICAL_MEMEMORY_MAPPING_START = 0xD0000000;
  constexpr physaddr_t PHYSICAL_MEMEMORY_MAPPING_END   = 0xF0000000;
  constexpr physaddr_t PHYSICAL_MEMEMORY_MAPPING_SIZE  = PHYSICAL_MEMEMORY_MAPPING_END - PHYSICAL_MEMEMORY_MAPPING_START;

  inline uintptr_t physToVirt(physaddr_t phyaddr)
  {
    ASSERT(phyaddr <= PHYSICAL_MEMEMORY_MAPPING_SIZE);
    return uintptr_t(phyaddr + PHYSICAL_MEMEMORY_MAPPING_START);
  }

  inline physaddr_t virtToPhys(uintptr_t virtaddr)
  {
    ASSERT(virtaddr >= PHYSICAL_MEMEMORY_MAPPING_START);
    ASSERT(virtaddr <= PHYSICAL_MEMEMORY_MAPPING_END);
    return physaddr_t(virtaddr - PHYSICAL_MEMEMORY_MAPPING_START);
  }

  void archInitialize();
}
