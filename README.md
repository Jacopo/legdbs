Ptrace-less breakpoints, a.k.a. poor-mans detours.

Mainly to serve LegitBS' idea that ptrace is Bad(TM).
(But works for any case where ptrace is disabled / undesirable.)
All it needs is an opcode that causes a fault signal to be sent,
and freedom to catch it. Put them in `legdbs.c`.



*Limitations*:

- Cannot handle jumping in the middle of a multi-byte breakpoint
- No multithreading (may want to use `SIGSTOP` to emulate mutexes)
- Since it is mainly useful for static programs, you cannot use the regular libc. Weird stuff is necessary to use C. See my inline-notlibc repo. 


*TODO*:

- Needs manual info on the next instruction (helper script that finds it?)
- More testing :)



Usage (see `demo_handler.c`)
----------------------------

    void foo(int sig, siginfo_t *info, struct ucontext *) { ... }
    install_signal_handler();
    set_breakpoint(where_to_break, postbreak_instruction, foo);
    set_breakpoint(other, other_postbreak_instruction, foo);


### Note that `set_breakpoint` specifies TWO separate breakpoints

 - The "wanted" one, disabled after hitting it.
 - The "service" one used to set the "wanted" breakpoint again.
   It must be set at the first instruction after addr that does NOT
   overlap with the breakpoint opcode bytes.

If the original code has instructions A, B, C, D:

    orig.  want.  serv.
     A_1    BRK    A_1   <- addr
     A_2    BRK    A_2
     B_1    BRK    B_1
     B_2    B_2    B_2   <- addr + sizeof(brk_opcode)
     B_3    B_3    B_3
     C_1    C_1    BRK   <- postbreak_instruction
     C_2    C_2    BRK
     D_1    D_1    BRK
     D_2    D_2    D_2   <- postbreak_instruction + sizeof(brk_opcode)
     ...    ...    ...

After hitting the breakpoint at addr, execution will resume as in serv.,
so that the code behaves as it was supposed to.
However, we need to stop as soon as possible to restore the breakpoint,
hence the service breakpoint.


Note: will exit(109) on failure.
