#if !defined(lint) && !defined(DOS)
static char rcsid[] = "$Id: imap.c,v 1.1.1.1 2001-02-19 07:11:40 ghudson Exp $";
#endif
/*----------------------------------------------------------------------

            T H E    P I N E    M A I L   S Y S T E M

   Laurence Lundblade and Mike Seibel
   Networks and Distributed Computing
   Computing and Communications
   University of Washington
   Administration Builiding, AG-44
   Seattle, Washington, 98195, USA
   Internet: lgl@CAC.Washington.EDU
             mikes@CAC.Washington.EDU

   Please address all bugs and comments to "pine-bugs@cac.washington.edu"


   Pine and Pico are registered trademarks of the University of Washington.
   No commercial use of these trademarks may be made without prior written
   permission of the University of Washington.

   Pine, Pico, and Pilot software and its included text are Copyright
   1989-2001 by the University of Washington.

   The full text of our legal notices is contained in the file called
   CPYRIGHT, included with this distribution.


   USENET News reading additions in part by L Lundblade / NorthWestNet, 1993
   lgl@nwnet.net

   Pine is in part based on The Elm Mail System:
    ***********************************************************************
    *  The Elm Mail System  -  Revision: 2.13                             *
    *                                                                     *
    * 			Copyright (c) 1986, 1987 Dave Taylor              *
    * 			Copyright (c) 1988, 1989 USENET Community Trust   *
    ***********************************************************************
 

  ----------------------------------------------------------------------*/

/*======================================================================
    imap.c
    The call back routines for the c-client/imap
       - handles error messages and other notification
       - handles prelimirary notification of new mail and expunged mail
       - prompting for imap server login and password 

 ====*/

#include "headers.h"


/*
 * struct used to keep track of password/host/user triples.
 * The problem is we only want to try user names and passwords if
 * we've already tried talking to this host before.
 * 
 */
typedef struct _mmlogin_s {
    char	   *user,
		   *passwd;
    unsigned	    altflag:1;
    STRLIST_S	   *hosts;
    struct _mmlogin_s *next;
} MMLOGIN_S;


typedef	struct _se_app_s {
    char *folder;
    long  flags;
} SE_APP_S;


/*
 * Internal prototypes
 */
void  mm_login_alt_cue PROTO((NETMBX *));
long  imap_seq_exec PROTO((MAILSTREAM *, char *,
			   long (*) PROTO((MAILSTREAM *, long, void *)),
			   void *));
long  imap_seq_exec_append PROTO((MAILSTREAM *, long, void *));
char *imap_get_user PROTO((MMLOGIN_S *, STRLIST_S *, int));
int   imap_get_passwd PROTO((MMLOGIN_S *, char *, char *, STRLIST_S *, int));
void  imap_set_passwd PROTO((MMLOGIN_S **, char *, char *, STRLIST_S *, int));
int   imap_same_host PROTO((STRLIST_S *, STRLIST_S *));
#ifdef	PASSFILE
char  xlate_in PROTO((int));
char  xlate_out PROTO((char));
char *passfile_name PROTO((char *, char *));
int   read_passfile PROTO((char *, MMLOGIN_S **));
void  write_passfile PROTO((char *, MMLOGIN_S *));
int   get_passfile_passwd PROTO((char *, char *, char *, STRLIST_S *, int));
void  set_passfile_passwd PROTO((char *, char *, char *, STRLIST_S *, int));
#endif


/*
 * Exported globals setup by searching functions to tell mm_searched
 * where to put message numbers that matched the search criteria,
 * and to allow mm_searched to return number of matches.
 */
MAILSTREAM *mm_search_stream;
long	    mm_search_count  = 0L;
MAILSTATUS  mm_status_result;

MM_LIST_S  *mm_list_info;

/*
 * Local global to hook cached list of host/user/passwd triples to.
 */
static	MMLOGIN_S	*mm_login_list = NULL;



/*----------------------------------------------------------------------
      Write imap debugging information into log file

   Args: strings -- the string for the debug file

 Result: message written to the debug log file
  ----*/
void
mm_dlog(string)
    char *string;
{
#ifdef	_WINDOWS
    mswin_imaptelemetry(string);
#endif
#ifdef	DEBUG
    dprint(0, (debugfile, "IMAP DEBUG %s: %s\n", debug_time(1), string));
#endif
}



/*----------------------------------------------------------------------
      Queue imap log message for display in the message line

   Args: string -- The message 
         errflg -- flag set to 1 if pertains to an error

 Result: Message queued for display

 The c-client/imap reports most of it's status and errors here
  ---*/
void
mm_log(string, errflg)
    char *string;
    long  errflg;
{
    char        message[sizeof(ps_global->c_client_error)];
    char       *occurance;
    int         was_capitalized;
    time_t      now;
    struct tm  *tm_now;

    now = time((time_t *)0);
    tm_now = localtime(&now);

    dprint(ps_global->debug_imap ? 0 : (errflg == ERROR ? 1 : 2),
	   (debugfile,
	    "IMAP %2.2d:%2.2d:%2.2d %d/%d mm_log %s: %s\n",
	    tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, tm_now->tm_mon+1,
	    tm_now->tm_mday,
	    (errflg == ERROR)
	      ? "ERROR"
	      : (errflg == WARN)
		  ? "warn"
		  : (errflg == PARSE)
		      ? "parse"
		      : "babble",
	    string));

    if(errflg == ERROR && !strncmp(string, "[TRYCREATE]", 11)){
	ps_global->try_to_create = 1;
	return;
    }
    else if(ps_global->try_to_create
       || (ps_global->dead_stream
	   && (!strncmp(string, "[CLOSED]", 8) || strstr(string, "No-op"))))
      /*
       * Don't display if creating new folder OR
       * warning about a dead stream ...
       */
      return;

    /*---- replace all "mailbox" with "folder" ------*/
    strncpy(message, string, sizeof(message));
    message[sizeof(message) - 1] = '\0';
    occurance = srchstr(message, "mailbox");
    while(occurance) {
	if(!*(occurance+7) || isspace((unsigned char)*(occurance+8))){
	    was_capitalized = isupper((unsigned char)*occurance);
	    rplstr(occurance, 7, (errflg == PARSE ? "address" : "folder"));
	    if(was_capitalized)
	      *occurance = (errflg == PARSE ? 'A' : 'F');
	}
	else
	  occurance += 7;

        occurance = srchstr(occurance, "mailbox");
    }

    if(errflg == ERROR)
      ps_global->mm_log_error = 1;

    if(errflg == PARSE || (errflg == ERROR && ps_global->noshow_error))
      strncpy(ps_global->c_client_error, message,
	      sizeof(ps_global->c_client_error));

    if(ps_global->noshow_error
       || (ps_global->noshow_warn && errflg == WARN)
       || !(errflg == ERROR || errflg == WARN))
      return; /* Only care about errors; don't print when asked not to */

    /*---- Display the message ------*/
    q_status_message((errflg == ERROR) ? (SM_ORDER | SM_DING) : SM_ORDER,
		     3, 5, message);
    strncpy(ps_global->last_error, message, sizeof(ps_global->last_error));
    ps_global->last_error[sizeof(ps_global->last_error) - 1] = '\0';
}



