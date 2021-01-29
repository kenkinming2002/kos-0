project('LittleBook', 'cpp', default_options : ['cpp_std=c++2a'])

add_project_arguments('-masm=intel', language : 'cpp')

add_project_arguments(['-fdata-sections', '-ffunction-sections'], language : ['c', 'cpp'])
add_project_link_arguments(['-Wl,--gc-sections'], language : ['c', 'cpp'])
add_project_link_arguments(['-Wl,--sort-section,alignment'], language : ['c', 'cpp'])

nasm = find_program('nasm', required : true)
asm_gen = generator(nasm,
  output : '@PLAINNAME@.o',
  arguments : ['-f', 'elf32', '-g', '-F', 'dwarf', '@INPUT@', '-o', '@OUTPUT@'],
)

subdir('common')
subdir('boot')
subdir('kernel')
subdir('userspace')

mkiso = find_program('bin/mkiso')
iso = custom_target('iso',
  build_by_default : true,
  output : 'os.iso',
  input  : ['grub.cfg', boot, kernel, main1, main2],
  command : [mkiso, '-p', '@PRIVATE_DIR@', '-o', '@OUTPUT@', '-c', '@INPUT@']
)

## TODO: Use crossfiles properties
qemu = find_program('qemu-system-i386') 
run_target('run',       command : [qemu, '-d', 'cpu_reset', '-m', '60', '-cdrom', iso])
run_target('run_debug', command : [qemu, '-d', 'cpu_reset', '-m', '60', '-cdrom', iso, '-serial' , 'unix:qemu-serial,server,nowait', '-monitor', 'unix:qemu-monitor,server,nowait', '-gdb', 'tcp::1024', '-S'])
