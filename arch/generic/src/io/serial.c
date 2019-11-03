#include <generic/io/serial.h>

//TODO: Remove this non generic include
#include <i686/asm/io.h>
#include <generic/utils/cast.h>

#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

/* The I/O port commands */

/* SERIAL_LINE_ENABLE_DLAB:
 * Tells the serial port to expect first the highest 8 bits on the data port,
 * then the lowest 8 bits will follow
 */
#define SERIAL_LINE_ENABLE_DLAB         0x80

void serial_confgure_baud_rate(unsigned short com, unsigned short divisor)
{
  outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
  outb(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF);
  outb(SERIAL_DATA_PORT(com), divisor & 0x00FF);
}

void serial_configure_line(unsigned short com)
{
    /* Bit:     | 7 | 6 | 5 4 3 | 2 | 1 0 |
     * Content: | d | b | prty  | s | dl  |
     * Value:   | 0 | 0 | 0 0 0 | 0 | 1 1 | = 0x03
     */
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

void serial_configure_buffer(unsigned short com)
{
  outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}

void serial_configure_modem(unsigned short com)
{
  outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

void serial_configure(unsigned short com, unsigned short divisor)
{
  serial_confgure_baud_rate(com, divisor);
  serial_configure_line(com);
  serial_configure_buffer(com);
  serial_configure_modem(com);
}

struct serial_line_status
{
  int data_ready                         : 1;
  int overrun                            : 1;
  int parity_error                       : 1;
  int framing_error                      : 1;
  int break_indicator                    : 1;
  int transmitter_holding_register_empty : 1;
  int transmitter_empty                  : 1;
  int impending_erro                     : 1;
} __attribute((packed));

struct serial_line_status serial_read_line_status(unsigned int com)
{
  unsigned char status = inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
  return FORCE_CAST(struct serial_line_status, status);
}

int serial_write(unsigned short com, const char* buf, unsigned int len)
{
  for(unsigned int i=0; i<len; i++)
  {
    while(!serial_read_line_status(com).transmitter_holding_register_empty); // wait for fifo to be empty
    outb(SERIAL_DATA_PORT(com), buf[i]); // write the data
  }

  return len; // TODO: support non-blocking write
}

int serial_read(unsigned short com, char* buf, unsigned int len)
{
  for(unsigned int i=0; i<len; i++)
  {
    while(!serial_read_line_status(com).data_ready);
    buf[i] = inb(SERIAL_DATA_PORT(com));
  }

  return len; //TODO: support non-blocking read
}
