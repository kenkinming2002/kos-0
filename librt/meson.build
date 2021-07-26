librt_src = [
  'src/Log.cpp',
  'src/Strings.cpp',
  'src/Assert.cpp',
  'src/New.cpp',
  'src/Panic.cpp',
  'src/Trace.cpp',
  'src/Hooks.cpp',
  'src/cxxabi.cpp',
  'src/liballocImpl.cpp',
  'src/liballoc_1_1.c',
  'src/containers/IntrusiveListBase.cpp'
]

librt_inc = include_directories('include')

librt        = static_library('rt',        librt_src, include_directories : librt_inc, pic : false)
librt_native = static_library('rt_native', librt_src, include_directories : librt_inc, native : true)

librt_dep        = declare_dependency(include_directories : librt_inc, link_with: librt)
librt_native_dep = declare_dependency(include_directories : librt_inc, link_with: librt_native)

thread_dep = dependency('threads')
librt_test = executable('librt_test', 'src/Test.cpp',
  dependencies : [librt_native_dep, thread_dep],
  native : true)

test('librt', librt_test)

