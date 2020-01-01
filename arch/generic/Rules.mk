CPPFLAGS 	 += -I $(generic_cwd)/include/
OBJECTS 	 += $(generic_cwd)/generic.o

generic_OBJECTS += $(generic_cwd)/src/io/Framebuffer.o $(generic_cwd)/src/io/serial.o $(generic_cwd)/src/io/Print.o
generic_OBJECTS += $(generic_cwd)/src/core/memory/PhysicalPageFrameAllocator.o $(generic_cwd)/src/core/memory/VirtualPageFrameAllocator.o \
									 $(generic_cwd)/src/core/memory/PageFrameAllocator.o 
generic_OBJECTS += $(generic_cwd)/src/utils/Format.o

$(generic_cwd)/generic.o: $(generic_OBJECTS)
	$(LD) -r $(generic_OBJECTS) -o $@ 

include $(generic_OBJECTS:.o=.d)
