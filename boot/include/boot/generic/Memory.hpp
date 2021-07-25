#pragma once

#include <stddef.h>
#include <stdint.h>

namespace boot::memory
{
  void* allocPages(size_t count);
  void* alloc(size_t size);

  struct Region
  {
    uintptr_t addr;
    size_t length;
  };

  Region getUsedRegion();
}

