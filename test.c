/* Copyright 2015 Jacopo Corbetta <jacopo.corbetta@gmail.com> */

#include <stdio.h>
#include <stdlib.h>

extern unsigned long prova();
extern unsigned long provashort();

int main()
{
    for (int i = 0; i < 5; i++) {
        printf(" | Calling prova...\n");
        int ret = prova();
        printf(" ^ returned 0x%x (%d)\n", ret, ret);
        if (ret != 0x41414141)
            printf(" ! WRONG RETURN VALUE!\n");
    }

    for (int i = 0; i < 5; i++) {
        printf(" | Calling provashort...\n");
        int ret = provashort();
        printf(" ^ returned 0x%x (%d)\n", ret, ret);
        if (ret != 0x42424242)
            printf(" ! WRONG RETURN VALUE!\n");
    }

    printf(" | exit...\n");
    exit(0);
}
