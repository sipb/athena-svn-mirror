/************************************************************************/
/*      
/*                      postdata.c
/*      
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	9/1/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postdata.c,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postdata.c,v 1.1 1993-10-12 05:35:06 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*      
/*      Purpose:
/*	
/*	Contains file and database manipulation routines for postit.
/*	In general, file handling routines are dealing with the local
/*	file, which contains the data as entered or edited by the user.
/*	Database manipulation routines interact with the central 
/*	whatsup database.
/*      
/************************************************************************/

#ifndef lint
static char rcsid_postdata_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postdata.c,v 1.1 1993-10-12 05:35:06 probe Exp $";
#endif

#include "mit-copyright.h"
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include "gdb.h"
#include "postit.h"

	/*----------------------------------------------------------*/
	/*	
	/*			read_file
	/*	
	/*	Read the entire file into the local buffer
	/*	Blow up if there is an error.
	/*	
	/*	Reset all line counters and parse switches to reflect
	/*	a new file.
	/*	
	/*----------------------------------------------------------*/

int
read_file(name, buf, size)
char *name;
char *buf;
unsigned long size;
{
	int d;					/* file descriptor */
	int cc;					/* number of chars in file */

	line_num= 1;
	error_found = FALSE;
	parse_fail= FALSE;

        d = open(name,O_RDONLY,444);
	if (d <= 0) {
		perror(fname);
		exit(32);
	}
	/*
	 * Read the whole file, skip if error
	 */
	if ((cc = read(d,buf,(int) size)) <= 0) {
		perror(fname);
		exit(64);
	}
        (void) close(d);

	return cc;

}

	/*----------------------------------------------------------*/
	/*	
	/*			write_prefilled
	/*	
	/*	This routine is called if the user has expressed
	/*	an interest in replacing an existing whatsup entry.
	/*	
	/*----------------------------------------------------------*/

char *prefilled_header[] = {
"#========================================================================\n",
"#                        WHATSUP EVENT DETAILS                          |\n",
"#                                                                       |\n",
"#       Make your changes in the fields below.  This information,       |\n",
"#       including time listings, will COMPLETELY REPLACE the old info.  |\n",
"#                                                                       |\n",
"#       1) ALL fields should be filled in. (You may have to scroll down |\n",
"#          to get to fields which don't fit on the screen now.)         |\n",
"#                                                                       |\n",
"#       2) You will have a chance to give up on posting the event       |\n",
"#          if you get into trouble, just leave the editor               |\n",
"#                                                                       |\n",
"#   IF YOU ARE USING GNUEMACS, M-] GOES TO NEXT FIELD, M-[ TO PREVIOUS  |\n",
"#   (hold down ALT and type ], or type ESC(F11 on VS2) and then ])      |\n",
"#========================================================================\n",
"\n",
"$BeginEvent:   DO NOT CHANGE THIS LINE\n",
"\n",};

