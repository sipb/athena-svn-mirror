/************************************************************************/
/*      
/*                      postprint.c
/*      
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	9/1/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postprint.c,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postprint.c,v 1.1 1993-10-12 05:35:11 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*      
/*      Purpose:
/*	
/*	Postit routines to format data to terminal.
/*      
/************************************************************************/

#ifndef lint
static char rcsid_postprint_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postprint.c,v 1.1 1993-10-12 05:35:11 probe Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <curses.h>
#include "gdb.h"
#include "postit.h"

	/*----------------------------------------------------------*/
	/*	
	/*			print_event
	/*	
	/*	Given an event description in a relation, print it.
	/*	
	/*----------------------------------------------------------*/

int
print_event(rel)
RELATION rel;
{
	TUPLE t;
	TUPLE_DESCRIPTOR desc;
	int Index;
	int Date, time;
	STRING *Place, *type, *comments, *Title;
	char local_comments[2048];

       /*
        * Make sure there's something in the tuple, and print the
        * first time info
        */
	t = FIRST_TUPLE_IN_RELATION(rel);
	if (t==NULL) {
		fprintf(stderr, "Error: an empty relation was retrieved\nPlease report this bug to noah@athena.mit.edu\n");
		exit(64);
	}
	desc = DESCRIPTOR_FROM_TUPLE(t);

	printf("\n*****************************************\n\n");


       /*
        * Pick up all the fields in the first relation and print them
        */
	Index = field_index(desc, "place");
	Place = (STRING *)FIELD_FROM_TUPLE(t,Index);
	Index = field_index(desc, "type");
	type = (STRING *)FIELD_FROM_TUPLE(t,Index);
	Index = field_index(desc, "title");
	Title = (STRING *)FIELD_FROM_TUPLE(t,Index);
	Index = field_index(desc, "comments");
	comments = (STRING *)FIELD_FROM_TUPLE(t,Index);

	printf("Title:\t%s\n", STRING_DATA(*Title));
	printf("Place:\t%s\n", STRING_DATA(*Place));
	printf("Type:\t%s\n", STRING_DATA(*type));
	(void) strcpy(local_comments, STRING_DATA(*comments));
	format_string(local_comments, 70-sizeof("$Comments: "), 70, '\t');
	printf("Comments: %s\n\n", local_comments);


       /*
        * Print all time fields
        */
	do {
		Index = field_index(desc, "date");
		Date = *(int *)FIELD_FROM_TUPLE(t,Index);
		Index = field_index(desc, "time");
		time = *(int *)FIELD_FROM_TUPLE(t,Index);

		fprint_time(stdout, Date, time);
		t = NEXT_TUPLE_IN_RELATION(rel, t);
	} while (t != NULL);

	printf("\n*****************************************\n");
}

	/*----------------------------------------------------------*/
	/*	
	/*			fprint_time
	/*	
	/*	Format and print a date and time field for review
	/*	
	/*----------------------------------------------------------*/


int
fprint_time(f, Date, time)
FILE *f;
int Date;
int time;
{
	int year, month, day, hour, mins;
	char ap;

	year = Date/10000;
	month = (Date/100)%100;
	day = Date%100;

	hour = time/100;
	mins = time%100;
	if (time == 0)
		ap = 'M';
	else if (time == 1200) {
		ap = 'N';
	} else if (hour>=12)  {
		ap = 'P';
		if (hour > 12) 
			hour-=12;		
	} else
		ap = 'A';

	fprintf(f, "When:   %d/%d/%d %d:%02d%c\n", month, day, year, hour, 
		mins, ap);

}

        /*----------------------------------------------------------*/
        /*      
        /*                  write_out_entry
        /*      
        /*      Called each time an entry or sub_entry is complete
        /*      to write it out.  Any fields whose data is not
        /*      null should be valid.
        /*      
        /*----------------------------------------------------------*/

int
write_out_entry()
{
	register int type;

	int id;


	new_entry();

       /*
        * Get excess blanks out of the fields
        */
        for(type=0; type<NTYPES; type++) {
                if (*fields[type].data != '\0') {
			compress_blanks(fields[type].data);
                }
        }

       /*
        * Set up all the data in the form we really want it
        */
	make_title();
	make_place();
	make_comments();
	make_times();
	make_type();
	if (parse_fail && mode == POSTIT)
		return;

       /*
        * Make sure that any " characters in the strings are escaped
        * in the manner that Ingres wants them
        */
	escape_quotes(event_type);
	escape_quotes(place);
	escape_quotes(title);
	escape_quotes(comment);
	print_entry();

       /*
        * If we're in postit, then our callers will figure out what to
        * do with the single parsed entry.  Otherwise, we're in bulkpost,
        * and we have to handle it here.
        */
	switch (mode) {
	      case POSTIT:
		break;
	      case BULK_CHECKING:
		if (parse_fail || error_found) {
			go_on = get_yn("\nDo you want to keep going?");
			printf("\n\t************************************\n\n");
			if (parse_fail)
				error_found = TRUE;
			parse_fail = FALSE;
			return;
		}
		if (!get_yn("\nIs all of the information for this entry correct? ")) 
			error_found = TRUE;	/* tell outer loop that */
						/* overall parse was not */
						/* successful */
		parse_fail = FALSE;		/* parse fail is per entry, */
						/* and we're going to try */
						/* another*/
		printf("\n\t************************************\n\n");

		break;
	      case BULK_UPDATING:
		if (parse_fail || error_found) {
			fprintf(stderr,"Parse error found during update phase. Giving up.\nPlease report this problem to whatsup@athena, and save a copy\nof your data file for reference.  Thank you.\n");
			exit(128);
		}
		id = create_new_entry();
		if (id >0)
			printf("\nWhatsup event i.d. number %d assigned to this event.\n\n\t***********************************************\n\n", id);
		break;
	      default:
		fprintf(stderr, "Internal error: bad mode = %d\n", mode);
		exit(64);
	}
	if (mode == POSTIT)
		return;


}

	/*----------------------------------------------------------*/
	/*	
	/*			print_entry
	/*	
	/*	Print the contents of an entry which has been 
	/*	parsed into the global variable fields.
	/*	
	/*----------------------------------------------------------*/

int
print_entry()
{
	register int i;
	char local_comments[1000];

       /*
        * Do word wraps on the comments
        */
	(void) strcpy(local_comments, comment);
	format_string(local_comments, 70-sizeof("$Comments: "), 70, '\t');
	printf(" Type:   %s\n Title:  %s\n Place:  %s\n Comment: %s\n",
	       event_type, title, place, local_comments);
	for (i=0; i<ntimes; i++) {
		(void) putchar(' ');		/* to line up with fields */
						/* above */
		fprint_time(stdout, date[i], Times[i]);
	}
	
	if (mode == POSTIT)			/* ugh-one proggram */
						/* doing too many things!*/
		printf("\n\t************************************\n\n");

}

	/*----------------------------------------------------------*/
	/*	
	/*			line_tag
	/*	
	/*	Prints out a line number tag on stderr, as a prefix
	/*	to an error message.
	/*	
	/*----------------------------------------------------------*/

int
line_tag()
{
	fprintf(stderr, "\"%s\", line %d: ", fname, line_num);
}



int
endwin()
{
    /* this keeps what_lib happy */
}

int
wclear(scr)
WINDOW *scr;
{
}

int
wrefresh(scr)
WINDOW *scr;
{
}

