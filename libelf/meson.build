libelf_src = ['src/libelf.cpp']
libelf     = static_library('elf', libelf_src, include_directories : 'include/', dependencies : librt_dep)
libelf_dep = declare_dependency(include_directories : 'include/', link_with: libelf)

