/************************************************************************/
/*      
/*                      bulkmaster.c
/*      
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	9/1/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/bulkmaster.c,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/bulkmaster.c,v 1.1 1992-11-08 19:06:37 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*      
/*      Purpose:
/*	
/*      This is the main program for bulkpost, which does bulk postings to 
/*	the whatsup calendar of events.  This program shares many subroutines
/*	with postit, which is the interactive single event poster.  
/*	The "mode" global variable indicates to some routines whether we are:
/*	
/*	1) Running interactively (postit)
/*	
/*	2) In bulkpost doing prechecking of the file (bulkpost)
/*	
/*	3) In bulkpost making actual changes to the database
/*	
/*      Some of this code is lifted from similar applications written
/*      by John Barrus.
/*      
/************************************************************************/

#ifndef lint
static char rcsid_bulkmaster_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/bulkmaster.c,v 1.1 1992-11-08 19:06:37 probe Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "gdb.h"
#include "postit.h"


/************************************************************************/
/*	
/*			global variables and #defines
/*	
/************************************************************************/

int mode;					/* are we in postit or */
						/* bulk-update?*/

int parse_fail = FALSE;				/* Set anytime during parse */
						/* to indicate that a fatal */
						/* error prevents acceptance */
						/* of the entry */
int error_found;				/* global parse error flag */
int go_on;					/* after an error has been */
						/* found in bulk post, this */
						/* switch indicates whether */
						/* the user wants to break */
						/* the parsing loop early */

int line_num = 1;				/* number of input line */
						/* we're parsing  */

struct tm today;				/* structure containing */
						/* today's date and time */
						/* broken out */

	/*----------------------------------------------------------*/
	/*	
	/*	Global variables specific to bulkpost
	/*	
	/*----------------------------------------------------------*/

char *input_file;				/* file name supplied */
						/* by our caller */


        /*----------------------------------------------------------*/
        /*      
        /*      Each field is parsed into the following structure
        /*      array.  
	/*	
	/*	There is a generalized parser which looks for 
	/*	$xxx: fields, where xxx is a name defined below.
	/*	
	/*	WARNING!! Each field name has a #define in
	/*	postit.h.  Be SURE to keep these and the #define
	/*	for NTYPES up to date when you edit this table.
        /*      
        /*----------------------------------------------------------*/

struct fields fields[NTYPES] = 
/********* WARNING:  Code changes required if new fields are introduced
 whose names begin with 'E' or 'D' as these may be confused with
Event and Date *****/
{"BeginEvent", NULL, 0, "",
"Place", NULL, 0, "",
"Type", NULL, 0, "",
"Title", NULL, 0, "",
"Comments", NULL, 0, "",
"When", NULL, 0, ",",				/* separate multiple times */
						/* with ','*/

};

int context;                                    /* MAJOR, MINOR OR 0 */
						/* this is for compatibility */
						/* with an old version of */
						/* the parser which had */
						/* a notion of entry and */
						/* sub-entry */

	/*----------------------------------------------------------*/
	/*	
	/*    		Data buffers for the field array
	/*	
	/*	The actual data for each field type as collected
	/*	here.
	/*	
	/*----------------------------------------------------------*/

#define CHARS_PER_FIELD 4096                    /* max chars per field */
char field_buffs[NTYPES][CHARS_PER_FIELD];

#define BSIZE 512000                              /* size of largest data */
                                                /* file we can handle */

extern int errno;

char fname[100]="";

	/*----------------------------------------------------------*/
	/*	
	/*		Calendar data areas
	/*	
	/*	These are the variables in which the actual calendar
	/*	data entries are built up.
	/*	
	/*----------------------------------------------------------*/

char event_type[100];
char title[255];
char place[100];
char comment[2000];
int uniq;
int date[100];
int Times[100];
int ntimes;

	/*----------------------------------------------------------*/
	/*	
	/*	Misc global variables
	/*	
	/*----------------------------------------------------------*/

char *buf;					/* whole file lives here */


int parsed_entry = FALSE;			/* did we parse an */
						/* new entry on this try?*/
TUPLE_DESCRIPTOR query_desc;

/************************************************************************/
/*	
/*			main (bulkpost)
/*	
/*	This is the main program for postit.
/*	
/*	1) Initialize
/*	
/*	2) Loop presenting the main menu and responding
/*	
/*	3) Terminate
/*	
/************************************************************************/

