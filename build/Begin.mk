CC = i686-elf-gcc
CFLAGS ?= -g -O0
CFLAGS += -ffreestanding -fno-exceptions -mgeneral-regs-only

CXX = i686-elf-g++
CXXFLAGS ?= -g -Og
CXXFLAGS += -std=c++2a -ffreestanding -fno-exceptions -Wall -Wextra -mgeneral-regs-only

CPPFLAGS += -DARCHITECTURE_i686

LD = i686-elf-ld
LDFLAGS += -nostdlib

AS = nasm
ASFLAGS += -f elf32 -g -F dwarf

OBJCOPY = i686-elf-objcopy

