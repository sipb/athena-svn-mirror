/************************************************************************/
/*	
/*			what_lib.c
/*	
/*	Common Library routines for the Project Athena Online Calendar
/*	
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	9/1/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/util/what_lib.c,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/util/what_lib.c,v 1.1 1993-10-12 05:35:18 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*	
/*	
/*	
/************************************************************************/

#ifndef lint
static char rcsid_what_lib_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/util/what_lib.c,v 1.1 1993-10-12 05:35:18 probe Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <curses.h>
#include "gdb.h"

DATABASE calendar_data;
OPERATION open_db;
OPERATION begin_tran;
OPERATION end_tran;
OPERATION bump_wm;
OPERATION append_entry;
OPERATION append_entry2;
OPERATION query_uniq;
TUPLE_DESCRIPTOR uniq_desc;
RELATION  uniq_rel;

	/*----------------------------------------------------------*/
	/*	
	/*			MakeQueryDescriptor
	/*	
	/*	Build the tuple descriptor for the query
	/*	
	/*----------------------------------------------------------*/

char *dsc_names[] = {"uniq", "date", "time", "type", "place", "title", "comments"};
FIELD_TYPE dsc_types[] = {INTEGER_T, INTEGER_T, INTEGER_T, STRING_T, STRING_T,
		      STRING_T, STRING_T};
TUPLE_DESCRIPTOR
MakeQueryDescriptor()
{
#define UNIQ 0
#define DATE 1
#define TIME 2
#define EVENT_TYPE 3
#define PLACE 4
#define TITLE 5
#define COMMENTS 6
	int nflds = 7;

	return create_tuple_descriptor(nflds, dsc_names, dsc_types);
}

	/*----------------------------------------------------------*/
	/*	
	/*			PrimeDb
	/*	
	/*	Starts the business of accessing the database
	/*	
	/*----------------------------------------------------------*/

char *dsc2_names[] = {"uniq"};
FIELD_TYPE dsc2_types[] = {INTEGER_T};
int
PrimeDb(name)
char *name;
{
	open_db = create_operation();
	begin_tran = create_operation();
	end_tran = create_operation();
	bump_wm = create_operation();
	append_entry = create_operation();
	append_entry2 = create_operation();
	query_uniq = create_operation();

	uniq_desc = create_tuple_descriptor(1, dsc2_names, dsc2_types);

	start_accessing_db(open_db, name, &calendar_data);
	if (OP_DONE(open_db) && OP_STATUS(open_db) != OP_COMPLETE) {
		(void) wclear(stdscr);
		(void) wrefresh(stdscr);
		endwin();
		fprintf(stderr, "\nCould not access calendar database...server machine may be down.\n");
		exit(8);
	}
}


	/*----------------------------------------------------------*/
	/*	
	/*			DoQuery
	/*	
	/*	Based on the parameters gathered, get a relation
	/*	describing the events.
	/*	
	/*----------------------------------------------------------*/

int
DoQuery(relp, query, query_desc)
RELATION *relp;
char *query;
TUPLE_DESCRIPTOR query_desc;
{
	*relp = create_relation(query_desc);

       /*
        * Make sure we really have access to the database
        */

	if (!OP_DONE(open_db))
		complete_operation(open_db);

	if (OP_STATUS(open_db)!=OP_COMPLETE ||
	    DB_STATUS(calendar_data) != DB_OPEN)  {
		(void) wclear(stdscr);
		(void) wrefresh(stdscr);
		endwin();
		fprintf(stderr, "Could not access calendar database\n");
		exit(8);
	}

       /*
        * Do the query
        */
	if (db_query(calendar_data, *relp, query) == OP_SUCCESS) 
		return 0;			/* indicate success */
	else {
		return 4;
	}
		
}

	/*----------------------------------------------------------*/
	/*	
	/*			limit_retrieves
	/*	
	/*	Call this routine to set a maximum number of items
	/*	that will be retrieved on a search.  0==no max.
	/*	Note that no error is given when the limit is hit.
	/*	Use a limit of one more than you want to detect overruns.
	/*	(e.g. 51 when you want 50--if you get the 51st,
	/*	then there were too many)
	/*	
	/*----------------------------------------------------------*/

