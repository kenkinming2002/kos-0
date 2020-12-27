main1_asm_obj = asm_gen.process('main1.s')
main1 = executable('main1', 
  main1_asm_obj,
  link_args : ['-T', meson.current_source_dir()+'/link.ld'],
  link_depends : 'link.ld'
)

main2_asm_obj = asm_gen.process('main2.s')
main2 = executable('main2', 
  main2_asm_obj,
  link_args : ['-T', meson.current_source_dir()+'/link.ld'],
  link_depends : 'link.ld'
)
