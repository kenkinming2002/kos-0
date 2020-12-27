#pragma once

#include <stdint.h>
#include <stddef.h>

namespace core
{
  struct Module
  {
    uintptr_t phyaddr;
    size_t    length;
  };

  // FIXME: Use a container type
  extern size_t modulesCount;
  extern Module* modules;
}
