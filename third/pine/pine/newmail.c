#if !defined(lint) && !defined(DOS)
static char rcsid[] = "$Id: newmail.c,v 1.1.1.2 2003-02-12 08:01:28 ghudson Exp $";
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
   1989-2002 by the University of Washington.

   The full text of our legal notices is contained in the file called
   CPYRIGHT, included with this distribution.


   Pine is in part based on The Elm Mail System:
    ***********************************************************************
    *  The Elm Mail System  -  Revision: 2.13                             *
    *                                                                     *
    * 			Copyright (c) 1986, 1987 Dave Taylor              *
    * 			Copyright (c) 1988, 1989 USENET Community Trust   *
    ***********************************************************************
 

  ----------------------------------------------------------------------*/

/*======================================================================
     newmail.c

   Check for new mail and queue up notification

 ====*/

#include "headers.h"


/*
 * Internal prototypes
 */
int  check_point PROTO((CheckPointTime, int));
void new_mail_mess PROTO((MAILSTREAM *, char *, long, long));
void fixup_flags PROTO((MAILSTREAM *, MSGNO_S *, long));


static long mailbox_mail_since_command = 0L,
	    inbox_mail_since_command   = 0L;

/*----------------------------------------------------------------------
     new_mail() - check for new mail in the inbox
 
  Input:  force       -- flag indicating we should check for new mail no matter
          time_for_check_point -- 0: GoodTime, 1: BadTime, 2: VeryBadTime
	  flags -- whether to q a new mail status message or defer the sort

  Result: returns -1 if there was no new mail. Otherwise returns the
          sorted message number of the smallest message number that
          was changed. That is the screen needs to be repainted from
          that message down.

  Limit frequency of checks because checks use some resources. That is
  they generate an IMAP packet or require locking the local mailbox.
  (Acutally the lock isn't required, a stat will do, but the current
   c-client mail code locks before it stats.)

  Returns >= 0 only if there is a change in the given mail stream. Otherwise
  this returns -1.  On return the message counts in the pine
  structure are updated to reflect the current number of messages including
  any new mail and any expunging.
 
 --- */
