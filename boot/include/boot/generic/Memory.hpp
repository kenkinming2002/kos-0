#pragma once

#include <boot/generic/BootInformation.hpp>

#include <stddef.h>

namespace boot::memory
{
  void* allocPages(BootInformation& bootInformation, size_t count, ReservedMemoryRegion::Type type);
  void* alloc(BootInformation& bootInformation, size_t size, ReservedMemoryRegion::Type type);
}

