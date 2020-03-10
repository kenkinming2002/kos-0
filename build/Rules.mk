CLEANFILES += kernel.elf

%.d: %.c
	set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.cpp
	set -e; rm -f $@; $(CC) -M $(CPPFLAGS) $< | sed 's#.*\.o#$@ $(basename $@).o#' > $@ 

%.d: %.s
	touch $@

CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)

kernel.elf: kernel/link.ld $(ICXXABI_OBJECT) $(CRTBEGIN_OBJ) $(CRTI_OBJECT) $(OBJECTS) $(CRTN_OBJECT) $(CRTEND_OBJ) $(CRT0_OBJECT)
	$(CXX) -T kernel/link.ld $(ICXXABI_OBJECT) $(CRTI_OBJECT) $(CRTBEGIN_OBJ) $(OBJECTS) $(CRTEND_OBJ) $(CRTN_OBJECT) $(CRT0_OBJECT) $(LDFLAGS) -o $@

.PHONY: doxygen
doxygen: Doxyfile
	doxygen $<

Doxyfile: Doxyfile.in
	sed "s,@CPPFLAGS@,$(CPPFLAGS)," $< > $@

.PHONY: clean
clean:
	-find -type f -name '*.o' -delete
	-find -type f -name '*.d' -delete
	-rm -f $(CLEANFILES)