long
new_mail(force, time_for_check_point, flags)
    int force, time_for_check_point, flags;
{
    static time_t last_check = 0;
    static time_t last_check_point_call = 0;
    time_t        now;
    long          n, rv = 0;
    MAILSTREAM   *stream;
    register struct pine *pine_state;
    int           checknow = 0;

    dprint(9, (debugfile, "new mail called (%d %d %d)\n",
               force, time_for_check_point, flags));
    pine_state = ps_global;  /*  this gets called out of the composer which
                              *  doesn't know about pine_state
                              */
    now = time(0);

    if(time_for_check_point == 0)
      adrbk_maintenance();

    if(pine_state->need_to_rethread)
      force = 1;

    if(!force && pine_state->unsorted_newmail)
      force = !(flags & NM_DEFER_SORT);

    /*
     * only check every 15 seconds, unless we're compelled to
     */
    if(!(stream = pine_state->mail_stream)
       || !(timeo || force
	    || pine_state->inbox_changed || pine_state->mail_box_changed)
       || (now-last_check_point_call <= 15 && time_for_check_point != 0
	   && !pine_state->mail_box_changed && !force))
      return(-1);
    else if(force || now-last_check >= timeo-2){ /* 2: check each timeout */
	checknow++;
        last_check = now;
    }

    last_check_point_call = now;

    if(!check_point((time_for_check_point == 0)
		      ? GoodTime
		      : (time_for_check_point == 1)
			  ? BadTime : VeryBadTime, flags)
       && checknow){
	if((flags & NM_STATUS_MSG) && F_ON(F_SHOW_DELAY_CUE, ps_global)
	   && !ps_global->in_init_seq){
	    check_cue_display(" *");	/* Show something to indicate delay */
	    MoveCursor(ps_global->ttyo->screen_rows -FOOTER_ROWS(ps_global),0);
	}
#ifdef _WINDOWS
	mswin_setcursor (MSWIN_CURSOR_BUSY);
#endif

#ifdef	DEBUG
	now = time(0);
	dprint(7, (debugfile, "Mail_Ping(mail_stream): %s\n", ctime(&now)));
#endif
        /*-- Ping the mailstream to check for new mail --*/
        dprint(6, (debugfile, "New mail checked \n"));
	if((char *)mail_ping(stream) == NULL)
	  pine_state->dead_stream = 1;

#ifdef	DEBUG
	now = time(0);
	dprint(7, (debugfile, "Ping complete: %s\n", ctime(&now)));
#endif
	if((flags & NM_STATUS_MSG)
	   && F_ON(F_SHOW_DELAY_CUE, ps_global)
	   && !ps_global->in_init_seq){
	  check_cue_display("  ");
	}
#ifdef _WINDOWS
	mswin_setcursor (MSWIN_CURSOR_ARROW);
#endif
    }

    if(checknow && pine_state->inbox_stream 
       && stream != pine_state->inbox_stream){
	if((flags & NM_STATUS_MSG)
	   && F_ON(F_SHOW_DELAY_CUE, ps_global)){
	  check_cue_display(" *"); 	/* Show something to indicate delay */
	  MoveCursor(ps_global->ttyo->screen_rows -FOOTER_ROWS(ps_global),0);
	}
#ifdef _WINDOWS
	mswin_setcursor (MSWIN_CURSOR_BUSY);
#endif

#ifdef	DEBUG
	now = time(0);
	dprint(7, (debugfile, "Mail_Ping(inbox_stream): %s\n", ctime(&now)));
#endif
	if((char *)mail_ping(pine_state->inbox_stream) == NULL)
	  pine_state->dead_inbox = 1;

#ifdef	DEBUG
	now = time(0);
	dprint(7, (debugfile, "Ping complete: %s\n", ctime(&now)));
#endif
	if((flags & NM_STATUS_MSG)
	   && F_ON(F_SHOW_DELAY_CUE, ps_global))
	    check_cue_display("  ");
#ifdef _WINDOWS
	mswin_setcursor (MSWIN_CURSOR_ARROW);
#endif
    }

    /*-------------------------------------------------------------
       Mail box state changed, could be additions or deletions.
      -------------------------------------------------------------*/
    if(pine_state->mail_box_changed || pine_state->unsorted_newmail) {
        dprint(7, (debugfile,
        "New mail, %s,  new_mail_count:%d  expunge count:%d,  max_msgno:%d\n",
                   pine_state->mail_stream == pine_state->inbox_stream ?
                      "inbox" : "other",
                   pine_state->new_mail_count,
                   pine_state->expunge_count,
                   mn_get_total(pine_state->msgmap)));

	if(pine_state->mail_box_changed)
	  fixup_flags(pine_state->mail_stream, pine_state->msgmap,
		    pine_state->new_mail_count);

	if(pine_state->new_mail_count)
	  process_filter_patterns(pine_state->mail_stream,
				  pine_state->msgmap,
				  ps_global->new_mail_count);

	/*
	 * Lastly, worry about sorting if we got something new, otherwise
	 * it was taken care of inside mm_expunge...
	 */
	if((pine_state->new_mail_count > 0L
	    || pine_state->unsorted_newmail
	    || pine_state->need_to_rethread)
	   && !((flags & NM_DEFER_SORT)
		|| any_lflagged(pine_state->msgmap, MN_HIDE)))
	  refresh_sort(pine_state->msgmap,
		       (flags & NM_STATUS_MSG) ? SRT_VRB : SRT_NON);
	else if(pine_state->new_mail_count > 0L)
	  pine_state->unsorted_newmail = 1;

        if(pine_state->new_mail_count > 0) {
            mailbox_mail_since_command += pine_state->new_mail_count;
	    rv                         += pine_state->new_mail_count;
	    ps_global->new_mail_count   = 0L;

	    if(flags & NM_STATUS_MSG){
		for(n = pine_state->mail_stream->nmsgs; n > 1L; n--)
		  if(!get_lflag(pine_state->mail_stream, NULL, n, MN_EXLD))
		    break;

		if(n)
		  new_mail_mess(pine_state->mail_stream,
			 (pine_state->mail_stream == pine_state->inbox_stream)
			    ? NULL :  pine_state->cur_folder,
			 mailbox_mail_since_command, n);
	    }
        }

	if(flags & NM_STATUS_MSG)
	  pine_state->mail_box_changed = 0;
    }

    if(pine_state->inbox_changed
       && pine_state->inbox_stream != pine_state->mail_stream) {
        /*--  New mail for the inbox, queue up the notification           --*/
        /*-- If this happens then inbox is not current stream that's open --*/
        dprint(7, (debugfile,
         "New mail, inbox, new_mail_count:%ld expunge: %ld,  max_msgno %ld\n",
                   pine_state->inbox_new_mail_count,
                   pine_state->inbox_expunge_count,
                   mn_get_total(pine_state->inbox_msgmap)));

	fixup_flags(pine_state->inbox_stream, pine_state->inbox_msgmap,
		    pine_state->inbox_new_mail_count);

	if(pine_state->inbox_new_mail_count)
	  process_filter_patterns(pine_state->inbox_stream,
				  pine_state->inbox_msgmap,
				  pine_state->inbox_new_mail_count);

        if(pine_state->inbox_new_mail_count > 0) {
            inbox_mail_since_command       += pine_state->inbox_new_mail_count;
	    rv                             += pine_state->inbox_new_mail_count;
            ps_global->inbox_new_mail_count   = 0L;

	    if(flags & NM_STATUS_MSG){
		for(n = pine_state->inbox_stream->nmsgs; n > 1L; n--)
		  if(!get_lflag(pine_state->inbox_stream, NULL, n, MN_EXLD))
		    break;

		if(n)
		  new_mail_mess(pine_state->inbox_stream, NULL,
			    inbox_mail_since_command, n);
	    }
        }

	if(flags & NM_STATUS_MSG)
	  pine_state->inbox_changed = 0;
    }

    rv += pine_state->expunge_count;

    dprint(6, (debugfile, "******** new mail returning %ld  ********\n", 
	       rv ? rv : -1));
    return(rv ? rv : -1);
}


