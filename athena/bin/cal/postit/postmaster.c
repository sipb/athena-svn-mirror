/************************************************************************/
/*      
/*                      postmaster.c
/*      
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	9/1/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postmaster.c,v $
/*	$Author: ghudson $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postmaster.c,v 1.2 1996-09-19 22:15:55 ghudson Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*      
/*      Purpose:
/*	
/*	This is the main routine for the postit program, which allows 
/*	users to interactively add, update, and delete whatsup calendar
/*	listings.
/*	
/*      
/*      Some of this code is lifted from similar applications written
/*      by John Barrus.
/*      
/************************************************************************/

#ifndef lint
static char rcsid_postmaster_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postmaster.c,v 1.2 1996-09-19 22:15:55 ghudson Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
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
int line_num = 1;				/* number of input line */
						/* we're parsing  */

int go_on;					/* after an error has been */
						/* found in bulk post, this */
						/* switch indicates whether */
						/* the user wants to break */
						/* the parsing loop early */

struct tm today;				/* structure containing */
						/* today's date and time */
						/* broken out */


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

#define BSIZE 50000                              /* size of largest data */
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
/*			main (postit)
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

	int dothis;				/* what we're doing */


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
        * Tell the world we're in postit, not bulkpost
        */
	mode = POSTIT;

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

	while (TRUE) {
               /*
                * Show menu to user and get response number
                */
		dothis = what_to_do();

               /*
                * Do what we were asked to do
                */
		switch (dothis) {
		      case CREATING:
			create_entry();
			break;
		      case DELETING:
			(void) delete_entry();
			break;
		      case REPLACING:
			replace_entry();
			break;
		      case EXITING:
			exit(0);
		}
	}
        
}                                               /* main */

	/*----------------------------------------------------------*/
	/*	
	/*			create_entry
	/*	
	/*	User has asked to create a new calendar entry.
	/*	
	/*	1) Write a blank form to a file
	/*	
	/*	2) Call the user's favorite editor to get the facts
	/*	
	/*	3) Parse it and play back to user for verificaion
	/*	
	/*	4) Call the database library routines to update the
	/*	   database.
	/*	
	/*----------------------------------------------------------*/

int
create_entry()
{
	int cc;					/* number of chars in file*/
	int retry;				/* retry the edit ? */
	int newid;				/* event number of newly */
						/* created entry */

       /*
        * Write a file with the blank form for user to fill in
        */
	write_proto(fname);

       /*
        * Let user edit the file, parse it, play it back, and loop
        * doing that until the user is satisfied or gives up
        */
	do {
               /*
                * Edit the form
                */
		edit_proto(fname);		/* edit the form */

               /*
                * Parse the data supplied by user and play back as parsed
                */
		printf("\n\n");
		cc = read_file(fname, buf, (unsigned long)BSIZE); 
						/* read file into core */
		do_whole_buffer(buf, cc);	/* parse the whole file */

               /*
                * Decide whether to retry.  Retry is done if user doesn't
                * like the data as played back or an error was found and
                * user asks to re-edit.
                */
		retry = parse_fail;
		if (!retry)
		  retry = !get_yn("\nPLEASE CHECK CAREFULLY! Is all of this information correct?");
		if (retry) {
			retry = get_yn("\nWould you like to re-edit the entry?");
			if (!retry)
				return;	/* we go back to main menu */
		}
	} while (retry);

       /*
        * User is satisfied, add the data to the calendar
        */
	printf("\nAdding new entry to calendar database...\n");
	newid = create_new_entry();		
	if (newid>0)
		printf("Whatsup event i.d. number %d created.\n\n", newid);
}

	/*----------------------------------------------------------*/
	/*	
	/*			delete_entry
	/*	
	/*	Called when user wants to try and delete an existing
	/*	database entry.
	/*	
	/*	1) Prompt for the event id number
	/*	
	/*      2) Get the data so user can make sure it's the right one
	/*	
	/*	3) Use database library to do the deletion
	/*
	/*----------------------------------------------------------*/

int
delete_entry()
{
	int id;					/* number of event to */
						/* be deleted */
	RELATION old_event;			/* contents of original */
						/* event listing */
	int rc;					/* return code buffer */

       /*
        * Loop asking user to choose event.  Show him/her the existing
        * entry for that number, or loop if not found.  At end of loop,
        * we have the event number.
        */
	do {
		id = which_event("Enter i.d. number of event to delete (0 to return to menu)");
		if (id<=0) 
			return;
		rc = get_event(id, &old_event);
		if (rc != 0 || old_event == NULL||
		    FIRST_TUPLE_IN_RELATION(old_event)==NULL)
		  printf("Event %d is not in the whatsup database\n", id);
	} while (rc !=0 || old_event == NULL ||
		 FIRST_TUPLE_IN_RELATION(old_event)==NULL);

       /*
        * Ask user whether this is really the one to delete
        */
 	printf("The description for event %d is...\n\n", id);
	print_event(old_event);
	delete_relation(old_event);
	if (!get_yn("Is this the event you want to delete?"))
		return;

       /*
        * Do the deletion
        */
	printf("Deleting event %d from database\n", id);
	(void) delete_event(id);
	printf("Event %d deleted\n\n", id);
}

	/*----------------------------------------------------------*/
	/*	
	/*			replace_entry
	/*	
	/*	User wants to update the entry for an an event listing
	/*	in the calendar.
	/*	
	/*	1) Prompt for id number of event to be updated.
	/*	
	/*	2) Retrieve the data from the database and make
	/*	   sure that it's the right one by asking user.
	/*	
	/*	3) Put that data into a prefilled form and call the
	/*	   editor.
	/*	
	/*	4) Parse the data and play it back to the user for
	/*	   verification.
	/*	
	/*	5) Use the database library to update the database.
	/*	
	/*----------------------------------------------------------*/

