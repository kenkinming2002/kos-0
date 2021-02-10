librt_src = [
  'src/Log.cpp',
  'src/Strings.cpp',
  'src/Assert.cpp',
  'src/New.cpp',
  'src/Panic.cpp'
]
librt = static_library('librt', librt_src, include_directories : 'include/', pic: false)
librt_dep = declare_dependency(include_directories : 'include/', link_with: librt)

