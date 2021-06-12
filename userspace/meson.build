crt0_asm_obj = asm_gen.process('crt0.s')
syscalls_asm_obj = asm_gen.process('Syscalls.s')

main1_src = ['main1.cpp', 'Hooks.cpp']
main1 = executable('main1',
  crt0_asm_obj, syscalls_asm_obj, main1_src,
  include_directories : ['../include'],
  link_args : ['-T', meson.current_source_dir()+'/link.ld'],
  link_depends : 'link.ld',
  dependencies : [librt_dep]
)

main2_src = ['main2.cpp']
main2 = executable('main2',
  crt0_asm_obj, syscalls_asm_obj, main2_src,
  include_directories : ['../include'],
  link_args : ['-T', meson.current_source_dir()+'/link.ld'],
  link_depends : 'link.ld',
  dependencies : [librt_dep]
)