/*----------------------------------------------------------------------
         recieve notification from IMAP

  Args: stream  --  Mail stream message is relavant to 
        string  --  The message text
        errflag --  Set if it is a serious error

 Result: message displayed in status line

 The facility is for general notices, such as connection to server;
 server shutting down etc... It is used infrequently.
  ----------------------------------------------------------------------*/
void
mm_notify(stream, string, errflag)
    MAILSTREAM *stream;
    char       *string;
    long        errflag;
{
    /* be sure to log the message... */
#ifdef DEBUG
    if(ps_global->debug_imap)
      dprint(0,
	     (debugfile, "IMAP mm_notify %s : %s (%s) : %s\n",
               (!errflag) ? "NIL" : 
		 (errflag == ERROR) ? "error" :
		   (errflag == WARN) ? "warning" :
		     (errflag == BYE) ? "bye" : "unknown",
	       (stream && stream->mailbox) ? stream->mailbox : "-no folder-",
	       (stream && stream == ps_global->inbox_stream) ? "inboxstream" :
		 (stream && stream == ps_global->mail_stream) ? "mailstream" :
		   (stream) ? "not inboxstream or mailstream" : "nostream",
	       string));
#endif

    sprintf(ps_global->last_error, "%.50s : %.*s",
	    (stream && stream->mailbox) ? stream->mailbox : "-no folder-",
	    min(MAX_SCREEN_COLS, sizeof(ps_global->last_error)-70),
	    string);
    ps_global->last_error[ps_global->ttyo ? ps_global->ttyo->screen_cols
		    : sizeof(ps_global->last_error)-1] = '\0';

    /*
     * Then either set special bits in the pine struct or
     * display the message if it's tagged as an "ALERT" or
     * its errflag > NIL (i.e., WARN, or ERROR)
     */
    if(errflag == BYE){
	if(stream == ps_global->mail_stream){
	    if(ps_global->dead_stream)
	      return;
	    else
	      ps_global->dead_stream = 1;
	}
	else if(stream && stream == ps_global->inbox_stream){
	    if(ps_global->dead_inbox)
	      return;
	    else
	      ps_global->dead_inbox = 1;
	}
    }
    else if(!strncmp(string, "[TRYCREATE]", 11))
      ps_global->try_to_create = 1;
    else if(!strncmp(string, "[ALERT]", 7))
      q_status_message2(SM_MODAL, 3, 3, "Alert received while accessing \"%s\":  %s",
			(stream && stream->mailbox)
			  ? stream->mailbox : "-no folder-",
			rfc1522_decode((unsigned char *)tmp_20k_buf,
				       SIZEOF_20KBUF, string, NULL));
    else if(!strncmp(string, "[UNSEEN ", 8)){
	char *p;
	long  n = 0;

	for(p = string + 8; isdigit(*p); p++)
	  n = (n * 10) + (*p - '0');

	ps_global->first_unseen = n;
    }
    else if(!strncmp(string, "[READ-ONLY]", 11)
	    && !(stream && stream->mailbox && IS_NEWS(stream)))
      q_status_message2(SM_ORDER | SM_DING, 3, 3, "%s : %s",
			(stream && stream->mailbox)
			  ? stream->mailbox : "-no folder-",
			string + 11);
    else if(errflag && errflag != BYE)
      q_status_message(SM_ORDER | ((errflag == ERROR) ? SM_DING : 0),
		       3, 6, ps_global->last_error);
}



/*----------------------------------------------------------------------
       receive notification of new mail from imap daemon

   Args: stream -- The stream the message count report is for.
         number -- The number of messages now in folder.
 
  Result: Sets value in pine state indicating new mailbox size

     Called when the number of messages in the mailbox goes up.  This
 may also be called as a result of an expunge. It increments the
 new_mail_count based on a the difference between the current idea of
 the maximum number of messages and what mm_exists claims. The new mail
 notification is done in newmail.c

 Only worry about the cases when the number grows, as mm_expunged
 handles shrinkage...

 ----*/
void
mm_exists(stream, number)
    MAILSTREAM    *stream;
    unsigned long  number;
{
    long new_this_call, n;
    int	 exbits = 0;

#ifdef DEBUG
    if(ps_global->debug_imap > 1)
      dprint(0,
	   (debugfile, "=== mm_exists(%lu,%s) called ===\n", number,
     !stream ? "(no stream)" : !stream->mailbox ? "(null)" : stream->mailbox));
#endif

    if(stream == ps_global->mail_stream){
	if(mn_get_nmsgs(ps_global->msgmap) != (long) number){
	    ps_global->mail_box_changed = 1;
	    ps_global->mangled_header	= 1;
	}

        if(mn_get_nmsgs(ps_global->msgmap) < (long) number){
	    new_this_call = (long) number - mn_get_nmsgs(ps_global->msgmap);
	    ps_global->new_mail_count += new_this_call;
	    mn_add_raw(ps_global->msgmap, new_this_call);

	    /*
	     * Set local "recent" and "hidden" bits...
	     */
	    for(n = 0; n < new_this_call; n++, number--){
		if(msgno_exceptions(stream, number, "0", &exbits, FALSE))
		  exbits |= MSG_EX_RECENT;
		else
		  exbits = MSG_EX_RECENT;

		msgno_exceptions(stream, number, "0", &exbits, TRUE);

		/*
		 * If we're zoomed, then hide this message too since
		 * it couldn't have possibly been selected yet...
		 */
		if(any_lflagged(ps_global->msgmap, MN_HIDE))
		  set_lflag(stream, ps_global->msgmap, 
			    mn_get_total(ps_global->msgmap) - n, 
			    MN_HIDE, 1);
	    }
	}
    } else if(stream == ps_global->inbox_stream) {
	if(mn_get_nmsgs(ps_global->inbox_msgmap) != (long) number)
	  ps_global->inbox_changed = 1;

        if(mn_get_nmsgs(ps_global->inbox_msgmap) < (long) number){
	    new_this_call = (long) number
				      - mn_get_nmsgs(ps_global->inbox_msgmap);
	    ps_global->inbox_new_mail_count += new_this_call;
	    mn_add_raw(ps_global->inbox_msgmap, new_this_call);

	    /*
	     * Set local "recent" and "hidden" bits...
	     */
	    for(n = 0; n < new_this_call; n++, number--){
		if(msgno_exceptions(stream, number, "0", &exbits, FALSE))
		  exbits |= MSG_EX_RECENT;
		else
		  exbits = MSG_EX_RECENT;

		msgno_exceptions(stream, number, "0", &exbits, TRUE);

		/*
		 * If we're zoomed, then hide this message too since
		 * it couldn't have possibly been selected yet...
		 */
		if(any_lflagged(ps_global->inbox_msgmap, MN_HIDE))
		  set_lflag(stream,ps_global->inbox_msgmap,
			    mn_get_total(ps_global->inbox_msgmap) - n,
			    MN_HIDE,1);
	    }
	}
    }
    /* else ignore streams we're not directly interested in */
}



