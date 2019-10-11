OBJECTS =
OBJECTS += src/core/boot/loader.o src/core/boot/multiboot2_header.o
OBJECTS += src/core/init/multiboot2.o src/core/init/Framebuffer.o src/core/init/mmap.o
OBJECTS += src/core/kmain.o src/core/gdt.o src/core/interrupt.o src/core/page.o src/core/pic.o src/core/interrupt_handler.o
OBJECTS += src/io/Framebuffer.o src/io/serial.o 
OBJECTS += src/asm/io.o src/asm/gdt.o src/asm/idt.o src/asm/page.o

CC = i686-elf-gcc
CFLAGS = -ffreestanding -I include/ -g -O3 #-Wall -Wextra

CXX = i686-elf-g++
CXXFLAGS = -std=c++17 -ffreestanding -I include/ -g -O3 #-Wall -Wextra

LD = i686-elf-ld
LDFLAGS = -nostdlib -T src/link.ld --gc-sections

AS = nasm
ASFLAGS = -f elf32 -g -F dwarf

OBJCOPY = i686-elf-objcopy

QEMUFLAGS 			= -serial file:serial.log
QEMUFLAGS_DEBUG = -serial file:serial.log -s -S


all: os.iso

## ISO
os.iso: kernel.elf program
	cp program iso/modules/
	cp kernel.elf iso/boot/kernel.elf
	grub-mkrescue -o os.iso iso -d /usr/lib/grub/i386-pc

## Kernel
kernel.elf: $(OBJECTS) src/link.ld
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@


## Program
program: src/program.s
	$(AS) -f bin $< -o $@


## Phonies
.PHONY: run clean
run: os.iso
	qemu-system-i386 -cdrom $< -m 256 -boot d $(QEMUFLAGS)

run_debug: os.iso
	./scripts/run_debug.sh "$(QEMUFLAGS_DEBUG)"

clean:
	find src/ -type f -name '*.o' -delete
	rm -f program kernel.elf os.iso
