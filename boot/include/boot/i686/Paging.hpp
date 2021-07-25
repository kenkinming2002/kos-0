#pragma once

#include <common/i686/memory/Paging.hpp>

#include <limits.h>
#include <stdint.h>

namespace boot::memory
{
  using namespace common::memory;

  uintptr_t map(uintptr_t phyaddr, uintptr_t virtaddr, size_t length, Access access, Permission permission);

  void initializePaging();
  void enablePaging();

}