/*----------------------------------------------------------------------
    Receive notification from IMAP that a message has been expunged

   Args: stream -- The stream/folder the message is expunged from
         number -- The message number that was expunged

mm_expunged is always called on an expunge.  Simply remove all 
reference to the expunged message, shifting internal mappings as
necessary.
  ----*/
void
mm_expunged(stream, number)
    MAILSTREAM    *stream;
    unsigned long  number;
{
    MESSAGECACHE *mc;

#ifdef DEBUG
    if(ps_global->debug_imap > 1)
      dprint(0,
	   (debugfile, "mm_expunged(%s,%lu)\n",
	       stream
		? (stream->mailbox
		    ? stream->mailbox
		    : "(no stream)")
		: "(null)", number));
#endif

    /*
     * If we ever deal with more than two streams, this'll break
     */
    if(stream == ps_global->mail_stream){
	long i;

	if(i = mn_raw2m(ps_global->msgmap, (long) number)){
	    /* flush invalid cache entries */
	    while(i <= mn_get_total(ps_global->msgmap))
	      clear_index_cache_ent(i++);

	    /* expunged something we're viewing? */
	    if(!ps_global->expunge_in_progress
	       && (mn_is_cur(ps_global->msgmap,
			     mn_raw2m(ps_global->msgmap, (long) number))
		   && (ps_global->prev_screen == mail_view_screen
		       || ps_global->prev_screen == attachment_screen))){
		ps_global->next_screen = mail_index_screen;
		q_status_message(SM_ORDER | SM_DING , 3, 3,
				 "Message you were viewing is gone!");
	    }

	    ps_global->mail_box_changed = 1;
	    ps_global->mangled_header = 1;
	    ps_global->expunge_count++;
	}

	/*
	 * Keep on top of our special flag counts.
	 * 
	 * NOTE: This is allowed since mail_expunged releases
	 * data for this message after the callback.
	 */
	if(mc = mail_elt(stream, number)){
	    if(mc->spare)
	      ps_global->msgmap->flagged_hid--;

	    if(mc->spare2)
	      ps_global->msgmap->flagged_exld--;

	    if(mc->spare3)
	      ps_global->msgmap->flagged_tmp--;

	    if(mc->sparep)
	      msgno_free_exceptions((PARTEX_S **) &mc->sparep);
	}

	/*
	 * if it's in the sort array, flush it, otherwise
	 * decrement raw sequence numbers greater than "number"
	 */
	mn_flush_raw(ps_global->msgmap, (long) number);
    }
    else if(stream == ps_global->inbox_stream){
	long i;

	if(i = mn_raw2m(ps_global->inbox_msgmap, (long) number)){
	    ps_global->inbox_changed = 1;
	    ps_global->inbox_expunge_count++;
	}

	mn_flush_raw(ps_global->inbox_msgmap, (long) number);

	if(mc = mail_elt(stream, number)){
	    if(mc->spare)
	      ps_global->inbox_msgmap->flagged_hid--;

	    if(mc->spare2)
	      ps_global->inbox_msgmap->flagged_exld--;

	    if(mc->spare3)
	      ps_global->inbox_msgmap->flagged_tmp--;

	    if(mc->sparep)
	      msgno_free_exceptions((PARTEX_S **) &mc->sparep);
	}
    }
}



/*---------------------------------------------------------------------- 
        receive notification that search found something           

 Input:  mail stream and message number of located item

 Result: nothing, not used by pine
  ----*/
void
mm_searched(stream, number)
    MAILSTREAM    *stream;
    unsigned long  number;
{
    mail_elt(stream, number)->searched = 1;
    if(stream == mm_search_stream)
      mm_search_count++;
}



/*----------------------------------------------------------------------
      Get login and password from user for IMAP login
  
  Args:  mb -- The mail box property struct
         user   -- Buffer to return the user name in 
         passwd -- Buffer to return the passwd in
         trial  -- The trial number or number of attempts to login
    user is at least size NETMAXUSER
    passwd is apparently at least MAILTMPLEN, but mrc has asked us to
      use a max size of about 100 instead

 Result: username and password passed back to imap
  ----*/
