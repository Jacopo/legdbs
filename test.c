/* Copyright 2015 Jacopo Corbetta <jacopo.corbetta@gmail.com> */

#ifdef INDEPENDENT
#include "../inline-notlibc/inline-notlibc.h"
#else
#include <stdio.h>
#endif

extern unsigned long prova();
extern unsigned long provashort();

#define PRINT(args...) fprintf(stderr, args)

int main()
{
    for (int i = 0; i < 5; i++) {
        PRINT(" | Calling prova...\n");
        int ret = prova();
        PRINT(" ^ returned 0x%x (%d)\n", ret, ret);
        if (ret != 0x41414141)
            PRINT(" ! WRONG RETURN VALUE!\n");
    }

    for (int i = 0; i < 5; i++) {
        PRINT(" | Calling provashort...\n");
        int ret = provashort();
        PRINT(" ^ returned 0x%x (%d)\n", ret, ret);
        if (ret != 0x42424242)
            PRINT(" ! WRONG RETURN VALUE!\n");
    }

    PRINT(" | exit...\n");
    return 0;
}
