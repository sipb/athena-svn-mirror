/*
 * $Id: sintable.c,v 1.2 1999-01-22 23:17:09 ghudson Exp $
 *
 * Copyright 1990, 1991 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 */

#if  (!defined(lint))  &&  (!defined(SABER))
static char rcsid[] =
"$Id: sintable.c,v 1.2 1999-01-22 23:17:09 ghudson Exp $";
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
