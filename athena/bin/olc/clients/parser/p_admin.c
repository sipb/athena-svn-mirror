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
 *	$Id: p_admin.c,v 1.3 1999-01-22 23:12:41 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: p_admin.c,v 1.3 1999-01-22 23:12:41 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include <olc/olc.h>
#include <olc/olc_parser.h>

extern int num_of_args;

#ifdef ZEPHYR
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
  while(*arguments != (char *) NULL) {
    if (string_eq(*arguments, "-punt")) {
      if (what != -1) {
	fprintf(stderr,"Usage is: \tzephyr -unpunt\n");
	fprintf(stderr,"          \tzephyr -punt [minutes_to_punt]\n");
	return(ERROR);
      }
      what = 1;
      ++arguments;
      if(*arguments != NULL) { /* override default */
	how_long = atoi(*arguments);
	arguments++;
      }
      continue;
    }
    if (string_eq(*arguments,"-unpunt")) {
      if (what != -1) {
	fprintf(stderr,"Usage is: \tzephyr -unpunt\n");
	fprintf(stderr,"          \tzephyr -punt [minutes_to_punt]\n");
	return(ERROR);
      }
      what = 0;
      ++arguments;
    }
  }

  arguments = handle_argument(arguments, &Request, &status);
  if(status)
    return(ERROR);
	
  arguments += num_of_args;		/* HACKHACKHACK */
  
  if(what == -1) { /* error */
    fprintf(stderr,"Usage is: \tzephyr -unpunt\n");
    fprintf(stderr,"          \tzephyr -punt [minutes_to_punt]\n");
    return(ERROR);
  }


  return(t_toggle_zephyr(&Request,what,how_long));
}
#endif
