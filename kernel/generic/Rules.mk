CPPFLAGS 	 += -I $(generic_cwd)/include/
OBJECTS 	 += $(generic_cwd)/generic.o

generic_OBJECTS += $(generic_cwd)/src/io/Framebuffer.o $(generic_cwd)/src/io/Serial.o $(generic_cwd)/src/io/Print.o 
generic_OBJECTS += $(generic_cwd)/src/io/PS2Controller.o $(generic_cwd)/src/io/PS2Keyboard.o

generic_OBJECTS += $(generic_cwd)/src/core/memory/LinkedListMemoryRegionAllocator.o 
generic_OBJECTS += $(generic_cwd)/src/core/memory/PhysicalMemoryRegionAllocator.o $(generic_cwd)/src/core/memory/VirtualMemoryRegionAllocator.o 

generic_OBJECTS += $(generic_cwd)/src/core/memory/PageFrameAllocator.o  
generic_OBJECTS += $(generic_cwd)/src/core/Memory.o

generic_OBJECTS += $(generic_cwd)/src/core/Process.o
generic_OBJECTS += $(generic_cwd)/src/core/multiprocessing/ThreadID.o

generic_OBJECTS += $(generic_cwd)/src/core/IPC.o
generic_OBJECTS += $(generic_cwd)/src/core/ipc/Service.o
generic_OBJECTS += $(generic_cwd)/src/core/ipc/Message.o

generic_OBJECTS += $(generic_cwd)/src/stdlib/string.o

ICXXABI_OBJECT = $(generic_cwd)/src/icxxabi.o

$(generic_cwd)/generic.o: $(generic_OBJECTS)
	$(LD) -r $(generic_OBJECTS) -o $@ 

include $(generic_OBJECTS:.o=.d)
