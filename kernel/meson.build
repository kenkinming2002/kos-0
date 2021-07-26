core_src = files(
  'src/generic/kmain.cpp',
  'src/generic/Init.cpp',
  'src/generic/PerCPU.cpp',
  'src/generic/BootInformation.cpp',
  'src/generic/librt/Hooks.cpp',

  'src/generic/log/Log.cpp',

  'src/generic/vfs/VFS.cpp',
  'src/generic/vfs/Initrd.cpp',
  'src/generic/vfs/Tar.cpp',
  'src/generic/vfs/Vnode.cpp',
  'src/generic/vfs/File.cpp',
  'src/generic/vfs/FileDescriptors.cpp',
  'src/generic/vfs/Syscalls.cpp',
  'src/generic/vfs/Mountable.cpp',
  'src/generic/vfs/mountables/filesystems/Tmpfs.cpp',
  'src/generic/vfs/mountables/devices/Serial.cpp',

  'src/generic/devices/Framebuffer.cpp',

  'src/generic/memory/MemoryArea.cpp',
  'src/generic/memory/Heap.cpp',
  'src/generic/memory/Virtual.cpp',
  'src/generic/memory/Memory.cpp',
  'src/generic/memory/Syscalls.cpp',
  'src/i686/memory/MemoryMapping.cpp',

  'src/x86/interrupts/PIC.cpp',
  'src/x86/interrupts/8259.cpp',
  'src/x86/interrupts/APIC.cpp',
  'src/i686/interrupts/Interrupts.cpp',

  'src/i686/internals/Internals.cpp',
  'src/i686/internals/Segmentation.cpp',

  'src/i686/syscalls/Syscalls.cpp',
  'src/i686/syscalls/Access.cpp',

  'src/generic/tasks/Tasks.cpp',
  'src/generic/tasks/Scheduler.cpp',
  'src/generic/tasks/Elf.cpp',
  'src/generic/tasks/Syscalls.cpp',
  'src/i686/tasks/Task.cpp',

  'src/x86/assembly/io.cpp',
  'src/x86/assembly/msr.cpp',
  'src/x86/acpi/ACPI.cpp'
)

core_asm_obj = asm_gen.process(
  'src/i686/Loader.s',
  'src/i686/interrupts/ISR.s',
  'src/i686/syscalls/Entry.s',
  'src/i686/tasks/Switch.s',
  'src/i686/tasks/Entry.s',
  preserve_path_from : meson.current_source_dir()
)

kernel = executable('kernel',
  core_src, core_asm_obj,
  include_directories : ['../boot/include/', 'include/'],
  cpp_args : ['-fstack-protector', '-fstack-protector-strong', '-fstack-protector-all'],
  link_args : ['-T', meson.current_source_dir()+'/link.ld', '-Wl,-no-dynamic-linker'],
  link_depends : 'link.ld',
  dependencies : [libcommon_dep, librt_dep, libelf_dep],
  pie : true,
  gnu_symbol_visibility : 'hidden',
  export_dynamic : true
)


kernel_dep = declare_dependency(include_directories : 'include')
