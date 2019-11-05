#pragma once

#include <stdint.h>

namespace assembly
{
  /** outb:
   *  Sends the given data to the given I/O port. Defined in io.s
   *
   *  @param port The I/O port to send the data to
   *  @param data The data to send to the I/O port
   */
  void outb(uint16_t port, uint8_t data);

  /** inb
   *  Reads data from the given I/O port.
   *
   *  @param port The I/O port to read data from
   *  @return The read data
   */
  unsigned char inb(unsigned short port);
}
