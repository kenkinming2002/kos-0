project('LittleBook', 'cpp', default_options : ['cpp_std=c++2a', 'build.cpp_std=c++2a'])

add_global_arguments('-fno-rtti',  language: ['cpp'])

# We cannot use fancy registers such as floating point and sse registers
add_global_arguments('-mgeneral-regs-only', language: ['c', 'cpp'])

# Freestanding without exceptions
add_global_arguments('-fno-exceptions', language: ['cpp'])
add_global_arguments('-ffreestanding',  language: ['c', 'cpp'])
add_global_link_arguments('-nostdlib',  language: ['c', 'cpp'])

# Intel inline assembly syntax
add_global_arguments('-masm=intel', language : ['c', 'cpp'])

# Strip unused functions
add_global_arguments(['-fdata-sections', '-ffunction-sections'], language : ['c', 'cpp'])
add_global_link_arguments(['-Wl,--gc-sections'], language : ['c', 'cpp'])
add_global_link_arguments(['-Wl,--sort-section,alignment'], language : ['c', 'cpp'])

nasm = find_program('nasm', required : true)
asm_gen = generator(nasm,
  output : '@PLAINNAME@.o',
  arguments : ['-f', 'elf32', '-g', '-F', 'dwarf', '@INPUT@', '-o', '@OUTPUT@'],
)

subdir('librt')
subdir('libcommon')
subdir('libelf')
subdir('boot')
subdir('kernel')
subdir('userspace')

mkinitrd = find_program('bin/mkinitrd')
initrd = custom_target('initrd',
  build_always_stale : true, # We do not have wildcard
  output : 'initrd',
  command : [mkinitrd, '@OUTPUT@', '@SOURCE_ROOT@/initrd/']
)

mkiso = find_program('bin/mkiso')
iso = custom_target('iso',
  input  : ['grub.cfg', boot, kernel, initrd, main1, main2],
  output : 'os.iso',
  command : [mkiso, '-p', '@PRIVATE_DIR@', '-o', '@OUTPUT@', '-c', '@INPUT@']
)

## TODO: Use crossfiles properties
qemu = find_program('qemu-system-i386')
run_target('run',       command : [qemu, '-d', 'cpu_reset', '-m', '60', '-cdrom', iso, '-serial', 'unix:qemu-serial,server,nowait'])
run_target('run_debug', command : [qemu, '-d', 'cpu_reset', '-m', '60', '-cdrom', iso, '-serial', 'unix:qemu-serial,server,nowait', '-monitor', 'unix:qemu-monitor,server,nowait', '-gdb', 'tcp::1024', '-S'])
