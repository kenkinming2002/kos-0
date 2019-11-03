CPPFLAGS += -I $(generic_cwd)/include/
LIBS += $(generic_cwd)/libgeneric.a
CLEANFILES += $(generic_cwd)/libgeneric.a

generic_OBJECTS = $(generic_cwd)/src/init/Multiboot2.o $(generic_cwd)/src/io/Framebuffer.o $(generic_cwd)/src/io/serial.o

$(generic_cwd)/libgeneric.a: $(generic_OBJECTS)

include $(generic_OBJECTS:.o=.d)
