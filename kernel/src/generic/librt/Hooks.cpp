#include <generic/memory/Memory.hpp>
#include <generic/devices/Framebuffer.hpp>

namespace rt::hooks
{
  void log(const char* str, size_t length) { core::devices::Framebuffer::instance().write(str, length); }
  void* malloc(size_t size) { return core::memory::malloc(size); }
  void free(void* ptr) { return core::memory::free(ptr); }
  [[noreturn]] void abort() { for(;;) asm("hlt"); }
}