int
replace_entry()
{
	int id;					/* number of event to */
						/* be deleted */
	int newid;				/* the new event number */
						/* being created */
	RELATION old_event;			/* contents of original */
						/* event listing */
	int rc;					/* return code buffer */
	int cc;					/* number of chars in file*/
	int retry;				/* retry the edit ? */


       /*
        * Loop asking user which event to replace.  Retrieve the 
        * data for it.  Loop if it does not exist.
        */
	do {
		id = which_event("Enter i.d. number of event to replace (0 to return to menu)");
		if (id<=0) 
			return;
		rc = get_event(id, &old_event);
		if (rc != 0 || old_event == NULL||
		    FIRST_TUPLE_IN_RELATION(old_event)==NULL)
		  printf("Event %d is not in the whatsup database\n", id);
	} while (rc !=0 || old_event == NULL ||
		 FIRST_TUPLE_IN_RELATION(old_event)==NULL);

       /*
        * Show the existing entry to the user, and ask if it's
        * the right one.  Return to main menu if not.
        */
	printf("The description for event %d is...\n\n", id);
	print_event(old_event);
	if (!get_yn("Is this the event you want to update?")) {
		delete_relation(old_event);
		return;
	}

       /*
        * User says this is the right one.  Write out a file containing
        * an event entry form with the old data filled in.
        * 
        * Loop asking the user to edit it, parsing it, and playing
        * back the parsed data until the user is satisfied or gives up.
        */
	write_prefilled(fname, old_event);
	do {
		edit_proto(fname);
		printf("\n\n");
		cc = read_file(fname, buf, (unsigned long)BSIZE);
		do_whole_buffer(buf, cc);
		retry = parse_fail;
		if (!retry)
		  retry = !get_yn("\nPLEASE CHECK CAREFULLY! Is all of this information correct?");
		if (retry) {
			retry = get_yn("\nWould you like to re-edit the entry?");
			if (!retry)
				return;
		}
	} while (retry);

       /*
        * User wants to go ahead and make the replacement.  First,
        * create a completely new entry with the new data.
        */
	printf("Now creating a new entry with revised info..hang on\n");
	newid = create_new_entry();
	if (newid<=0) {
		printf("Retaining old entry under id number %d\n", id);
		delete_relation(old_event);
		return;
	}
	printf("Whatsup event i.d. number %d created.\n\n", newid);

       /*
        * Create succeeded, delete old version.  Note create/delete
        * combination is not atomic.  Two copies may be left in case
        * of error.  I don't trust Ingres on long transactions
        */
	printf("Now deleting old entry...\n");
	if (delete_event(id)==0)
		printf("Old entry deleted\n");
	else
		printf("Error deleting old information from database\nWarning: both copies are probably still there.\nPlease note the new and the old event i.d. and report\nthis problem to whatsup@athena.  Thank you.\n");
	
	delete_relation(old_event);
       
}

/*----------------------------------------------------------*/
/*	
/*			master_init
/*	
/*	Gives us a chance to set up before doing anything.
/*	Args are same as for main program.
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
	av = *argv;


       /*
        * handle the secret -db option
        */
        (void) strcpy(whatsup_db, "whatsup-mit");
        if (*argc>1) {
                if(strcmp(av[1], "-db") != 0) {
                        fprintf(stderr, "Correct form is: postit.\n\nNo arguments are needed on the command line.\n\n");
			exit(16);
                } else {
                       /*
                        * site specification attempted
                        */
                        if (*argc < 3) {
                                fprintf(stderr, "Correct form is postit <-db dbname@site>\n");
                                exit(16);
                        }
                        (void) strcpy(whatsup_db, av[2]);
			*argc -= 2;
			*argv += 2;

                }
        }

	gdb_init();

	hello_prompt();

	caveats();


        fprintf(stderr, "Attempting connection to %s...", whatsup_db);
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
	char *env;
	char *getenv();

       /*
        * Get today's date and time
        */
	init_times();

       /*
        * Set up the name for the scratch file.  Make it $HOME/whatsup.new
        * or if there is no $HOME, just whatsup.new in current dir.
        */
	env = getenv("HOME");
	if (env != NULL && *env !=' ' && *env != '\0' ) {
		(void) strcpy(fname, env);
		(void) strcat(fname, "/");
	}
	(void) strcat(fname, "whatsup.new");
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


