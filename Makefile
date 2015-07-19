CC := gcc
CFLAGS := -Wall -Wextra -Wshadow -Wc++-compat -Wwrite-strings -Wpointer-arith -g3 -ggdb -gdwarf-4
CFLAGS += --std=gnu11 -D_GNU_SOURCE

# TODO: Right now the full test is for 32 bit code only
CFLAGS += -m32

all: test test_vanilla
test: test.c test.o.nasm demo_handler.c legdbs.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -DTEST_WRAP_MAIN -Wl,--wrap=main -o $@ $^ $(LOADLIBES) $(LDLIBS)
test_vanilla: test.c test.o.nasm
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

%.o.nasm: %.nasm
	nasm -Wall -f elf32 -F dwarf -g $< -o $@


check: all
	./test.sh


.PHONY: clean all check
clean:
	rm -f test test_vanilla *.o.nasm
