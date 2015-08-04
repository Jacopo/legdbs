CC := gcc
CFLAGS := -Wall -Wextra -Wwrite-strings -Wpointer-arith -g3 -ggdb -gdwarf-4 -O3 -march=native
CFLAGS += --std=gnu11 -D_GNU_SOURCE

# TODO: Right now the full test is for 32 bit code only
CFLAGS += -m32

all: test_vanilla modified_test test_fake



##### This is how you would use it against a static executable #####

# 1) Build your handler.o, using legdbs.{h,c}
#
#    Include before_entry to setup the breakpoints.
#    You can use target symbols as original_xxx. Also, note the gcc flags.
#
HANDLER_CFLAGS = -nostdlib -static -Wl,--build-id=none
inject_me.o: demo_handler.c legdbs.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(HANDLER_CFLAGS) -DINDEPENDENT -DTEST_BEFORE_ENTRY -Wl,-r -o $@ $^

# 2) Modify the target program
#
modified_test: test_vanilla inject_me.o
	../elf_add_code/add_code_32 --before-entry test_vanilla inject_me.o > $@
	chmod a+x $@


##### This is a "cheating" test, links the handler together with the target program #####
# (Approximates an LD_PRELOAD use)
test_fake: test.c test.o.nasm indep_start.c demo_handler.c legdbs.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -DTEST_WRAP_MAIN -Wl,--wrap=main -o $@ $^ $(LOADLIBES) $(LDLIBS)



##### Test program #####
test_vanilla: test.c indep_start.c test.o.nasm
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -static -o $@ $^ $(LOADLIBES) $(LDLIBS)
%.o.nasm: %.nasm
	nasm -Wall -f elf32 -F dwarf -g $< -o $@

check: all
	bash -c 'diff <(./test_vanilla 2>&1 | grep -v -F '[INFO]') test_vanilla.expected_out'
	bash -c 'diff <(./test_fake 2>&1 | grep -v -F '[INFO]') test.expected_out'
	bash -c 'diff <(./modified_test 2>&1 | grep -v -F '[INFO]') test.expected_out'
	@echo "All tests passed :)"


.PHONY: clean all check
clean:
	rm -f test_fake test_vanilla modified_test inject_me.o *.o.nasm
