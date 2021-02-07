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

  /*********************
   * Read-Write Section *
   *********************/
  . = ALIGN(0x1000);
  .data :
  {
    *(.data .data.*)
  }

  .bss (NOLOAD) :
  {
    *(.bss .bss.* COMMON)
  }
}
