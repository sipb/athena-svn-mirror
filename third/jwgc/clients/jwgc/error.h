#ifndef error_MODULE
#define error_MODULE

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#include <stdio.h>

#define  ERROR(a)                { fprintf(stderr, "jwgc: "); \
				   fprintf(stderr, a);\
				   fflush(stderr); }

#define  ERROR2(a,b)             { fprintf(stderr, "jwgc: "); \
				   fprintf(stderr, a, b);\
				   fflush(stderr); }

#define  ERROR3(a,b,c)           { fprintf(stderr, "jwgc: "); \
				   fprintf(stderr, a, b, c);\
				   fflush(stderr); }

#define  ERROR4(a,b,c,d)         { fprintf(stderr, "jwgc: "); \
				   fprintf(stderr, a, b, c, d);\
				   fflush(stderr); }

#endif
