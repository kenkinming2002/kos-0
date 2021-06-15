#pragma once

#include <generic/memory/Pages.hpp>

#include <librt/Optional.hpp>

#include <stddef.h>

namespace core::memory
{
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

  void initialize();

  rt::Optional<Pages> allocPages(size_t count);
  void freePages(Pages pages);

  void* malloc(size_t size);
  void* realloc(void* ptr, size_t size);
  void* calloc(size_t nmemb, size_t size);
  void free(void* ptr);
}
