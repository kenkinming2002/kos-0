#pragma once

#include <boot/generic/BootInformation.hpp>

#include <common/i686/memory/Paging.hpp>

#include <limits.h>
#include <stdint.h>

namespace boot::memory
{
  enum { MAP_FAILED = UINTPTR_MAX };

  int map(uintptr_t phyaddr, uintptr_t virtaddr, size_t length, common::memory::Access access, common::memory::Permission permission);
  uintptr_t map(uintptr_t phyaddr, size_t length, common::memory::Access access, common::memory::Permission permission);

  void initPaging();
  int updateBootInformationPaging();
}
