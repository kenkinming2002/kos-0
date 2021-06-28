subdir('bin')

initrd_src = [
  '@SOURCE_ROOT@/userspace/initrd/src/dir',
  '@SOURCE_ROOT@/userspace/initrd/src/stub',
  init
]

mkinitrd = find_program('../../bin/mkinitrd')
initrd = custom_target('initrd',
  build_always_stale : true, # We do not have wildcard
  output : 'initrd',
  command : [mkinitrd, '-p', '@PRIVATE_DIR@', '-o', '@OUTPUT@', initrd_src]
)


