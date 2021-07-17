#include <generic/memory/Memory.hpp>
#include <generic/devices/Framebuffer.hpp>

namespace rt::hooks
{
  void* malloc(size_t size) { return core::memory::malloc(size); }
  void free(void* ptr) { return core::memory::free(ptr); }
  bool validAddress(void* ptr) { return reinterpret_cast<uintptr_t>(ptr) > 0xC0000000; }
  [[noreturn]] void abort() { for(;;) asm("hlt"); }
}
