/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for administrative commands 
 *
 *	Lucien Van Elsen
 *	MIT Project Athena
 *
 * Copyright (C) 1991 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: p_admin.c,v 1.8 1999-08-02 12:25:20 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_admin.c,v 1.8 1999-08-02 12:25:20 ghudson Exp $";
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
  ERRCODE status = SUCCESS;
  int how_long = -1;
  int what = -1;

  if(fill_request(&Request) != SUCCESS)
    return(ERROR);

  Request.request_type = OLC_TOGGLE_ZEPHYR;

  if(arguments == NULL)
    return ERROR;
  arguments++;

  while(*arguments != NULL)
    {
      if(is_flag(*arguments, "-punt", 2))
	{
	  if(what < 0)
	    {
	      what = 1;
	      ++arguments;
	      if((*arguments != NULL) && (*arguments[0] != '-'))
		/* override default */
		{
		  how_long = atoi(*arguments);
		  ++arguments;
		}
	      continue;
	    }
	  else /* They already told us what to do?! */
	    {
	      what = -1; /* Do nothing. */
	      break;
	    }
	}
      if(is_flag(*arguments,"-unpunt",2))
	{
	  if(what < 0)
	    {
	      what = 0;
	      ++arguments;
	      continue;
	    }
	  else /* They already told us what to do?! */
	    {
	      what = -1; /* Do nothing. */
	      break;
	    }
	}
      status = handle_common_arguments(&arguments, &Request);
      if (status != SUCCESS)
	{
	  what = -1; /* Do nothing, even if we were already asked to. */
	  break;
	}
    }

  if (what < 0) /* error */
    {
      fprintf(stderr,"Usage is: \tzephyr [-unpunt "
	      "| -punt [<minutes_to_punt>]]\n");
      return ERROR;
    }

  return t_toggle_zephyr(&Request,what,how_long);
}
#endif /* HAVE_ZEPHYR */
