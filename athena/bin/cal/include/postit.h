/************************************************************************/
/*      
/*                      postit.h
/*      
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	8/21/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/include/postit.h,v $
/*	$Author: lwvanels $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/include/postit.h,v 1.3 1991-09-04 11:22:45 lwvanels Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*      
/*	Declarations to be shared among all postit modules.
/*      
/************************************************************************/


	/*----------------------------------------------------------*/
	/*	
	/*			#defines 
	/*	
	/*----------------------------------------------------------*/

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

/*
 * Following is the name of the file that users are told to read if they
 * are concerned about the information logging done by postit.
 */

#define LOGINFO "/usr/unsupported/whatsup.logging"

       /*
        * Symbolic names for the responses to the main menu prompt,
        * used in switch to decide what we're doing
        */
#define DELETING 1
#define REPLACING 2
#define EXITING 3
#define CREATING 4

	/*----------------------------------------------------------*/
	/*	
	/*	Mode switch to indicate whether we are in postit
	/*	or bulkpost.
	/*	
	/*----------------------------------------------------------*/


extern int mode;				/* are we in postit or */
						/* bulk-update?*/
#define POSTIT 1				/* we're running postit */
#define BULK_CHECKING 2				/* we're in the checking */
						/* phase of bulk-post*/
#define BULK_UPDATING 3				/* we're in bulk-post */
						/* making the actual updates */


	/*----------------------------------------------------------*/
	/*	
	/*	File names
	/*	
	/*----------------------------------------------------------*/

#define EMACSMACS "/usr/athena/lib/elisp/postit.elc" /* load these when */
						 /* using gnuemacs*/


	/*----------------------------------------------------------*/
	/*	
	/*	Field delimiters
	/*	
	/*----------------------------------------------------------*/

#define MAGIC_CHAR '$'
#define FIELD_END_CHAR ':'
#define COMMENT_CHAR '#'

        /*----------------------------------------------------------*/
        /*      
        /*      Each field is parsed into the following structure
        /*      array.  Master copy is in formmaster.c.
        /*      
        /*----------------------------------------------------------*/

#define NTYPES 6

struct fields {
        char *type;                             /* string i.d. of this */
                                                /* field */
        char *data;                             /* pointer to the actual */
                                                /* data*/
        int context;                            /* tells whether to */
                                                /* clean up for major, */
                                                /* minor*/
	char *append;				/* if the same field */
						/* appears twice, this */
						/* string is put between the */
						/* text parts.  \0 means */
						/* just report and keep */
						/* the last*/
#define MAJOR 0x0001
#define MINOR 0x002
};

#define BEGINEVENT 0
#define PLACE 1
#define TYPE 2
#define TITLE 3
#define COMMENTS 4
#define WHEN 5
extern struct fields fields[NTYPES];

extern char fname[100];				/* name of the file */
						/* we're working on */


	/*----------------------------------------------------------*/
	/*	
	/*	Parse failure indicator
	/*	
	/*----------------------------------------------------------*/

extern int parse_fail;

	/*----------------------------------------------------------*/
	/*	
	/*	Ingres field sizes--used to check for truncation
	/*	
	/*----------------------------------------------------------*/

#define PLACE_SIZE 45				/* should be 60, but */
						/* whatsup is stupid about */
						/* display for now */
#define COMMENTS_SIZE 800
#define TYPE_SIZE 16
#define TITLE_SIZE 70


	/*----------------------------------------------------------*/
	/*	
	/*		Calendar data areas
	/*	
	/*	These are the variables in which the actual calendar
	/*	data entries are built up.
	/*	
	/*----------------------------------------------------------*/

extern char event_type[100];
extern char title[255];
extern char place[100];
extern char comment[2000];
extern int uniq;
extern int date[100];
extern int Times[100];
extern int ntimes;
extern TUPLE_DESCRIPTOR query_desc;


	/*----------------------------------------------------------*/
	/*	
	/*		Misc global variables.
	/*	
	/*----------------------------------------------------------*/

extern int parsed_entry;			/* did we parse an */
						/* new entry on this try?*/

extern int context;				/* MAJOR, MINOR OR 0 */
						/* this is for compatibility */
						/* with an old version of */
						/* the parser which had */
						/* a notion of entry and */
						/* sub-entry */

extern struct tm today;				/* structure containing */
						/* today's date and time */
						/* broken out */



/************************************************************************/
/*	
/*	Globals which are used only by bulkpost
/*	
/************************************************************************/

extern char *input_file;			/* file name supplied */
						/* by our caller */
extern int error_found;				/* global parse error flag */

extern int line_num;				/* number of input line */
						/* we're parsing  */


extern int go_on;				/* after an error has been */
						/* found in bulk post, this */
						/* switch indicates whether */
						/* the user wants to break */
						/* the parsing loop early */


