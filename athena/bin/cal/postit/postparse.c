/************************************************************************/
/*      
/*                      postparse.c
/*      
/*
/*	Author: Noah Mendelsohn
/*	IBM T.J. Watson Research and Mit Project Athena
/*
/*	Revised:	9/1/87
/*
/*	$Source: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postparse.c,v $
/*	$Author: probe $
/*	$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postparse.c,v 1.1 1993-10-12 05:35:09 probe Exp $
/*
/*	Copyright 1987 by the Massachusetts Institute of Technology.
/*	For copying and distribution information, see the file mit-copyright.h
/*      
/*      Purpose:
/*	
/*	File parsing routines used by postit.
/*      
/************************************************************************/

#ifndef lint
static char rcsid_postparse_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/cal/postit/postparse.c,v 1.1 1993-10-12 05:35:09 probe Exp $";
#endif


#include "mit-copyright.h"
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include "gdb.h"
#include "postit.h"

	/*----------------------------------------------------------*/
	/*	
	/*		find_field
	/*	
	/*	Skip to a new field
	/*	
	/*----------------------------------------------------------*/

char *
find_field(cp, ep)
char *cp;
char *ep;
{
	register char *rcp = cp;
	register char *rep = ep;

       /*
        * Always start a new line
        */
	while (rcp < rep && *rcp++ != '\n')
	  ;
	line_num++;
	

       /*
        * If this line isn't a field start, then skip it
        */
	while (rcp < rep && *rcp != MAGIC_CHAR) {
		while (rcp < rep && *rcp++ != '\n')
		  ;
		line_num++;
	
	}
	return rcp;
}

        /*----------------------------------------------------------*/
        /*      
        /*                      do_whole_buffer
        /*      
        /*      Go through an entire file, which has already
        /*      been read into a malloc'd buffer, and do all
        /*      processing for each entry.
        /*      
        /*----------------------------------------------------------*/

int
do_whole_buffer(buf, cc)
char *buf;                                      /* pointer to data buffer */
int  cc;                                        /* number of bytes */
                                                /* from file */
{
        register char *next;                    /* next byte to scan */
        register char *endp;                    /* pointer to byte past */
                                                /* end of file */
	int nparsed;				/* number parsed */
        char *do_an_entry();

       /*
        * Assume success
        */
	parse_fail = FALSE;
	go_on = TRUE;

       /*
        * Set up the control pointers
        */
        next = buf;
        endp = next + cc;
       /* 
        * Scan for first line beginning with MAGIC_CHAR
        */
        while (*next != MAGIC_CHAR) {
               /*
                * Skip to end of any line with a comment char
                */
		while (next<endp && *next != '\n')
			next++;
		line_num++;
                next++;
		if (next>=endp)
			break;
	}

        if (next >=endp) {
                fprintf(stderr,"Error: file %s does not contain any line beginning with a '%c' charcter.\nEach field in a postit data file must begin with '%c'\n",
                        fname, MAGIC_CHAR, MAGIC_CHAR);
                parse_fail = TRUE;
		return;
        }

       /*
        * Parse all the entries
        * 
        * If we're in postit, then just return on an error, as there is
        * only one entry anyway.
        * 
        * If we're in bulkpost, then set the global error flag 
        * and keep going.
        */
	nparsed = 0;
        while (next != NULL) {
                next = do_an_entry(next, endp);
		if (parse_fail) {
			if (mode==POSTIT)
				return;
			else
				error_found = TRUE;
		}
		if (parsed_entry) {
			if (mode==POSTIT && nparsed) {
				fprintf(stderr, "ERROR: Only one event at a time may be created or updated with postit.\n\nYou may edit your file to list only one entry, or you may exit from postit\nand use the bulkpost program to add multiple listings to the calendar from a\nsingle input file.\n");
				error_found = TRUE;
				parse_fail = TRUE;
				return;
			}
			write_out_entry();
			nparsed++;
		}
               /*
                * Users of bulkpost have the option to break the loop
                * after each item is printed if there has been an error.
                */
		if (error_found && mode== BULK_CHECKING && !go_on)
			return;

		if (parse_fail) {
			if (mode==POSTIT)
				return;
			else {
				error_found = TRUE;
				parse_fail = FALSE;
			}
		}
		parsed_entry = FALSE;
	}
	
       /*
        * Make sure we parsed at least one entry
        */
	if (nparsed <=0) {
		fprintf(stderr, "Error: could not find a complete event description.\n");
		parse_fail = TRUE;
		error_found = TRUE;
	}
        
}



        /*----------------------------------------------------------*/
        /*      
        /*                      do_an_entry
        /*      
        /*----------------------------------------------------------*/

