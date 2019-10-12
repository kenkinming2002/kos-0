#include <core/PIC.hpp>

#include <asm/io.h>

//#define PIC1_PORT_A 0x20
//#define PIC2_PORT_A 0xA0
//
///* The PIC interrupts have been remapped */
//#define PIC1_START_INTERRUPT 0x20
//#define PIC2_START_INTERRUPT 0x28
//#define PIC2_END_INTERRUPT   PIC2_START_INTERRUPT + 7
//
//#define PIC_ACK     0x20

#define PIC1         0x20
#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1+1)

#define PIC2         0xA0
#define PIC2_COMMAND PIC1
#define PIC2_DATA    (PIC1+1)

#define ICW1_ICW4      0x01
#define ICW1_SINGLE    0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL     0x08
#define ICW1_INIT      0x10

#define ICW4_8086	      0x01
#define ICW4_AUTO	      0x02
#define ICW4_BUF_SLAVE	0x08
#define ICW4_BUF_MASTER	0x0C
#define ICW4_SFNM	      0x10

#define PIC_EOI 0x20

int PICController::init() const
{
  return remap(0x20, 0x2F);
}


int PICController::remap(uint8_t master_offset, uint8_t slave_offset) const
{
  // Initialize an 8259 PIC
  uint8_t mask_master, mask_slave;

  mask_master = inb(PIC1_DATA);
  mask_slave  = inb(PIC1_DATA);

  // ICW1
  outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

  // ICW3
  outb(PIC1_DATA, master_offset);
  outb(PIC2_DATA, slave_offset);

  // ICW3
  outb(PIC1_DATA, 4);
  outb(PIC2_DATA, 2);

  // ICW4
  outb(PIC1_DATA, ICW4_8086);
  outb(PIC2_DATA, ICW4_8086);

  outb(PIC1_DATA, mask_master);
  outb(PIC2_DATA, mask_slave);

  return 0;
}

int PICController::acknowledge(uint32_t interrupt) const
{
  if(interrupt>=8)
    outb(PIC2_COMMAND, PIC_EOI);

  outb(PIC1_COMMAND, PIC_EOI);

  return 0;
}
