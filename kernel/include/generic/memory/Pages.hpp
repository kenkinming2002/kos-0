#pragma once

#include <common/i686/memory/Paging.hpp>

#include <librt/Panic.hpp>
#include <librt/Log.hpp>

#include <stdint.h>
#include <stddef.h>

namespace core::memory
{
  using common::memory::PAGE_SIZE;
  using common::memory::LARGE_PAGE_SIZE;

  struct Pages
  {
  public:
    static Pages from(uintptr_t addr, size_t length)
    {
      if(addr % PAGE_SIZE != 0 || length % PAGE_SIZE !=0)
        rt::panic("Unaligned pages");

      return Pages{addr / PAGE_SIZE, length / PAGE_SIZE};
    }

    static Pages fromConservative(uintptr_t addr, size_t length)
    {
      size_t beginIndex = (addr + (PAGE_SIZE-1)) / PAGE_SIZE;
      size_t endIndex   = (addr + length)        / PAGE_SIZE;
      if(beginIndex<endIndex)
        return Pages{beginIndex, endIndex-beginIndex};
      else
        return Pages{0, 0}; // We don't even have a page
    }

    static Pages fromAggressive(uintptr_t addr, size_t length)
    {
      size_t beginIndex = addr                            / PAGE_SIZE;
      size_t endIndex   = (addr + length + (PAGE_SIZE-1)) / PAGE_SIZE;
      return Pages{beginIndex, endIndex-beginIndex};
    }

  public:
    uintptr_t address() const { return reinterpret_cast<uintptr_t>(index * PAGE_SIZE); }
    size_t length()     const { return count * PAGE_SIZE; }

  public:
    /* TODO: Use a smaller integer size */
    size_t index, count;
  };
}
