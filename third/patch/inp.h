/* inputting files to be patched */

/* $Id: inp.h,v 1.1.1.2 1998-02-12 21:43:18 ghudson Exp $ */

XTERN LINENUM input_lines;		/* how long is input file in lines */

char const *ifetch PARAMS ((LINENUM, int, size_t *));
void get_input_file PARAMS ((char const *, char const *));
void re_input PARAMS ((void));
void scan_input PARAMS ((char *));
