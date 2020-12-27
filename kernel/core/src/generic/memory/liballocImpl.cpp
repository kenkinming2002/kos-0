#include <core/generic/memory/Memory.hpp>

#include <core/generic/Panic.hpp>

// NOTE: It has to be Reentrant and MT-Safe
extern "C" int liballoc_lock() { return 0; }
extern "C" int liballoc_unlock() { return 0; }

extern "C" void* liballoc_alloc(size_t count)
{
  auto pages = core::memory::allocMappedPages(count);
  if(!pages)
    return nullptr;

  return reinterpret_cast<void*>(pages->address());
}

extern "C" int liballoc_free(void* ptr, size_t count)
{
  auto pages = core::memory::Pages{reinterpret_cast<uintptr_t>(ptr) / core::memory::PAGE_SIZE, count};
  core::memory::freeMappedPages(pages);
  return 0;
}

