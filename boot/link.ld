ENTRY(loader)

SECTIONS {
  /*****************
   * Boot sections *
   *****************/
  . = 0x100000;

  boot_memory_begin = .;

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
