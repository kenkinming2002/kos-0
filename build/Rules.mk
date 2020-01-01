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

kernel.elf: src/link.ld $(OBJECTS)
	$(LD) -T src/link.ld $(OBJECTS) $(LDFLAGS) -o $@

.PHONY: clean
clean:
	-find -type f -name '*.o' -delete
	-find -type f -name '*.d' -delete
	-rm -f $(CLEANFILES)

