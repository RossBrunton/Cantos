#ifndef __H_MAIN_PRINTK__
#define __H_MAIN_PRINTK__

#include <stdarg.h>

void printk(char *fmt, ...);
void vprintk(char *fmt, va_list ap);

#endif
