ENTRY(loader)

SECTIONS {
  /*****************
   * AP Trampoline *
   *****************/
  . = 0x8000;

  .ap_trampoline :
  {
    KEEP(*(.ap_trampoline))
  }

  /*****************
   * Boot sections *
   *****************/
  . = 0x100000;

  .text :
  {
    KEEP(*(.multiboot2_header))
    *(.text .text.*)
  }

  .rodata :
  {
    *(.rodata .rodata.*)
  }

  .data :
  {
    *(.data .data.*)
  }

  .got.plt :
  {
    *(.got.plt)
  }

  .bss (NOLOAD) :
  {
    *(.bss .bss.*)
  }
}
