#pragma once

#include <generic/memory/Pages.hpp>

#include <librt/Optional.hpp>

#include <stddef.h>

namespace core::memory
{
  void initializePhysical();

  rt::Optional<Pages> allocPhysicalPages(size_t count);
  void freePhysicalPages(Pages pages);
}