/*----------------------------------------------------------------------
     Format and queue a "new mail" message

  Args: stream     -- mailstream on which a mail has arrived
        folder     -- Name of folder, NULL if inbox
        number     -- number of new messages since last command
        max_num    -- The number of messages now on stream

 Not too much worry here about the length of the message because the
status_message code will fit what it can on the screen and truncation on
the right is about what we want which is what will happen.
  ----*/
void
new_mail_mess(stream, folder, number, max_num)
     MAILSTREAM *stream;
     long        number, max_num;
     char       *folder;
{
    ENVELOPE	*e;
    char	*subject = NULL, *from = NULL, tmp[MAILTMPLEN+1],
		 intro[MAX_SCREEN_COLS+1], subj_leadin[MAILTMPLEN];
    static char *carray[] = { "regarding",
				"concerning",
				"about",
				"as to",
				"as regards",
				"as respects",
				"in re",
				"re",
				"respecting",
				"in point of",
				"with regard to",
				"subject:"
    };

    e = mail_fetchstructure(stream, max_num, NULL);

    if(e && e->from){
        if(e->from->personal && e->from->personal[0]){
	    /*
	     * The reason we use so many characters for tmp is because we
	     * may have multiple MIME3 chunks and we don't want to truncate
	     * in the middle of one of them before decoding.
	     */
	    sprintf(tmp, "%.*s", MAILTMPLEN, e->from->personal);
 	    from = cpystr((char *) rfc1522_decode((unsigned char *)tmp_20k_buf,
 						  SIZEOF_20KBUF, tmp, NULL));
 	}
 	else{
 	    sprintf(tmp, "%.40s%s%.40s", 
 		    e->from->mailbox,
 		    e->from->host ? "@" : "",
 		    e->from->host ? e->from->host : "");
 	    from = cpystr(tmp);
 	}
    }

    if(number <= 1L) {
        if(e && e->subject && e->subject[0]){
 	    sprintf(tmp, "%.*s", MAILTMPLEN, e->subject);
 	    subject = cpystr((char *) rfc1522_decode((unsigned char *)tmp_20k_buf,
 						     SIZEOF_20KBUF, tmp, NULL));
	}

	sprintf(subj_leadin, " %s ", carray[(unsigned)random()%12]);
        if(!from)
          subj_leadin[1] = toupper((unsigned char)subj_leadin[1]);
    }

    if(!subject)
      subject = cpystr("");
      
    if(!folder) {
        if(number > 1)
          sprintf(intro, "%ld new messages!", number);
        else
	  sprintf(intro, "New mail%s!",
		  (e && address_is_us(e->to, ps_global)) ? " to you" : "");
    }
    else {
	long fl, tot, newfl;

	if(number > 1)
 	  sprintf(intro, "%ld messages saved to folder \"%.80s\"",
 		  number, folder);
	else
	  sprintf(intro, "Mail saved to folder \"%.80s\"", folder);
	
	if((fl=strlen(folder)) > 10 &&
	   (tot=strlen(intro) + strlen(from ? from : "") + strlen(subject)) >
					   ps_global->ttyo->screen_cols - 2){
	    newfl = max(10, fl-(tot-(ps_global->ttyo->screen_cols - 2)));
	    if(number > 1)
	      sprintf(intro, "%ld messages saved to folder \"...%.80s\"",
		      number, folder+(fl-(newfl-3)));
	    else
	      sprintf(intro, "Mail saved to folder \"...%.80s\"",
		      folder+(fl-(newfl-3)));
	}
    }

    q_status_message7(SM_ASYNC | SM_DING, 0, 60,
 		      "%s%s%s%.80s%s%.80s%s", intro,
 		      from ? ((number > 1L) ? " Most recent f" : " F") : "",
 		      from ? "rom " : "",
 		      from ? from : "",
 		      (number <= 1L) ? (subject[0] ? subj_leadin : "")
				     : "",
 		      (number <= 1L) ? (subject[0] ? subject
						   : from ? " w" : " W")
				     : "",
 		      (number <= 1L) ? (subject[0] ? "" : "ith no subject")
				     : "");

    sprintf(tmp_20k_buf, "%s%s%s%.80s", intro,
	    from ? ((number > 1L) ? " Most recent f" : " F") : "",
	    from ? "rom " : "",
	    from ? from : "");
    icon_text(tmp_20k_buf);

    if(from)
      fs_give((void **) &from);

    if(subject)
      fs_give((void **) &subject);
}



