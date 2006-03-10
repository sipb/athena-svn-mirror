/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#include "buffer.h"

static char *buffer = 0;

string 
buffer_to_string()
{
	return (buffer);
}

void 
clear_buffer()
{
	if (buffer)
		free(buffer);

	buffer = string_Copy("");
}

void 
append_buffer(str)
	char *str;
{
	buffer = string_Concat2(buffer, str);
}
