#pragma once

#ifdef INDEPENDENT
#include "../inline-notlibc/inline-notlibc.h"
#else
#include <stdint.h>
#include <signal.h>
#include <sys/ucontext.h>
#endif

typedef void (*breakpoint_handler_t)(int sig, siginfo_t *info, struct ucontext *);
void set_breakpoint(uintptr_t addr, uintptr_t postbreak_instruction, breakpoint_handler_t f);
void install_signal_handler();
