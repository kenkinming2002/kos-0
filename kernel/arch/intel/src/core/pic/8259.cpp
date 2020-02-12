#include <intel/core/pic/8259.hpp>

#include <intel/asm/io.hpp>

namespace core::pic::controller8259
{
  constexpr static size_t MASTER_COMMAND_PORT = 0x0020;
  constexpr static size_t MASTER_DATA_PORT    = 0x0021;

  constexpr static size_t SLAVE_COMMAND_PORT = 0x00A0;
  constexpr static size_t SLAVE_DATA_PORT    = 0x00A1;

  constexpr static uint8_t EOI = 0x20;

  constexpr static uint8_t ICW1_ICW4       = 0x01;		/* ICW4 (not) needed */
  constexpr static uint8_t ICW1_SINGLE     = 0x02;		/* Single (cascade) mode */
  constexpr static uint8_t ICW1_INTERVAL4  = 0x04;		/* Call address interval 4 (8) */
  constexpr static uint8_t ICW1_LEVEL	     = 0x08;		/* Level triggered (edge) mode */
  constexpr static uint8_t ICW1_INIT	     = 0x10;		/* Initialization - required! */

  constexpr static uint8_t ICW4_8086	     = 0x01;		/* 8086/88 (MCS-80/85) mode */
  constexpr static uint8_t ICW4_AUTO	     = 0x02;		/* Auto (normal) EOI */
  constexpr static uint8_t ICW4_BUF_SLAVE	 = 0x08;		/* Buffered mode/slave */
  constexpr static uint8_t ICW4_BUF_MASTER = 0x0C;		/* Buffered mode/master */
  constexpr static uint8_t ICW4_SFNM	     = 0x10;		/* Special fully nested (not) */

  constexpr static uint8_t MASTER_CASCADE_MASK = ~static_cast<uint8_t>(1<<2);

  int init()
  {
    // Initialization Command Word 1 - starts the initialization sequence (in cascade mode)
    assembly::outb(MASTER_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);  
    //io_wait();
    assembly::outb(SLAVE_COMMAND_PORT, ICW1_INIT | ICW1_ICW4);
    //io_wait();
    
    // Initialization Command Word 2 - master and slave offset
    assembly::outb(MASTER_DATA_PORT, DEFAULT_MASTER_OFFSET);
    //io_wait();
    assembly::outb(SLAVE_DATA_PORT, DEFAULT_SLAVE_OFFSET);
    //io_wait();

    // Initialization Command Word 3
    assembly::outb(MASTER_DATA_PORT, 4);// ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    //io_wait();
    assembly::outb(SLAVE_DATA_PORT, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    //io_wait();
   
    // Initialization Command Word 4
    assembly::outb(MASTER_DATA_PORT, ICW4_8086);
    //io_wait();
    assembly::outb(SLAVE_DATA_PORT, ICW4_8086);
    //io_wait();

    // Set mask
    assembly::outb(MASTER_DATA_PORT, MASTER_CASCADE_MASK);
    assembly::outb(SLAVE_DATA_PORT, 0u);

    asm("sti");

    return 0;
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
