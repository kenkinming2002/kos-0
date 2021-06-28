ENTRY(_start)

SECTIONS {
  . = ALIGN(0x1000);
  .text :
  {
    *(.text.*)
  }

  .rodata :
  {
    *(.rodata)
  }

  . = ALIGN(0x1000);
  .data :
  {
    *(.data)
  }

  .bss (NOLOAD) :
  {
    *(.bss)
  }

  . = ALIGN(0x1000);
  _end = .;
}



