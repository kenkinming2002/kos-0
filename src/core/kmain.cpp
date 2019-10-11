extern "C"
{
#include <core/gdt.h>
#include <core/pic.h>
#include <core/interrupt.h>
#include <core/interrupt_handler.h>

#include <core/init/multiboot2.h>
#include <core/init/mmap.h>

#include <io/serial.h>

#include <asm/page.h>
#include <core/page.h>

#include <grub/multiboot2.h>
}

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
idt_entry_t idt_entries[256];
gdt_entry_t gdt_entries[3];
__attribute((aligned(0x1000)))struct page_directory page_directories[1024];

#define virtual_address(addr) (addr+0xC0000000)

extern "C" int kmain(void* addr)
{
  //parse_multiboot(virtual_address(addr));
  multiboot2_boot_information_parse(virtual_address(addr));
  framebuffer_init();

  serial_configure(SERIAL_COM1_BASE, 1);
  serial_write(SERIAL_COM1_BASE, str, 11);

  gdt_init_entry(&gdt_entries[0], 0, 0,          RING0, NONE_SEGMENT);
  gdt_init_entry(&gdt_entries[1], 0, 0xffffffff, RING0, CODE_SEGMENT_RD);
  gdt_init_entry(&gdt_entries[2], 0, 0xffffffff, RING0, DATA_SEGMENT);
  gdt_update(gdt_entries, 3);

  pic_init();

  for(size_t i=0; i<256; ++i)
    idt_init_entry(&idt_entries[i], IRQ32_INTERRUPT_GATE, IRQ_RING0, 0x08, reinterpret_cast<void*>(&keyboard_interrupt_handler));
  idt_update(&idt_entries);

  FrameBufferCursor cursor = {0,0};
  g_frameBuffer.write(cursor, "HELLO WORLD", 11, FrameBufferColor::WHITE, FrameBufferColor::BLACK);

  return 0;
}