char *
do_an_entry(start, endp)
char *start;                                    /* entry scan starts here */
char *endp;                                     /* never scan past here */
{
        char *ret_val;
        char *set_up_entry();

	parsed_entry = FALSE;

        ret_val = set_up_entry(start, endp);


        return ret_val;
}

        /*----------------------------------------------------------*/
        /*      
        /*                   set_up_entry
        /*      
        /*      Scan for the next entry and copy out the pieces.
        /*      Sub_entries are handled specially.
        /*      
        /*      Note: start should ALWAYS point to the MAGIC_CHAR which marks
        /*      a new entry, and this routine will always return
        /*      a pointer to such a !, except when NULL is returned
        /*      at EOF.
        /*      
        /*----------------------------------------------------------*/

char *
set_up_entry(start, endp)
char *start;
char *endp;
{
        register char *next;                    /* next byte to process */
	register char *cp;
	int local_parsefail = FALSE;

	char holdc;

        char *parse_a_field();
       /*
        * In this version there are no major or minor events,
        * so we just clean up and set up as major
        */

        clean_up(MAJOR);
        context = MAJOR;

       /*
        * Start with the next unprocessed byte
        */
        next = start;

       /*
        * Loop until we're looking at a BeginEvent
        * entry.  
        */
        while (*(next+1) != 'B') {
		for (cp = next; cp < endp && *cp != '\n'; cp++)
		  ;
		if (cp < endp) {
			holdc = *cp;
			*cp = '\0';
			line_tag();
			fprintf(stderr, "Skipping field: %s\n", next);
			next = find_field(next, endp);
			parse_fail = TRUE;
			*cp = holdc;			
			return next;
		}
		next = parse_a_field(next, endp);
		if (parse_fail)
			return next;
	}

       /*
	* Get rid of partial parses from scan for B
        */

        clean_up(MAJOR);
        context = MAJOR;

        if (next == NULL)
                return NULL;

       /*
        * Go through parsing fields until an entire entry is done
        */
        do {
                next = parse_a_field(next, endp);
		local_parsefail |= parse_fail;
        } while (next != NULL && (*(next+1)!= 'B' ));

	parse_fail = local_parsefail;

        return next;
}


        /*----------------------------------------------------------*/
        /*      
        /*                 clean_up
        /*      
        /*      This routine is called to clear out fields
        /*      which should not be inherited from previous
        /*      parsing.  Each field is marked with a context
        /*      flag which indicates whether it was parsed
        /*      as part of a major or a minor entry.  
        /*      
        /*----------------------------------------------------------*/

int
clean_up(Context)
{
        register int i;

       /*
        * check each one and clean up if necessary
        */
        for(i=0; i<NTYPES; i++) {
                if (fields[i].context == Context)
                        fields[i].data[0] = '\0'; /* null the string */
        }

}

	/*----------------------------------------------------------*/
	/*	
	/*			snarf_comments
	/*	
	/*	Given a pointer to a # at the beginning of a line,
	/*	run it up past the last such line in a row and return
	/*	the result.  Never go past end, however.
	/*	
	/*----------------------------------------------------------*/

char *
snarf_comments(cp, endp)
char *cp;
char *endp;
{
	register char *rcp = cp;		/* register copy of next */
						/* char pointer*/
	register char *ep = endp;		/* register copy of end */
						/* pointer*/

       /*
        * Skip until eof or to first char of next non-comment line
        */
	rcp++;
 	do {					/* rcp past COMMENT_CHAR */
		while (*rcp++ != '\n' && rcp < ep)
		  ;				/* skip past newline */
		line_num++;
	} while (rcp < ep && *rcp++ == COMMENT_CHAR);

	if (rcp<=ep)
		rcp--;

	return rcp;
}


        /*----------------------------------------------------------*/
        /*      
        /*                      parse_a_field
        /*      
        /*      Given a pointer to a MAGIC_CHAR in a buffer, parse the
        /*      corresponding field into its appropriate global
        /*      field variable.
        /*      
        /*----------------------------------------------------------*/

