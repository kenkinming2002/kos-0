#pragma once

#include <generic/vfs/Inode.hpp>
#include <generic/vfs/Mountable.hpp>

#include <stdint.h>
#include <stddef.h>
#include <utility>
#include <algorithm>

namespace core::vfs
{
  void initializeSerial();

  class Serial : public Mountable
  {
  public:
    Result<rt::SharedPtr<Inode>> mount(rt::StringRef arg) override;
    rt::StringRef name() override;
  };

  enum COMBase : uint16_t
  {
    COM1_BASE = 0x3F8,
    COM2_BASE = 0x2F8,
    COM3_BASE = 0x3E8,
    COM4_BASE = 0x2E8
  };

  class SerialInode : public Inode
  {
  public:
    constexpr static size_t BUFFER_SIZE = 64;

  public:
    SerialInode() = default;
    SerialInode(uint16_t comBase);

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

  private:
    bool transmitFIFOEmpty() const;
    bool dataReady() const;

  private:
    // Unimplemented
    void onTransmitterHoldingRegisterEmpty();
    void onReceivedDataAvailable();

  public:
    Result<ssize_t> read(char* buf, size_t count, size_t pos) override;
    Result<ssize_t> write(const char* buf, size_t count, size_t pos) override;

  private:
    uint16_t m_comBase;
  };
}
