CPPFLAGS 	 += -I $(i686_cwd)/include/
OBJECTS  	 += $(i686_cwd)/libi686.o

i686_OBJECTS += $(i686_cwd)/src/boot/loader.o $(i686_cwd)/src/boot/multiboot2_header.o 
i686_OBJECTS += $(i686_cwd)/src/boot/lower_half.o $(i686_cwd)/src/boot/higher_half.o  $(i686_cwd)/src/boot/boot.o

i686_OBJECTS += $(i686_cwd)/src/core/Interrupt.o $(i686_cwd)/src/core/Segmentation.o
i686_OBJECTS += $(i686_cwd)/src/core/memory/Paging.o $(i686_cwd)/src/core/memory/MemoryMapping.o

i686_OBJECTS += $(i686_cwd)/src/core/Syscall.o 
i686_OBJECTS += $(i686_cwd)/src/core/MultiProcessing.o
i686_OBJECTS += $(i686_cwd)/src/core/multiprocessing/Yield.o
i686_OBJECTS += $(i686_cwd)/src/core/multiprocessing/SwitchProcess.o

i686_OBJECTS += $(i686_cwd)/src/asm/Process.o

i686_OBJECTS += $(i686_cwd)/src/boot/memory/Paging.o 

CRTI_OBJECT = $(i686_cwd)/src/crti.o
CRTN_OBJECT = $(i686_cwd)/src/crtn.o
CRT0_OBJECT = $(i686_cwd)/src/crt0.o

$(i686_cwd)/libi686.o: $(i686_OBJECTS)
	$(LD) -r $(i686_OBJECTS) -o $@ 

include $(i686_OBJECTS:.o=.d)

