#pragma once

#include <generic/memory/Pages.hpp>

#include <librt/Optional.hpp>

#include <stddef.h>

namespace core::memory
{
  void initialize();

  rt::Optional<Pages> allocPhysicalPages(size_t count);
  void freePhysicalPages(Pages pages);

  rt::Optional<Pages> allocVirtualPages(size_t count);
  void freeVirtualPages(Pages pages);

  rt::Optional<Pages> allocMappedPages(size_t count);
  void freeMappedPages(Pages pages);

  void* malloc(size_t size);
  void* realloc(void* ptr, size_t size);
  void* calloc(size_t nmemb, size_t size);
  void free(void* ptr);

}
