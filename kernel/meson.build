core_src = files(
  'src/generic/kmain.cpp',
  'src/generic/BootInformation.cpp',

  'src/generic/stdlib/assert.cpp',
  'src/generic/stdlib/string.cpp',
  'src/generic/stdlib/new.cpp',

  'src/generic/io/Print.cpp',

  'src/generic/devices/Framebuffer.cpp',

  'src/generic/memory/Memory.cpp',
  'src/generic/memory/LinkedListPagesAllocator.cpp',
  'src/generic/memory/liballocImpl.cpp',
  'src/i686/memory/MemoryMapping.cpp',

  'src/x86/interrupts/8259.cpp',
  'src/i686/interrupts/Interrupts.cpp',

  'src/i686/internals/Internals.cpp',
  'src/i686/internals/Segmentation.cpp',

  'src/i686/syscalls/Syscalls.cpp',

  'src/generic/tasks/Scheduler.cpp',
  'src/generic/tasks/Elf.cpp',
  'src/i686/tasks/Task.cpp',

  'src/x86/assembly/io.cpp',
  'src/x86/assembly/msr.cpp',

  'src/i686/crt0.cpp'
)

core_asm_obj = asm_gen.process(
  'src/i686/Loader.s',
  'src/i686/interrupts/ISR.s',
  'src/i686/syscalls/Entry.s',
  'src/i686/tasks/Switch.s',
  'src/i686/tasks/Entry.s',
  preserve_path_from : meson.current_source_dir()
)

liballoc_proj = subproject('liballoc')
liballoc_dep = liballoc_proj.get_variable('liballoc_dep')

kernel = executable('kernel', 
  core_src, core_asm_obj,
  include_directories : ['../common/include/', '../boot/include/', 'include/'],
  link_args : ['-T', meson.current_source_dir()+'/link.ld'],
  link_depends : 'link.ld',
  dependencies : [liballoc_dep, common_dep]
)