char *
parse_a_field(start, endp)
char *start;
char *endp;
{
        register char *ep = endp;               /* fast copy */
        register char *cp;
        register char *target;

        int type;

       /*
        * Check for quick return
        */

        if (start == NULL || ep - start < 2)
                return NULL;

       /*
        * Find the field name
        */
        for (cp = start+1; cp<endp && *cp != FIELD_END_CHAR; cp++) 
          ;
        if (cp >=endp) {
		line_tag();
                fprintf(stderr,"EOF scanning for '%c'\n",
			FIELD_END_CHAR);
		parse_fail = TRUE;
		return NULL;
        }
       /*
        * Mark it as a string and look it up in the table
        */

        *cp = '\0';                             /* so string routines */
                                                /* will believe it*/

        for(type=0; type<NTYPES; type++) {
                if (strcmp(start+1, fields[type].type) == 0)
                        break;
        }

        if (type>=NTYPES) {
		line_tag();
                fprintf(stderr," unknown field type %c%s%c\n", MAGIC_CHAR, start+1, FIELD_END_CHAR);
		parse_fail = TRUE;
		return (find_field(cp+1, endp));
        }

       /*
        * Indicate that we really did find a new field
        */
	parsed_entry = TRUE;

       /*
        * Skip any leading blanks
        */
	while  (cp<endp && *cp==' ')
		cp++;
       /*
        * See whether we are going to tack this on the end
        * of an existing entry
        */
        target = fields[type].data;
	if (*target != '\0' && *fields[type].append!='\0') {
               /*
                * We are going to put the append string in 
                * the middle and then tack ours on the end
                */
		(void) strcat(target, fields[type].append);
		target += strlen(target);
	}
       /*
        * Warn if we're replacing a field, but not if we're overriding
        * an inheritance from an outer scope
        */
	if (*target !='\0' &&fields[type].context>=context  ) {
		line_tag();
		fprintf(stderr,"unexpected duplicate field of type '%s'.\n",
			fields[type].type);
		parse_fail = TRUE;
		return(find_field(cp, endp));
	}
       /*
        * type is now the index into the field table for the correct type
        * field entry.  Mark the context in which we're parsing this,
        * so cleanup will know when to get rid of it later.
        */

        fields[type].context = context;

       /*
        * Append the data to the local buffer
        */
        for (cp = cp+1; cp<endp;) {
                *target = *cp++;
                if (*target == '\n') {
			line_num++;
                        *target++ = ' ';
                        while (cp < endp && *cp=='\n') {
                                cp++;           /* skip \n\n  */
				line_num++;
			}
                       /*
                        * eat up all comment lines in the input
                        */
                        if (cp < endp && *cp == COMMENT_CHAR)
                                cp = snarf_comments(cp, endp);
                       /*
                        * See if we're up to a new field.  This MUST come
                        * after the comment snarfing.
                        */
                        if (cp < endp && *cp == MAGIC_CHAR)
                                break;
                } else {
                        if (*target == '\t')
                                *target = ' ';
                        target++;
                }
        }

        *target = '\0';

        if (cp < endp)
                return cp;
        else
                return NULL;
}


	/*----------------------------------------------------------*/
	/*	
	/*			move_if_needed
	/*	
	/*	given a source pointer, target pointer, and count,
	/*	do a counted move, but only if the move would
	/*	do some real work.
	/*	
	/*----------------------------------------------------------*/


int
move_if_needed(target, source, len)
char *target;
char *source;
int len;
{
	if (target != source && len >0) 
		(void) strncpy(target, source, len);
}

	/*----------------------------------------------------------*/
	/*	
	/*	`	   compress_blanks
	/*	
	/*	Given a pointer to a string, remove leading blanks
	/*	and turn all multiple blanks to singles.
	/*	
	/*----------------------------------------------------------*/