int
write_prefilled(name, old_event)
char *name;
RELATION old_event;
{
	char local_comments[1000];
	FILE *f;
	int nmsgs = sizeof(prefilled_header)/sizeof(char *);
	register int i;
	TUPLE t;
	TUPLE_DESCRIPTOR desc;
	int Index;
	int Date, time;
	STRING *Place, *type, *comments, *Title;

	f = fopen(name, "w");
	if (f==NULL) {
		fprintf(stderr, "Could not write file %s\nMake sure your HOME environment variable is set properly.\n", name);
		exit(32);
	}
	
	for (i=0; i<nmsgs; i++)
		fputs(prefilled_header[i], f);


       /*
        * Make sure there's something in the tuple, and print the
        * first time info
        */
	t = FIRST_TUPLE_IN_RELATION(old_event);
	if (t==NULL) {
		fprintf(stderr, "Error: an empty relation was retrieved\nPlease report this bug to noah@athena.mit.edu\n");
		exit(64);
	}
	desc = DESCRIPTOR_FROM_TUPLE(t);

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

	fputs("#========================================================================\n",f);
	fputs("#         Supply a one word type code.  See postit man page or existing |\n",f);
	fputs("#         whatsup entries for suggestions.  (Future versions of         |\n",f);
	fputs("#         postit and whatsup will give help choosing types.)            |\n",f);
	fputs("#========================================================================\n",f);
	fprintf(f,"$Type: %s\n", STRING_DATA(*type));
	fputs("#========================================================================\n",f);
	fputs("#         Specify event title (70 chars max) and place where event will |\n",f);
	fputs("#         be held (45 chars max).                                       |\n",f);
	fputs("#========================================================================\n",f);
	fprintf(f,"$Title: %s\n", STRING_DATA(*Title));
	fprintf(f,"$Place: %s\n", STRING_DATA(*Place));
       /*
        * Print all time fields
        */
	fputs("#========================================================================\n",f);
	fputs("#          The correct form for \"When\" is MM/DD/YY HH:MM(A or P)        |\n",f);
	fputs("#          You may repeat the When line as often as you like, e.g.:     |\n",f);
	fputs("#                $When: 02/04/87  3:30P                                 |\n",f);
	fputs("#                $When: 02/04/87 15:30                                  |\n",f);
	fputs("#                $When: 02/05/87  8:00A                                 |\n",f);
	fputs("#         will list the event twice on Feb. 4, and once on Feb. 5       |\n",f);
	fputs("#========================================================================\n",f);
	do {
		Index = field_index(desc, "date");
		Date = *(int *)FIELD_FROM_TUPLE(t,Index);
		Index = field_index(desc, "time");
		time = *(int *)FIELD_FROM_TUPLE(t,Index);

		fprintf(f,"$");			/* so the time fields */
						/* will pick up a $*/
		fprint_time(f,Date, time);
		t = NEXT_TUPLE_IN_RELATION(old_event, t);
	} while (t != NULL);

       /*
        * Write out comments
        */

	fputs("#========================================================================\n",f);
	fputs("#         Comments may extend over approx. 10 lines (800) chars.        |\n",f);
	fputs("#         You may start a new line anytime; postit will re-wrap         |\n",f);
	fputs("#         the information you supply into a single paragraph.           |\n",f);
	fputs("#========================================================================\n",f);
	(void) strcpy(local_comments, STRING_DATA(*comments));
	format_string(local_comments, 70-sizeof("$Comments: "), 70, '\0');
	fprintf(f,"$Comments: %s\n", local_comments);

	(void) fclose(f);
}
	/*----------------------------------------------------------*/
	/*	
	/*			write_proto
	/*	
	/*	Write out the prototype file.
	/*	
	/*----------------------------------------------------------*/