int
limit_retrieves(maxnum)
int maxnum;
{
	char buffer[80];

       /*
        * Format the request
        */
	(void) sprintf(buffer, "gdb retrieve_limit %d",  maxnum);

       /*
        * Make sure we really have access to the database
        */

	if (!OP_DONE(open_db))
		complete_operation(open_db);

	if (OP_STATUS(open_db)!=OP_COMPLETE ||
	    DB_STATUS(calendar_data) != DB_OPEN)  {	
		(void) wclear(stdscr);
		(void) wrefresh(stdscr);
		endwin();
		fprintf(stderr, "Could not access calendar database\n");
		exit(8);
	}

       /*
        * Set the limit
        */
	if( perform_db_operation(calendar_data, buffer ) !=
	   OP_SUCCESS) {
		   endwin();
		   fprintf(stderr, "Could not set retrieve limit to %d\n",
			   maxnum);
		   exit(64);
	}

	return 0;


}

	/*----------------------------------------------------------*/
	/*	
	/*			add_db_entry
	/*	
	/*	Call this routine to register a new event in the
	/*	database.
	/*	
	/*----------------------------------------------------------*/

int
add_db_entry(date, time, type, place, title, comment)
int date;
int time;
char *type;
char *place;
char *title;
char *comment;
{
	char buffer[2048], buffer2[2048];
	TUPLE t;
	int uniq;

	uniq_rel = create_relation(uniq_desc);
	

       /*
        * Make sure we really have access to the database
        */

	if (!OP_DONE(open_db))
		complete_operation(open_db);

	if (OP_STATUS(open_db)!=OP_COMPLETE ||
	    DB_STATUS(calendar_data) != DB_OPEN)  {
		(void) wclear(stdscr);
		(void) wrefresh(stdscr);
		endwin();
		fprintf(stderr, "Could not access calendar database\n");
		exit(8);
	}

       /*
        * Do the update transaction
        */
	start_performing_db_operation(begin_tran, calendar_data,
				      "begin transaction");
	start_performing_db_operation(bump_wm, calendar_data,
			      "replace highwater (mark=highwater.mark+1)");
	(void) sprintf(buffer,"append calitems(uniq=highwater.mark, type=lowercase(\"%s\"), place=\"%s\", title=\"%s\", comments=\"%s\")", type, place, title, comment);
	start_performing_db_operation(append_entry, calendar_data, buffer );
	(void) sprintf(buffer2,"append caltimes(uniq=highwater.mark, date=%d, time=%d)", date, time);
	start_performing_db_operation(append_entry2, calendar_data, buffer2 );
	start_db_query(query_uniq, calendar_data, uniq_rel, 
		       "(>*uniq*<=highwater.mark)");
	complete_operation(query_uniq);
       /*
        * check for loss of connection errors
        */
	if (OP_STATUS(begin_tran) != OP_COMPLETE ||
	    OP_STATUS(bump_wm) != OP_COMPLETE ||
	    OP_STATUS(append_entry) != OP_COMPLETE ||
	    OP_STATUS(append_entry2) != OP_COMPLETE ||
	    OP_STATUS(query_uniq) != OP_COMPLETE) {
		    delete_relation(uniq_rel);
		    fprintf(stderr, "Connection to database lost\n");
		    exit(64);
	    }
        /*
         * Check for unsuccessful execution 
         */
	if (OP_RESULT(begin_tran) != OP_SUCCESS ||
	    OP_RESULT(bump_wm) != OP_SUCCESS ||
	    OP_RESULT(append_entry) != OP_SUCCESS ||
	    OP_RESULT(append_entry2) != OP_SUCCESS ||
	    OP_RESULT(query_uniq) != OP_SUCCESS) {

		    fprintf(stderr,"Could not update, aborting transaction\n");
		    perform_db_operation(calendar_data,"abort");
		    delete_relation(uniq_rel);
		    return (-2);
	} else {
		    if(perform_db_operation(calendar_data,"end transaction")!=
		       OP_SUCCESS) {
			       fprintf(stderr,"Could not commit update\n");
			       return (-1);
		    } else {
			    t = FIRST_TUPLE_IN_RELATION(uniq_rel);
			    uniq = *(int *)FIELD_FROM_TUPLE(t,0);
			    delete_relation(uniq_rel);
			    return uniq;
			    
		    }
	      
	}	    

}
	/*----------------------------------------------------------*/
	/*	
	/*			add_a_time
	/*	
	/*	Call this routine to register additional times for
	/*	events in the database.
	/*	
	/*----------------------------------------------------------*/

