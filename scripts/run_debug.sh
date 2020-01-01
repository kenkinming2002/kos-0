#!/bin/sh

urxvt -e qemu-system-i386 -boot d -cdrom os.iso -m 256 -monitor stdio $1 &
sleep 1
urxvt -e gdb -x debug.gdbinit &