char *proto_header[] = {
"#========================================================================\n",
"#                        WHATSUP NEW EVENT FORM                         |\n",
"#                                                                       |\n",
"#       Fill in the fields below with details of the event to be        |\n",
"#       posted to the calendar.                                         |\n",
"#                                                                       |\n",
"#       1) ALL fields should be filled in. (You may have to scroll down |\n",
"#          to get to fields which don't fit on the screen now.)         |\n",
"#                                                                       |\n",
"#       2) You will have a chance to give up on posting the event       |\n",
"#          if you get into trouble, just leave the editor               |\n",
"#                                                                       |\n",
"#   IF YOU ARE USING GNUEMACS, M-] GOES TO NEXT FIELD, M-[ TO PREVIOUS  |\n",
"#   (hold down ALT and type ], or type ESC(F11 on VS2) and then ])      |\n",
"#========================================================================\n",
"$BeginEvent:   DO NOT CHANGE THIS LINE\n",
"#========================================================================\n",
"#         Supply a one word type code.  See postit man page or existing |\n",
"#         whatsup entries for suggestions.  (Future versions of         |\n",
"#         postit and whatsup will give help choosing types.)            |\n",
"#========================================================================\n",
"$Type: \n",
"#========================================================================\n",
"#         Specify event title (70 chars max) and place where event will |\n",
"#         be held (45 chars max).                                       |\n",
"#========================================================================\n",
"$Title: \n",
"$Place: \n",
"#========================================================================\n",
"#        The correct form for \"When\" is MM/DD/YY HH:MM(A or P)         |\n",
"#        You may repeat the When line as often as you like, e.g.:       |\n",
"#                                                                       |\n",
"#                $When: 02/04/87  5:30P                                 |\n",
"#                $When: 02/04/87 15:30                                  |\n",
"#                $When: 02/05/87  8:00a                                 |\n",
"#                                                                       |\n",
"#         will list the event twice on Feb. 4, and once on Feb. 5       |\n",
"#========================================================================\n",
"$When: \n",
"#========================================================================\n",
"#         Comments may extend over approx. 10 lines (800) chars.        |\n",
"#         You may start a new line anytime; postit will re-wrap         |\n",
"#         the information you supply into a single paragraph.           |\n",
"#========================================================================\n",
"$Comments: \n",
};
int
write_proto(name)
char *name;
{
	FILE *f;
	int nmsgs = sizeof(proto_header)/sizeof(char *);
	register int i;

	f = fopen(name, "w");
	if (f==NULL) {
		fprintf(stderr, "Could not write file %s\nMake sure your HOME environment variable is set properly.\n", name);
		exit(32);
	}
	
	for (i=0; i<nmsgs; i++)
		fputs(proto_header[i], f);

	(void) fclose(f);

}

	/*----------------------------------------------------------*/
	/*	
	/*			edit_proto
	/*	
	/*	Edit the prototype file
	/*	
	/*----------------------------------------------------------*/

char *getenv();

int
edit_proto(name)
char *name;
{
	char *env;
	char *macs;

	env = getenv("POSTITEDITOR");

	if (env == NULL)
		env = getenv("VISUAL");

	if (env == NULL)
		env = getenv("EDITOR");

	if (env == NULL)
		env = "/usr/athena/emacs";

	printf("Invoking %s...\n", env);

	macs = getenv("POSTITEMACSMACROS");
	if (macs==NULL)
		macs = EMACSMACS;

	if (fork() == 0) {
               /*
                * Child process.  Heuristic: if name of command
                * ends in emacs, assume that's what it is and try
                * to load macros
                */
		if (strcmp(env+strlen(env)-5, "emacs")==0)
			execlp(env, env, name, "-l", macs, 0);
		else
			execlp(env, env, name, 0);
		
	}
	(void) wait((union wait *)0);		/* this may need work */


}


/************************************************************************/
/*	
/*			Database routines
/*	
/************************************************************************/


	/*----------------------------------------------------------*/
	/*	
	/*			get_event
	/*	
	/*	Retrieve a relation containing the description of
	/*	a supplied event.
	/*	
	/*----------------------------------------------------------*/

int
get_event(id, relp)
int id;						/* id number of the event */
RELATION *relp;					/* pointer to rel to be */
						/* created */
{
	char buffer[300];

	(void) sprintf(buffer, "(>*date*<=caltimes.date, >*time*<=caltimes.time, >*uniq*<=calitems.uniq, >*place*<=calitems.place, >*type*<=trim(calitems.type), >*title*<=calitems.title, >*comments*<=calitems.comments) where calitems.uniq = %d and caltimes.uniq=calitems.uniq", id);

	printf("\nRetrieving description of event %d\n", id);

	return DoQuery(relp, buffer, query_desc);
}


	/*----------------------------------------------------------*/
	/*	
	/*			create_new_entry
	/*	
	/*	Called to actually record the new event in the
	/*	database.  Returns its i.d. number in the db.
	/*	
	/*----------------------------------------------------------*/

int
create_new_entry()
{
	register int i;

	if (ntimes) {
		uniq=add_db_entry(date[0], Times[0], event_type,
				  place, title, comment);
	}

	if (uniq<0) {
		(void) fprintf(stderr, "Could not add item to database\n");
		return uniq;
        }

	if (ntimes>1 && uniq >=0) {
		for(i=1; i<ntimes; i++)  
			(void) add_a_time(uniq, date[i], Times[i]);
	}

	return uniq;
}

