#ifndef formatter_MODULE
#define formatter_MODULE

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

typedef struct _desctype {
    struct _desctype *next;

    short int code;
#define DT_EOF	0	/* End of message.	*/
#define DT_ENV	1	/* Open environment.	*/
#define DT_STR	2	/* Display string.	*/
#define DT_END	3	/* Close environment.	*/
#define DT_NL	4	/* Newline.		*/
    
    char *str;		/* Name of environment, string to be displayed.	*/
    short int len;	/* Length of string/environment name for
			   ENV, STR, END.  Undefined for EOF */
} desctype;

extern desctype *disp_get_cmds();
extern void free_desc();

#endif
