CPPFLAGS += -I $(intel_cwd)/include/
LIBS += $(intel_cwd)/libintel.a
CLEANFILES += $(intel_cwd)/libintel.a

intel_OBJECTS += $(intel_cwd)/src/core/pic/8259.o

$(intel_cwd)/libintel.a: $(intel_OBJECTS)

include $(intel_OBJECTS:.o=.d)
