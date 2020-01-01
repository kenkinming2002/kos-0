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

%.a:
	$(AR) rcs $@ $?

kernel.elf: src/link.ld $(OBJECTS) $(filter %.a,$(LIBS))
	$(LD) -T src/link.ld $(OBJECTS) $(LDFLAGS) --start-group $(LIBS) --end-group -o $@
include  $(OBJECTS:.o=.d)

.PHONY: clean
clean:
	-find -type f -name '*.o' -delete
	-find -type f -name '*.d' -delete
	-rm -f $(CLEANFILES)

