#pragma once

#include <generic/memory/Memory.hpp>

namespace core::memory
{
  uintptr_t kmap(physaddr_t phyaddr);
  void kunmap(uintptr_t addr);

  struct KMapGuard
  {
  public:
    KMapGuard(physaddr_t physaddr) { addr = kmap(physaddr); }
    ~KMapGuard() { kunmap(addr); }

  public:
    uintptr_t addr;
  };
}

