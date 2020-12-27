#include <core/x86/assembly/io.hpp>

namespace assembly
{
  void outb(uint16_t port, uint8_t data)
  {
    asm volatile ("out %[port], %[data]" : : [port]"d"(port), [data]"a"(data));
  }

  uint8_t inb(uint16_t port)
  {
    uint8_t byte;
    asm volatile ("in %[byte], %[port] " : [byte]"=a"(byte) : [port]"d"(port));
    return byte;
  }

  void ioWait()
  {
    outb(0x80, 0);
  }
}
