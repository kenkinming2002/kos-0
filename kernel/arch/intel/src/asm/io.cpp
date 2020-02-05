#include <intel/asm/io.hpp>

namespace assembly
{
  void outb(uint16_t port, uint8_t data)
  {
    asm volatile ( R"(
      .intel_syntax noprefix
        out %[port], %[data]
      .att_syntax prefix
      )"
      :
      : [port]"d"(port), [data]"a"(data)
      :
    );
  }

  unsigned char inb(unsigned short port)
  {
    unsigned char byte;
    asm volatile ( R"(
      .intel_syntax noprefix
        in %[byte], %[port]
      .att_syntax prefix
      )"
      : [byte]"=a"(byte)
      : [port]"d"(port)
      :
    );
    return byte;
  }
}
