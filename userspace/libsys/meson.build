libsys_asm_objs = asm_gen.process('src/crt0.s', 'src/Syscalls.s')
libsys_src = ['src/Hooks.cpp', 'src/brk.cpp']
libsys_inc = include_directories('include')

libsys = static_library('sys', libsys_asm_objs, libsys_src,
  include_directories : libsys_inc,
  dependencies : [librt_dep, kernel_dep]
)

libsys_dep = declare_dependency(
  include_directories : libsys_inc,
  link_with : libsys,
  dependencies : kernel_dep
)

