#!/bin/sh

qemu-system-i386 -boot d -cdrom os.iso -m 256 $1 &
sleep 1
exec gdb -x debug.gdbinit
