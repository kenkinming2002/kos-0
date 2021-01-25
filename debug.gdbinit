file build/debug/boot/boot
file build/debug/kernel/kernel

target remote localhost:1024
set print pretty on

break kmain
