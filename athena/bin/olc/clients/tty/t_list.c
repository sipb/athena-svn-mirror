/*
 * This file is part of the OLC On-Line Consulting System.
 * It contains procedures for exectuting olc commands.
 *
 *      Win Treese
 *      Dan Morgan
 *      Bill Saphir
 *      MIT Project Athena
 *
 *      Ken Raeburn
 *      MIT Information Systems
 *
 *      Tom Coppeto
 *	Chris VanHaren
 *	Lucien Van Elsen
 *      MIT Project Athena
 *
 * Copyright (C) 1989,1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: t_list.c,v 1.24 1999-06-28 22:52:17 ghudson Exp $
 */

#ifndef lint
#ifndef SABER
static char rcsid[] ="$Id: t_list.c,v 1.24 1999-06-28 22:52:17 ghudson Exp $";
#endif
#endif

#include <mit-copyright.h>
#include "config.h"

#include <olc/olc.h>
#include <olc/olc_tty.h>
#include <time.h>

static const char *const default_sort_order[] = {
    "foo",
    "unconnected_consultants_last",
    "q_status",
    "time",
    0,
};

ERRCODE
t_list_queue(Request,sort,queues,topics,users,stati,comments,file,display)
    REQUEST *Request;
    char **sort;
    char *queues;
    char *topics;
    char *users;
    int stati;
    int comments;
    char *file;
    int display;
{
    ERRCODE status;
    LIST *list;

    status = OListQueue(Request,&list,queues,topics,users,stati);
    switch (status) {
    case SUCCESS:
	OSortListByRule(list, default_sort_order);
	t_display_list(list,comments,file);
	if(display == TRUE)
	    display_file(file);
	else
	    cat_file(file);
	free(list);
	break;

    case ERROR:
	fprintf(stderr, "Error listing conversations.\n");
	break;

    case EMPTY_LIST:
	if (Request->options)
	    printf ("No questions match given status.\n");
	else if (topics[0] != '\0')
	    printf ("No questions match requested topic.\n");
	else if (stati != 0)
	  printf ("No questions match requested status.\n");
	else
	    printf ("The queue is empty.\n");
	status = SUCCESS;
	break;

    default:
	status = handle_response(status, Request);
	break;
    }

    return(status);
}

static char old_status[30] = "";

void output_status_header (file, status)
    FILE *file;
    const char *status;
{
    if (status == 0) {
	old_status[0] = '\0';
	return;
    }
    if (! strcmp (status, "pending")
	|| ! strcmp (status, "unseen")
	|| ! strcmp (status, "done")
	|| ! strcmp (status, "cancel"))
	status = "active";
    if (! strncmp (old_status, status, sizeof (old_status)))
	return;
    if (old_status[0])
	fprintf (file, "\n");	/* extra newline separating */
    fprintf (file, "[%s]\n", status);
    strncpy (old_status, status, sizeof (old_status));
}

static void trim_hostname (name, suffix)
    char *name;
    const char *suffix;
{
    char *p;
    p = strchr (name, '.');
    if (p)
	if (!strcmp (p+1, suffix))
	    *p = 0;
    if (strlen (name) > 20)	/* must match field size below! */
	strcpy (name + 19, "*");
}

static const char listing_format[] =
    "%-20.20s%-4.4s %-7.7s  %-8.8s%-4.4s %-3.3s  %2.2s %-13.13s %s";
/*   usr@mach[nn]   status__cons  [nn]   cstatus__nseen topic time*/
static const char time_format[] =
    "%02d/%02d %02d%02d";

