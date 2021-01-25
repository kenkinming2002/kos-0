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

  .bss (NOLOAD) : 
  {
    *(.bss .bss.*)
  }

  kernelImageBegin = .;
  .kernelImage ALIGN(0x1000) (NOLOAD) :
  {
    *(.kernelImage .kernelImage.*)
    . = ALIGN(0x1000);
  }
  kernelImageEnd = .;

  bootInformationBegin = .;
  .bootInformation ALIGN(0x1000) (NOLOAD) :
  {
    *(.bootInformation .bootInformation.*)
    . = ALIGN(0x1000);
  }
  bootInformationEnd = .;

  pagingBegin = .;
  .paging ALIGN(0x1000) (NOLOAD) :
  {
    *(.paging .paging.*)
    . = ALIGN(0x1000);
  }
  pagingEnd = .;

  boot_memory_end = .;
}
