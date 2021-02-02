#pragma once

#include <optional>
#include <stddef.h>

#include <generic/memory/Pages.hpp>

namespace core::memory
{
  void initialize();

  std::optional<Pages> allocPhysicalPages(size_t count);
  void freePhysicalPages(Pages pages);

  std::optional<Pages> allocVirtualPages(size_t count);
  void freeVirtualPages(Pages pages);

  std::optional<Pages> allocMappedPages(size_t count);
  void freeMappedPages(Pages pages);

  void* malloc(size_t size);
  void* realloc(void* ptr, size_t size);
  void* calloc(size_t nmemb, size_t size);
  void free(void* ptr);

}
