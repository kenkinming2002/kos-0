ENTRY(loader)

SECTIONS {
  /*********************
   * Read-Only Section *
   *********************/
  . = 0xC0000000;
  .text :
  {
    *(.text .text.*)
  }

  .rodata :
  {
    *(.rodata .rodata.*)
  }

  .init :
  {
    KEEP(*crti*.o(.init))
    KEEP(*crtbegin*.o(.init))
    KEEP(*crtend*.o(.init))
    KEEP(*crtn*.o(.init))
  }

  .fini :
  {
    KEEP(*crti*.o(.fini))
    KEEP(*crtbegin*.o(.fini))
    KEEP(*crtend*.o(.fini))
    KEEP(*crtn*.o(.fini))
  }

  /*********************
   * Read-Write Section *
   *********************/
  . = ALIGN(0x1000);
  .ctors :
  {
    KEEP(*crtbegin*.o(.ctors))

    KEEP(*(EXCLUDE_FILE(*crtend*.o) .ctors))
    KEEP(*(SORT(.ctors.*)))

    KEEP(*(.ctors))
  }

  .dtors :
  {
    KEEP(*crtbegin*.o(.dtors))

    KEEP(*(EXCLUDE_FILE(*crtend*.o) .dtors))
    KEEP(*(SORT(.dtors.*)))

    KEEP(*(.dtors))
  }

  .data :
  {
    *(.data .data.*)
  }

  .bss (NOLOAD) :
  {
    *(.bss .bss.* COMMON)
  }

  . = ALIGN(0x1000);
}
