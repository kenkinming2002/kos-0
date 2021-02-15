#pragma once

#include <generic/memory/Pages.hpp>
#include <generic/memory/Physical.hpp>
#include <generic/memory/Virtual.hpp>

#include <librt/Optional.hpp>

#include <stddef.h>

namespace core::memory
{
  void initialize();

  rt::Optional<Pages> mapPages(Pages physicalPages);
  rt::Optional<Pages> allocMappedPages(size_t count);
  void freeMappedPages(Pages pages);


  void* malloc(size_t size);
  void* realloc(void* ptr, size_t size);
  void* calloc(size_t nmemb, size_t size);
  void free(void* ptr);

}
