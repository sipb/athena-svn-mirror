/*------------------------------------------------------------
small string utility functions

written by Stefan Ulrich <stefanulrich@users.sourceforge.net> 2000/03/03

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL PAUL VOJTA OR ANY OTHER AUTHOR OF THIS SOFTWARE BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------*/


#include <string.h>
#include <stdio.h>
#include "xdvi-config.h"
#include "xdvi.h"
#include "string-utils.h"


/*------------------------------------------------------------
 *  str_is_prefix
 * 
 *  Arguments:
 *	 char *str1, char *str2 - strings to be compared
 *
 *  Returns:
 *	 Boolean
 *		 
 *  Purpose:
 *	Check if <str1> is a prefix of <str2> or vice versa.
 *      Returns True if it is, False if it isn't.
 *------------------------------------------------------------*/

Boolean str_is_prefix(str1, str2)
    char *
    str1, *str2;
{
    int i;
    Boolean retval = True;

    for (i = 0; *(str1 + i) != '\0' && *(str2 + i) != '\0'; i++) {
	if (*(str1 + i) != *(str2 + i)) {
	    retval = False;
	    break;
	}
    }
    return retval;
}


/*------------------------------------------------------------
 *  str_is_postfix
 * 
 *  Arguments:
 *	 char *str1, char *str2 - strings to be compared
 *
 *  Returns:
 *	 Boolean
 *		 
 *  Purpose:
 *	Check if <str1> is a postfix of <str2> or vice versa.
 *      Returns True if it is, False if it isn't.
 *------------------------------------------------------------*/

Boolean str_is_postfix(str1, str2)
    char *
    str1, *str2;
{
    int len1 = strlen(str1);
    int len2 = strlen(str2);

    while (len1 > len2) {
	str1++;
	len1--;
    }
    while (len2 > len1) {
	str2++;
	len2--;
    }
    if (strcmp(str1, str2) == 0) {
	return True;
    }
    else {
	return False;
    }
}

/*------------------------------------------------------------
 *  Routines for determining the length of a digit
 * 
 *  Arguments:
 *	 int/long n
 *
 *  Returns:
 *	 int ret
 *		 
 *  Purpose:
 *	Return length of printed representation of integer/long <n>,
 *	including the minus sign for negative digits.
 *------------------------------------------------------------*/

int
length_of_int(int n)
{
    int ret = 0;
    if (n < 0) {
	ret++;
	n *= -1;
    }
    else if (n == 0) {
	return 1;
    }
    while (n >= 1) {
	n /= 10;
	ret++;
    }
    return ret;
}


int
length_of_ulong(unsigned long n)
{
    int ret = 0;

    if (n == 0) {
	return 1;
    }
    while (n >= 1) {
	n /= 10;
	ret++;
    }
    return ret;
}

/* expand filename to include `.dvi' extension and full path name;
   returns malloc()ed string (caller is responsible for free()ing).
*/
char *
normalize_and_expand_filename(char *filename)
{
    char *expanded_filename = NULL;
    char *save_ptr = NULL;
    char *path_name = NULL;
    size_t path_name_len = 512;
    
    size_t len;

    /* skip over `file:' prefix if present */
    if (memcmp(filename, "file:", 5) == 0) {
	filename += 5;
    }

    len = strlen(filename) + 5; /* 5 in case we need to add `.dvi\0' */

    /* save original pointer, since expanded_filename will be incremented */
    save_ptr = expanded_filename = xmalloc(len);

    Strcpy(expanded_filename, filename);

    /* append ".dvi" extension if not present */
    if (!str_is_postfix(expanded_filename, ".dvi")) {
	strcat(expanded_filename, ".dvi");
    }

    /* skip over `./' in relative paths */
    if (memcmp(expanded_filename, "./", 2) == 0) {
	expanded_filename += 2;
    }

    /* expand to full path if needed */
    if (expanded_filename[0] != DIR_SEPARATOR) {
	for (;;) {
	    char *tmp;
	    path_name = xrealloc(path_name, path_name_len);
	    if ((tmp = getcwd(path_name, path_name_len)) == NULL && errno == ERANGE) {
		path_name_len *= 2;
	    }
	    else {
		path_name = tmp;
		break;
	    }
	}
	len += strlen(path_name) + 1; /* for DIR_SEPARATOR */
	path_name = xrealloc(path_name, len);
	strcat(path_name, "/");
	strcat(path_name, expanded_filename);
	free(save_ptr);
	expanded_filename = path_name;
    }

    TRACE_CLIENT((stderr, "dvi_name: `%s'", filename));
    TRACE_CLIENT((stderr, "expanded_filename: `%s'", expanded_filename));
    
    return expanded_filename;
}

