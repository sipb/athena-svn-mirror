/*
------------------------------------------------------------------------------
By Bob Jenkins, September 1996
lookupa.h, a hash function for table lookup, same function as lookup.c.
You may use this code in any way you wish.  It has no warranty.
Source is http://ourworld.compuserve.com/homepages/bob_jenkins/lookupa.h
------------------------------------------------------------------------------
*/

#ifndef BJSTANDARD
#include "bjstandard.h"
#endif

#ifndef BJLOOKUPA
#define BJLOOKUPA


ub4  bjlookup(/*_ ub1 *k, ub4 length, ub4 level _*/);
void bjchecksum(/*_ ub1 *k, ub4 length, ub4 *state _*/);

#endif /* BJLOOKUPA */
