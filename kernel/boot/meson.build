boot_src = files(
  'src/i686/Boot.cpp',
  'src/i686/HigherHalf.cpp',
  'src/i686/LowerHalf.cpp',
)

boot_asm_obj = asm_gen.process(
  'src/i686/Multiboot2Header.s',
  'src/i686/Loader.s'
)