int
compress_blanks(stp)
char *stp;
{
	register char *cp=stp;			/* next byte to scan */
	register char *target=stp;		/* place to put next */
						/* move  */
	char *source=stp;			/* next move starts here */
	register int count=0;			/* number of bytes in */
						/* next move */
	int blanks=TRUE;			/* true iff this is */
						/* a blank field */

	while (*cp != '\0') {
		if (*cp++ == ' ') {
			if(blanks) {
				move_if_needed(target, source,count);
				target = target + count;
				while (*cp == ' ')
					cp++;
				source  = cp;
				count = 0;
				if (*cp == '\0')
					break;	/* blanks at end */
				blanks  = FALSE;
				continue;
			} else {
				blanks= TRUE;
			}
		} else
			blanks = FALSE;
		count++;
	}

	move_if_needed(target, source, count+1); /* +1 gets the null */
	  
	
}
	/*----------------------------------------------------------*/
	/*	
	/*	`	   stript_font_controls
	/*	
	/*	Replace every occurrence of .it, .st, .bo  with blanks.  
	/*	Note that this may lead to lots of gas in the string.
	/*	
	/*----------------------------------------------------------*/

int
strip_font_controls(stp)
char *stp;
{
	register char *cp=stp;			/* next byte to scan */

	while (*cp != '\0') {
		if (*cp++ != '.')
			continue;
		cp--;
               /*
                * We found a '.', 
                */
		if (strlen(cp) < 3)
			return;
		if (strncmp(cp, ".it", 3) == 0 ||
		    strncmp(cp, ".bo", 3) == 0 ||
		    strncmp(cp, ".st", 3) == 0) {
			    (void) strncpy(cp, "   ", 3);
			    cp +=3;
		    }
		else
			cp++;
	}
}

	/*----------------------------------------------------------*/
	/*	
	/*			format_string
	/*	
	/*	Given a  single contiguous string with
	/*	blanks and \n characters.  Insert a \n at every
	/*	point where the line would be too long for the screen,
	/*	always doing this between words.  
	/*	
	/*	The first line can be forced artificially to be
	/*	shorter than the others.
	/*	
	/*	Note that the string may be shortened by the blank
	/*	removal done at the beginning of lines, so watch
	/*	out for dynamically allocated memory.
	/*	
	/*	If the last parm is a non-null char, then it is
	/*	added to the beginning of each succeeding line, 
	/*	possibly extending the string.  When this feature
	/*	is used, the total amount of data must be less than
	/*	4k.  Usual use is to add a tab or space for indentation.
	/*	
	/*----------------------------------------------------------*/

int
format_string(sp, first_len, other_len, tab)
char *sp;
int  first_len;					/* length of 1st line */
int  other_len;					/* length of all others */
char tab;
{
	register char *blank = NULL;		/* ptr to previous blank */
	register char *next = sp;		/* next char to check */
	register int len = 0;			/* length of the current */
						/* line so far */
	int target = first_len;			/* length of line we're */
						/* trying to build */
	int start_sentence = FALSE;		/* Next non-blank wants */
						/* to be uppercased. */
	int flush_blanks = FALSE;		/* true if we're getting */
						/* rid of blanks.  Generally */
						/* at the start of a line */
						/* that we split */
	char local_buffer[4096];

       /*
        * Flush leading blanks
        */
	while (*next==' ')
		next++;
       /*
        * Return if string is null
        */
	if (*next == '\0')
		return;



/**	UPCASE(*next);				/* first char is upper */

       /*
        * Loop through the whole input string
        */
	while (*(++next) != '\0') {
		len++;
		if (*next == ' ') {
			blank = next; 
			if (flush_blanks) {
				(void) strcpy(next, next+1);
				next--;		/* because loop increments */
						/* it.  UGH */
				continue;
			}
		}
		else {
			flush_blanks = FALSE;
			if (*next == '\n') {
				len = 0;
				continue;
			} else
				if (start_sentence) {
					/** UPCASE(*next); **/ /* we used to*/
						/* try and uppercase the */
						/* start of each sentence. */
					start_sentence = FALSE;
				}
		}
		if (*next == '.') {
			start_sentence = TRUE;
		}
               /*
                * See if we have to try and split line.  If we fail, then
                * just keep going.  Sooner or later we'll hit a blank
                * and split there, which is the best we can do.
                */
		if (len > target) {
			if (blank != NULL) {	/* is there a place for \n */
				*blank = '\n';
				len = next - blank;
				flush_blanks = FALSE;
                               /*
                                * if we're inserting tabs at the beginning
                                * of subsequent lines, then do it.
                                */
				if (tab!='\0') {
					(void) strcpy(local_buffer, blank+1);
					*(blank+1) = tab;
					(void) strcpy(blank+2, local_buffer);
					len++;
					next++;
				}
				blank = NULL;
			} 
			target = other_len;	/* we're not on first */
						/* line anymore */
		}
		
	}
}

	/*----------------------------------------------------------*/
	/*	
	/*			file_init
	/*	
	/*	Gives us a chance to set up before each file is
	/*	processed.
	/*	
	/*----------------------------------------------------------*/