void
mm_login(mb, user, pwd, trial)
    NETMBX *mb;
    char   *user;
    char   *pwd;
    long    trial;
{
    char      prompt[MAX_SCREEN_COLS], *last, *host;
    STRLIST_S hostlist[2];
    HelpType  help ;
    int       i, j, goal, ugoal, len, rc, q_line, flags;
#define NETMAXPASSWD 100

    q_line = -(ps_global->ttyo ? ps_global->ttyo->footer_rows : 3);

    if(ps_global->anonymous) {
        /*------ Anonymous login mode --------*/
        if(trial < 1) {
            strcpy(user, "anonymous");
            sprintf(pwd, "%s@%s",
		    ps_global->VAR_USER_ID ? ps_global->VAR_USER_ID : "?",
		    ps_global->hostname);
	}
	else
	  user[0] = pwd[0] = '\0';

        return;
    }

    /* make sure errors are seen */
    if(ps_global->ttyo)
      flush_status_messages(0);

    /*
     * set up host list for sybil servers...
     */
    hostlist[0].name = mb->host;
    if(mb->orighost[0] && strucmp(mb->host, mb->orighost)){
	hostlist[0].next = &hostlist[1];
	hostlist[1].name = mb->orighost;
	hostlist[1].next = NULL;
    }
    else
      hostlist[0].next = NULL;

    /*
     * Initialize user name with either 
     *     1) /user= value in the stream being logged into,
     *  or 2) the user name we're running under.
     */
    if(trial == 0L){
	strncpy(user, (*mb->user) ? mb->user :
		       ps_global->VAR_USER_ID ? ps_global->VAR_USER_ID : "",
		       NETMAXUSER);
	user[NETMAXUSER-1] = '\0';

	/* try last working password associated with this host. */
	if(imap_get_passwd(mm_login_list, pwd, user, hostlist, mb->altflag))
	  return;

#ifdef	PASSFILE
	/* check to see if there's a password left over from last session */
	if(get_passfile_passwd(ps_global->pinerc, pwd,
			       user, &hostlist[0], mb->altflag)){
	    imap_set_passwd(&mm_login_list, pwd, user,
			    &hostlist[0], mb->altflag);
	    return;
	}
#endif

	/*
	 * If no explicit user name supplied and we've not logged in
	 * with our local user name, see if we've visited this
	 * host before as someone else...
	 */
	if(!*mb->user
	   && (last = imap_get_user(mm_login_list, hostlist, mb->altflag)))
	  strncpy(user, last, NETMAXUSER);
    }

    user[NETMAXUSER-1] = '\0';

    ps_global->mangled_footer = 1;
    if(!*mb->user){
	help = NO_HELP;

	/* Dress up long hostnames */
	sprintf(prompt, "%sHOST: ",
		(!ps_global->ttyo && mb->altflag) ? "+ " : "");
	len = strlen(prompt);
	/* leave space for "HOST", "ENTER NAME", and 15 chars for input name */
	goal = (ps_global->ttyo ? ps_global->ttyo->screen_cols : 80) -
		(len + 20 +
		 min(15,
		     (ps_global->ttyo ? ps_global->ttyo->screen_cols : 80)/5));
	last = "  ENTER LOGIN NAME: ";
	if(goal < 9){
	    last = " LOGIN: ";
	    if((goal += 13) < 9){
		last += 1;
		goal = 0;
	    }
	}

	if(goal){
	    for(i = len, j = 0;
		i < sizeof(prompt) && (prompt[i] = mb->host[j]); i++, j++)
	      if(i == goal && mb->host[goal+1] && i < sizeof(prompt)){
		  strcpy(&prompt[i-3], "...");
		  break;
	      }
	}
	else
	  i = 0;

	strncpy(&prompt[i], last, sizeof(prompt)-strlen(prompt));
	prompt[sizeof(prompt)-1] = '\0';

	while(1) {
	    if(ps_global->ttyo)
	      mm_login_alt_cue(mb);

	    flags = OE_APPEND_CURRENT;
	    rc = optionally_enter(user, q_line, 0, NETMAXUSER,
				  prompt, NULL, help, &flags);
	    if(rc == 3) {
		help = help == NO_HELP ? h_oe_login : NO_HELP;
		continue;
	    }
	    if(rc != 4)
	      break;
	}

	if(rc == 1 || !user[0]) {
	    user[0]   = '\0';
	    pwd[0] = '\0';
	}
    }
    else
      strncpy(user, mb->user, NETMAXUSER);

    user[NETMAXUSER-1] = '\0';
    pwd[NETMAXPASSWD-1] = '\0';

    if(!user[0])
      return;

    help = NO_HELP;

    /* Dress up long host/user names */
    /* leave space for "HOST", "USER" "ENTER PWD", 12 for user 6 for pwd */
    sprintf(prompt, "%sHOST: ",
	    (!ps_global->ttyo && mb->altflag) ? "+ " : "");
    len = strlen(prompt);
    goal  = strlen(mb->host);
    ugoal = strlen(user);
    if((i = (ps_global->ttyo ? ps_global->ttyo->screen_cols : 80) -
		(len + 8 + 18 + 6)) < 14){
	goal = 0;		/* no host! */
	if((i = (ps_global->ttyo ? ps_global->ttyo->screen_cols : 80) -
		(6 + 18 + 6)) <= 6){
	    ugoal = 0;		/* no user! */
	    if((i = (ps_global->ttyo ? ps_global->ttyo->screen_cols : 80) -
		(18 + 6)) <= 0)
	      i = 0;
	}
	else{
	    ugoal = i;		/* whatever's left */
	    i     = 0;
	}
    }
    else
      while(goal + ugoal > i)
	if(goal > ugoal)
	  goal--;
	else
	  ugoal--;

    if(goal){
	sprintf(prompt, "%sHOST: ",
		(!ps_global->ttyo && mb->altflag) ? "+ " : "");
	for(i = len, j = 0;
	    i < sizeof(prompt) && (prompt[i] = mb->host[j]); i++, j++)
	  if(i == goal && mb->host[goal+1] && i < sizeof(prompt)){
	      strcpy(&prompt[i-3], "...");
	      break;
	  }
    }
    else
      i = 0;

    if(ugoal){
	strncpy(&prompt[i], &"  USER: "[i ? 0 : 2], sizeof(prompt)-i);
	for(i += strlen(&prompt[i]), j = 0;
	    i < sizeof(prompt) && (prompt[i] = user[j]); i++, j++)
	  if(j == ugoal && user[ugoal+1] && i < sizeof(prompt)){
	      strcpy(&prompt[i-3], "...");
	      break;
	  }
    }

    strncpy(&prompt[i], &"  ENTER PASSWORD: "[i ? 0 : 8], sizeof(prompt)-i);

    *pwd = '\0';
    while(1) {
	if(ps_global->ttyo)
	  mm_login_alt_cue(mb);

	flags = OE_PASSWD;
        rc = optionally_enter(pwd, q_line, 0, NETMAXPASSWD,
			      prompt, NULL, help, &flags);
        if(rc == 3) {
            help = help == NO_HELP ? h_oe_passwd : NO_HELP;
        }
	else if(rc == 4){
	}
	else
          break;
    }

    if(rc == 1 || !pwd[0]) {
        user[0] = pwd[0] = '\0';
        return;
    }

    /* remember the password for next time */
    imap_set_passwd(&mm_login_list, pwd, user, hostlist, mb->altflag);
#ifdef	PASSFILE
    /* if requested, remember it on disk for next session */
    set_passfile_passwd(ps_global->pinerc, pwd, user, hostlist, mb->altflag);
#endif
}



void
mm_login_alt_cue(mb)
    NETMBX *mb;
{
    if(ps_global->ttyo){
	COLOR_PAIR  *lastc;

	lastc = pico_set_colors(ps_global->VAR_TITLE_FORE_COLOR,
				ps_global->VAR_TITLE_BACK_COLOR,
				PSC_REV | PSC_RET);

	mark_titlebar_dirty();
	PutLine0(0, ps_global->ttyo->screen_cols - 1, 
		 mb->altflag ? "+" : " ");

	if(lastc){
	    (void)pico_set_colorp(lastc, PSC_NONE);
	    free_color_pair(&lastc);
	}

	fflush(stdout);
    }
}




/*----------------------------------------------------------------------
       Receive notification of an error writing to disk
      
  Args: stream  -- The stream the error occured on
        errcode -- The system error code (errno)
        serious -- Flag indicating error is serious (mail may be lost)

Result: If error is non serious, the stream is marked as having an error
        and deletes are disallowed until error clears
        If error is serious this goes modal, allowing the user to retry
        or get a shell escape to fix the condition. When the condition is
        serious it means that mail existing in the mailbox will be lost
        if Pine exits without writing, so we try to induce the user to 
        fix the error, go get someone that can fix the error, or whatever
        and don't provide an easy way out.
  ----*/
