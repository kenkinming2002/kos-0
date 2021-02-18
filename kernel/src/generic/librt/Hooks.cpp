#include <generic/memory/Memory.hpp>
#include <generic/devices/Framebuffer.hpp>

namespace rt::hooks
{
  void* malloc(size_t size) { return core::memory::malloc(size); }
  void free(void* ptr) { return core::memory::free(ptr); }
  [[noreturn]] void abort() { for(;;) asm("hlt"); }
}
