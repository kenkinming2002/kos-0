subdir('boot')
subdir('core')

liballoc_proj = subproject('liballoc')
liballoc_dep = liballoc_proj.get_variable('liballoc_dep')

compiler = meson.get_compiler('cpp')
crtbegin_obj = run_command(compiler, '-print-file-name=crtbegin.o').stdout().split('\n')[0]
crtend_obj   = run_command(compiler, '-print-file-name=crtend.o').stdout().split('\n')[0]

kernel = executable('kernel', 
  boot_src, boot_asm_obj, core_src, core_asm_obj,
  objects : [crtbegin_obj, crtend_obj],
  include_directories : ['common/include/', 'boot/include/', 'core/include/', 'utils/include/'],
  link_args : ['-T', meson.current_source_dir()+'/link.ld'],
  link_depends : 'link.ld',
  dependencies : liballoc_dep
)

