#include <core/interrupt_handler.h>


#include <io/serial.h>

#include <asm/page.h>

#include <grub/multiboot2.h>

#include <core/Interrupt.hpp>
#include <core/PIC.hpp>
#include <core/Paging.hpp>
#include <core/Segmentation.hpp>

#include <core/init/Multiboot2.hpp>
#include <core/init/MemoryMap.hpp>
#include <core/init/Framebuffer.hpp>

#include <io/Framebuffer.hpp>

const char str[] = "Hello World!";

char* itoa(unsigned value, unsigned base, char* buf, unsigned length)
{
  for(int i = length-1; i>=0; --i)
  {
    unsigned quotient = value / base;
    unsigned remainder = value % base;

    buf[i] = '0' + remainder;

    if(!quotient)
      return &buf[i];

    value = quotient;
  }

  return 0;
}

//fb_t fb;
IDTEntry idtEntries[256];
GDTEntry gdtEntries[3];
__attribute((aligned(0x1000)))PageDirectory pageDirectories[1024];

PICController g_picController;

#define virtual_address(addr) (addr+0xC0000000)

extern "C" int kmain(void* addr)
{
  init::multiboot2::parseBootInformation(virtual_address(addr));
  framebuffer_init();

  serial_configure(SERIAL_COM1_BASE, 1);
  serial_write(SERIAL_COM1_BASE, str, 11);

  gdtEntries[0] = GDTEntry(0, 0,          PrivillegeLevel::RING0, SegmentType::NONE_SEGMENT);
  gdtEntries[1] = GDTEntry(0, 0xffffffff, PrivillegeLevel::RING0, SegmentType::CODE_SEGMENT_RD);
  gdtEntries[2] = GDTEntry(0, 0xffffffff, PrivillegeLevel::RING0, SegmentType::DATA_SEGMENT);

  GDT(gdtEntries, 3).load();

  g_picController.init();

  for(size_t i=0; i<256; ++i)
    idtEntries[i] = IDTEntry(InterruptType::INTERRUPT_GATE_32, PrivillegeLevel::RING0, 0x08, reinterpret_cast<void*>(&keyboard_interrupt_handler));
  IDT(idtEntries).load();

  asm("int $20");

  FrameBufferCursor cursor = {0,0};
  g_frameBuffer.write(cursor, "HELLO WORLD", 11, FrameBufferColor::WHITE, FrameBufferColor::BLACK);

  return 0;
}
