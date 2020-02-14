#include <generic/io/Serial.hpp>

//TODO: Remove this non generic include
#include <intel/asm/io.hpp>

#if 0
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
  assembly::outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
  assembly::outb(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF);
  assembly::outb(SERIAL_DATA_PORT(com), divisor & 0x00FF);
}

void serial_configure_line(unsigned short com)
{
    /* Bit:     | 7 | 6 | 5 4 3 | 2 | 1 0 |
     * Content: | d | b | prty  | s | dl  |
     * Value:   | 0 | 0 | 0 0 0 | 0 | 1 1 | = 0x03
     */
  assembly::outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

void serial_configure_buffer(unsigned short com)
{
  assembly::outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}

void serial_configure_modem(unsigned short com)
{
  assembly::outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
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
  unsigned char status = assembly::inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
  return FORCE_CAST(struct serial_line_status, status);
}

int serial_write(unsigned short com, const char* buf, unsigned int len)
{
  for(unsigned int i=0; i<len; i++)
  {
    while(!serial_read_line_status(com).transmitter_holding_register_empty); // wait for fifo to be empty
    assembly::outb(SERIAL_DATA_PORT(com), buf[i]); // write the data
  }

  return len; // TODO: support non-blocking write
}

int serial_read(unsigned short com, char* buf, unsigned int len)
{
  for(unsigned int i=0; i<len; i++)
  {
    while(!serial_read_line_status(com).data_ready);
    buf[i] = assembly::inb(SERIAL_DATA_PORT(com));
  }

  return len; //TODO: support non-blocking read
}
#endif

namespace io
{
  constexpr uint16_t COM_DATA_PORT(uint16_t comBase) { return comBase; }
  constexpr uint16_t COM_INTERRUPT_ENABLE_PORT(uint16_t comBase) { return comBase+1; }
  constexpr uint16_t COM_INTERRUPT_IDENTIFICATION_PORT(uint16_t comBase) { return comBase+2; }
  constexpr uint16_t COM_FIFO_CONTROL_PORT(uint16_t comBase) { return comBase+2; }

  constexpr uint16_t COM_LINE_CONTROL_PORT(uint16_t comBase) { return comBase+3; }
  constexpr uint16_t COM_MODEM_CONTROL_PORT(uint16_t comBase) { return comBase+4; }

  constexpr uint16_t COM_LINE_STATUS_PORT(uint16_t comBase) { return comBase+5; }
  constexpr uint16_t COM_MODEM_STATUS_PORT(uint16_t comBase) { return comBase+6; }

  constexpr uint16_t COM_DIVISOR_LATCH_PORT_LOW(uint16_t comBase) { return comBase; }
  constexpr uint16_t COM_DIVISOR_LATCH_PORT_HIGH(uint16_t comBase) { return comBase+1; }

  SerialPort::SerialPort(uint16_t comBase) : m_comBase(comBase)
  {
    configureBaudRateAndLine(1u);
    configureFIFO(3u);
    configureModem();

    //configureInterrupt();
  }

  void SerialPort::configureBaudRateAndLine(uint16_t divisor, Parity parity) const
  {
    // Set DLAB
    assembly::outb(COM_LINE_CONTROL_PORT(m_comBase), 1 << 7);

    // Set Baud Rate
    assembly::outb(COM_DIVISOR_LATCH_PORT_LOW(m_comBase), divisor & 0xFF);
    assembly::outb(COM_DIVISOR_LATCH_PORT_HIGH(m_comBase), divisor >> 8);

    assembly::outb(COM_LINE_CONTROL_PORT(m_comBase), static_cast<uint8_t>(parity) << 3 | 0x3 /* 1 stop bit and 8 bits */);
  }

  void SerialPort::configureFIFO(uint8_t interruptTriggerLevel) const
  {
    assembly::outb(COM_FIFO_CONTROL_PORT(m_comBase), interruptTriggerLevel << 6 | 
                                                     0x7 /*Clear Transmit/Receive FIFO & Enable FIFO*/);
  }

  void SerialPort::configureModem() const
  {
    // We are always ready to send/receive data
    assembly::outb(COM_MODEM_CONTROL_PORT(m_comBase), 0x3);
  }

  void SerialPort::configureInterrupt() const
  {
    // Enable interrupt on Transmitter Holding Register Empty and
    //                     Received Data Available
    assembly::outb(COM_INTERRUPT_ENABLE_PORT(m_comBase), 0x3);
  }

  bool SerialPort::transmitFIFOEmpty() const
  {
    return assembly::inb(COM_LINE_STATUS_PORT(m_comBase)) & 0x20;
  }

  bool SerialPort::dataReady() const
  {
    return assembly::inb(COM_LINE_STATUS_PORT(m_comBase)) & 0x01;
  }

  size_t SerialPort::write(const char* buf, size_t count)
  {
    for(unsigned int i=0; i<count; i++)
    {
      while(!transmitFIFOEmpty()); // wait for fifo to be empty
      assembly::outb(COM_DATA_PORT(m_comBase), buf[i]); // write the data
    }
    return count; // TODO: support non-blocking write
  }

  size_t SerialPort::read(char* buf, size_t count)
  {
    for(unsigned int i=0; i<count; i++)
    {
      while(!dataReady()); // wait for fifo to be empty
      buf[i] = assembly::inb(COM_DATA_PORT(m_comBase)); // write the data
    }
    return count; // TODO: support non-blocking write
  }

  __attribute__((init_priority(101))) SerialPort com1Port(COM1_BASE);
}


