#include <librt/Hooks.hpp>

namespace rt::hooks
{
  void log(const char* str, size_t length)
  {
    static size_t framebufferIndex = 0;
    auto* framebuffer = reinterpret_cast<char(*)[2]>(0xb8000);
    for(size_t i=0; i<length; ++i)
      framebuffer[framebufferIndex++][0] = str[i];
  }

  [[noreturn]] void abort() { for(;;) asm("hlt"); }
}
