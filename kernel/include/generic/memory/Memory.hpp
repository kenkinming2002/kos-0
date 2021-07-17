#pragma once

#include <librt/Optional.hpp>

#include <stddef.h>
#include <stdint.h>

namespace core::memory
{
  void initialize();

  void* allocPages(size_t count);
  void freePages(void* pages, size_t count);

  void* malloc(size_t size);
  void* realloc(void* ptr, size_t size);
  void* calloc(size_t nmemb, size_t size);
  void free(void* ptr);
}
