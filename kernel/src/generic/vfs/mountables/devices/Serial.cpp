#include "librt/SharedPtr.hpp"
#include <generic/vfs/mountables/devices/Serial.hpp>

#include <librt/Global.hpp>

#include <x86/assembly/io.hpp>

namespace core::vfs
{
  namespace
  {
    rt::Global<Serial> serial;
  }

  void initializeSerial()
  {
    serial.construct();
    registerMountable(serial());
  }

  Result<rt::SharedPtr<Inode>> Serial::mount(rt::StringRef arg)
  {
    rt::SharedPtr<Inode> inode;
    if(arg == "1")
      return rt::SharedPtr<Inode>(rt::makeShared<SerialInode>(COM1_BASE));
    else if(arg == "2")
      return rt::SharedPtr<Inode>(rt::makeShared<SerialInode>(COM2_BASE));
    else if(arg == "3")
      return rt::SharedPtr<Inode>(rt::makeShared<SerialInode>(COM3_BASE));
    else if(arg == "4")
      return rt::SharedPtr<Inode>(rt::makeShared<SerialInode>(COM4_BASE));
    else
    {
      ASSERT(false);
      return ErrorCode::INVALID;
    }
  }

  rt::StringRef Serial::name() { return "serial"; }

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

  SerialInode::SerialInode(uint16_t comBase) : m_comBase(comBase)
  {
    configureBaudRateAndLine(1u);
    configureFIFO(3u);
    configureModem();

    //configureInterrupt();
  }

  void SerialInode::configureBaudRateAndLine(uint16_t divisor, Parity parity) const
  {
    // Set DLAB
    assembly::outb(COM_LINE_CONTROL_PORT(m_comBase), 1 << 7);

    // Set Baud Rate
    assembly::outb(COM_DIVISOR_LATCH_PORT_LOW(m_comBase), divisor & 0xFF);
    assembly::outb(COM_DIVISOR_LATCH_PORT_HIGH(m_comBase), divisor >> 8);

    assembly::outb(COM_LINE_CONTROL_PORT(m_comBase), static_cast<uint8_t>(parity) << 3 | 0x3 /* 1 stop bit and 8 bits */);
  }

  void SerialInode::configureFIFO(uint8_t interruptTriggerLevel) const
  {
    assembly::outb(COM_FIFO_CONTROL_PORT(m_comBase), interruptTriggerLevel << 6 |
                                                     0x7 /*Clear Transmit/Receive FIFO & Enable FIFO*/);
  }

  void SerialInode::configureModem() const
  {
    // We are always ready to send/receive data
    assembly::outb(COM_MODEM_CONTROL_PORT(m_comBase), 0x3);
  }

  void SerialInode::configureInterrupt() const
  {
    // Enable interrupt on Transmitter Holding Register Empty and
    //                     Received Data Available
    assembly::outb(COM_INTERRUPT_ENABLE_PORT(m_comBase), 0x3);
  }

  bool SerialInode::transmitFIFOEmpty() const
  {
    return assembly::inb(COM_LINE_STATUS_PORT(m_comBase)) & 0x20;
  }

  bool SerialInode::dataReady() const
  {
    return assembly::inb(COM_LINE_STATUS_PORT(m_comBase)) & 0x01;
  }

  Result<ssize_t> SerialInode::read(char* buf, size_t count, size_t pos)
  {
    for(unsigned int i=0; i<count; i++)
    {
      while(!dataReady()); // wait for fifo to be empty
      buf[i] = assembly::inb(COM_DATA_PORT(m_comBase)); // write the data
    }
    return count; // TODO: support non-blocking write
  }

  Result<ssize_t> SerialInode::write(const char* buf, size_t count, size_t pos)
  {
    for(unsigned int i=0; i<count; i++)
    {
      while(!transmitFIFOEmpty()); // wait for fifo to be empty
      assembly::outb(COM_DATA_PORT(m_comBase), buf[i]); // write the data
    }
    return count; // TODO: support non-blocking write
  }
}


