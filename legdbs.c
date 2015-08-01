#ifdef INDEPENDENT
#include "../inline-notlibc/inline-notlibc.h"
#else
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#endif

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#ifndef VAL_TO_STR
#   define VAL_TO_STR(x) __STRING(x)
#endif
#ifndef V
#   define V(x) if (unlikely(!(x)))   errx(109, __FILE__ ":" VAL_TO_STR(__LINE__) " %s, it's not %s", __PRETTY_FUNCTION__, #x)
#endif
#ifndef VS
#   define VS(x) if (unlikely((x) == -1)) err(109, __FILE__ ":" VAL_TO_STR(__LINE__) " %s, %s failed (returned -1)", __PRETTY_FUNCTION__, #x)
#endif

/* Config */
#define MAX_BREAKPOINTS 10
static const bool USE_SIGALTSTACK = true;

/* Architecture-specific */
#if defined(__i386__) || defined(__x86_64__)
    static const uint8_t BRK_OPCODE[] = { 0x0F, 0x0B };
    static const int BRK_SIGNAL = SIGILL;
    static const uintptr_t PAGESIZE = 4096;
    static const uintptr_t SIGALTSTACK_BASE = 0x0AAA0000u;
    static const bool STACK_GROWSDOWN = true; /* Used for SIGALTSTACK */
#else
#   error "I need info on this architecture!"
#endif

/* API declarations */
#include "legdbs.h"


/* Global state */
typedef struct {
    uintptr_t addr;
    uint8_t original_opcodes[sizeof(BRK_OPCODE)];
    uintptr_t postbreak_instruction;
    uint8_t postbreak_instruction_opcodes[sizeof(BRK_OPCODE)];
    breakpoint_handler_t handler;
} breakpoint;
static breakpoint breakpoints[MAX_BREAKPOINTS];
static int active_breakpoints = 0;
static breakpoint* breakpoint_being_serviced = NULL;

static breakpoint* breakpoint_info_at(uintptr_t addr)
{
    for (int i = 0; i < active_breakpoints; i++)
        if (breakpoints[i].addr == addr)
            return &breakpoints[i];
    return NULL;
}


void set_breakpoint(uintptr_t addr, uintptr_t postbreak_instruction, breakpoint_handler_t handler)
{
    if (active_breakpoints >= MAX_BREAKPOINTS)
        errx(101, "Too many breakpoints!");

    // TODO: Also check for overlaps
    if (breakpoint_info_at(addr) != NULL)
        errx(101, "I already set a breakpoint at %p!", (void*) addr);

    // Note: this check is, necessarily, incomplete
    if (postbreak_instruction < (addr + sizeof(BRK_OPCODE)))
        errx(101, "No way that's the postbreak_instruction, the breakpoint opcode is not even over yet!");

    breakpoint *b = &breakpoints[active_breakpoints++];
    b->addr = addr;
    memcpy(b->original_opcodes, (void*) addr, sizeof(BRK_OPCODE));
    b->postbreak_instruction = postbreak_instruction;
    memcpy(b->postbreak_instruction_opcodes, (void*) postbreak_instruction, sizeof(BRK_OPCODE));
    b->handler = handler;

    uintptr_t page_addr = addr & ~(PAGESIZE-1);
    VS(mprotect((void*) page_addr,
                (postbreak_instruction + sizeof(BRK_OPCODE)) - page_addr,
                PROT_READ | PROT_WRITE | PROT_EXEC));

    memcpy((void*) addr, BRK_OPCODE, sizeof(BRK_OPCODE));
}




static void signal_handler(int sig, siginfo_t *info, void *_context)
{
    uintptr_t addr = (uintptr_t) info->si_addr;
    breakpoint *b;

    if (breakpoint_being_serviced != NULL) {
        /* Just restores the "wanted" breakpoint */
        b = breakpoint_being_serviced;
        V(addr == b->postbreak_instruction);
        memcpy((void*) b->addr, BRK_OPCODE, sizeof(BRK_OPCODE));
        memcpy((void*) addr, b->postbreak_instruction_opcodes, sizeof(BRK_OPCODE));
        breakpoint_being_serviced = NULL;
        return;
    }

    /* Handles the desired breakpoint */
    V((b = breakpoint_info_at(addr)) != NULL);

    memcpy((void*) addr, b->original_opcodes, sizeof(BRK_OPCODE));
    b->handler(sig, info, (struct ucontext*) _context);
    memcpy((void*) b->postbreak_instruction, BRK_OPCODE, sizeof(BRK_OPCODE));
    breakpoint_being_serviced = b;
}


void install_signal_handler()
{
    if (USE_SIGALTSTACK) {
        void *alloc = mmap((void*) SIGALTSTACK_BASE,
                SIGSTKSZ,
                PROT_READ | PROT_WRITE,
                MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS
                | MAP_STACK | (STACK_GROWSDOWN ? MAP_GROWSDOWN : 0), -1, 0);
        V(alloc != MAP_FAILED);
        V(((uintptr_t) alloc) == SIGALTSTACK_BASE);

        stack_t sigstack = { .ss_sp = (void*) SIGALTSTACK_BASE,
                             .ss_size = SIGSTKSZ,
                             .ss_flags = 0 };
        stack_t old_sigstack;
        VS(sigaltstack(&sigstack, &old_sigstack));
        V(old_sigstack.ss_flags = SS_DISABLE);
    }

    struct sigaction act, old_act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = signal_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART | (USE_SIGALTSTACK ? SA_ONSTACK : 0);

    VS(sigaction(BRK_SIGNAL, &act, &old_act));

    if (old_act.sa_handler != SIG_DFL || old_act.sa_flags != 0)
        errx(101, "Something had already set the signal handler!");
}
