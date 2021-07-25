#include <librt/Hooks.hpp>

namespace rt::hooks
{
  static constexpr size_t WIDTH = 80;
  void log(const char* str, size_t length)
  {
    static size_t framebufferIndex = 0;
    auto* framebuffer = reinterpret_cast<char(*)[2]>(0xb8000);
    for(size_t i=0; i<length; ++i)
    {
      switch(str[i])
      {
      case '\n':
        framebufferIndex = (framebufferIndex + WIDTH - 1) / WIDTH * WIDTH;
        break;
      default:
        framebuffer[framebufferIndex++][0] = str[i];
        break;
      }
    }
  }

  [[noreturn]] void abort() { for(;;) asm("hlt"); }
}