int
file_init(Fname)
char *Fname;
{
	fprintf(stderr, "File: %s\n", Fname);
}

	/*----------------------------------------------------------*/
	/*	
	/*			file_term
	/*	
	/*	Gives us a chance to clean up after each file is
	/*	processed.
	/*	
	/*----------------------------------------------------------*/

int
file_term(Fname)
char *Fname;
{
}

	/*----------------------------------------------------------*/
	/*	
	/*			field_size_check
	/*	
	/*	Checks whether the information supplied for a
	/*	given field is being truncated, and if so, gives
	/*	an error message.
	/*	
	/*----------------------------------------------------------*/

int
field_size_check(str, size, fld_name)
char *str;					/* data to be checked */
int  size;					/* expected max size */
char *fld_name;					/* string name of the field */
{
       /*
        * Check the field size, return if OK
        */
	if (strlen(str)<=size)
		return;
       /*
        * Generate a parse failure and an error message if too long.
        * Also, truncate the field to the expected max size.
        */
	line_tag();
	fprintf(stderr, "maximum size of the %s field is %d characters. Please shorten it.\n", fld_name, size);
	parse_fail = TRUE;
	str[size] = '\0';
}
	/*----------------------------------------------------------*/
	/*	
	/*			null_field_check
	/*	
	/*	Checks whether the information supplied for a
	/*	given field is missing, and if so, gives
	/*	an error message.
	/*	
	/*----------------------------------------------------------*/

int
null_field_check(str, fld_name)
char *str;					/* data to be checked */
char *fld_name;					/* string name of the field */
{
	register char *cp = str;

       /*
        * Look for non-blank character
        */

	while (*cp==' ')
		cp++;
	if (*cp != '\0')
		return;
       /*
        * Generate a parse failure and an error message.
        */
	line_tag();
	fprintf(stderr, "ERROR: You must supply a %s field for each event.\n", fld_name);
	parse_fail = TRUE;
}


	/*----------------------------------------------------------*/
	/*	
	/*			make_title
	/*	
	/*	Put together the title field for the Ingres database.
	/*	
	/*----------------------------------------------------------*/

int
make_title()
{
	field_size_check(fields[TITLE].data, TITLE_SIZE, "$Title");
	null_field_check(fields[TITLE].data, "$Title");
	(void) strcpy(title, fields[TITLE].data);
}
	/*----------------------------------------------------------*/
	/*	
	/*			make_place
	/*	
	/*	Put together the place field for the Ingres database.
	/*	
	/*----------------------------------------------------------*/

int
make_place()
{
	field_size_check(fields[PLACE].data, PLACE_SIZE, "$Place");
	null_field_check(fields[PLACE].data, "$Place");
	(void) strcpy(place, fields[PLACE].data);
}

	/*----------------------------------------------------------*/
	/*	
	/*			make_type
	/*	
	/*	Put together the type field for the Ingres database.
	/*	
	/*----------------------------------------------------------*/

int
make_type()
{
	field_size_check(fields[TYPE].data, TYPE_SIZE, "$Type");
	null_field_check(fields[TYPE].data, "$Type");
	(void) strcpy(event_type, fields[TYPE].data);
}

	/*----------------------------------------------------------*/
	/*	
	/*			add_at_end
	/*	
	/*	Given a target string and a source field, adds the
	/*	supplied field, with description to the end of the 
	/*	source field.
	/*	
	/*----------------------------------------------------------*/