long
mm_diskerror (stream, errcode, serious)
    MAILSTREAM *stream;
    long        errcode;
    long        serious;
{
    int  i, j;
    char *p, *q, *s;
    static ESCKEY_S de_opts[] = {
	{'r', 'r', "R", "Retry"},
	{'f', 'f', "F", "FileBrowser"},
	{'s', 's', "S", "ShellPrompt"},
	{-1, 0, NULL, NULL}
    };
#define	DE_COLS		(ps_global->ttyo->screen_cols)
#define	DE_LINE		(ps_global->ttyo->screen_rows - 3)

#define	DE_FOLDER(X)	((X) ? (X)->mailbox : "<no folder>")
#define	DE_PMT	\
   "Disk error!  Choose Retry, or the File browser or Shell to clean up: "
#define	DE_STR1		"SERIOUS DISK ERROR WRITING: \"%s\""
#define	DE_STR2	\
   "The reported error number is %s.  The last reported mail error was:"
    static char *de_msg[] = {
	"Please try to correct the error preventing Pine from saving your",
	"mail folder.  For example if the disk is out of space try removing",
	"unneeded files.  You might also contact your system administrator.",
	"",
	"Both Pine's File Browser and an option to enter the system's",
	"command prompt are offered to aid in fixing the problem.  When",
	"you believe the problem is resolved, choose the \"Retry\" option.",
	"Be aware that messages may be lost or this folder left in an",
	"inaccessible condition if you exit or kill Pine before the problem",
	"is resolved.",
	NULL};
    static char *de_shell_msg[] = {
	"\n\nPlease attempt to correct the error preventing saving of the",
	"mail folder.  If you do not know how to correct the problem, contact",
	"your system administrator.  To return to Pine, type \"exit\".",
	NULL};

    dprint(0, (debugfile,
       "\n***** DISK ERROR on stream %s. Error code %ld. Error is %sserious\n",
	       DE_FOLDER(stream), errcode, serious ? "" : "not "));
    dprint(0, (debugfile, "***** message: \"%s\"\n\n", ps_global->last_error));

    if(!serious) {
        if(stream == ps_global->mail_stream) {
            ps_global->io_error_on_stream = 1;
        }

        return (1) ;
    }

    while(1){
	/* replace pine's body display with screen full of explanatory text */
	ClearLine(2);
	PutLine1(2, max((DE_COLS - sizeof(DE_STR1)
					    - strlen(DE_FOLDER(stream)))/2, 0),
		 DE_STR1, DE_FOLDER(stream));
	ClearLine(3);
	PutLine1(3, 4, DE_STR2, long2string(errcode));
	     
	PutLine0(4, 0, "       \"");
	removing_leading_white_space(ps_global->last_error);
	for(i = 4, p = ps_global->last_error; *p && i < DE_LINE; ){
	    for(s = NULL, q = p; *q && q - p < DE_COLS - 16; q++)
	      if(isspace((unsigned char)*q))
		s = q;

	    if(*q && s)
	      q = s;

	    while(p < q)
	      Writechar(*p++, 0);

	    if(*(p = q)){
		ClearLine(++i);
		PutLine0(i, 0, "        ");
		while(*p && isspace((unsigned char)*p))
		  p++;
	    }
	    else{
		Writechar('\"', 0);
		CleartoEOLN();
		break;
	    }
	}

	ClearLine(++i);
	for(j = ++i; i < DE_LINE && de_msg[i-j]; i++){
	    ClearLine(i);
	    PutLine0(i, 0, "  ");
	    Write_to_screen(de_msg[i-j]);
	}

	while(i < DE_LINE)
	  ClearLine(i++);

	switch(radio_buttons(DE_PMT, -FOOTER_ROWS(ps_global), de_opts,
			     'r', 0, NO_HELP, RB_FLUSH_IN | RB_NO_NEWMAIL)){
	  case 'r' :				/* Retry! */
	    ps_global->mangled_screen = 1;
	    return(0L);

	  case 'f' :				/* File Browser */
	    {
		char full_filename[MAXPATH+1], filename[MAXPATH+1];

		filename[0] = '\0';
		build_path(full_filename, ps_global->home_dir, filename,
			   sizeof(full_filename));
		file_lister("DISK ERROR", full_filename, MAXPATH+1, 
                             filename, MAXPATH+1, FALSE, FB_SAVE);
	    }

	    break;

	  case 's' :
	    EndInverse();
	    end_keyboard(ps_global ? F_ON(F_USE_FK,ps_global) : 0);
	    end_tty_driver(ps_global);
	    for(i = 0; de_shell_msg[i]; i++)
	      puts(de_shell_msg[i]);

	    /*
	     * Don't use our piping mechanism to spawn a subshell here
	     * since it will the server (thus reentering c-client).
	     * Bad thing to do.
	     */
#ifdef	_WINDOWS
#else
	    system("csh");
#endif
	    init_tty_driver(ps_global);
	    init_keyboard(F_ON(F_USE_FK,ps_global));
	    break;
	}

	if(ps_global->redrawer)
	  (*ps_global->redrawer)();
    }
}


void
mm_fatal(message)
    char *message;
{
    panic(message);
}


void
mm_flags(stream,number)
    MAILSTREAM    *stream;
    unsigned long  number;
{
    /*
     * The idea here is to clean up any data pine might have cached
     * that has anything to do with the indicated message number.
     * At the momment, this amounts only to cached index lines, but
     * watch out for future changes...
     */
    if(stream == ps_global->mail_stream){
	long i;

	/* then clear index entry */
	if(i = mn_raw2m(ps_global->msgmap, (long) number)){
	    clear_index_cache_ent(i);

	    /* in case number is current, fix titlebar */
	    if(mn_is_cur(ps_global->msgmap, i))
	      ps_global->mangled_header = 1;

	    check_point_change();
	}
    }
}




/*
 *
 */
void
mm_status(stream, mailbox, status)
    MAILSTREAM *stream;
    char       *mailbox;
    MAILSTATUS *status;
{
    mm_status_result = *status;

#ifdef DEBUG
    if(ps_global->debug_imap < 3){
	dprint(0, (debugfile, " Mailbox %s",mailbox));
	if (status->flags & SA_MESSAGES)
	  dprint(0, (debugfile, ", %lu messages",status->messages));

	if (status->flags & SA_RECENT)
	  dprint(0, (debugfile, ", %lu recent",status->recent));

	if (status->flags & SA_UNSEEN)
	  dprint(0, (debugfile, ", %lu unseen",status->unseen));

	if (status->flags & SA_UIDVALIDITY)
	  dprint(0, (debugfile, ", %lu UID validity", status->uidvalidity));

	if (status->flags & SA_UIDNEXT)
	  dprint(0, (debugfile, ", %lu next UID",status->uidnext));

	dprint(0, (debugfile, "\n"));
    }
#endif
}


/*
 *
 */
void
mm_list(stream, delimiter, mailbox, attributes)
    MAILSTREAM *stream;
    int		delimiter;
    char       *mailbox;
    long	attributes;
{
#ifdef DEBUG
    if(ps_global->debug_imap > 2)
      dprint(0,
              (debugfile, "mm_list \"%s\": delim: '%c', %s%s%s%s\n",
	       mailbox, delimiter ? delimiter : 'X',
	       (attributes & LATT_NOINFERIORS) ? ", no inferiors" : "",
	       (attributes & LATT_NOSELECT) ? ", no select" : "",
	       (attributes & LATT_MARKED) ? ", marked" : "",
	       (attributes & LATT_UNMARKED) ? ", unmarked" : ""));
#endif

    if(!mm_list_info->stream || stream == mm_list_info->stream)
      (*mm_list_info->filter)(stream, mailbox, delimiter,
			      attributes, mm_list_info->data);
}


/*
 *
 */
