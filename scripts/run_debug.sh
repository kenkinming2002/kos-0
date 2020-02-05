#!/bin/sh

qemu-system-i386 -boot d -cdrom os.iso -m 256 \
  -serial unix:qemu-serial-socket,server,nowait \
  -monitor unix:qemu-monitor-socket,server,nowait \
  -s -S &

sleep 1

urxvt -e ./scripts/debug.sh &
