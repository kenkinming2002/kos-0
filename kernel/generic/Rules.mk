CPPFLAGS 	 += -I $(generic_cwd)/include/
OBJECTS 	 += $(generic_cwd)/generic.o

generic_OBJECTS += $(generic_cwd)/src/io/Framebuffer.o $(generic_cwd)/src/io/Serial.o $(generic_cwd)/src/io/Print.o $(generic_cwd)/src/io/PS2Controller.o
generic_OBJECTS += $(generic_cwd)/src/core/memory/PhysicalPageFrameAllocator.o $(generic_cwd)/src/core/memory/VirtualPageFrameAllocator.o \
									 $(generic_cwd)/src/core/memory/PageFrameAllocator.o  $(generic_cwd)/src/core/Memory.o
generic_OBJECTS += $(generic_cwd)/src/utils/Format.o

ICXXABI_OBJECT = $(generic_cwd)/src/icxxabi.o

$(generic_cwd)/generic.o: $(generic_OBJECTS)
	$(LD) -r $(generic_OBJECTS) -o $@ 

include $(generic_OBJECTS:.o=.d)
