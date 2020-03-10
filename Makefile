include build/Begin.mk

KOBJECTS = src/kmain.o liballoc/liballoc_1_1.o
CPPFLAGS += -I liballoc/ \
						-I boost/libs/intrusive/include/ -I boost/libs/config/include/ -I boost/libs/assert/include/ -I boost/libs/core/include/ \
						-I boost/libs/move/include/ -I boost/libs/static_assert/include/
include  $(KOBJECTS:.o=.d)
OBJECTS += $(KOBJECTS)

# TODO: include only the target
generic_cwd := kernel/generic
include $(generic_cwd)/Rules.mk
intel_cwd := kernel/arch/intel
include $(intel_cwd)/Rules.mk
i686_cwd := kernel/arch/i686
include $(i686_cwd)/Rules.mk

include build/Rules.mk

.DEFAULT_GOAL := all
all: os.iso

CPPFLAGS += -I include/

## ISO
os.iso: kernel.elf program1 program2
	cp program1 iso/modules/
	cp program2 iso/modules/
	cp kernel.elf iso/boot/kernel.elf
	grub-mkrescue -o os.iso iso -d /usr/lib/grub/i386-pc

## Program
program1: src/crt0.o src/syscall.o src/program1.o src/link.ld
	$(LD) -T src/link.ld src/syscall.o src/program1.o -o program1

program2: src/crt0.o src/syscall.o src/program2.o src/link.ld
	$(LD) -T src/link.ld src/syscall.o src/program2.o -o program2

include src/program1.d src/program2.d

# QEMU
QEMUFLAGS = -serial file:serial.log

.PHONY: run 
run: os.iso
	qemu-system-i386 -d cpu_reset -cdrom $< -m 512 -boot d $(QEMUFLAGS)

prepare_debug:
	urxvt -e ./scripts/debug.sh &

run_debug: os.iso
	./scripts/run_debug.sh
