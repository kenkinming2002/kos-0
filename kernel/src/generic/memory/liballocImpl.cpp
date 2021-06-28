#include <generic/memory/Memory.hpp>

#include <librt/Panic.hpp>

// NOTE: It has to be Reentrant and MT-Safe
extern "C" int liballoc_lock() { return 0; }
extern "C" int liballoc_unlock() { return 0; }

extern "C" void* liballoc_alloc(size_t count)
{
  return core::memory::allocPages(count);
}

extern "C" int liballoc_free(void* ptr, size_t count)
{
  core::memory::freePages(ptr, count);
  return 0;
}

