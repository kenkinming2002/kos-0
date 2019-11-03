LDFLAGS += -T src/link.ld 
CLEANFILES += kernel.elf

%.d: %.c
	set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.cpp
	set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.s
	touch $@

%.a:
	$(AR) rcs $@ $?

kernel.elf: src/link.ld $(OBJECTS) $(filter %.a,$(LIBS))
	$(LD) -T src/link.ld src/kmain.o $(LDFLAGS) $(LIBS) -o $@

.PHONY: clean
clean:
	-find -type f -name '*.o' -delete
	-find -type f -name '*.d' -delete
	-rm -f $(CLEANFILES)

