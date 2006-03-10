#ifndef text_operations_MODULE
#define text_operations_MODULE

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#include "character_class.h"

extern string protect();
extern string verbatim();
extern string lany();
extern string lbreak();
extern string lspan();
extern string rany();
extern string rbreak();
extern string rspan();
extern string paragraph();

#endif
