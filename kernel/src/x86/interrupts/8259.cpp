#include <x86/interrupts/8259.hpp>

#include <x86/assembly/io.hpp>

#include <librt/Log.hpp>

#include <stddef.h>

namespace core::interrupts
{
  constexpr static size_t MASTER_COMMAND_PORT = 0x0020;
  constexpr static size_t MASTER_DATA_PORT    = 0x0021;

  constexpr static size_t SLAVE_COMMAND_PORT = 0x00A0;
  constexpr static size_t SLAVE_DATA_PORT    = 0x00A1;

  constexpr static uint8_t EOI = 0x20;

  enum ICW1 : uint8_t
  {
    ICW1_ICW4            = 1 << 0, // ICW4 Needed(not needed)
    ICW1_SINGLE          = 1 << 1, // Single PIC(Dual PIC)
    ICW1_INTERVAL4       = 1 << 2, // Call Address Interval 4(8)
    ICW1_LEVEL_TRIGGERED = 1 << 3, // Level(Edge) Triggered Mode
    ICW1_INIT	           = 1 << 4, // Initialization Bit
  };

  enum ICW4 : uint8_t
  {
    ICW4_8086	      = 1 << 0,	// 8086/88 (MCS-80/85) Mode
    ICW4_AUTO	      = 1 << 1,	// Auto (normal) EOI
    ICW4_BUF_SLAVE	= 0x08  ,	// Buffered Mode/Slave
    ICW4_BUF_MASTER = 0x0C  ,	// Buffered Mode/Master
    ICW4_SFNM	      = 1 << 4,	// Special Fully Nested Mode
  };

  void initialize8259()
  {
    rt::log("Configuring 8259 PIC...");

    // Initialization Command Word 1 - starts the initialization sequence (in cascade mode)
    assembly::outb(MASTER_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);  
    assembly::ioWait();
    assembly::outb(SLAVE_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);
    assembly::ioWait();
    
    // Initialization Command Word 2 - set master and slave offset
    assembly::outb(MASTER_DATA_PORT, PIC_OFFSET);
    assembly::ioWait();
    assembly::outb(SLAVE_DATA_PORT, PIC_OFFSET+8);
    assembly::ioWait();

    // Initialization Command Word 3 - specify we use IRQ2 line to communicate
    //                                 between master and slave PIC
    assembly::outb(MASTER_DATA_PORT, 1 << 2);
    assembly::ioWait();
    assembly::outb(SLAVE_DATA_PORT, 2);
    assembly::ioWait();
   
    // Initialization Command Word 4
    assembly::outb(MASTER_DATA_PORT, ICW4_8086);
    assembly::ioWait();
    assembly::outb(SLAVE_DATA_PORT, ICW4_8086);
    assembly::ioWait();

    // Set mask
    assembly::outb(MASTER_DATA_PORT, ~static_cast<uint8_t>(1<<2)); // Mask all IRQs except IRQ2 used for cascading
    assembly::outb(SLAVE_DATA_PORT,  ~static_cast<uint8_t>(0));    // Mask all IRQs

    // TODO: Register handler for spurious interrupt

    rt::log("Done\n");
  }

  void setMask(uint8_t irqLine)
  {
    uint16_t port;
    uint8_t value;
 
    if(irqLine < 8) 
    {
        port = MASTER_DATA_PORT;
    } 
    else 
    {
        port = SLAVE_DATA_PORT;
        irqLine -= 8;
    }

    value = assembly::inb(port) | static_cast<uint8_t>(1 << irqLine);
    assembly::outb(port, value); 
  }

  void clearMask(uint8_t irqLine)
  {
    uint16_t port;
    uint8_t value;
 
    if(irqLine < 8) 
    {
        port = MASTER_DATA_PORT;
    } 
    else 
    {
        port = SLAVE_DATA_PORT;
        irqLine -= 8;
    }

    value = assembly::inb(port) & ~static_cast<uint8_t>(1 << irqLine);
    assembly::outb(port, value); 
  }

  void acknowledge(uint8_t interrupt)
  {
    if(interrupt>=8)
      assembly::outb(SLAVE_COMMAND_PORT, EOI);

    assembly::outb(MASTER_COMMAND_PORT, EOI);
  }
}
