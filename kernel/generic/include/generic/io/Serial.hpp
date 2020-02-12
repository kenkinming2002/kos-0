#pragma once

#include <stdint.h>
#include <stddef.h>
#include <utility>
#include <algorithm>

#if 0
#define SERIAL_COM1_BASE                0x3F8      /* COM1 base port */

void serial_configure(unsigned short com, unsigned short divisor);
int serial_write(unsigned short com, const char* buf, unsigned int len);
int serial_read(unsigned short com, char* buf, unsigned int len);
#endif

namespace io
{
  enum COMBase : uint16_t
  {
    COM1_BASE = 0x3F8,
    COM2_BASE = 0x2F8,
    COM3_BASE = 0x3E8,
    COM4_BASE = 0x2E8
  };


  class SerialPort
  {
  public:
    constexpr static size_t BUFFER_SIZE = 64;

  public:
    SerialPort() = default;
    SerialPort(uint16_t comBase);

  public:
    enum class Parity
    {
      NONE  = 0x0,
      ODD   = 0x1,
      EVEN  = 0x3,
      MARK  = 0x5,
      SPACE = 0x7,
    };
    void configureBaudRateAndLine(uint16_t divisor, Parity parity = Parity::NONE) const;

    /*
     * |Interrupt Trigger Level | 16 bytes FIFO | 64 bytes FIFO |
     * |------------------------|---------------|---------------|
     * |          0             |    1 bytes    |    1 bytes    |
     * |          1             |    4 bytes    |   16 bytes    |
     * |          2             |    8 bytes    |   32 bytes    |
     * |          3             |   14 bytes    |   56 bytes    |
     *
     * 64 bytes FIFO will be enabled if it is available(i.e. model of UART chip
     * is 16750).
     */
    void configureFIFO(uint8_t interruptTriggerLevel) const;
    void configureModem() const;
    void configureInterrupt() const;

  public:
    size_t write(const char* buf, size_t count);
    size_t read(char* buf, size_t count);

  private:
    bool transmitFIFOEmpty() const;
    bool dataReady() const;

  public:
    // Unimplemented
    void onTransmitterHoldingRegisterEmpty();
    void onReceivedDataAvailable();

  private:
    uint16_t m_comBase;
  };

  extern SerialPort com1Port;
}