ERRCODE
t_display_list(list,comments,file)
    LIST *list;
    int comments;
    char *file;
{
    LIST *l;
    char uinstbuf[10];
    char cinstbuf[10];
    char ustatusbuf[32];
    char chstatusbuf[10];
    char nseenbuf[5];
    char cbuf[32];
    char ubuf[128];
    char buf[32];
    char time_buf[32];
    struct tm time_s;
    FILE *fp;

    if(list->ustatus ==  END_OF_LIST) {
	printf("Empty list\n");
	return(SUCCESS);
    }

    fp = fopen(file,"w");
    if(fp == NULL) {
	fprintf(stderr,"t_display_list: unable to open temp. file %s.\n",file);
	return(ERROR);
    }

    fprintf (fp,
  "User                     Status   Consultant   Stat ## Topic         Time\n"
	     );
    output_status_header (fp, (const char *)NULL);
    for(l=list; l->ustatus != END_OF_LIST; ++l) {
	buf[0] = '\0';
	cbuf[0]= '\0';
	time_s = * localtime (&l->utime);
	sprintf (time_buf, time_format,
		 time_s.tm_mon+1, time_s.tm_mday,
		 time_s.tm_hour, time_s.tm_min);
	if((l->nseen >=0) && (l->connected.uid >= 0)) {
	    /* currently-connected user/consultant pair */
	    OGetStatusString(l->ukstatus,cbuf);
	    output_status_header (fp, cbuf);
	    sprintf(ustatusbuf,"%s",
		    (l->ustatus != LOGGED_OUT) ? cbuf : "logout");

	    if (l->ckstatus == OFF)
		chstatusbuf[0] = 0;
	    else {
		OGetStatusString(l->ckstatus,buf);
		sprintf(chstatusbuf,"%-3.3s",buf);
	    }
	    sprintf(uinstbuf,"[%d]",l->user.instance);
	    sprintf(cinstbuf,"[%d]",l->connected.instance);
	    sprintf(nseenbuf,"%d",l->nseen);
	    sprintf(cbuf,"%s",l->connected.username);
	    sprintf(ubuf,"%s@%s",l->user.username, l->user.machine);
	    trim_hostname (ubuf, "MIT.EDU");
	    fprintf(fp, listing_format,
		    ubuf, uinstbuf, ustatusbuf, l->connected.username,
		    cinstbuf,chstatusbuf,nseenbuf,l->topic, time_buf);
	    if (comments && (l->note[0] != '\0'))
		fprintf(fp,"\t\t[%-63.63s]",l->note);
	}
	else if ((l->nseen >= 0) && (l->connected.uid <0)) {
	    /* unconnected user */
	    OGetStatusString(l->ukstatus,cbuf);
	    output_status_header (fp, cbuf);
	    sprintf(ustatusbuf,"%s",
		    (l->ustatus != LOGGED_OUT) ? cbuf : "logout");
	    sprintf(uinstbuf,"[%d]",l->user.instance);
	    sprintf(nseenbuf,"%d",l->nseen);
	    sprintf(ubuf,"%s@%s",l->user.username, l->user.machine);
	    trim_hostname (ubuf, "MIT.EDU");
	    fprintf (fp, listing_format,
		     ubuf, uinstbuf, ustatusbuf, "", "", "",
		     nseenbuf, l->topic, time_buf);
	    if(comments && (l->note[0] != '\0'))
		fprintf(fp,"\t\t[%-63.63s]",l->note);
	}
	else if((l->nseen < 0) && (l->connected.uid < 0)) {
	    /* unconnected consultant */
	    OGetStatusString(l->ukstatus,buf);
	    sprintf(chstatusbuf,"%-3.3s",buf);
	    sprintf(cbuf,"%s",l->user.username);
	    sprintf(cinstbuf,"[%d]",l->user.instance);
	    fprintf (fp, listing_format,
		     "", "", "", cbuf, cinstbuf, chstatusbuf, "", "", "");
	}
	else {
	    fprintf(fp,"**unknown list entry***");
	}
	fprintf(fp,"\n");
    }

    for(l=list; l->ustatus != END_OF_LIST; ++l)
	if((l->nseen < 0) && (l->connected.uid < 0))
	{
	}
    fclose(fp);
    return(SUCCESS);
}