void
mm_lsub(stream, delimiter, mailbox, attributes)
  MAILSTREAM *stream;
  int	      delimiter;
  char	     *mailbox;
  long	      attributes;
{
#ifdef DEBUG
    if(ps_global->debug_imap > 2)
      dprint(0,
              (debugfile, "LSUB \"%s\": delim: '%c', %s%s%s%s\n",
	       mailbox, delimiter ? delimiter : 'X',
	       (attributes & LATT_NOINFERIORS) ? ", no inferiors" : "",
	       (attributes & LATT_NOSELECT) ? ", no select" : "",
	       (attributes & LATT_MARKED) ? ", marked" : "",
	       (attributes & LATT_UNMARKED) ? ", unmarked" : ""));
#endif

    if(!mm_list_info->stream || stream == mm_list_info->stream)
      (*mm_list_info->filter)(stream, mailbox, delimiter,
			      attributes, mm_list_info->data);
}


/*
 * pine_tcptimeout - C-client callback to handle tcp-related timeouts.
 */
long
pine_tcptimeout(elapsed, sincelast)
    long elapsed, sincelast;
{
    long rv = 1L;			/* keep trying by default */
    int	 ch;

#ifdef	DEBUG
    dprint(1, (debugfile, "tcptimeout: waited %s seconds\n",
	       long2string(elapsed)));
    fflush(debugfile);
#endif

    if(ps_global->noshow_timeout)
      return(rv);

    suspend_busy_alarm();
    
    /*
     * Prompt after a minute (since by then things are probably really bad)
     * A prompt timeout means "keep trying"...
     */
    if(elapsed >= (long)ps_global->tcp_query_timeout){
	int clear_inverse;

	ClearLine(ps_global->ttyo->screen_rows - FOOTER_ROWS(ps_global));
	if(clear_inverse = !InverseState())
	  StartInverse();

	PutLine1(ps_global->ttyo->screen_rows - FOOTER_ROWS(ps_global), 0,
       "\007Waited %s seconds for server reply.  Break connection to server? ",
	   long2string(elapsed));
	CleartoEOLN();
	fflush(stdout);
	flush_input();
	ch = read_char(7);
	if(ch == 'y' || ch == 'Y')
	  rv = 0L;

	if(clear_inverse)
	  EndInverse();

	ClearLine(ps_global->ttyo->screen_rows - FOOTER_ROWS(ps_global));
    }

    if(rv == 1L){			/* just warn 'em something's up */
	q_status_message1(SM_ORDER, 0, 0,
		  "Waited %s seconds for server reply.  Still Waiting...",
		  long2string(elapsed));
	flush_status_messages(0);	/* make sure it's seen */
    }

    mark_status_dirty();		/* make sure it get's cleared */

    resume_busy_alarm((rv == 1) ? 3 : 0);

    return(rv);
}


char *
imap_referral(stream, ref, code)
    MAILSTREAM *stream;
    char       *ref;
    long	code;
{
    char *buf = NULL;

    if(ref && !struncmp(ref, "imap://", 7)){
	char *folder = NULL;
	long  uid_val, uid;
	int rv;

	rv = url_imap_folder(ref, &folder, &uid, &uid_val, NULL, 1);
	switch(code){
	  case REFAUTHFAILED :
	  case REFAUTH :
	    if((rv & URL_IMAP_IMAILBOXLIST) && (rv & URL_IMAP_ISERVERONLY))
	      buf = cpystr(folder);

	    break;

	  case REFSELECT :
	  case REFCREATE :
	  case REFDELETE :
	  case REFRENAME :
	  case REFSUBSCRIBE :
	  case REFUNSUBSCRIBE :
	  case REFSTATUS :
	  case REFCOPY :
	  case REFAPPEND :
	    if(rv & URL_IMAP_IMESSAGELIST)
	      buf = cpystr(folder);

	    break;

	  default :
	    break;
	}

	if(folder)
	  fs_give((void **) &folder);
    }

    return(buf);
}


long
imap_proxycopy(stream, sequence, mailbox, flags)
    MAILSTREAM *stream;
    char       *sequence, *mailbox;
    long	flags;
{
    SE_APP_S args;

    args.folder = mailbox;
    args.flags  = flags;

    return(imap_seq_exec(stream, sequence, imap_seq_exec_append, &args));
}


long
imap_seq_exec(stream, sequence, func, args)
    MAILSTREAM	*stream;
    char	*sequence;
    long       (*func) PROTO((MAILSTREAM *, long, void *));
    void	*args;
{
    unsigned long i,j,x;

    while (*sequence) {		/* while there is something to parse */
	if (*sequence == '*') {	/* maximum message */
	    if(!(i = stream->nmsgs)){
		mm_log ("No messages, so no maximum message number",ERROR);
		return(0L);
	    }

	    sequence++;		/* skip past * */
	}
	else if (!(i = strtoul ((const char *) sequence,&sequence,10))
		 || (i > stream->nmsgs)){
	    mm_log ("Sequence invalid",ERROR);
	    return(0L);
	}

	switch (*sequence) {	/* see what the delimiter is */
	  case ':':			/* sequence range */
	    if (*++sequence == '*') {	/* maximum message */
		if (stream->nmsgs) j = stream->nmsgs;
		else {
		    mm_log ("No messages, so no maximum message number",ERROR);
		    return NIL;
		}

		sequence++;		/* skip past * */
	    }
				/* parse end of range */
	    else if (!(j = strtoul ((const char *) sequence,&sequence,10)) ||
		     (j > stream->nmsgs)) {
		mm_log ("Sequence range invalid",ERROR);
		return NIL;
	    }
	    if (*sequence && *sequence++ != ',') {
		mm_log ("Sequence range syntax error",ERROR);
		return NIL;
	    }

	    if (i > j) {		/* swap the range if backwards */
		x = i; i = j; j = x;
	    }
				/* mark each item in the sequence */
	    while (i <= j)
	      if(!(*func)(stream, j--, args)){
		  mail_elt (stream,j--)->sequence = T;
		  return(0L);
	      }

	    break;
	  case ',':			/* single message */
	    ++sequence;		/* skip the delimiter, fall into end case */
	  case '\0':		/* end of sequence, mark this message */
	    if(!(*func)(stream, i, args)){
		mail_elt (stream,i)->sequence = T;
		return(0L);
	    }
	    break;
	  default:			/* anything else is a syntax error! */
	    mm_log ("Sequence syntax error",ERROR);
	    return NIL;
	}
    }

    return T;			/* successfully parsed sequence */
}


long
imap_seq_exec_append(stream, msgno, args)
    MAILSTREAM *stream;
    long	msgno;
    void       *args;
{
    char	 *save_folder, flags[64], date[64];
    CONTEXT_S    *cntxt = NULL;
    int		  our_stream = 0;
    long	  rv = 0L;
    MAILSTREAM   *save_stream;
    SE_APP_S	 *sa = (SE_APP_S *) args;
    MESSAGECACHE *mc;
    STORE_S      *so;

    save_folder = (strucmp(sa->folder, ps_global->inbox_name) == 0)
		    ? ps_global->VAR_INBOX_PATH : sa->folder;

    save_stream = save_msg_stream(cntxt, save_folder, &our_stream);

    if(so = so_get(CharStar, NULL, WRITE_ACCESS)){
	/* store flags before the fetch so UNSEEN bit isn't flipped */
	mc = mail_elt(stream, msgno);
	flag_string(mc, F_ANS|F_FLAG|F_SEEN, flags);
	if(mc->day)
	  mail_date(date, mc);
	else
	  *date = '\0';

	rv = save_fetch_append(stream, msgno, NULL,
			       save_stream, save_folder, NULL,
			       mc->rfc822_size, flags, date, so);
	if(rv < 0 || ps_global->expunge_count){
	    cmd_cancelled("Attached message Save");
	    rv = 0L;
	}
	/* else whatever broke in save_fetch_append shoulda bitched */

	so_give(&so);
    }
    else{
	dprint(1, (debugfile, "Can't allocate store for save: %s\n",
		   error_description(errno)));
	q_status_message(SM_ORDER | SM_DING, 3, 4,
			 "Problem creating space for message text.");
    }

    if(our_stream)
      mail_close(save_stream);

    return(rv);
}


