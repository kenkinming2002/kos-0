#include <generic/log/Log.hpp>

#include <generic/devices/Framebuffer.hpp>

#include <x86/assembly/io.hpp>

namespace core::log
{
  namespace
  {
    Logger logger;
  }

  void initialize()
  {
    devices::Framebuffer::initializeLog();
  }

  void registerLogger(Logger logger)
  {
    core::log::logger = logger;
  }
}

namespace
{
  // Temporary hack
  static constexpr auto comBase = 0x3F8;
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

  bool transmitFIFOEmpty()
  {
    return assembly::inb(COM_LINE_STATUS_PORT(comBase)) & 0x20;
  }

  void serial_write(const char* str, size_t length)
  {
    for(unsigned int i=0; i<length; i++)
    {
      while(!transmitFIFOEmpty()); // wait for fifo to be empty
      assembly::outb(COM_DATA_PORT(comBase), str[i]); // write the data
    }
  }
}

namespace rt::hooks
{
  void log(const char* str, size_t length)
  {
    core::log::logger(str, length);
    serial_write(str, length);
  }
}

