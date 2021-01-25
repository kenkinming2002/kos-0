#include <common/generic/io/Print.hpp>

namespace common::io
{
  void print(std::string_view str)
  {
    static size_t framebufferIndex = 0;
    auto* framebuffer = reinterpret_cast<char(*)[2]>(0xb8000);
    for(auto c : str)
      framebuffer[framebufferIndex++][0] = c;
  }
}
