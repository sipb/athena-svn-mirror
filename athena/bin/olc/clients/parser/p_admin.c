/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for administrative commands 
 *
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: p_admin.c,v 1.4 1999-03-06 16:47:59 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_admin.c,v 1.4 1999-03-06 16:47:59 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_parser.h>

extern int num_of_args;

#ifdef HAVE_ZEPHYR
ERRCODE
do_olc_zephyr(arguments)
     char **arguments;
{
  REQUEST Request;
  int status;
  int how_long = -1;
  int what = -1;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);
  
  Request.request_type = OLC_TOGGLE_ZEPHYR;
  arguments++;
  while (*arguments != NULL) {
    if (string_eq(*arguments, "-punt") && (what < 0)) {
      what = 1;
      ++arguments;
      if (*arguments != NULL) { /* override default */
	how_long = atoi(*arguments);
	++arguments;
      }
      continue;
    }
    if (string_eq(*arguments,"-unpunt") && (what < 0)) {
      what = 0;
      ++arguments;
      continue;
    }

    arguments = handle_argument(arguments, &Request, &status);
    if (status) {
      what = -1;
      break;
    }
  }

  if (what < 0) { /* error */
    fprintf(stderr,"Usage is: \tzephyr [-unpunt "
	           "| -punt [<minutes_to_punt>]]\n");
    return(ERROR);
  }

  return(t_toggle_zephyr(&Request,what,how_long));
}
#endif /* HAVE_ZEPHYR */
