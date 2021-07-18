#pragma once

#include <librt/Optional.hpp>
#include <generic/BootInformation.hpp>

#include <stddef.h>
#include <stdint.h>

namespace core::memory
{
  using namespace common::memory;
  using physaddr_t = uintptr_t;

  void initialize();

  void* allocPages(size_t count);
  void freePages(void* pages, size_t count);

  void* malloc(size_t size);
  void* realloc(void* ptr, size_t size);
  void* calloc(size_t nmemb, size_t size);
  void free(void* ptr);


  // We will tweak that later, but prefer not to clobber mapping using existing
  // recursive mapping mechanism first
  inline uintptr_t physToVirt(physaddr_t phyaddr) { return uintptr_t(phyaddr + bootInformation->physicalMemoryOffset); }
  inline physaddr_t virtToPhys(uintptr_t virtaddr) { return physaddr_t(virtaddr - bootInformation->physicalMemoryOffset); }

}
