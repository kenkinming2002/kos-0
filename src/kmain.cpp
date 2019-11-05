#include <io/serial.h>

#include <grub/multiboot2.h>

#include <core/Interrupt.hpp>
#include <core/Segmentation.hpp>
#include <core/Paging.hpp>

#include <intel/core/pic/8259.hpp>

#include <init/Multiboot2.hpp>

#include <io/Framebuffer.hpp>

const char str[] = "Hello World!";

#define virtual_address(addr) (addr+0xC0000000)

void handler()
{
}


extern "C" int kmain(void* addr)
{
  core::Interrupt interrupt;
  core::Segmentation segmentation;

  core::pic::Controller8259 controller8259;

  /** Old-Styled Initialization **/
  init::multiboot2::parseBootInformation(virtual_address(addr));


  //serial_configure(SERIAL_COM1_BASE, 1);
  //serial_write(SERIAL_COM1_BASE, str, 11);

  for(int i=0; i<256; ++i)
    interrupt.installHandler(i, core::PrivillegeLevel::RING0, reinterpret_cast<core::Handler>(&handler));

  io::framebuffer::init();


  asm("int $20");

  io::FrameBuffer::Cursor cursor = {0,0};
  io::frameBuffer.write(cursor, "HELLO WORLD", 11, io::FrameBuffer::Color::WHITE, io::FrameBuffer::Color::BLACK);

  return 0;
}
