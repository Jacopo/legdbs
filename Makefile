CC := gcc
CFLAGS := -Wall -Wextra -Wwrite-strings -Wpointer-arith -g3 -ggdb -gdwarf-4 -O3 -march=native
CFLAGS += --std=gnu11 -D_GNU_SOURCE

# TODO: Right now the full test is for 32 bit code only
CFLAGS += -m32

all: test test_vanilla inject_me.o
test: test.c test.o.nasm indep_start.c demo_handler.c legdbs.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -DTEST_WRAP_MAIN -Wl,--wrap=main -o $@ $^ $(LOADLIBES) $(LDLIBS)
test_vanilla: test.c indep_start.c test.o.nasm
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

%.o.nasm: %.nasm
	nasm -Wall -f elf32 -F dwarf -g $< -o $@

# NOTE: .data, .text, .bss are automatically set by add_code, but the build id NOTE would
#       need a separate header. Since it's useless anyway, might as well disable it.
INDEP_CFLAGS = -nostdlib -DINDEPENDENT -static
INDEP_CFLAGS += -Wl,--build-id=none
inject_me.o: demo_handler.c legdbs.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INDEP_CFLAGS) -DTEST_BEFORE_ENTRY -Wl,-r -o $@ $^


check: all
	./test.sh


.PHONY: clean all check
clean:
	rm -f test test_vanilla inject_me.o *.o.nasm
