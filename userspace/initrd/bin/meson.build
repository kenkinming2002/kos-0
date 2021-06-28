init_src = ['init.cpp']
init = executable('init',
  init_src,
  link_args : ['-T', meson.current_source_dir()+'/link.ld'],
  link_depends : 'link.ld',
  dependencies : [libsys_dep, librt_dep]
)


