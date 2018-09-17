#ifndef ZIO_READER_H_INCLUDED
#define ZIO_READER_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _NOMAIN_ //necessary to avoid multiple definitions of "debug" from con_interaction
#include "zio-con.h"
#undef _NOMAIN_

#include "zio-list.h"


/* filestuff */
void readlin(FILE*, kostring*);

#endif // ZIO_READER_H_INCLUDED