/*----------------------------------------------------------------------
  Straighten out any local flag problems here.  We can't take care of
  them in the mm_exists or mm_expunged callbacks since the flags
  themselves are in an MESSAGECACHE and we're not allowed to reenter
  c-client from a callback...

 Args: stream -- mail stream to operate on
       msgmap -- messages in that stream to fix up
       new_msgs -- number of new messages

 Result: returns with local flags as they should be

  ----*/
void
fixup_flags(stream, msgmap, new_msgs)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    long	new_msgs;
{
    /*
     * Deal with the case where expunged away all of the 
     * zoomed messages.  Unhide everything in that case...
     */
    if(mn_get_total(msgmap) > 0L){
	long i;

	if(any_lflagged(msgmap, MN_HIDE) >= mn_get_total(msgmap)){
	    for(i = 1L; i <= mn_get_total(msgmap); i++)
	      set_lflag(stream, msgmap, i, MN_HIDE, 0);

	    mn_set_cur(msgmap, THREADING()
				 ? first_sorted_flagged(F_NONE, stream, 0L,
				               (THREADING() ? 0 : FSF_SKIP_CHID)
					       | FSF_LAST)
				 : mn_get_total(msgmap));
	}
	else if(any_lflagged(msgmap, MN_HIDE)){
	    /*
	     * if we got here, there are some hidden messages and
	     * some not.  Make sure the current message is one
	     * that's not...
	     */
	    for(i = mn_get_cur(msgmap); i <= mn_get_total(msgmap); i++)
	      if(!msgline_hidden(stream, msgmap, i, 0)){
		  mn_set_cur(msgmap, i);
		  break;
	      }

	    for(i = mn_get_cur(msgmap); i > 0L; i--)
	      if(!msgline_hidden(stream, msgmap, i, 0)){
		  mn_set_cur(msgmap, i);
		  break;
	      }
	}
    }
}



