#pragma once

#include <stdint.h>
#include <stddef.h>

#include <core/generic/Panic.hpp>
#include <core/generic/io/Print.hpp>

namespace core::memory
{
  static constexpr size_t PAGE_SIZE       = 4096;
  static constexpr size_t LARGE_PAGE_SIZE = PAGE_SIZE * 1024;

  struct Pages
  {
  public:
    static Pages from(uintptr_t addr, size_t length)
    {
      if(addr % PAGE_SIZE != 0 || length % PAGE_SIZE !=0)
        panic("Unaligned pages");

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
