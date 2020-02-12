CC = i686-elf-gcc
CFLAGS += -ffreestanding -g -O0 -fno-exceptions -mgeneral-regs-only

CXX = i686-elf-g++
CXXFLAGS += -std=c++2a -ffreestanding -g -O0 -fno-exceptions -fno-inline -Wall -Wextra -mgeneral-regs-only

CPPFLAGS += -DARCHITECTURE_i686

LD = i686-elf-ld
LDFLAGS += -nostdlib

AS = nasm
ASFLAGS += -f elf32 -g -F dwarf

OBJCOPY = i686-elf-objcopy