/*----------------------------------------------------------------------
    Force write of the main file so user doesn't lose too much when
 something bad happens. The only thing that can get lost is flags, such 
 as when new mail arrives, is read, deleted or answered.

 Args: timing      -- indicates if it's a good time for to do a checkpoint

  Result: returns 1 if checkpoint was written, 
                  0 if not.

NOTE: mail_check will also notice new mail arrival, so it's imperative that
code exist after this function is called that can deal with updating the 
various pieces of pine's state associated with the message count and such.

Only need to checkpoint current stream because there are no changes going
on with other streams when we're not manipulating them.
  ----*/
static int check_count		= 0;  /* number of changes since last chk_pt */
static long first_status_change = 0;  /* time of 1st change since last chk_pt*/
static long last_status_change	= 0;  /* time of last change                 */
static long check_count_ave	= 10 * 10;

check_point(timing, flags)
    CheckPointTime timing;
    int		   flags;
{
    int     freq, tm;
    long    adj_cca;
    long    tmp;
#ifdef	DEBUG
    time_t  now;
#endif

    dprint(9, (debugfile, "check_point(%s)\n", 
               timing == GoodTime ? "GoodTime" :
               timing == BadTime  ? "BadTime" :
               timing == VeryBadTime  ? "VeryBadTime" : "DoItNow"));

    if(!ps_global->mail_stream || ps_global->mail_stream->rdonly ||
							     check_count == 0)
	 return(0);

    freq = CHECK_POINT_FREQ * (timing==GoodTime ? 1 : timing==BadTime ? 3 : 4);
    tm   = CHECK_POINT_TIME * (timing==GoodTime ? 1 : timing==BadTime ? 2 : 3);

    if(!last_status_change)
        last_status_change = time(0);

    tmp = 10*(time(0)-last_status_change);
    adj_cca = (check_count_ave > tmp || check_count_ave > 200)
		? check_count_ave
		: min((check_count_ave + tmp)/2, 200);

    dprint(9, (debugfile, "freq %d tm %d changes %d since_1st_change %d\n",
	       freq, tm, check_count, time(0)-first_status_change));
    dprint(9, (debugfile, "since_status_chg %d chk_cnt_ave %d (tenths)\n",
	       tmp, check_count_ave));
    dprint(9, (debugfile, "adj_chk_cnt_ave %d (tenths)\n", adj_cca));
    dprint(9, (debugfile, "Check:if changes(%d)xadj_cca(%d) >= freq(%d)x200\n",
	       check_count, adj_cca, freq));
    dprint(9, (debugfile, "      is %d >= %d ?\n",
	       check_count*adj_cca, 200*freq));

    /* the 200 comes from 20 seconds for an average status change time
       multiplied by 10 tenths per second */
    if((timing == DoItNow || (check_count * adj_cca >= freq * 200) ||
       (time(0) - first_status_change >= tm))){
#ifdef	DEBUG
	now = time(0);
	dprint(7, (debugfile,
                     "Doing checkpoint: %s  Since 1st status change: %d\n",
                     ctime(&now), now - first_status_change));
#endif
	if((flags & NM_STATUS_MSG) && F_ON(F_SHOW_DELAY_CUE, ps_global)){
	    check_cue_display("**");	/* Show something indicate delay*/
	    MoveCursor(ps_global->ttyo->screen_rows -FOOTER_ROWS(ps_global),0);
	}
#ifdef _WINDOWS
	mswin_setcursor (MSWIN_CURSOR_BUSY);
#endif

        mail_check(ps_global->mail_stream);
					/* Causes mail file to be written */
#ifdef	DEBUG
	now = time(0);
	dprint(7, (debugfile, "Checkpoint complete: %s\n", ctime(&now)));
#endif
        check_count = 0;
        first_status_change = time(0);
	if((flags & NM_STATUS_MSG) && F_ON(F_SHOW_DELAY_CUE, ps_global))
	    check_cue_display("  ");
#ifdef _WINDOWS
	mswin_setcursor (MSWIN_CURSOR_ARROW);
#endif

        return(1);
    } else {
        return(0);
    }
}