int
add_a_time(uniq, date, time)
int uniq;
int date;
int time;
{
	char buffer2[2048];

       /*
        * Make sure we really have access to the database
        */

	if (!OP_DONE(open_db))
		complete_operation(open_db);

	if (OP_STATUS(open_db)!=OP_COMPLETE ||
	    DB_STATUS(calendar_data) != DB_OPEN)  {
		(void) wclear(stdscr);
		(void) wrefresh(stdscr);
		endwin();
		fprintf(stderr, "Could not access calendar database\n");
		exit(8);
	}

       /*
        * Do the update transaction
        */
	(void) sprintf(buffer2,"append caltimes(uniq=%d, date=%d, time=%d)", uniq, date, time);
	if( perform_db_operation(calendar_data, buffer2 ) !=
	   OP_SUCCESS) {
		   fprintf(stderr, "Could not add new times for event #%d\n",
			   uniq);
		   exit(64);
	}

	return 0;
}

	/*----------------------------------------------------------*/
	/*	
	/*			delete_event
	/*	
	/*	This routine is used to delete an event given its
	/*	unique i.d.
	/*	
	/*----------------------------------------------------------*/

int
delete_event(uniq)
int uniq;
{
	char buffer[2048], buffer2[2048];

       /*
        * Make sure we really have access to the database
        */

	if (!OP_DONE(open_db))
		complete_operation(open_db);

	if (OP_STATUS(open_db)!=OP_COMPLETE ||
	    DB_STATUS(calendar_data) != DB_OPEN)  {
		(void) wclear(stdscr);
		(void) wrefresh(stdscr);
		endwin();
		fprintf(stderr, "Could not access calendar database\n");
		exit(8);
	}

       /*
        * Do the update transaction
        */
	start_performing_db_operation(begin_tran, calendar_data,
				      "begin transaction");
	(void) sprintf(buffer,"delete calitems where (calitems.uniq=%d)", uniq);
	start_performing_db_operation(append_entry, calendar_data, buffer );
	(void) sprintf(buffer2,"delete caltimes where (caltimes.uniq = %d)", uniq);
	start_performing_db_operation(append_entry2, calendar_data, buffer2 );
	complete_operation(append_entry2);
       /*
        * check for loss of connection errors
        */
	if (OP_STATUS(begin_tran) != OP_COMPLETE ||
	    OP_STATUS(append_entry) != OP_COMPLETE ||
	    OP_STATUS(append_entry2) != OP_COMPLETE ) {
		    fprintf(stderr, "Connection to database lost\n");
		    exit(64);
	    }
        /*
         * Check for unsuccessful execution 
         */
	if (OP_RESULT(begin_tran) != OP_SUCCESS ||
	    OP_RESULT(append_entry) != OP_SUCCESS ||
	    OP_RESULT(append_entry2) != OP_SUCCESS) {

		    fprintf(stderr,"Could not delete event %d, aborting transaction\n", uniq );
		    perform_db_operation(calendar_data,"abort");
		    return (-2);
	} else {
		    if(perform_db_operation(calendar_data,"end transaction")!=
		       OP_SUCCESS) {
			       fprintf(stderr,"Could not commit deletion\n");
			       return (-1);
		    } else {
			    return 0;
			    
		    }
	      
	}	    

}