int
add_at_end(target, field_ptr)
char *target;
struct fields *field_ptr;
{
	register char *targ = target;

       /*
        * Just return if the field to be added is blank
        */
	if (*field_ptr->data == '\0')
		return;
       /*
        * If necessary, skip the existing stuff, and put a blank at the
        * end if needed.
        */
	if (*targ != '\0') {
		targ += strlen(targ);	/* point to NULL at end*/
		if (*(targ-1) != ' ') {
			*targ++ = ' ';
		}
	}

       /*
        * targ now points to place where the new stuff goes
        */

	(void) sprintf(targ, "%s: %s", field_ptr->type, field_ptr->data);

       /*
        * Make any trailing punctuation into a period and a blank
        */

	targ += strlen(targ);

	while (*(--targ) == ' ' || ispunct(*targ))
	  ;					/* skip back over blanks */
						/* and punctuation*/

	*(++targ) = '.';
	*(++targ) = ' ';
	*(++targ) = '\0';

}


	/*----------------------------------------------------------*/
	/*	
	/*			make_comments
	/*	
	/*	Put together the comment field for the Ingres database.
	/*	
	/*----------------------------------------------------------*/

int
make_comments()
{
	field_size_check(fields[COMMENTS].data, COMMENTS_SIZE, "$Comments");
	(void) strcpy(comment, fields[COMMENTS].data);
}

	/*----------------------------------------------------------*/
	/*	
	/*			Error
	/*	
	/*	For use by the parse_time routine, because whatsup
	/*	does its error reporting in this manner.
	/*	
	/*----------------------------------------------------------*/

int
Error(str)
char *str;
{
	fprintf(stderr, "\n\n%s\n", str);
}
/*
 *			parse_time
 *
 *	For consistency, this routine is a modified version of the
 * 	time parser from whatsup.  This means that times will be
 *      accepted in a strictly compatible manner, but it does make
 *      some of the code a bit messy.  In postit, this routine
 *      is called from the similarly named parse_a_time.
 */
struct t_struct {
	int dummy;
};						/* dummy for parm in */
						/* following */

int
parse_time(typed, n, colon, tp, hour, minute, ok)
char typed[];				/* chars as typed */
int n;					/* number of chars to parse */
int colon;				/* offset to s colon, if any */
struct t_struct *tp;			/* previous values of this field,
					   used for defaulting */
