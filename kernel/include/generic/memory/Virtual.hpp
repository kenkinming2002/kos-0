#pragma once

#include <generic/memory/Pages.hpp>

#include <librt/Optional.hpp>

#include <stddef.h>

namespace core::memory
{
  void initializeVirtual();

  rt::Optional<Pages> allocVirtualPages(size_t count);
  void freeVirtualPages(Pages pages);
}
