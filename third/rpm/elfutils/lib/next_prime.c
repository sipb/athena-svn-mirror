/* Determine prime number.
   Copyright (C) 2000, 2002 Free Software Foundation, Inc.
   Written by Ulrich Drepper <drepper@redhat.com>, 2000.

   This program is Open Source software; you can redistribute it and/or
   modify it under the terms of the Open Software License version 1.0 as
   published by the Open Source Initiative.

   You should have received a copy of the Open Software License along
   with this program; if not, you may obtain a copy of the Open Software
   License version 1.0 from http://www.opensource.org/licenses/osl.php or
   by writing the Open Source Initiative c/o Lawrence Rosen, Esq.,
   3001 King Ranch Road, Ukiah, CA 95482.   */

#include <stddef.h>


/* Test whether CANDIDATE is a prime.  */
static int
is_prime (size_t candidate)
{
  /* No even number and none less than 10 will be passed here.  */
  size_t divn = 3;
  size_t sq = divn * divn;

  while (sq < candidate && candidate % divn != 0)
    {
      size_t old_sq = sq;
      ++divn;
      sq += 4 * divn;
      if (sq < old_sq)
	return 1;
      ++divn;
    }

  return candidate % divn != 0;
}


/* We need primes for the table size.  */
size_t
next_prime (size_t seed)
{
  /* Make it definitely odd.  */
  seed |= 1;

  while (!is_prime (seed))
    seed += 2;

  return seed;
}