/*----------------------------------------------------------------------
    Call this when we need to tell the check pointing mechanism about
  mailbox state changes.
  ----------------------------------------------------------------------*/
void
check_point_change()
{
    if(!last_status_change)
        last_status_change = time(0) - 10;  /* first change 10 seconds */

    if(!check_count++)
      first_status_change = time(0);
    /*
     * check_count_ave is the exponentially decaying average time between
     * status changes, in tenths of seconds, except never grow by more
     * than double, but always at least one (unless there's a fulll moon).
     */
    check_count_ave = min((check_count_ave +
                max(10*(time(0)-last_status_change),2))/2, 2*check_count_ave);

    last_status_change = time(0);
}



/*----------------------------------------------------------------------
    Call this when a mail file is written to reset timer and counter
  for next check_point.
  ----------------------------------------------------------------------*/
void
reset_check_point()
{
    check_count = 0;
    first_status_change = time(0);
}



/*----------------------------------------------------------------------
    Zero the counters that keep track of mail accumulated between
   commands.
 ----*/
void
zero_new_mail_count()
{
    dprint(9, (debugfile, "New_mail_count zeroed\n"));

    /*
     * Decide if likewise need to clean up the new mail icon...
     */
    if(mailbox_mail_since_command || inbox_mail_since_command)
      icon_text(NULL);

    mailbox_mail_since_command = 0L;
    inbox_mail_since_command   = 0L;
}


/*----------------------------------------------------------------------
     Check and see if all the stream are alive

Returns:  0 if there was no change
          1 if streams have died since last call

Also outputs a message that the streams have died
 ----*/
streams_died()
{
    int rv = 0, inbox = 0;

    if(ps_global->dead_stream && !ps_global->noticed_dead_stream) {
        rv = 1;
        ps_global->noticed_dead_stream = 1;
        if(ps_global->mail_stream == ps_global->inbox_stream)
          ps_global->noticed_dead_inbox = 1;
    }

    if(ps_global->dead_inbox && !ps_global->noticed_dead_inbox) {
        rv = 1;
        ps_global->noticed_dead_inbox = 1;
        inbox = 1;
    }
    if(rv == 1) 
      q_status_message1(SM_ORDER | SM_DING, 3, 6,
                        "MAIL FOLDER \"%.200s\" CLOSED DUE TO ACCESS ERROR",
                        pretty_fn(inbox ? ps_global->inbox_name
				  	: ps_global->cur_folder));
    return(rv);
}
        