int *hour, *minute;			/* returned parse */
int *ok;				/* returned true iff legal */
{
	int h, m;                       /* working values of hour, min */
	register int i = 0;		/* next char to parse */

	/*
	 * If typed field is empty (probably because someone BS'd over it),
	 * just return
	 */
	if (n == 0) {
		*ok = FALSE;
		return;
	}
	/*
	 * Set up defaults in working values
	 */
	h = 0;
	m = 0;
	
	/*
	 * Parse the supplied numbers - first, the case where an hour
	 * is specified.  Only exception is a leading colon.
	 */
	if (colon != 0) {       	/* there's an : */
		m = 0;			/* min defaults to 0 when hour
					   specified */
		h = typed[i++]-'0';
		if (i==n)
		        goto hour_parse_done;
		if (typed[i] == 'P') {
			h += 12;
			i++;
			goto hour_parse_done;
		}
		if (typed[i] == 'A') {
			i++;
			goto hour_parse_done;
		}
		if (typed[i] != ':') {
			h = h*10 + typed[i++]-'0';
			if (i<n && typed[i] == 'M') {
				if (h==12 && m==0 && colon ==(-1)) {
					h=0;
					i++;
					goto all_done;
				} else {
					Error("Correct form is 12M (or just 0) for midnight.");
					*ok = FALSE;
					return;
				}
			}
			if (i<n && typed[i] == 'N') {
				if (h==12 && m==0 && colon == (-1)) {
					h=12;
					i++;
					goto all_done;
				} else {
					Error("Use either 12, 12:00, 12N, or 12:00N for noon.");
					*ok = FALSE;
					return;
				}
			}
			if (i<n &&typed[i] == 'P') {
				if (h<12)
					h += 12;
				i++;
				goto hour_parse_done;
			}
			if (i<n && typed[i] == 'A') {
				if (h==12)
				        h=0;
				i++;
				goto hour_parse_done;
			}
			if (i<n && typed[i] != ':') {
				Error("ERROR: a colon must separate hours from minutes.");
				*ok = FALSE;
				return;
			} else
				i++;    /* skip possible colon in position 3 */
		} else
			i++;		/* skip the colon in position 2 */
	/*
	 * A leading colon just causes us to parse the rest as
 	 * a minute.  Skip it.
	 */
	} else 
		i++;			/* skip colon in position 1 */
	/*
	 * Parse the minutes
	 */
hour_parse_done:
	if (i<n)
		m = 0;
	while (i<n) {
		if (typed[i] == 'P') {
			if (h<12)
				h += 12;
			break;
		}
		if (typed[i] == 'A') {
			if (h==12)
				h = 0;
			break;
		}
		if (typed[i] == 'M') {
			if (h==12 && m==0) {
				h=0;
				i++;
				goto all_done;
			} else {
				Error("Correct form is 12M (or just 0) for midnight.");
				*ok = FALSE;
				return;
			}
		}
		if (typed[i] == 'N') {
			if (h==12 && m==0) {
				i++;
				goto all_done;
			} else {
				Error("Correct form is  12, 12:00, 12N, or 12:00N for noon.");
				*ok = FALSE;
				return;
			}
		}
		m = m*10 + typed[i++] - '0';
	}
		
	/*
	 * Check the values
	 */
	if (h>23) {
		Error("ERROR: Hour must be between 0 and 23.");
		*ok = FALSE;
		return;
	}
	if (m>59) {
		Error("ERROR: Minute must be between 0 and 59.");
		*ok = FALSE;
		return;
	}
all_done:

	/*
	 * Looks ok, update the values and return
	 */
	*hour = h;
	*minute = m;
	*ok = TRUE;
}

	/*----------------------------------------------------------*/
	/*	
	/*			parse a time
	/*	
	/*	Given a pointer to a place in a string in which
	/*	a time is stored in one of the following forms:
	/*	
	/*		mm/dd/yy ,  hh:mm
	/*	
	/*	where dd is a day hh is the hour in 24 hour form and
	/*	mm is the optional minute, fill in the next entry in
	/*	the date and time array, incrementing ntimes
	/*	appropriatly, Returns a pointer past the data parsed,
	/*	or NULL if there was nothing more to parse.  Commas
	/*	prior to an entry are ignored.
	/*		
	/*	
	/*----------------------------------------------------------*/

