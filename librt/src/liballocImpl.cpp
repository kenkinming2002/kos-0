#include <librt/Hooks.hpp>
#include <librt/SpinLock.hpp>

constinit static rt::SpinLock lock;
extern "C" int liballoc_lock()
{
  lock.lock();
  return 0;
}

extern "C" int liballoc_unlock()
{
  lock.unlock();
  return 0;
}

extern "C" void* liballoc_alloc(size_t count)
{
  return rt::hooks::allocPages(count);
}

extern "C" int liballoc_free(void* ptr, size_t count)
{
  return rt::hooks::freePages(ptr, count);
}

#if 0
extern "C" void* liballoc_alloc(size_t count)
{
  return core::memory::allocPages(count);
}

extern "C" int liballoc_free(void* ptr, size_t count)
{
  core::memory::freePages(ptr, count);
  return 0;
}
#endif