/*----------------------------------------------------------------------
  This can be used to prevent the flickering of the check_cue char
  caused by numerous (5000+) fetches by c-client.  Right now, the only
  practical use found is newsgroup subsciption.

  check_cue_display will check if this global is set, and won't clear
  the check_cue_char if set.
  ----*/
void
set_read_predicted(i)
     int i;
{
    ps_global->read_predicted = i==1;
#ifdef _WINDOWS
    if(!i && F_ON(F_SHOW_DELAY_CUE, ps_global))
      check_cue_display("  ");
#endif
}

/*----------------------------------------------------------------------
   Exported method to display status of mail check

   Args: putstr -- should be NO LONGER THAN 2 bytes

 Result: putstr displayed at upper-left-hand corner of screen
  ----*/
void
check_cue_display(putstr)
     char* putstr;
{
    COLOR_PAIR *lastc;
    struct variable *vars = ps_global->vars;
    static char check_cue_char;
  
    if(ps_global->read_predicted && 
       (check_cue_char == putstr[0]
	|| putstr[0] == ' ' && putstr[1] == '\0'))
        return;
    else{
        if(putstr[0] == ' ')
	  check_cue_char = '\0';
	else
	  check_cue_char = putstr[0];
    }

    lastc = pico_set_colors(VAR_TITLE_FORE_COLOR, VAR_TITLE_BACK_COLOR,
			    PSC_REV|PSC_RET);
    PutLine0(0, 0, putstr);		/* show delay cue */
    if(lastc){
	(void)pico_set_colorp(lastc, PSC_NONE);
	free_color_pair(&lastc);
    }

    fflush(stdout);
}

/*----------------------------------------------------------------------
   Exported method to retrieve logged in user name associated with stream

   Args: host -- host to find associated login name with.

 Result: 
  ----*/
void *
pine_block_notify(reason, data)
    int   reason;
    void *data;
{
    static int mask = 0;
    static int deep = 0;

    switch(reason){
      case BLOCK_SENSITIVE:		/* sensitive code, disallow alarms */
	alrm_signal_block();
	break;

      case BLOCK_NONSENSITIVE:		/* non-sensitive code, allow alarms */
	alrm_signal_unblock();
	break;

      case BLOCK_TCPWRITE:		/* blocked on TCP write */
      case BLOCK_FILELOCK:		/* blocked on file locking */
#ifdef	_WINDOWS
	if(F_ON(F_SHOW_DELAY_CUE, ps_global))
	  check_cue_display(">");

	mswin_setcursor(MSWIN_CURSOR_BUSY);
#endif
	break;

      case BLOCK_DNSLOOKUP:		/* blocked on DNS lookup */
      case BLOCK_TCPOPEN:		/* blocked on TCP open */
      case BLOCK_TCPREAD:		/* blocked on TCP read */
      case BLOCK_TCPCLOSE:		/* blocked on TCP close */
#ifdef	_WINDOWS
	if(F_ON(F_SHOW_DELAY_CUE, ps_global))
	  check_cue_display("<");

	mswin_setcursor(MSWIN_CURSOR_BUSY);
#endif
	break;

      default :
      case BLOCK_NONE:			/* not blocked */
#ifdef	_WINDOWS
	if(F_ON(F_SHOW_DELAY_CUE, ps_global))
	  check_cue_display(" ");
#endif
	break;

    }

    return(NULL);
}



/*----------------------------------------------------------------------
   Exported method to retrieve logged in user name associated with stream

   Args: host -- host to find associated login name with.

 Result: 
  ----*/
char *
cached_user_name(host)
    char *host;
{
    MMLOGIN_S *l;
    STRLIST_S *h;

    if((l = mm_login_list) && host)
      do
	for(h = l->hosts; h; h = h->next)
	  if(!strucmp(host, h->name))
	    return(l->user);
      while(l = l->next);

    return(NULL);
}


int
imap_same_host(hl1, hl2)
    STRLIST_S *hl1, *hl2;
{
    STRLIST_S *lp;

    for( ; hl1; hl1 = hl1->next)
      for(lp = hl2; lp; lp = lp->next)
      if(!strucmp(hl1->name, lp->name))
	return(TRUE);

    return(FALSE);
}


char *
imap_get_user(m_list, hostlist, altflag)
    MMLOGIN_S *m_list;
    STRLIST_S *hostlist;
    int	       altflag;
{
    MMLOGIN_S *l;
    STRLIST_S *h_list;
    char      *h;
    
    for(l = m_list; l; l = l->next)
      /* host name and user name must match */
      if(imap_same_host(l->hosts, hostlist) && altflag == m_list->altflag)
	return(l->user);

    return(NULL);
}



int
imap_get_passwd(m_list, passwd, user, hostlist, altflag)
    MMLOGIN_S *m_list;
    char      *passwd, *user;
    STRLIST_S *hostlist;
    int	       altflag;
{
    MMLOGIN_S *l;
    STRLIST_S *h_list;
    char      *h;
    
    for(l = m_list; l; l = l->next)
      /* host name and user name must match */
      if(imap_same_host(l->hosts, hostlist)
	 && (!*user || !strcmp(user, l->user))
	 && (*user && l->altflag == altflag)){
	  strcpy(user, l->user);
	  strcpy(passwd, l->passwd);
	  return(TRUE);
      }

    return(FALSE);
}



void
imap_set_passwd(l, passwd, user, hostlist, altflag)
    MMLOGIN_S **l;
    char       *passwd, *user;
    STRLIST_S  *hostlist;
{
    STRLIST_S *h_list, **listp;

    for(; *l; l = &(*l)->next)
      if(imap_same_host((*l)->hosts, hostlist)
	 && !strcmp(user, (*l)->user)
	 && altflag == (*l)->altflag)
	if(strcmp(passwd, (*l)->passwd)){
	    fs_give((void **)&(*l)->passwd);
	    break;
	}
	else
	  return;

    if(!*l){
	*l = (MMLOGIN_S *)fs_get(sizeof(MMLOGIN_S));
	memset(*l, 0, sizeof(MMLOGIN_S));
    }

    if((*l)->passwd)
      fs_give((void **) &(*l)->passwd);

    (*l)->passwd = cpystr(passwd);

    (*l)->altflag = altflag;

    if(!(*l)->user)
      (*l)->user = cpystr(user);

    for( ; hostlist; hostlist = hostlist->next){
	for(listp = &(*l)->hosts;
	    *listp && strucmp((*listp)->name, hostlist->name);
	    listp = &(*listp)->next)
	  ;

	if(!*listp){
	    *listp = (STRLIST_S *) fs_get(sizeof(STRLIST_S));
	    (*listp)->name = cpystr(hostlist->name);
	    (*listp)->next = NULL;
	}
    }
}



