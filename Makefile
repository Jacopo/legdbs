CC := gcc
CFLAGS := -Wall -Wextra -Wwrite-strings -Wpointer-arith -g3 -ggdb -gdwarf-4
CFLAGS += --std=gnu11 -D_GNU_SOURCE

# TODO: Right now the full test is for 32 bit code only
CFLAGS += -m32

all: test test_vanilla
test: test.c test.o.nasm indep_start.c demo_handler.c legdbs.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -DTEST_WRAP_MAIN -Wl,--wrap=main -o $@ $^ $(LOADLIBES) $(LDLIBS)
test_vanilla: test.c indep_start.c test.o.nasm
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

%.o.nasm: %.nasm
	nasm -Wall -f elf32 -F dwarf -g $< -o $@


# -nostdinc -isystem/usr/lib/gcc/x86_64-linux-gnu/4.9/include
# -ffreestanding
CFLAGS += -nostdlib -DINDEPENDENT
CFLAGS += -static -Wl,-Ttext=0x066000000 -Wl,-Tdata=0x066200000 -Wl,-Tbss=0x066400000 -Wl,--build-id=none
inject_me: demo_handler.c legdbs.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^


check: all
	./test.sh


.PHONY: clean all check
clean:
	rm -f test test_vanilla *.o.nasm
