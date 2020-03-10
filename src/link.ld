OUTPUT_FORMAT(binary)

STARTUP(src/crt0.o)

SECTIONS {
  .text :
  {
    *(.text.*)
  }

  .rodata :
  {
    *(.rodata)
  }

  .data :
  {
    *(.data)
  }

  .bss :
  {
    *(.bss)
  }
}


