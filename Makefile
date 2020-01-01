include build/Begin.mk

OBJECTS += src/kmain.o

# TODO: include only the target
generic_cwd := arch/generic
include $(generic_cwd)/Rules.mk
intel_cwd := arch/intel
include $(intel_cwd)/Rules.mk
i686_cwd := arch/i686
include $(i686_cwd)/Rules.mk

include build/Rules.mk

.DEFAULT_GOAL := all
all: os.iso

CPPFLAGS += -I include/

## ISO
os.iso: kernel.elf program
	cp program iso/modules/
	cp kernel.elf iso/boot/kernel.elf
	grub-mkrescue -o os.iso iso -d /usr/lib/grub/i386-pc

## Program
program: src/program.s
	$(AS) -f bin $< -o $@

# QEMU
QEMUFLAGS 			= -serial file:serial.log
QEMUFLAGS_DEBUG = -serial file:serial.log -s -S 

.PHONY: run 
run: os.iso
	qemu-system-i386 -cdrom $< -m 512 -boot d $(QEMUFLAGS)

run_debug: os.iso
	./scripts/run_debug.sh "$(QEMUFLAGS_DEBUG)"
