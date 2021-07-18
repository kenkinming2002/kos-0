#pragma once

#include <boot/generic/BootInformation.hpp>

#include <common/i686/memory/Paging.hpp>

#include <limits.h>
#include <stdint.h>

namespace boot::memory
{
  using namespace common::memory;

  void initializePaging(BootInformation& bootInformation);
  uintptr_t map(BootInformation& bootInformation, uintptr_t phyaddr, uintptr_t virtaddr, size_t length, common::memory::Access access, common::memory::Permission permission);

}