void
imap_flush_passwd_cache()
{
    MMLOGIN_S *l;

    while(l = mm_login_list){
	mm_login_list = mm_login_list->next;
	if(l->user)
	  fs_give((void **) &l->user);

	free_strlist(&l->hosts);

	if(l->passwd)
	  fs_give((void **) &l->passwd);

	fs_give((void **) &l);
    }
}


#ifdef	PASSFILE
/* 
 * Specific functions to support caching username/passwd/host
 * triples on disk for use from one session to the next...
 */

#define	FIRSTCH		0x20
#define	LASTCH		0x7e
#define	TABSZ		(LASTCH - FIRSTCH + 1)

static	int		xlate_key;
static	MMLOGIN_S	*passfile_cache = NULL;



/*
 * xlate_in() - xlate_in the given character
 */
char
xlate_in(c)
    int	c;
{
    register int  eti;

    eti = xlate_key;
    if((c >= FIRSTCH) && (c <= LASTCH)){
        eti += (c - FIRSTCH);
	eti -= (eti >= 2*TABSZ) ? 2*TABSZ : (eti >= TABSZ) ? TABSZ : 0;
        return((xlate_key = eti) + FIRSTCH);
    }
    else
      return(c);
}



/*
 * xlate_out() - xlate_out the given character
 */
char
xlate_out(c)
    char c;
{
    register int  dti;
    register int  xch;

    if((c >= FIRSTCH) && (c <= LASTCH)){
        xch  = c - (dti = xlate_key);
	xch += (xch < FIRSTCH-TABSZ) ? 2*TABSZ : (xch < FIRSTCH) ? TABSZ : 0;
        dti  = (xch - FIRSTCH) + dti;
	dti -= (dti >= 2*TABSZ) ? 2*TABSZ : (dti >= TABSZ) ? TABSZ : 0;
        xlate_key = dti;
        return(xch);
    }
    else
      return(c);
}



char *
passfile_name(pinerc, path)
    char *pinerc, *path;
{
    struct stat  sbuf;
    char	*p = NULL;
    int		 i, j;

    if(!path || !pinerc || pinerc[0] == '\0')
      return(NULL);

    if(p = last_cmpnt(pinerc))
      for(i = 0; pinerc < p; pinerc++, i++)
	path[i] = *pinerc;
    else
      i = 0;

    for(j = 0; path[i] = PASSFILE[j]; i++, j++)
      ;

#if	defined(DOS) || defined(OS2)
    return((stat(path, &sbuf) == 0
	    && ((sbuf.st_mode & S_IFMT) == S_IFREG))
	     ? path : NULL);
#else
    /* First, make sure it's ours and not sym-linked */
    if(lstat(path, &sbuf) == 0
       && ((sbuf.st_mode & S_IFMT) == S_IFREG)
       && sbuf.st_uid == getuid()){
	/* if too liberal permissions, fix them */
	if((sbuf.st_mode & 077) != 0)
	  if(chmod(path, sbuf.st_mode & ~077) != 0)
	    return(NULL);
	
	return(path);
    }
    else
      return(NULL);
#endif
}



int
read_passfile(pinerc, l)
    char       *pinerc;
    MMLOGIN_S **l;
{
    char  tmp[MAILTMPLEN], *ui[4];
    int   i, j, n;
    FILE *fp;

    /* if there's no password to read, bag it!! */
    if(!passfile_name(pinerc, tmp) || !(fp = fopen(tmp, "r")))
      return(0);

    for(n = 0; fgets(tmp, sizeof(tmp), fp); n++){
	/*** do any necessary DEcryption here ***/
	xlate_key = n;
	for(i = 0; tmp[i]; i++)
	  tmp[i] = xlate_out(tmp[i]);

	if(i && tmp[i-1] == '\n')
	  tmp[i-1] = '\0';			/* blast '\n' */

	ui[0] = ui[1] = ui[2] = ui[3] = NULL;
	for(i = 0, j = 0; tmp[i] && j < 4; j++){
	    for(ui[j] = &tmp[i]; tmp[i] && tmp[i] != '\t'; i++)
	      ;					/* find end of data */

	    if(tmp[i])
	      tmp[i++] = '\0';			/* tie off data */
	}

	if(ui[0] && ui[1] && ui[2]){		/* valid field? */
	    STRLIST_S hostlist[1];
	    int	      flags = ui[3] ? atoi(ui[3]) : 0;

	    hostlist[0].name = ui[2];
	    hostlist[0].next = NULL;
	    imap_set_passwd(l, ui[0], ui[1], hostlist, flags & 0x01);
	}
    }

    fclose(fp);
    return(1);
}



void
write_passfile(pinerc, l)
    char      *pinerc;
    MMLOGIN_S *l;
{
    char  tmp[MAILTMPLEN];
    int   i, n;
    FILE *fp;

    /* if there's no password to read, bag it!! */
    if(!passfile_name(pinerc, tmp) || !(fp = fopen(tmp, "w")))
      return;

    for(n = 0; l; l = l->next, n++){
	/*** do any necessary ENcryption here ***/
	sprintf(tmp, "%.100s\t%.100s\t%.100s\t%d\n", l->passwd, l->user,
		l->hosts->name, l->altflag);
	xlate_key = n;
	for(i = 0; tmp[i]; i++)
	  tmp[i] = xlate_in(tmp[i]);

	fputs(tmp, fp);
    }

    fclose(fp);
}



/*
 * get_passfile_passwd - return the password contained in the special passord
 *            cache.  The file is assumed to be in the same directory
 *            as the pinerc with the name defined above.
 */
int
get_passfile_passwd(pinerc, passwd, user, hostlist, altflag)
    char      *pinerc, *passwd, *user;
    STRLIST_S *hostlist;
    int	       altflag;
{
    return((passfile_cache || read_passfile(pinerc, &passfile_cache))
	     ? imap_get_passwd(passfile_cache, passwd,
			       user, hostlist, altflag)
	     : 0);
}



/*
 * set_passfile_passwd - set the password file entry associated with
 *            cache.  The file is assumed to be in the same directory
 *            as the pinerc with the name defined above.
 */
void
set_passfile_passwd(pinerc, passwd, user, hostlist, altflag)
    char      *pinerc, *passwd, *user;
    STRLIST_S *hostlist;
    int	       altflag;
{
    if((passfile_cache || read_passfile(pinerc, &passfile_cache))
       && want_to("Preserve password on DISK for next login", 'y', 'x',
		  NO_HELP, WT_NORM) == 'y'){
	imap_set_passwd(&passfile_cache, passwd, user, hostlist, altflag);
	write_passfile(pinerc, passfile_cache);
    }
}
#endif	/* PASSFILE */
