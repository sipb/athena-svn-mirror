/************************************************************************/
/*      
/*                      postprompts.c
/*      
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	9/1/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postprompts.c,v $
/*	$Author: ghudson $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postprompts.c,v 1.2 1996-09-19 22:15:58 ghudson Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*      
/*      Purpose:
/*      
/*	This file contains prompting routines used by postit.
/*	
/************************************************************************/

#ifndef lint
static char rcsid_postprompts_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postprompts.c,v 1.2 1996-09-19 22:15:58 ghudson Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include "gdb.h"
#include "postit.h"


	/*----------------------------------------------------------*/
	/*	
	/*	 		what_to_do
	/*	
	/*	Presents the main menu.  Ask the user what s/he wants
	/*	to do and return the ans.
	/*	
	/*----------------------------------------------------------*/

int
what_to_do()
{
	char response[80];
	int resp;
	do {
		printf("\nWould you like to\n\t1) Create a new event listing?\n\t2) Edit an existing calendar entry?\n\t3) Delete an existing event listing entirely?\n\t4) Exit from postit right now?\n\nRespond with 1,2 3, or 4: ");
		if (fgets(response, 79, stdin) == NULL) 
			continue;

	} while (sscanf(response, "%d", &resp) != 1 || resp<1 || resp>4);
	switch (resp) {
	      case 1:
		return CREATING;
	      case 2: 
		return REPLACING;
	      case 3:
		return DELETING;
	      case 4:
		return EXITING;
	}

	return EXITING;				/* just to keep lint */
						/* happy */
}

	/*----------------------------------------------------------*/
	/*	
	/*			which_event
	/*	
	/*	Enter the i.d. number of an existing event.
	/*	
	/*----------------------------------------------------------*/

int
which_event(prompt)
char *prompt;
{
	char response[80];
	register char *rp;
	int id = (-1);

	printf("\n\n");
	printf("To delete or update an event using postit you must know its event\n");
	printf("i.d. number in the whatsup database.  If you DON'T know the correct\n");
	printf("number, enter 0 to give up for now, and use the whatsup program\n");
	printf("to look up the event.  The detail display provided by whatsup includes\n");
	printf("the \"Event number\".\n\n"); 


	while (id<0) {
		printf("\n%s: ", prompt);
		if (fgets(response, 79, stdin) == NULL) {
			continue;
		}

               /*
                * Check for null or blank response
                */

		rp = response;
		while (*rp++ == ' ')
			;
		if (*rp == '\0')
			continue;
               /*
                * Parse a number response
                */
		if (sscanf(response, "%d", &id) != 1)
			id = -1;		/* force re-loop */
	}

	return id;
}

/*
 * 			re_alloc_string
 * 
 * 	Frees an existing string and reallocates it to hold 
 * 	a new one.
 * 
 * 	Note: allocation and freeing are done using the GDB
 * 	db_alloc and db_free routines.
 */

int
re_alloc_string(targp, src)
char **targp;
char *src;
{
       /*
        * Only if there was a string there, de-allocate it
        */
	if (*targp != NULL) {
		db_free(*targp, strlen(*targp)+1);
	}

       /*
        * Allocate space for the new string.
        */
	*targp = db_alloc(strlen(src)+1);
       /*
        * Copy the new string
        */
	(void) strcpy(*targp, src);
}

/*
 *			Hello_prompt
 *
 *	Before really getting going, tell the user what his/her
 *	options are.
 */
int
hello_prompt()
{
	char *bigname, *us;

	if (mode == POSTIT) {
		bigname = "POSTIT";
		us = "------";
	} else {
		bigname = "BULKPOST";
		us = "--------";
	}
printf("                      WELCOME TO %s (Ver 0.5)\n", bigname);
printf("                      -----------%s----------\n", us);
printf("\n");
if (mode==POSTIT) {
	printf("     This program allows you to create new listings for the whatsup calendar.\n");
	printf("     It also allows you to change or delete existing listings.\n");
} else {
	printf("     This program lets you add listings from a file to the  whatsup calendar.\n");
	printf("     It also lets you check a listings file without changing the calendar.\n\n");
}
	/*
	 * Continue anonymously 
	 */
	 if (mode == POSTIT) 
	 	re_alloc_string(&gdb_uname, "**POSTIT**");
	 else
	 	re_alloc_string(&gdb_uname, "**BULKPOST**");
	
	
	re_alloc_string(&gdb_host,  "**NOHOST**");
	return;
}

	/*----------------------------------------------------------*/
	/*	
	/*			caveats
	/*	
	/*		print information about using postit
	/*	
	/*----------------------------------------------------------*/

int
caveats()
{
	if (mode == POSTIT)
	  printf("\n     The postit program makes no attempt to keep listings private or to\n");
	else
	  printf("\n     The bulkpost program makes no attempt to keep listings private or to\n");
	printf("     restrict people from using the calendar in any manner they see fit.\n");
	printf("     Of course, this means that the calendar will be as good or as bad as\n");
	printf("     the information that you provide to it.  Please check the information\n");
	printf("     that you supply carefully, and remember to update it as necessary.\n");
	printf("\n");
	if (mode == POSTIT)
	  printf("     Please read the man page on postit if you have not done so.\n");
	else
	  printf("     Please read the man page on bulkpost if you have not done so.\n");
	printf("\t############################################\n");
}

	/*----------------------------------------------------------*/
	/*	

	/*			get_yn
	/*	
	/*	Ask a yes or no question.  Returns TRUE for yes.
	/*	
	/*----------------------------------------------------------*/

int
get_yn(prompt)
char *prompt;
{
	char response[80];
	register char *rsp;
	char yn;

	do {
		printf("%s (y or n):  ", prompt);
		if (fgets(response, 79, stdin) == NULL) 
			continue;
		for (rsp = response; *rsp==' ' ;rsp++)
		  ;
		if (!isalpha(*rsp))
			continue;
		yn = *rsp;
		if (isupper(yn))
			yn = tolower(yn);
	} while (yn != 'y' && yn != 'n');
	if (yn=='y')
		return TRUE;
	else
		return FALSE;
	
	
}

