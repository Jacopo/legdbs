#include "legdbs.h"

#ifdef INDEPENDENT
#include "../inline-notlibc/inline-notlibc.h"
#else
#include <stdio.h>
#include <stdint.h>
#include <ucontext.h>
#endif
#include <asm/processor-flags.h>

static void print_ucontext(const struct ucontext *);


void demo_handler(int sig, siginfo_t *info, struct ucontext *ctx)
{
    // Do something...

    printf("GOT IT (signal %d, si_code %d)\n", sig, info->si_code);
    fprintf(stderr, "Breakpoint hit at %p\n", info->si_addr);

    // Sample reading of the CPU state
    print_ucontext(ctx);
}


void print_ucontext(const struct ucontext *ctx) {
# ifdef __i386__

# ifdef INDEPENDENT
    // The linux-provided header has a (legacy?) struct definition
#   define CTXR(x) (ctx->uc_mcontext.x)
#   define PRINT_REG16(x) fprintf(stderr, "  "#x" = 0x%02hx = %hu", CTXR(x), CTXR(x))
#   define PRINT_REG32(x) fprintf(stderr, "  "#x" = 0x%08lx  (s: %ld\tu: %lu)\n", CTXR(x), CTXR(x), CTXR(x))
#   define PRINT_FLAG(x) if (CTXR(eflags) & X86_EFLAGS_##x) fprintf(stderr, __STRING(x) " ")
    PRINT_REG32(eip);
    PRINT_REG32(eax); PRINT_REG32(ebx); PRINT_REG32(ecx); PRINT_REG32(edx);
    PRINT_REG32(esi); PRINT_REG32(edi); PRINT_REG32(ebp); PRINT_REG32(esp);
    PRINT_REG16(ss); PRINT_REG16(cs); fprintf(stderr, "\n");
    PRINT_REG16(ds); PRINT_REG16(fs); fprintf(stderr, "\n");
    PRINT_REG16(es); PRINT_REG16(gs); fprintf(stderr, "\n");
# else /* INDPENDENT */
// Instead, glibc defines indexes on an array
#   define CTXR(x) (ctx->uc_mcontext.gregs[REG_ ## x])
#   define PRINT_REG16(x) fprintf(stderr, "  "#x" = 0x%02hx = %hu", CTXR(x), CTXR(x))
#   define PRINT_REG32(x) fprintf(stderr, "  "#x" = 0x%08x  (s: %d\tu: %u)\n", CTXR(x), CTXR(x), CTXR(x))
#   define PRINT_FLAG(x) if (CTXR(EFL) & X86_EFLAGS_##x) fprintf(stderr, __STRING(x) " ")
    PRINT_REG32(EIP);
    PRINT_REG32(EAX); PRINT_REG32(EBX); PRINT_REG32(ECX); PRINT_REG32(EDX);
    PRINT_REG32(ESI); PRINT_REG32(EDI); PRINT_REG32(EBP); PRINT_REG32(ESP);
    PRINT_REG16(SS); PRINT_REG16(CS); fprintf(stderr, "\n");
    PRINT_REG16(DS); PRINT_REG16(FS); fprintf(stderr, "\n");
    PRINT_REG16(ES); PRINT_REG16(GS); fprintf(stderr, "\n");
    //PRINT_REG32(UESP); PRINT_REG32(TRAPNO);
# endif /* INDPENDENT */
    fprintf(stderr, "  [ "); PRINT_FLAG(OF); PRINT_FLAG(DF); PRINT_FLAG(SF);
    PRINT_FLAG(ZF); PRINT_FLAG(AF); PRINT_FLAG(PF); PRINT_FLAG(CF);
    PRINT_FLAG(RF); PRINT_FLAG(ID);
    fprintf(stderr, "]\n");

# endif /* __i386__ */
}



#if defined(TEST_BEFORE_ENTRY)
void before_entry()
{
// TODO:    uintptr_t prova = xxx;
    install_signal_handler();
//    set_breakpoint(prova, prova_postinst, demo_handler);
//    set_breakpoint(provashort, provashort_postinst, demo_handler);
    /* entry_helper calls the real _start for us */
}
#elif defined(TEST_WRAP_MAIN)
/* Useful for automated testing */
int __wrap_main(int argc, char* argv[], char *envp[])
{
    /* Cheating a bit here, using the symbols */
    extern unsigned long prova();
    extern unsigned long prova_postinst();
    extern unsigned long provashort();
    extern unsigned long provashort_postinst();

    install_signal_handler();
    set_breakpoint((uintptr_t) prova, (uintptr_t) prova_postinst, demo_handler);
    set_breakpoint((uintptr_t) provashort, (uintptr_t) provashort_postinst, demo_handler);

    extern int __real_main(int argc, char* argv[], char *envp[]);
    return __real_main(argc, argv, envp);
}
#endif
