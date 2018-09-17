#ifndef __zio_con__
#define __zio_con__

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include <windows.h>

#ifndef _NOMAIN_
extern short int debug = 1;
#endif // _NOMAIN_

extern void mlog(char *, ...);
extern void mwarn(char *, ...);
extern void merror(char *, ...);
extern void mdebug(char *, ...);

#endif // __zio_con__