int
main(ac,av)
int ac;
char *av[];
{

       /*
        * Variables
        */


	int try_again;				/* switch to indicate if */
						/* user wants to try again */
						/* original entry*/
       /*
        * Functions not returning integer
        */

	TUPLE_DESCRIPTOR MakeQueryDescriptor();
        extern char *malloc();


	/*----------------------------------------------------------*/
	/*	
	/*		EXECUTION BEGINS HERE
	/*	
	/*----------------------------------------------------------*/

       /*
        * Indicate that we are in the first phase of bulkpost
        */
	mode = BULK_CHECKING;

       /*
        * Allocate a buffer to hold all of the data
        */
	buf = malloc((unsigned) BSIZE);

       /*
        * Handle command line options and initialize global variables
        */
	master_init(&ac, &av);
	initialize_globals();

       /*
        * Set up the descriptor to be used in doing queries 
        */
	query_desc = MakeQueryDescriptor();
	

	/*----------------------------------------------------------*/
	/*	
	/*	Loop once for each response to the main menu
	/*	
	/*----------------------------------------------------------*/

	do {
		printf("\n\n\t*****************************************\n\nNow checking all entries in input file \"%s\"...\n\n", input_file);
		parse_input_file(input_file);
		if (error_found)
			try_again = get_yn("Uncorrected errors remain... would you like to edit the input file?\nIf not, bulkpost will exit.\n\nEdit the file?");
		else
			try_again = FALSE;
		if (try_again) {
			edit_proto(input_file);		
			line_num = 1;
		}
	
	} while(error_found && try_again);

	/*----------------------------------------------------------*/
	/*	
	/*	If user has given up and left uncorrected errors,
	/*	then just leave.  We never do updates until the entire
	/*	file is correct.
	/*	
	/*	Otherwise, confirm that s/he really wants to go ahead
	/*	with the updates.
	/*	
	/*----------------------------------------------------------*/

	if (error_found) {
		printf("\nUncorrected errors remain in your input file.\nNO updates have been made to the calendar.\n");
		exit (4);
	} else {
		printf("\n\n\t*****************************************\n\nAll information in input file \"%s\" appears to be correct.\n\n", input_file);
		if (!get_yn("Do you wish to go ahead and update the calendar database?")) {
			printf("\nThe calendar database remains unchanged.\n\n");
			exit(0);
		}
	}

	/*----------------------------------------------------------*/
	/*	
	/*	All of the data is syntactically correct, and the
	/*	user has accepted it.  Now go do all of the updates.
	/*	
	/*----------------------------------------------------------*/

	mode = BULK_UPDATING;
	error_found = FALSE;

	printf("\n\n\t***********************************************\n\nAll information in file %s is correct.\nStarting update of calender database\n\n", input_file);

	parse_input_file(input_file);

	exit(0);
}
	  

	/*----------------------------------------------------------*/
	/*	
	/*			parse_input_file
	/*	
	/*	Read in the entire input file and have it parsed.
	/*	
	/*----------------------------------------------------------*/

int
parse_input_file(Fname)
char *Fname;
{
	int cc;					/* number of chars in file*/

       /*
        * Read the entire file into an in-core buffer
        */
	cc = read_file(Fname, buf, (unsigned long)BSIZE); /* read file into core */
	if (cc == BSIZE) {
		fprintf(stderr, "Input file is too long to be handled by bulkpost.  Max is %d bytes.\nPlease split it and try again.\n", BSIZE);
		exit(64);
	}

       /*
        * Parse the entire buffer.  Depending on the setting of mode,
        * this will either do just error detection or actual update of the 
        * database
        */
	do_whole_buffer(buf, cc);
}

/*----------------------------------------------------------*/
/*	
/*			master_init
/*	
/*	Gives us a chance to set up before doing anything.
/*	Args are same as for main program.
/*	
/*	The main argument we process is the input filename.
/*	
/*	In this case, we handle the secret -db option, which
/*	lets the user go for something other than the default
/*	database.  And we start the db connection asynchronously.
/*	
/*----------------------------------------------------------*/

int
master_init(argc, argv)
int *argc;
char ***argv;
{
        char whatsup_db[100];
	char **av;
	struct stat stat_buf;


	av = *argv;


       /*
        * handle the secret -db option
        */
        (void) strcpy(whatsup_db, "whatsup-mit");

	if (*argc <= 1) {
		fprintf(stderr, "Correct form is: bulkpost <datafile> \n");
		exit (4);
        }

	if(strcmp(av[1], "-db") == 0) {
		/*
		 * site specification attempted
		 */
		if (*argc < 3) {
			fprintf(stderr, "Correct form is bulkpost <-db dbname@site>\n");
			exit(16);
		}
		(void) strcpy(whatsup_db, av[2]);
		*argc -= 2;
		*argv += 2;
		av = *argv;
		
	}

       /*
        * Make sure a filename is supplied, and make a note of it
        */
	if (*argc != 2) {
		fprintf(stderr, "Correct form is: bulkpost -db dbname@site <datafile> \n");
		exit (4);
        }

	input_file = av[1];

       /*
        * make sure the file exists
        */
	if (stat(input_file,  &stat_buf)) {
		perror(input_file);
		exit(8);
	}

       /*
        * Initialize GDB (this must be done before giving user the hello
        * mesage so processing of anonymous users can be done.
        */
	gdb_init();

       /*
        * Welcome the user and ask if bulkpost is to be used anonymously
        */
	hello_prompt();
	caveats();

        fprintf(stderr, "Attempting connection to %s...", whatsup_db);

       /*
        * Tell remote site to start opening the Ingres database.
        */
	PrimeDb(whatsup_db);
	fprintf(stderr,"connection started.\n");

}

        /*----------------------------------------------------------*/
        /*      
        /*              initialize_globals
        /*      
        /*      Set up values for global variables.  Especially,
        /*      the field description array has to have its
        /*      data pointers set.
        /*      
        /*----------------------------------------------------------*/

int
initialize_globals()
{
        register int i;

       /*
        * Get today's date and time
        */
	init_times();

       /*
        * Set up file name global for compatibility with postit
        */
	(void) strcpy(fname, input_file);

       /*
        * Give each field holder a data buffer to use
        */
        for(i=0; i<NTYPES; i++) {
                fields[i].data = field_buffs[i];
                field_buffs[i][0] = '\0';       /* null the string */
        }

       /*
        * Indicate that we're not in either a major or a minor entry yet
        */
        context = 0;
}


