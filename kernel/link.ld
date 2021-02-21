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

  /* Dynamic Relocation Information */
  .rel.dyn :
  {
    *(.rel.dyn)
  }

  /* Dynamic Symbols */
  .dynsym :
  {
    *(.dynsym)
  }

  .dynstr :
  {
    *(.dynstr)
  }

  .gnu.hash :
  {
    *(.gnu.hash)
  }


  /*********************
   * Read-Write Section *
   *********************/
  . = ALIGN(0x1000);
  .data :
  {
    *(.data .data.*)
  }

  .got.plt :
  {
    *(.got.plt)
  }

  .dynamic :
  {
    *(.dynamic)
  }

  .bss (NOLOAD) :
  {
    *(.bss .bss.* COMMON)
  }
}
