#include <stdio.h>
#include <stdlib.h>

#define VERSION "#{VERSION}"
#define SUBVER  "#{Svn_Revision}"

/* 可执行必须有这个段才能执行 */
#if __pic__ || __PIC__
const char __invoke_dynamic_linker__[] __attribute__ ((section(".interp"))) 
    = 
#if __x86_64__
	"/lib64/ld-linux-x86-64.so.2"
#else
	"/lib/ld-linux.so.2"
#endif
	;


void version()
{
    printf("Boyaa libreactor_debug_%s.%s\n",VERSION,SUBVER);
    exit(0) ;
}
#endif
