/*
 * wconfig.c
 *
 * Copyright 1994 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 * Program to take the place of the configure shell script under DOS.
 * The makefile.in files are constructed in such a way that all this
 * program needs to do is uncomment lines beginning ##DOS by removing the
 * first 5 characters of the line.  This will allow lines like:
 * ##DOS!include ..\config\common to become: !include ..\config\common.
 *
 * Syntax: wconfig <input >output
 * 
 */

#include <mit-copyright.h>
#include <stdio.h>

int main(argc, argv)
    int argc;
    char *argv[];
{
    char buf [1024];
    int l;

    while (gets(buf) != NULL) {
	if (strncmp("##DOS", buf, 4) == 0) {
	    l = strlen(buf) - 5;
	    memmove (buf, &buf[5], l + 1);
	}
	puts(buf);
    }	
}
