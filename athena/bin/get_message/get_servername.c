/* Copyright 1988, 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

static const char rcsid[] = "$Id: get_servername.c,v 1.1 1999-12-08 22:06:44 danw Exp $";

#include "globalmessage.h"
#include <hesiod.h>
#include "hesiod_err.h"

Code_t get_servername(char ***ret_name)
{
  char **retval, **_data;	/* for copying out the hesiod data */
  int datacnt, i;		/* number of hesiod records returned */

  /* Guard against NULL arguments */
  if ((!ret_name)) {
    return(GMS_NULL_ARG_ERR);
  }
  
  /* Fetch the list of servers from Hesiod. */
  _data = hes_resolve(GMS_NAME_CLASS, GMS_NAME_TYPE);
  
  if(!_data) {
    /* deal with hesiod error */
    return(hesiod_error());
  }

  /* Copy the Hesiod data into stable space. */
  for(datacnt=0; _data[datacnt]; datacnt++); /* count the data */

  if (!datacnt) {
      /* an answer, but no contents! */
      return(HESIOD_ER_INVALID);	/* XXX */
  }
  retval = (char **)malloc(datacnt * sizeof(char *));
  for(i=0; i<datacnt; i++) {
    retval[i] = malloc(strlen(_data[i])+1);
    if(!retval[i]) {
      /* malloc failed... */
      for(;--i;free(retval[i]));
      return(GMS_MALLOC_ERR);
    }
    strcpy(retval[i], _data[i]);
  }
  
  /* Clean up and return normally. */
  *ret_name = retval;
  return(0);
}
