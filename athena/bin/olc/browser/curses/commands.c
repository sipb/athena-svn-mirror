/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/commands.c,v $
 *	$Author: treese $
 *	$Locker:  $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/commands.c,v 1.1 1986-01-18 18:08:48 treese Exp $
 */

#ifndef lint
static char *rcsid_commands_c = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/commands.c,v 1.1 1986-01-18 18:08:48 treese Exp $";
#endif	lint

/* This file is part of the CREF finder.  It contains
 *
 *	Win Treese
 *	MIT Project Athena
 *
 *	Copyright (c) 1985 by the Massachusetts Institute of Technology
 *
 *	Created:
 *
 *	$Log: not supported by cvs2svn $
 *
 */

#ifndef lint
static char rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/olc/browser/curses/commands.c,v 1.1 1986-01-18 18:08:48 treese Exp $";
#endif

#include <stdio.h>				/* Standard I/O definitions. */
#include <curses.h>				/* Curses package defs. */
#include <ctype.h>				/* Character type macros. */
#include <strings.h>				/* String defs. */
#include <sys/file.h>				/* System file definitions. */
#include <errno.h>				/* System error codes. */

#include "cref.h"				/* CREF finder defs. */

/* Global variables. */

extern COMMAND Command_Table[];			/* CREF command table. */
extern int errno;				/* System error number. */

/* Function:	print_help() prints help information for CREF commands.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

print_help()
{
	int comm_count;				/* Number of commands. */
	int comm_index;				/* Index in command table. */

	comm_count = get_comm_count();
	clear();
	refresh();
	printf("Commands are:\n\n");
	for (comm_index = 0; comm_index < comm_count; comm_index++)
		{
		printf("\t%c\t%s\n", Command_Table[comm_index].command,
			Command_Table[comm_index].help_string);
		}
	printf("   <space>\tDisplay next index page.\n");
	printf("   <number>\tDisplay specified entry.\n");
	standout();
	mvaddstr(LINES-1, 0, "Hit any key to continue");
	standend();
	refresh();
	getch();
	clear();
	make_display();
}


/* Function:	top() moves to the top of the directory hierarchy.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

top()
{
	set_current_dir(ROOT_DIR);
	make_display();
}

/* Function:	manual() moves to the CRM section of CREF.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

manual()
{
	char manual_dir[FILENAME_SIZE];		/* Directory for CRM. */

	strcpy(manual_dir, ROOT_DIR);
	strcat(manual_dir, "/");
	strcat(manual_dir, MANUAL_DIR);
	set_current_dir(manual_dir);
	make_display();
}

/* Function:	prev_entry() displays the previous CREF entry.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

prev_entry()
{
	int new_index;				/* New entry index. */

	if ( (new_index = get_current_index() - 1) < 1)
		message(1, "Beginning of entries.");
	else display_entry(new_index);
}

/* Function:	next_entry() displays the previous CREF entry.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

next_entry()
{
	int new_index;				/* New entry index. */

	if ( (new_index = get_current_index() + 1) > entry_count())
		message(1, "No more entries.");
	else display_entry(new_index);
}

/* Function:	up_level() moves up one level in the CREF hierarchy.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

up_level()
{
	char new_dir[FILENAME_SIZE];		/* New current directory. */
	char *tail;				/* Ptr. to tail of path. */

	get_current_dir(new_dir);
	tail = rindex(new_dir, '/');
	if (tail != NULL)
		{
		*tail = (char) NULL;
		if ( strlen(new_dir) >= strlen(ROOT_DIR) )
			set_current_dir(new_dir);
		}
	make_display();
}

/* Function:	save_file() saves an entry in a file.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

save_to_file()
{
	ENTRY *save_entry;			/* Entry to save. */
	int out_fd;				/* Ouput file descriptor. */
	int in_fd;				/* Input file descriptor. */
	char inbuf[LINE_LENGTH];		/* Input buffer. */
	char error[ERRSIZE];			/* Error message. */
	int save_index;				/* Index of desired entry. */
	int nbytes;				/* Number of bytes read. */

	message(1, "Save entry? ");
	getstr(inbuf);
	save_index = atoi(inbuf);
	if ( (save_entry = get_entry(save_index)) == NULL)
		{
		message(2, "Invalid entry number.");
		return;
		}
	if (save_entry->type == CREF_DIR)
		{
		message(2, "Can't save directory entry.");
		return;
		}
	if ( (in_fd = open(save_entry->filename, O_RDONLY, 0)) < 0)
		{
		if (errno == EPERM)
			sprintf(error,"You are not allowed to read this file");
		else sprintf(error, "Unable to open CREF file %s\n", inbuf);
		message(1, error);
		return;
		}
	message(1, "Filename? ");
	getstr(inbuf);
	message(1, "");
	if ((out_fd = open(inbuf, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
		{
		sprintf(error, "Unable to open file %s\n", inbuf);
		message(1, error);
		return;
		}
	while ( (nbytes = read(in_fd, inbuf, LINE_LENGTH)) == LINE_LENGTH)
		write(out_fd, inbuf, LINE_LENGTH);
	write(out_fd, inbuf, nbytes);
	close(out_fd);
	close(in_fd);
}

/* Function:	next_page() displays the next page of the index.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

next_page()
{
	int new_start;				/* New starting index. */

	new_start = get_index_start() + MAX_INDEX_LINES - 2;
	set_index_start(new_start);
	make_display();
}

/* Function:	prev_page() displays the previous page of the index.
 * Arguments:	None.
 * Returns:	Nothing.
 * Notes:
 */

prev_page()
{
	int new_start;				/* New starting index. */

	new_start = get_index_start() - MAX_INDEX_LINES + 2;
	if (new_start < 1)
		new_start = 1;
	set_index_start(new_start);
	make_display();
}


/* Function:	quit() quits the CREF finder program.
 * Arguments:	None.
 * Returns:	Doesn't return.
 * Notes:
 */

quit()
{
	move(LINES-1, 0);
	refresh();
	echo();
	noraw();
	endwin();
	exit(SUCCESS);
}
