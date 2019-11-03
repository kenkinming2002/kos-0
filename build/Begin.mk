CC = i686-elf-gcc
CFLAGS += -ffreestanding -g -O3

CXX = i686-elf-g++
CXXFLAGS += -std=c++17 -ffreestanding -g -O3

CPPFLAGS += -DARCHITECTURE_i686

LD = i686-elf-ld
LDFLAGS += -nostdlib

AS = nasm
ASFLAGS += -f elf32 -g -F dwarf

OBJCOPY = i686-elf-objcopy


