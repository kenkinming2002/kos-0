#pragma once

#include <boot/generic/BootInformation.hpp>

#include <common/i686/memory/Paging.hpp>

#include <limits.h>
#include <stdint.h>

namespace boot::memory
{
  using namespace common::memory;
  extern PageDirectory* pageDirectory;

  enum { MAP_FAILED = UINTPTR_MAX };
  void initializePaging(BootInformation& bootInformation);

  [[nodiscard]] uintptr_t map(BootInformation& bootInformation, uintptr_t phyaddr, uintptr_t virtaddr, size_t length, common::memory::Access access, common::memory::Permission permission);
  [[nodiscard]] uintptr_t map(BootInformation& bootInformation, uintptr_t phyaddr, size_t length, common::memory::Access access, common::memory::Permission permission);

}
