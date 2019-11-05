CPPFLAGS += -I $(i686_cwd)/include/
LIBS += --whole-archive $(i686_cwd)/libi686.a --no-whole-archive
CLEANFILES += $(i686_cwd)/libi686.a

i686_OBJECTS =
i686_OBJECTS += $(i686_cwd)/src/boot/loader.o $(i686_cwd)/src/boot/multiboot2_header.o
i686_OBJECTS += $(i686_cwd)/src/core/Interrupt.o $(i686_cwd)/src/core/Paging.o $(i686_cwd)/src/core/Segmentation.o

$(i686_cwd)/libi686.a: $(i686_OBJECTS)

include $(i686_OBJECTS:.o=.d)

