/* Separate file so --wrap can work */

#ifdef INDEPENDENT
#include "../inline-notlibc/inline-notlibc.h"
extern int main();
void _start() { exit_group(main()); }
#endif
