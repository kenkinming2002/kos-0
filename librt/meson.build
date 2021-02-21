librt_src = [
  'src/Log.cpp',
  'src/Strings.cpp',
  'src/Assert.cpp',
  'src/New.cpp',
  'src/Panic.cpp'
]

librt_inc = include_directories('include')

librt     = static_library('rt', librt_src, include_directories : librt_inc)
librt_dep = declare_dependency(include_directories : librt_inc, link_with: librt)

