ENTRY(loader)

SECTIONS {
  /*******************
   * Normal sections *
   *******************/
  . = 0xC0000000;

  kernel_virtual_start  = .;
  kernel_physical_start = . - 0xC0000000;

  /*********************
   * Read-Only Section *
   *********************/

  kernel_read_only_section_begin = .;

  .text : AT(ADDR(.text)-0xC0000000)
  {
    *(.text .text.*)
  }

  .rodata : AT(ADDR(.rodata)-0xC0000000)
  {
    *(.rodata .rodata.*)
  }

  .init : AT(ADDR(.init)-0xC0000000)
  {
    KEEP(*crti*.o(.init))
    KEEP(*crtbegin*.o(.init))
    KEEP(*crtend*.o(.init))
    KEEP(*crtn*.o(.init))
  }

  .fini : AT(ADDR(.fini)-0xC0000000)
  {
    KEEP(*crti*.o(.fini))
    KEEP(*crtbegin*.o(.fini))
    KEEP(*crtend*.o(.fini))
    KEEP(*crtn*.o(.fini))
  }

  . = ALIGN(0x1000);
  kernel_read_only_section_end = .;

  /*********************
   * Read-Write Section *
   *********************/
  kernel_read_write_section_begin = .;

  .data : AT(ADDR(.data)-0xC0000000)
  {
    *(.data .data.*)
  }

  .bss (NOLOAD) : AT(ADDR(.bss)-0xC0000000)
  {
    *(COMMON)
    *(.bss .bss.*)
  }

  .ctors : AT(ADDR(.ctors)-0xC0000000)
  {
    KEEP(*crtbegin*.o(.ctors))

    KEEP(*(EXCLUDE_FILE(*crtend*.o) .ctors))
    KEEP(*(SORT(.ctors.*)))

    KEEP(*(.ctors))
  }

  .dtors : AT(ADDR(.dtors)-0xC0000000)
  {
    KEEP(*crtbegin*.o(.dtors))

    KEEP(*(EXCLUDE_FILE(*crtend*.o) .dtors))
    KEEP(*(SORT(.dtors.*)))

    KEEP(*(.dtors))
  }

  . = ALIGN(0x1000);
  kernel_read_write_section_end = .;

  kernel_virtual_end = .;
  kernel_physical_end = . - 0xC0000000;

}
