/* This file is part of the Project Athena Global Message System.
 * Created by: Mark W. Eichin <eichin@athena.mit.edu>
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/gms/get_servername.c,v $
 * $Author: eichin $
 *
 *	Copyright (c) 1988 by the Massachusetts Institute of Technology.
 *	For copying and distribution information, see the file
 *	"mit-copyright.h". 
 */
#include <mit-copyright.h>
#ifndef lint
static char rcsid_get_servername_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/gms/get_servername.c,v 1.1 1988-09-26 15:39:47 eichin Exp $";
#endif lint

#include "globalmessage.h"
#include <hesiod.h>

Code_t get_servername(ret_name)
     char ***ret_name;		/* pointer to array of string... */
{
  char **retval, **data;	/* for copying out the hesiod data */
  int datacnt, i;		/* number of hesiod records returned */

  /* Guard against NULL arguments */
  if ((!ret_name)) {
    return(GMS_NULL_ARG_ERR);
  }
  
  /* Fetch the list of servers from Hesiod. */
  data = hes_resolve(GMS_NAME_CLASS, GMS_NAME_TYPE);
  
  if(!data) {
    /* deal with hesiod error */
    return(hesiod_error());
  }

  /* Copy the Hesiod data into stable space. */
  for(datacnt=0; data[datacnt++]; ); /* count the data */

  retval = (char **)malloc(datacnt * sizeof(char *));
  for(i=0; i<datacnt; i++) {
    retval[i] = malloc(strlen(data[i])+1);
    if(!retval[i]) {
      /* malloc failed... */
      for(;--i;free(retval[i]));
      return(GMS_MALLOC_ERR);
    }
    strcpy(retval[i], data[i]);
  }
  
  /* Clean up and return normally. */
  *ret_name = retval;
  return(0);
}
