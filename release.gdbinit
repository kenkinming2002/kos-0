set disassembly-flavor intel
set disassemble-next-line on

file build/release/boot/boot
file build/release/kernel/kernel

target remote localhost:1024
set print pretty on

break kmain

