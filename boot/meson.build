boot_src = files(
  'src/generic/BootInfomation.cpp',
  'src/generic/Kernel.cpp',
  'src/generic/Memory.cpp',
  'src/i686/librt/Hooks.cpp',
  'src/i686/Paging.cpp',
  'src/i686/bmain.cpp'
)

boot_asm_obj = asm_gen.process(
  'src/generic/Multiboot2Header.s',
  'src/i686/Loader.s'
)

boot = executable('boot',
  boot_src, boot_asm_obj,
  include_directories : ['include/'],
  link_args : ['-T', meson.current_source_dir()+'/link.ld'],
  link_depends : 'link.ld',
  dependencies : [libcommon_dep, librt_dep]
)

