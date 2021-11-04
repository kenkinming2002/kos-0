#include <librt/Hooks.hpp>

extern "C" int liballoc_lock()
{
  return rt::hooks::lockAllocator();
}

extern "C" int liballoc_unlock()
{
  return rt::hooks::unlockAllocator();
}

extern "C" void* liballoc_alloc(size_t count)
{
  return rt::hooks::allocPages(count);
}

extern "C" int liballoc_free(void* ptr, size_t count)
{
  return rt::hooks::freePages(ptr, count);
}

