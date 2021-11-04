#include <generic/memory/Memory.hpp>

namespace rt::hooks
{
  void* allocPages(size_t count)
  {
    return core::memory::allocPages(count);
  }

  int freePages(void* ptr, size_t count)
  {
    core::memory::freePages(ptr, count);
    return 0;
  }

  bool validAddress(void* ptr) { return reinterpret_cast<uintptr_t>(ptr) > 0xC0000000; }
  [[noreturn]] void abort() { for(;;) asm("hlt"); }

  static constinit core::SpinLock lock;
  int lockAllocator()
  {
    lock.lock();
    return 0;
  }

  int unlockAllocator()
  {
    lock.unlock();
    return 0;
  }
}
