/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/Xj/sintable.c,v $
 * $Author: ghudson $ 
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char rcsid[] =
"$Header: /afs/dev.mit.edu/source/repository/athena/lib/Xj/sintable.c,v 1.1 1998-12-17 16:23:13 ghudson Exp $";
#endif

#include "mit-copyright.h"
#include <math.h>
#include <stdio.h>
#include "AClock.h"

main()
{
  int i;

  printf("static int sinTable[%d] = {", ANGLES);

  for (i = 0; i < ANGLES; i++)
    {
      if ((i % 11) == 0)
	printf("\n");

      printf(" %d,", 
	     (int)( ((double)SCALE) *
		   (sin(
			((double) i) *
			HALFPI /
			((double) ANGLES)))));
    }
  printf(" };\n");
  exit(0);
}
