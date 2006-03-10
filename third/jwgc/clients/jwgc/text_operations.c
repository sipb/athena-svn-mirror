/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#include "text_operations.h"
#include "char_stack.h"

string 
lany(text_ptr, str)
	string *text_ptr;
	string str;
{
	string result, whats_left;
	char *p = *text_ptr;

	while (*p && *str)
		p++, str++;

	result = string_CreateFromData(*text_ptr, p - *text_ptr);
	whats_left = string_Copy(p);
	free(*text_ptr);
	*text_ptr = whats_left;

	return (result);
}

string 
lbreak(text_ptr, set)
	string *text_ptr;
	character_class set;
{
	string result, whats_left;
	char *p = *text_ptr;

	while (*p && !set[(int) *p])
		p++;

	result = string_CreateFromData(*text_ptr, p - *text_ptr);
	whats_left = string_Copy(p);
	free(*text_ptr);
	*text_ptr = whats_left;

	return (result);
}

string 
lspan(text_ptr, set)
	string *text_ptr;
	character_class set;
{
	string result, whats_left;
	char *p = *text_ptr;

	while (*p && set[(int) *p])
		p++;

	result = string_CreateFromData(*text_ptr, p - *text_ptr);
	whats_left = string_Copy(p);
	free(*text_ptr);
	*text_ptr = whats_left;

	return (result);
}

string 
rany(text_ptr, str)
	string *text_ptr;
	string str;
{
	string result, whats_left;
	string text = *text_ptr;
	char *p = text + strlen(text);

	while (text < p && *str)
		p--, str++;

	result = string_Copy(p);
	whats_left = string_CreateFromData(text, p - text);
	free(text);
	*text_ptr = whats_left;

	return (result);
}

string 
rbreak(text_ptr, set)
	string *text_ptr;
	character_class set;
{
	string result, whats_left;
	string text = *text_ptr;
	char *p = text + strlen(text);

	while (text < p && !set[(int) p[-1]])
		p--;

	result = string_Copy(p);
	whats_left = string_CreateFromData(text, p - text);
	free(text);
	*text_ptr = whats_left;

	return (result);
}

string 
rspan(text_ptr, set)
	string *text_ptr;
	character_class set;
{
	string result, whats_left;
	string text = *text_ptr;
	char *p = text + strlen(text);

	while (text < p && set[(int) p[-1]])
		p--;

	result = string_Copy(p);
	whats_left = string_CreateFromData(text, p - text);
	free(text);
	*text_ptr = whats_left;

	return (result);
}

string 
paragraph(text_ptr, width)
	string *text_ptr;
	int width;
{
	string result;
	char *p, *back, *s, *t;

	result = string_Copy(*text_ptr);

	p = result;
	s = result;
	t = result;
	while (1) {
		if (strlen(p) > width) {
			p = p + width;
			t = p;
			back = p;
			while (*back != ' ' && *back != '\n' &&
			       *back != '\t' && back != s) {
				back--;
			}
			if (back != s && *back != '\n') {
				*back = '\n';
			}
			else {
				back = t;
				while (*back != ' ' && *back != '\n' &&
					*back != '\t' && *back != '\0') {
					back++;
				}
				if (*back != '\0' && *back != '\n') {
					*back = '\n';
				}
				if (*back == '\0') {
					break;
				} 
			}
			p = back + 1;
			if (*p == '\0') {
				break;
			} 
		}
		else {
			break;
		}
	}

	return (result);
}
