/*
 * $Id: col.c,v 1.10 1999-01-22 23:08:50 ghudson Exp $
 *
 * This program is part of a package including delete, undelete,
 * lsdel, expunge and purge.  The software suite is meant as a
 * replacement for rm which allows for file recovery.
 * 
 * Copyright (c) 1989 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copying.h."
 */

/*
 * Note that this function has a lot of options I'm not really using
 * because I took it out of other code that needed a lot more
 * versatility.
 */

#include <string.h>
#include "errors.h"
#include "delete_errs.h"
#include "col.h"
#include "mit-copying.h"
#include "util.h"

static int calc_string_width(), calc_widths(), num_width();
static void trim_strings();

int column_array(char **strings, int num_to_print, int screen_width,
		 int column_width, int number_of_columns, int margin,
		 int spread_flag, int number_flag, int var_col_flag,
		 FILE *outfile)
{
     char buf[BUFSIZ];
     int updown, leftright, height;
     int string_width;
     int numwidth;

     numwidth = num_width(num_to_print);
     if (! var_col_flag) {
	  string_width = calc_string_width(column_width, margin, number_flag,
					   num_to_print);
	  if (string_width <= 0) {
	       set_error(COL_COLUMNS_TOO_THIN);
	       error("calc_string_width");
	       return error_code;
	  }
	  trim_strings(strings, num_to_print, string_width);
     } else if (calc_widths(strings, &screen_width, &column_width,
			    &number_of_columns, num_to_print, &margin,
			    spread_flag, number_flag)) {
	  error("calc_widths");
	  return error_code;
     }
     height = num_to_print / number_of_columns;
     if (num_to_print % number_of_columns)
	  height++;
     
     if (number_flag) for (updown = 0; updown < height; updown++) {
	  for (leftright = updown; leftright < num_to_print; ) {
	       (void) sprintf(buf, "%*d. %s", numwidth, leftright+1,
			      strings[leftright]);
	       if ((leftright += height) >= num_to_print)
		    fprintf(outfile, "%s", buf );
	       else
		    fprintf(outfile, "%*s", -column_width, buf);
	  }
	  fprintf(outfile, "\n");
     } else for (updown = 0; updown < height; updown++) {
	  for (leftright = updown; leftright < num_to_print; ) {
	       (void) sprintf(buf, "%s", strings[leftright]);
	       if ((leftright += height) >= num_to_print)
		    fprintf(outfile, "%s", buf );
	       else
		    fprintf(outfile, "%*s", -column_width, buf);
	  }
	  fprintf(outfile, "\n");
     }
     return 0;
}

static int calc_string_width(column_width, margin, number_flag, max_number)
{
     int string_width;
     
     string_width = column_width - margin;
     if (number_flag)
	  string_width = string_width - num_width(max_number) - strlen(". ");
     return string_width;
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
     
     maxlen = templen = 0;
     for (loop = 0; loop < num_to_print; loop++)
	  if (maxlen < (templen = strlen(strings[loop])))
	       maxlen = templen;

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
     return 0;
}


	       

static int num_width(number)
int number;
{
     char buf[BUFSIZ];

     (void) sprintf(buf, "%d", number);
     return strlen(buf);
}
