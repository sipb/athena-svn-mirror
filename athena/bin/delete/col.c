/*
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/delete/col.c,v $
 * $Author: jik $
 *
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
     static char rcsid_col_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/delete/col.c,v 1.2 1989-03-27 12:05:12 jik Exp $";
#endif

/*
 * Note that this function has a lot of options I'm not really using
 * because I took it out of other code that needed a lot more
 * versatility.
 */

#include <stdio.h>
#include <strings.h>
#include "col.h"
#include "mit-copyright.h"


static int calc_string_width();
static void trim_strings();
extern char *malloc();
extern char *whoami;

int column_array(strings, num_to_print, screen_width, column_width,
		 number_of_columns, margin, spread_flag, 
		 number_flag, var_col_flag, outfile)
char **strings;
FILE *outfile;
{
     char buf[BUFSIZ];
     int updown, leftright, height;
     int string_width;
     int numwidth;
     

     numwidth = num_width(num_to_print);
     if (! var_col_flag) {
	  string_width = calc_string_width(column_width, margin, number_flag,
					   num_to_print);
	  if (string_width < 0) {
	       fprintf(stderr,
		       "%s: do_wait: your columns aren't wide enough!\n",
		       whoami);
	       return(1);
	  }
	  trim_strings(strings, num_to_print, string_width);
     } else if (calc_widths(strings, &screen_width, &column_width,
			    &number_of_columns, num_to_print, &margin,
			    spread_flag, number_flag))
	  return(1);

     height = num_to_print / number_of_columns;
     if (num_to_print % number_of_columns)
	  height++;
     
     if (number_flag) for (updown = 0; updown < height; updown++) {
	  for (leftright = updown; leftright < num_to_print;
	       leftright += height) {
	       (void) sprintf(buf, "%*d. %s", numwidth, leftright+1,
			      strings[leftright]);
	       fprintf(outfile, "%*s", -column_width, buf);
	  }
	  fprintf(outfile, "\n");
     } else for (updown = 0; updown < height; updown++) {
	  for (leftright = updown; leftright < num_to_print;
	       leftright += height) {
	       (void) sprintf(buf, "%s", strings[leftright]);
	       fprintf(outfile, "%*s", -column_width, buf);
	  }
	  fprintf(outfile, "\n");
     }
     
     return(0);
}

static int calc_string_width(column_width, margin, number_flag, max_number)
{
     int string_width;
     
     string_width = column_width - margin;
     if (number_flag)
	  string_width = string_width - num_width(max_number) - strlen(". ");
     return(string_width);
}


static void trim_strings(strings, number, width)
char **strings;
{
     int loop;
     
     for (loop = 0; loop < number; loop++)
	  if (strlen(strings[loop]) > width)
	       strings[loop][width] = '\0';
}


static int calc_widths(strings, screen_width, column_width, number_of_columns,
		       num_to_print, margin, spread_flag, number_flag)
int *screen_width, *column_width, *number_of_columns, *margin;
char **strings;
{
     int loop;
     int maxlen, templen;
     int spread;
     
#ifdef DEBUG
     printf("calc_widths starting with screen_width %d column_width %d number_of_columns %d margin %d num_to_print %d spread_flag %d number_flag %d\n", *screen_width, *column_width, *number_of_columns, *margin, num_to_print, spread_flag, number_flag);
#endif
     maxlen = templen = 0;
     for (loop = 0; loop < num_to_print; loop++)
	  if (maxlen < (templen = strlen(strings[loop])))
	       maxlen = templen;
#ifdef DEBUG
     printf("calc_widths maxlen %d\n", maxlen);
#endif
     *column_width = maxlen;
     
     if (number_flag)
	  *column_width = *column_width + num_width(num_to_print) +
	       strlen(". ");

     if (! spread_flag) {
	  *column_width += *margin;
	  if (! *number_of_columns) {
	       *number_of_columns = *screen_width / *column_width;
	       if (! *number_of_columns) {
		    (*number_of_columns)++;
		    *column_width -= *margin;
		    *margin = 0;
		    *screen_width = *column_width;
	       }
	  }
	  else
	       *screen_width = *number_of_columns * *column_width;
     } else {
	  if (! *number_of_columns) {
	       *number_of_columns = *screen_width / (*column_width + *margin);
	       if (! *number_of_columns) {
		    (*number_of_columns)++;
		    *screen_width = *column_width;
		    *margin = 0;
	       }
	       spread = (*screen_width - *number_of_columns * *column_width)
		    / *number_of_columns;
	       *column_width += spread;
	  }
	  else {
	       if (*number_of_columns * (*column_width + *margin) >
		   *screen_width) {
		    *column_width += *margin;
		    *screen_width = *column_width;
	       } else {
		    spread = (*screen_width - (*number_of_columns *
					       *column_width)) /
						    *number_of_columns;
		    *column_width += spread;
	       }
	  }
     }
#ifdef DEBUG
     printf("calc_widths returning screen_width %d column_width %d number_of_columns %d margin %d\n", *screen_width, *column_width, *number_of_columns, *margin);
#endif
     return(0);
}


	       

static int num_width(number)
{
     char buf[BUFSIZ];

     return(strlen(sprintf(buf, "%d", number)));
}
