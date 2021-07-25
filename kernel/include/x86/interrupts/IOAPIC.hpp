#pragma once

#include <generic/memory/Memory.hpp>
#include <generic/memory/Virtual.hpp>

#include <librt/Assert.hpp>
#include <librt/Strings.hpp>

#include <stdint.h>
#include <stddef.h>

namespace core::interrupts
{
  enum class DeliveryMode
  {
    FIXED           = 0b000,
    LOWEST_PRIORITY = 0b001,
    SMI             = 0b010,
    NMI             = 0b100,
    INIT            = 0b101,
    EXTINT          = 0b111
  };

  enum class DestinationMode { PHYSICAL = 0, LOGICAL = 1 };
  enum class Polarity { ACTIVE_HIGH = 0, ACTIVE_LOW = 1 };
  enum class TriggerMode { EDGE = 0, LEVEL = 1 };

  struct [[gnu::packed]] RedirectionEntry
  {
  public:
    constexpr RedirectionEntry() = default;
    constexpr RedirectionEntry(uint8_t vector, DeliveryMode deliveryMode, DestinationMode destinationMode, Polarity polarity, TriggerMode triggerMode, uint8_t destination)
    {
      this->vector = vector;
      this->reserved = 0;
      this->masked   = 0;
      this->flags =
        static_cast<uint8_t>(deliveryMode)    << 0 |
        static_cast<uint8_t>(destinationMode) << 3 |
        /* Remote IRR */
        static_cast<uint8_t>(polarity)        << 5 |
        /* Delivery status */
        static_cast<uint8_t>(triggerMode)     << 7;
      this->destination = destination;
    }

  public:
    uint8_t  vector      = 0;
    uint8_t  flags       = 0;
    uint8_t  masked      = 1;
    uint32_t reserved    = 0;
    uint8_t  destination = 0;
  };

  class IOAPIC
  {
  public:
    static constexpr uint32_t IOAPICID  = 0x0;
    static constexpr uint32_t IOAPICVER = 0x1;
    static constexpr uint32_t IOAPICARB = 0x2;

  public:
    IOAPIC(memory::physaddr_t physaddr, uint32_t gsiBase)
    {
      // There is a problem, it is impossible to access such a high address
      m_physaddr  = physaddr;
      m_gsiBase   = gsiBase;
      m_gsiCount  = (read(IOAPICVER) >> 16) & 0xFF; // Retrive the third byte
    }

  public:
    uint32_t read(uint32_t offset) const
    {
      memory::KMapGuard guard(m_physaddr);
      *reinterpret_cast<uint32_t*>(guard.addr) = offset;
      return *reinterpret_cast<uint32_t*>(guard.addr+0x10);
    }

    void write(uint32_t offset, uint32_t data) const
    {
      memory::KMapGuard guard(m_physaddr);
      *reinterpret_cast<uint32_t*>(guard.addr)      = offset;
      *reinterpret_cast<uint32_t*>(guard.addr+0x10) = data;
    }

  public:
    bool handle(unsigned gsi) const { return m_gsiBase <= gsi && gsi < m_gsiBase + m_gsiCount; }

  public:
    RedirectionEntry readRedirectionEntry(unsigned gsi) const
    {
      size_t index = gsi - m_gsiBase;
      ASSERT(index < m_gsiCount);

      uint32_t data[2];
      data[0] = read(0x10+2*index);
      data[1] = read(0x10+2*index+1);

      RedirectionEntry redirectionEntry;
      rt::memcpy(&redirectionEntry, data, sizeof redirectionEntry);
      return redirectionEntry;
    }

    void writeRedirectionEntry(unsigned gsi, RedirectionEntry redirectionEntry) const
    {
      size_t index = gsi - m_gsiBase;
      ASSERT(index < m_gsiCount);

      uint32_t data[2] = {};
      static_assert(sizeof redirectionEntry == sizeof data);
      rt::memcpy(data, &redirectionEntry, sizeof redirectionEntry);
      write(0x10+2*index,   data[0]);
      write(0x10+2*index+1, data[1]);
    }

  private:
    memory::physaddr_t m_physaddr;
    uint32_t           m_gsiBase;
    uint32_t           m_gsiCount;

  };
}