char *
parse_a_time(datap)
char *datap;
{
	register char *cp = datap;		/* next byte to parse */
	register char *fp;			/* a scanning pointer */
						/* for looking for periods*/
	int colon = FALSE;			/*  true iff second form*/

	int day, hours, mins;	

	int ok;					/* TRUE iff time spec */
						/* looked OK */
	
	char *timep;				/* pointer to the start */
						/* of the time spec */
	char *workp;
	char holdc;

	int year, month;
       /*
        * Skip leading blanks and commas
        */
	while (*cp == ' ' || *cp == ',')
		cp++;

	if (*cp == '\0')
		return NULL;
       /*
        * find the second part of the number and see whether it 
        * contains a .  Keep a watch out for premature '\0' which
        * would indicate a poorly formed time field
        */

	fp = cp;

       /*
        * Scan past the day specification
        */
	while (*fp != ' ' && *fp != ',') {
		if (*fp == '\0') {
			if (mode!=POSTIT)
				line_tag();
			fprintf(stderr, "\nYou have supplied a date but no time.\nCorrect form is $When: MM/DD/YY HH:MM\n");
			ok = FALSE;
			goto bad;
		}
		if (*fp != '/') {
			if(!isdigit(*fp)) {
				if (mode != POSTIT)
					line_tag();
				fprintf(stderr, "\nImproper date specification.\nCorrect form is $When: MM/DD/YY HH:MM\n");
				ok = FALSE;
				goto bad;
			}
		} else 
			if (!isdigit(*(++fp))) {
				if (mode != POSTIT)
					line_tag();
				fprintf(stderr, "\nImproper date specification.\nCorrect form is $When: MM/DD/YY HH:MM\n");
				ok = FALSE;
				goto bad;
			}
		fp++;
	}

       /*
        * Scan past the white space separating the fields
        */
	while (*fp == ' ') {
		fp++;
	}

       /*
        * Make sure there is a second field
        */
	if (*fp == '\0') {
		if (mode != POSTIT)
			line_tag();
		fprintf(stderr, "\nYou have supplied a date but no time.\nCorrect form is $When: MM/DD/YY HH:MM\n");
		ok = FALSE;
		goto bad;
	}		

	timep= fp;				/* note where the time */
						/* info starts */

	while (*fp != ' ' && *fp != ',') {
		if (*fp == '\0') 
			break;

		if (*fp == ':') {
			colon = TRUE;
			if (!isdigit(*(fp+1))) {
				if (mode != POSTIT)
					line_tag();
				fprintf(stderr, "\nPoorly formed time specification (minutes are missing)\nCorrect form is $When: MM/DD/YY HH:MM\n");
				ok = FALSE;
				fp += 2;
				goto bad;
			}
		} 
		fp++;
	}

       /*
        * the colon switch is set and fp now points past the end of
        * the number
        */

	if (!colon) {
		if (mode != POSTIT)
			line_tag();
		fprintf(stderr, "\nPoorly formed time specification (missing colon)\nCorrect form is $When: MM/DD/YY HH:MM\n");
		ok = FALSE;
		goto bad;
	} else {

		if (sscanf(cp, "%d/%d/%d", &month, &day, &year) != 3) {
			if (mode != POSTIT)
				line_tag();
			fprintf(stderr,"\nInvalid date specification:  correct form is $When: MM/DD/YY HH:MM\n");
			ok = FALSE;
			goto bad;
		} else 
			date[ntimes]=10000*year+100*month+day;

		
		/*
		 * Make sure the am and pm specs are uppercased
		 */
		for (workp=timep; *workp!= '\0' && *workp!=' ' && *workp!=',';
		     workp++)
			if (isalpha(*workp) && islower(*workp))

				*workp = toupper(*workp);

		parse_time(timep, fp-timep, TRUE, (struct t_struct *)NULL, 
			   &hours, &mins, &ok);

               /*
                * Make sure date is in proper range, unless we're in the update
                * phase of a bulkpost.  In that case, the check was already
                * done during the checking phase.
                */
		if (mode != BULK_UPDATING && !check_date(year, month, day)) 
			ok = FALSE;
	}

       /*
        * See if there was an error parsing the time
        */
      bad:
	if (ok) {
		Times[ntimes]=hours*100 + mins;
		ntimes++;
	} else {
		parse_fail = TRUE;		/* set global failure ind. */
		holdc = *fp;
		*fp = '\0';			/* put in a null so only */
						/* the field we've parsed */
						/* will print*/
		fprintf(stderr, "Date and time field causing the error is: %s\n", 
			cp);
		*fp = holdc;			/* Restore for rest of */
						/* parse */
		fprintf(stderr, "In item with title: %s\n\n", fields[TITLE].data);

	}

	return fp;
	
}

	/*----------------------------------------------------------*/
	/*	
	/*			make_times
	/*	
	/*	Put together the date and time arrays
	/*	
	/*----------------------------------------------------------*/

int
make_times()
{
	register char *timetable = fields[WHEN].data;
	ntimes = 0;

	null_field_check(fields[WHEN].data, "$When");

	while (timetable != NULL && *timetable != '\0'  && !parse_fail)
		timetable = parse_a_time(timetable);
	
}
	/*----------------------------------------------------------*/
	/*	
	/*			new_entry
	/*	
	/*	Set up for a new entry.
	/*	
	/*----------------------------------------------------------*/

int
new_entry()
{
	ntimes = 0;
	*event_type = '\0';
	*title = '\0';
	*place = '\0';
	*comment = '\0';
}

	/*----------------------------------------------------------*/
	/*	
	/*			escape_quotes
	/*	
	/*	Put a \ before every " in the supplied string
	/*	
	/*----------------------------------------------------------*/

int
escape_quotes(sp)
char *sp;
{
	register char *next=sp;
	char buffer[3000];

	while (*next!='\0') {
		if (*next=='"') {
			(void) strcpy(buffer, next);
			*next = '\\';
			(void) strcpy(++next, buffer);
		}
		next++;
	}
}


