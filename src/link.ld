ENTRY(loader)

SECTIONS {
  /*******************
   * Normal sections *
   *******************/
  . = 0xC0100000; /* Set Virtual Address to 1MB above 3GB mark */

  kernel_virtual_start  = .;
  kernel_physical_start = . - 0xC0000000;

  .text ALIGN (0x1000) : AT(ADDR(.text)-0xC0000000)
  {
    KEEP(*(.multiboot2_header))
    *(.text.*)
  }

  .rodata ALIGN(0x1000) : AT(ADDR(.rodata)-0xC0000000)
  {
    *(.rodata.*)
  }

  .data ALIGN(0x1000) : AT(ADDR(.data)-0xC0000000)
  {
    *(.data.*)
  }

  .ctors : AT(ADDR(.ctors)-0xC0000000)
  {
    KEEP(*crtbegin.o(.ctors))
    KEEP(*crtbegin?.o(.ctors))

    KEEP(*(EXCLUDE_FILE(*crtend.o *crtend?.o) .ctors))
    KEEP(*(SORT(.ctors.*)))

    KEEP(*(.ctors))
  }

  .dtors : AT(ADDR(.dtors)-0xC0000000)
  {
    KEEP(*crtbegin.o(.dtors))
    KEEP(*crtbegin?.o(.dtors))

    KEEP(*(EXCLUDE_FILE(*crtend.o *crtend?.o) .dtors))
    KEEP(*(SORT(.dtors.*)))

    KEEP(*(.dtors))
  }

  .init : AT(ADDR(.init)-0xC0000000)
  {
    KEEP(*(SORT_NONE(.init)))
  }

  .fini : AT(ADDR(.fini)-0xC0000000)
  {
    KEEP(*(SORT_NONE(.fini)))
  }

  .bss ALIGN(0x1000) (NOLOAD) : AT(ADDR(.bss)-0xC0000000)
  {
    *(COMMON)
    *(.bss)
  }
  . = ALIGN(4096);

  kernel_virtual_end = .;
  kernel_physical_end = . - 0xC0000000;

  /*****************
   * Boot sections *
   *****************/
  . = . - 0xC0000000; /* Set Virtual Address back to 0GB mark, offset by physical address of preceding section */

  .boot.text ALIGN(0x1000) :
  {
    *(.text.boot.*)
  }

  .boot.rodata ALIGN(0x1000) :
  {
    *(.rodata.boot.*)
  }

  .boot.data ALIGN(0x1000) :
  {
    *(.data.boot.*)
  }

  .boot.bss ALIGN(0x1000) (NOLOAD) : 
  {
    *(.bss.boot.*)
  }
}
