#if !defined(lint) && !defined(DOS)
static char rcsid[] = "$Id: mailcmd.c,v 1.1.1.3 2003-05-01 01:13:02 ghudson Exp $";
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
   1989-2003 by the University of Washington.

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
     mailcmd.c
     The meat and pototoes of mail processing here:
       - initial command processing and dispatch
       - save message
       - capture address off incoming mail
       - jump to specific numbered message
       - open (broach) a new folder
       - search message headers (where is) command
  ====*/

#include "headers.h"
#include "../c-client/imap4r1.h"


/*
 * Internal Prototypes
 */
void      cmd_delete PROTO((struct pine *, MSGNO_S *, int, CmdWhere));
void      cmd_undelete PROTO((struct pine *, MSGNO_S *, int));
void      cmd_reply PROTO((struct pine *, MSGNO_S *, int));
void      cmd_forward PROTO((struct pine *, MSGNO_S *, int));
void      cmd_bounce PROTO((struct pine *, MSGNO_S *, int));
void      cmd_print PROTO((struct pine *, MSGNO_S *, int, CmdWhere));
void      cmd_save PROTO((struct pine *, MAILSTREAM *, MSGNO_S *, int,
			  CmdWhere));
void      cmd_export PROTO((struct pine *, MSGNO_S *, int, int));
void      cmd_pipe PROTO((struct pine *, MSGNO_S *, int));
PIPE_S	 *cmd_pipe_open PROTO((char *, char **, int, gf_io_t *));
void	  prime_raw_text_getc PROTO((MAILSTREAM *, long));
STORE_S	 *list_mgmt_text PROTO((RFC2369_S *, long));
void	  list_mgmt_screen PROTO((STORE_S *));
void      cmd_flag PROTO((struct pine *, MSGNO_S *, int));
int	  cmd_flag_prompt PROTO((struct pine *, struct flag_screen *));
long	  save PROTO((struct pine *, MAILSTREAM *,
		      CONTEXT_S *, char *, MSGNO_S *, int));
long	  save_fetch_append_cb PROTO((MAILSTREAM *, void *, char **, char **,
				      STRING **));

void      get_save_fldr_from_env PROTO((char *, int, ENVELOPE *,
					 struct pine *, long, char *));
void	  saved_date PROTO((char *, char *));
int	  save_ex_output_body PROTO((MAILSTREAM *, long, char *, BODY *,
				     unsigned long *, gf_io_t));
int	  save_ex_replace_body PROTO((char *, unsigned long *,BODY *,gf_io_t));
int	  save_ex_mask_types PROTO((char *, unsigned long *, gf_io_t));
int	  save_ex_explain_body PROTO((BODY *, unsigned long *, gf_io_t));
int	  save_ex_explain_parts PROTO((BODY *, int, unsigned long *, gf_io_t));
int	  save_ex_output_line PROTO((char *, unsigned long *, gf_io_t));
int	  create_for_save PROTO((MAILSTREAM *, CONTEXT_S *, char *));
void	  flag_string PROTO((MESSAGECACHE *, long, char *));
int	  select_sort PROTO((struct pine *, int, SortOrder *, int *));
void	  aggregate_select PROTO((struct pine *, MSGNO_S *, int, CmdWhere));
void	  thread_index_select PROTO((struct pine *, MSGNO_S *, int, CmdWhere));
int	  select_number PROTO((MAILSTREAM *, MSGNO_S *, long));
int	  select_thrd_number PROTO((MAILSTREAM *, MSGNO_S *, long));
int	  select_size PROTO((MAILSTREAM *, MSGNO_S *, long));
int	  select_date PROTO((MAILSTREAM *, MSGNO_S *, long));
int	  select_text PROTO((MAILSTREAM *, MSGNO_S *, long));
int	  select_flagged PROTO((MAILSTREAM *, MSGNO_S *, long));
void	  search_headers PROTO((struct pine *, MAILSTREAM *, int, MSGNO_S *));
char	 *currentf_sequence PROTO((MAILSTREAM *, MSGNO_S *, long, long *,int));
char	 *invalid_elt_sequence PROTO((MAILSTREAM *, MSGNO_S *));
char	 *selected_sequence PROTO((MAILSTREAM *, MSGNO_S *, long *));
int	  any_messages PROTO((MSGNO_S *, char *, char *));
int	  can_set_flag PROTO((struct pine *, char *, int));
int	  bezerk_delimiter PROTO((ENVELOPE *, gf_io_t, int));
int	  move_filtered_msgs PROTO((MAILSTREAM *, MSGNO_S *, char *, int));
void	  delete_filtered_msgs PROTO((MAILSTREAM *));
char	 *move_read_msgs PROTO((MAILSTREAM *, char *, char *, long));
int	  read_msg_prompt PROTO((long, char *));
char	 *move_read_incoming PROTO((MAILSTREAM *, CONTEXT_S *, char *,
				    char **, char *));
void	  cross_delete_crossposts PROTO((MAILSTREAM *));
void      menu_clear_cmd_binding PROTO((struct key_menu *, int));
int	  update_folder_spec PROTO((char *, char *));
SEARCHSET *visible_searchset PROTO((MAILSTREAM *, MSGNO_S *));
void      set_some_flags PROTO((MAILSTREAM *, MSGNO_S *, long, int));
int       some_filter_depends_on_state PROTO((void));
void      mail_expunge_prefilter PROTO((MAILSTREAM *));
unsigned  reset_startup_rule PROTO((MAILSTREAM *));
long      closest_jump_target PROTO((long, MAILSTREAM *, MSGNO_S *, int,
				     CmdWhere, char *));
long      get_level PROTO((int, int, SCROLL_S *));


#define SV_DELETE		0x1
#define SV_FOR_FILT		0x2
#define SV_FIX_DELS		0x4

typedef struct append_package {
  MAILSTREAM *stream;
  char *flags;
  char *date;
  STRING *msg;
  MSGNO_S *msgmap;
  long msgno;
  STORE_S *so;
} APPENDPACKAGE;

/*
 * List of Select options used by apply_* functions...
 */
static char *sel_pmt1 = "ALTER message selection : ";
ESCKEY_S sel_opts1[] = {
    {'a', 'a', "A", "unselect All"},
    {'c', 'c', "C", NULL},
    {'b', 'b', "B", "Broaden selctn"},
    {'n', 'n', "N", "Narrow selctn"},
    {'f', 'f', "F", "Flip selected"},
    {-1, 0, NULL, NULL}
};


char *sel_pmt2 = "SELECT criteria : ";
static ESCKEY_S sel_opts2[] = {
    {'a', 'a', "A", "select All"},
    {'c', 'c', "C", "select Cur"},
    {'n', 'n', "N", "Number"},
    {'d', 'd', "D", "Date"},
    {'t', 't', "T", "Text"},
    {'s', 's', "S", "Status"},
    {'z', 'z', "Z", "siZe"},
    {-1, 0, NULL, NULL}
};


static ESCKEY_S sel_opts3[] = {
    {'d', 'd',  "D", "Del"},
    {'u', 'u',  "U", "Undel"},
    {'r', 'r',  "R", "Reply"},
    {'f', 'f',  "F", "Forward"},
    {'%', '%',  "%", "Print"},
    {'t', 't',  "T", "TakeAddr"},
    {'s', 's',  "S", "Save"},
    {'e', 'e',  "E", "Export"},
    { -1,   0, NULL, NULL},
    { -1,   0, NULL, NULL},
    { -1,   0, NULL, NULL},
    { -1,   0, NULL, NULL},
    { -1,   0, NULL, NULL},
    { -1,   0, NULL, NULL},
    {-1,    0, NULL, NULL}
};

static ESCKEY_S sel_opts4[] = {
    {'a', 'a', "A", "select All"},
    {'c', 'c', "C", "select Curthrd"},
    {'n', 'n', "N", "Number"},
    {'d', 'd', "D", "Date"},
    {'t', 't', "T", "Text"},
    {'s', 's', "S", "Status"},
    {'z', 'z', "Z", "siZe"},
    {-1, 0, NULL, NULL}
};


static ESCKEY_S other_opts[] = {
    { -1,   0, NULL, NULL},
    { -1,   0, NULL, NULL},
    { -1,   0, NULL, NULL},
    { -1,   0, NULL, NULL},
    { -1,   0, NULL, NULL},
    {-1,    0, NULL, NULL}
};


static char *sel_flag = 
    "Select New, Deleted, Answered, or Important messages ? ";
static char *sel_flag_not = 
    "Select NOT New, NOT Deleted, NOT Answered or NOT Tagged msgs ? ";
static ESCKEY_S sel_flag_opt[] = {
    {'n', 'n', "N", "New"},
    {'*', '*', "*", "Important"},
    {'d', 'd', "D", "Deleted"},
    {'a', 'a', "A", "Answered"},
    {'!', '!', "!", "Not"},
    {-1, 0, NULL, NULL}
};


static ESCKEY_S sel_date_opt[] = {
    {0, 0, NULL, NULL},
    {ctrl('P'), 12, "^P", "Prev Day"},
    {ctrl('N'), 13, "^N", "Next Day"},
    {ctrl('X'), 11, "^X", "Cur Msg"},
    {ctrl('W'), 14, "^W", "Toggle When"},
    {KEY_UP,    12, "", ""},
    {KEY_DOWN,  13, "", ""},
    {-1, 0, NULL, NULL}
};


static char *sel_text =
    "Select based on To, From, Cc, Recip, Partic, Subject fields or All msg text ? ";
static char *sel_not_text =
    "Select based on NOT To, From, Cc, Recip, Partic, Subject or All msg text ? ";
static ESCKEY_S sel_text_opt[] = {
    {'f', 'f', "F", "From"},
    {'s', 's', "S", "Subject"},
    {'t', 't', "T", "To"},
    {'a', 'a', "A", "All Text"},
    {'c', 'c', "C", "Cc"},
    {'!', '!', "!", "Not"},
    {'r', 'r', "R", "Recipient"},
    {'p', 'p', "P", "Participant"},
    {'b', 'b', "B", "Body"},
    {-1, 0, NULL, NULL}
};

static char *select_num =
  "Enter comma-delimited list of numbers (dash between ranges): ";

static char *select_size_larger_msg =
  "Select messages with size larger than: ";

static char *select_size_smaller_msg =
  "Select messages with size smaller than: ";

static char *sel_size_larger  = "Larger";
static char *sel_size_smaller = "Smaller";
static ESCKEY_S sel_size_opt[] = {
    {0, 0, NULL, NULL},
    {ctrl('W'), 14, "^W", NULL},
    {-1, 0, NULL, NULL}
};


/*----------------------------------------------------------------------
         The giant switch on the commands for index and viewing

  Input:  command  -- The command char/code
          in_index -- flag indicating command is from index
          orig_command -- The original command typed before pre-processing
  Output: force_mailchk -- Set to tell caller to force call to new_mail().

  Result: Manifold

          Returns 1 if the message number or attachment to show changed 
 ---*/
int
process_cmd(state, stream, msgmap, command, in_index, force_mailchk)
    struct pine *state;
    MAILSTREAM  *stream;
    MSGNO_S     *msgmap;
    int		 command;
    CmdWhere	 in_index;
    int		*force_mailchk;
{
    int           question_line, a_changed, we_cancel, flags = 0, ret;
    long          new_msgno, del_count, old_msgno, i, old_max_msgno;
    long          start;
    char         *newfolder, prompt[MAX_SCREEN_COLS+1];
    CONTEXT_S    *tc;
    COLOR_PAIR   *lastc = NULL;
#if	defined(DOS) && !defined(_WINDOWS)
    extern long coreleft();
#endif

    dprint(4, (debugfile, "\n - process_cmd(cmd=%d) -\n", command));

    question_line         = -FOOTER_ROWS(state);
    state->mangled_screen = 0;
    state->mangled_footer = 0;
    state->mangled_header = 0;
    state->next_screen    = SCREEN_FUN_NULL;
    old_msgno             = mn_get_cur(msgmap);
    a_changed             = FALSE;
    *force_mailchk        = 0;

    switch (command) {
	/*------------- Help --------*/
      case MC_HELP :
	/*
	 * We're not using the h_mail_view portion of this right now because
	 * that call is being handled in scrolltool() before it gets
	 * here.  Leave it in case we change how it works.
	 */
	helper((in_index == MsgIndx)
		 ? h_mail_index
		 : (in_index == View)
		   ? h_mail_view
		   : h_mail_thread_index,
	       (in_index == MsgIndx)
	         ? "HELP FOR MESSAGE INDEX"
		 : (in_index == View)
		   ? "HELP FOR MESSAGE VIEW"
		   : "HELP FOR THREAD INDEX",
	       HLPD_NONE);
	dprint(4, (debugfile,"MAIL_CMD: did help command\n"));
	state->mangled_screen = 1;
	break;


          /*--------- Return to main menu ------------*/
      case MC_MAIN :
	state->next_screen = main_menu_screen;
#if	defined(DOS) && !defined(WIN32)
	flush_index_cache();		/* save room on PC */
#endif
	dprint(2, (debugfile,"MAIL_CMD: going back to main menu\n"));
	break;


          /*------- View message text --------*/
      case MC_VIEW_TEXT :
view_text:
	if(any_messages(msgmap, NULL, "to View")){
	    state->next_screen = mail_view_screen;
#if	defined(DOS) && !defined(WIN32)
	    flush_index_cache();		/* save room on PC */
#endif
	}

	break;


          /*------- View attachment --------*/
      case MC_VIEW_ATCH :
	state->next_screen = attachment_screen;
	dprint(2, (debugfile,"MAIL_CMD: going to attachment screen\n"));
	break;


          /*---------- Previous message ----------*/
      case MC_PREVITEM :
	if(any_messages(msgmap, NULL, NULL)){
	    if((i = mn_get_cur(msgmap)) > 1L){
		mn_dec_cur(stream, msgmap,
			   (in_index == View && THREADING()
			    && state->viewing_a_thread)
			     ? MH_THISTHD
			     : (in_index == View)
			       ? MH_ANYTHD : MH_NONE);
		if(i == mn_get_cur(msgmap)){
		    PINETHRD_S *thrd;

		    if(THRD_INDX_ENABLED()){
			mn_dec_cur(stream, msgmap, MH_ANYTHD);
			if(i == mn_get_cur(msgmap))
			  q_status_message1(SM_ORDER, 0, 2,
				      "Already on first %.200s in Zoomed Index",
				      THRD_INDX() ? "thread" : "message");
			else{
			    if(in_index == View
			       || F_ON(F_NEXT_THRD_WO_CONFIRM, state))
			      ret = 'y';
			    else
			      ret = want_to("View previous thread", 'y', 'x',
					    NO_HELP, WT_NORM);

			    if(ret == 'y'){
				q_status_message(SM_ORDER, 0, 2,
						 "Viewing previous thread");
				new_msgno = mn_get_cur(msgmap);
				mn_set_cur(msgmap, i);
				unview_thread(state, stream, msgmap);
				mn_set_cur(msgmap, new_msgno);
				if(THRD_AUTO_VIEW() && in_index == View){

				    thrd = fetch_thread(stream,
							mn_m2raw(msgmap,
								 new_msgno));
				    if(count_lflags_in_thread(stream, thrd,
							      msgmap,
							      MN_NONE) == 1){
					if(view_thread(state, stream, msgmap, 1)){
					    state->view_skipped_index = 1;
					    command = MC_VIEW_TEXT;
					    goto view_text;
					}
				    }
				}

				view_thread(state, stream, msgmap, 1);
				state->next_screen = SCREEN_FUN_NULL;
			    }
			    else
			      mn_set_cur(msgmap, i);	/* put it back */
			}
		    }
		    else
		      q_status_message1(SM_ORDER, 0, 2,
				  "Already on first %.200s in Zoomed Index",
				  THRD_INDX() ? "thread" : "message");
		}
	    }
	    else
	      q_status_message1(SM_ORDER, 0, 1, "Already on first %.200s",
				THRD_INDX() ? "thread" : "message");
	}

	break;


          /*---------- Next Message ----------*/
      case MC_NEXTITEM :
	if(mn_get_total(msgmap) > 0L
	   && ((i = mn_get_cur(msgmap)) < mn_get_total(msgmap))){
	    mn_inc_cur(stream, msgmap,
		       (in_index == View && THREADING()
		        && state->viewing_a_thread)
			 ? MH_THISTHD
			 : (in_index == View)
			   ? MH_ANYTHD : MH_NONE);
	    if(i == mn_get_cur(msgmap)){
		PINETHRD_S *thrd;

		if(THRD_INDX_ENABLED()){
		    if(!THRD_INDX())
		      mn_inc_cur(stream, msgmap, MH_ANYTHD);

		    if(i == mn_get_cur(msgmap)){
			if(any_lflagged(msgmap, MN_HIDE))
			  any_messages(NULL, "more", "in Zoomed Index");
			else
			  goto nfolder;
		    }
		    else{
			if(in_index == View
			   || F_ON(F_NEXT_THRD_WO_CONFIRM, state))
			  ret = 'y';
			else
			  ret = want_to("View next thread", 'y', 'x',
					NO_HELP, WT_NORM);

			if(ret == 'y'){
			    q_status_message(SM_ORDER, 0, 2,
					     "Viewing next thread");
			    new_msgno = mn_get_cur(msgmap);
			    mn_set_cur(msgmap, i);
			    unview_thread(state, stream, msgmap);
			    mn_set_cur(msgmap, new_msgno);
			    if(THRD_AUTO_VIEW() && in_index == View){

				thrd = fetch_thread(stream,
						    mn_m2raw(msgmap,
							     new_msgno));
				if(count_lflags_in_thread(stream, thrd,
							  msgmap,
							  MN_NONE) == 1){
				    if(view_thread(state, stream, msgmap, 1)){
					state->view_skipped_index = 1;
					command = MC_VIEW_TEXT;
					goto view_text;
				    }
				}
			    }

			    view_thread(state, stream, msgmap, 1);
			    state->next_screen = SCREEN_FUN_NULL;
			}
			else
			  mn_set_cur(msgmap, i);	/* put it back */
		    }
		}
		else if(THREADING()
			&& (thrd = fetch_thread(stream, mn_m2raw(msgmap, i)))
			&& thrd->next
			&& get_lflag(stream, NULL, thrd->rawno, MN_COLL)){
		       q_status_message(SM_ORDER, 0, 2,
			       "Expand collapsed thread to see more messages");
		}
		else
		  any_messages(NULL, "more", "in Zoomed Index");
	    }
	}
	else{
nfolder:
	    prompt[0] = '\0';
	    if(IS_NEWS(stream)
	       || (state->context_current->use & CNTXT_INCMNG)){
		char nextfolder[MAXPATH];

		strncpy(nextfolder, state->cur_folder, sizeof(nextfolder));
		nextfolder[sizeof(nextfolder)-1] = '\0';
		if(next_folder(NULL, nextfolder, nextfolder,
			       state->context_current, NULL, NULL))
		  strncpy(prompt, ".  Press TAB for next folder.",
			  sizeof(prompt));
		else
		  strncpy(prompt, ".  No more folders to TAB to.",
			  sizeof(prompt));
	    }

	    any_messages(NULL, (mn_get_total(msgmap) > 0L) ? "more" : NULL,
			 prompt[0] ? prompt : NULL);

	    if(!IS_NEWS(stream))
	      *force_mailchk = 1;
	}

	break;


          /*---------- Delete message ----------*/
      case MC_DELETE :
	cmd_delete(state, msgmap, 0, in_index);
	break;
          

          /*---------- Undelete message ----------*/
      case MC_UNDELETE :
	cmd_undelete(state, msgmap, 0);
	break;


          /*---------- Reply to message ----------*/
      case MC_REPLY :
	cmd_reply(state, msgmap, 0);
	break;


          /*---------- Forward message ----------*/
      case MC_FORWARD :
	cmd_forward(state, msgmap, 0);
	break;


          /*---------- Quit pine ------------*/
      case MC_QUIT :
	state->next_screen = quit_screen;
	dprint(1, (debugfile,"MAIL_CMD: quit\n"));		    
	break;


          /*---------- Compose message ----------*/
      case MC_COMPOSE :
	state->prev_screen = (in_index == View) ? mail_view_screen
						: mail_index_screen;
#if	defined(DOS) && !defined(WIN32)
	flush_index_cache();		/* save room on PC */
#endif
	compose_screen(state);
	state->mangled_screen = 1;
	if (state->next_screen)
	  a_changed = TRUE;
	break;


          /*---------- Alt Compose message ----------*/
      case MC_ROLE :
	state->prev_screen = (in_index == View) ? mail_view_screen
						: mail_index_screen;
#if	defined(DOS) && !defined(WIN32)
	flush_index_cache();		/* save room on PC */
#endif
	alt_compose_screen(state);
	state->mangled_screen = 1;
	if (state->next_screen)
	  a_changed = TRUE;
	break;


          /*--------- Folders menu ------------*/
      case MC_FOLDERS :
	state->start_in_context = 1;

          /*--------- Top of Folders list menu ------------*/
      case MC_COLLECTIONS :
	state->next_screen = folder_screen;
#if	defined(DOS) && !defined(WIN32)
	flush_index_cache();		/* save room on PC */
#endif
	dprint(2, (debugfile,"MAIL_CMD: going to folder/collection menu\n"));
	break;


          /*---------- Open specific new folder ----------*/
      case MC_GOTO :
	tc = (state->context_last && !NEWS_TEST(state->context_current)) 
	       ? state->context_last : state->context_current;

	newfolder = broach_folder(question_line, 1, &tc);
#if	defined(DOS) && !defined(_WINDOWS)
	if(newfolder && *newfolder == '{' && coreleft() < 20000){
	    q_status_message(SM_ORDER | SM_DING, 3, 3,
			     "Not enough memory to open IMAP folder");
	    newfolder = NULL;
	}
#endif
	if(newfolder){
	    visit_folder(state, newfolder, tc, NULL);
	    a_changed = TRUE;
	}

	break;
    	  
    	    
          /*------- Go to Index Screen ----------*/
      case MC_INDEX :
#if	defined(DOS) && !defined(WIN32)
	flush_index_cache();		/* save room on PC */
#endif
	state->next_screen = mail_index_screen;
	break;

          /*------- Skip to next interesting message -----------*/
      case MC_TAB :
	if(THRD_INDX()){
	    PINETHRD_S *thrd;

	    /*
	     * If we're in the thread index, start looking after this
	     * thread. We don't want to match something in the current
	     * thread.
	     */
	    start = mn_get_cur(msgmap);
	    thrd = fetch_thread(stream, mn_m2raw(msgmap, mn_get_cur(msgmap)));
	    if(mn_get_revsort(msgmap)){
		/* if reversed, top of thread is last one before next thread */
		if(thrd && thrd->top)
		  start = mn_raw2m(msgmap, thrd->top);
	    }
	    else{
		/* last msg of thread is at the ends of the branches/nexts */
		while(thrd){
		    start = mn_raw2m(msgmap, thrd->rawno);
		    if(thrd->branch)
		      thrd = fetch_thread(stream, thrd->branch);
		    else if(thrd->next)
		      thrd = fetch_thread(stream, thrd->next);
		    else
		      thrd = NULL;
		}
	    }

	    /*
	     * Flags is 0 in this case because we want to not skip
	     * messages inside of threads so that we can find threads
	     * which have some unseen messages even though the top-level
	     * of the thread is already seen.
	     * If new_msgno ends up being a message which is not visible
	     * because it isn't at the top-level, the current message #
	     * will be adjusted below in adjust_cur.
	     */
	    flags = 0;
	    new_msgno = next_sorted_flagged((F_UNDEL 
					     | F_UNSEEN
					     | ((F_ON(F_TAB_TO_NEW,state))
						 ? 0 : F_OR_FLAG)),
					    stream, start, &flags);
	}
	else if(THREADING() && state->viewing_a_thread){
	    PINETHRD_S *thrd, *topthrd = NULL;

	    start = mn_get_cur(msgmap);

	    /*
	     * Things are especially complicated when we're viewing_a_thread
	     * from the thread index. First we have to check within the
	     * current thread for a new message. If none is found, then
	     * we search in the next threads and offer to continue in
	     * them. Then we offer to go to the next folder.
	     */
	    flags = NSF_SKIP_CHID;
	    new_msgno = next_sorted_flagged((F_UNDEL 
					     | F_UNSEEN
					     | ((F_ON(F_TAB_TO_NEW,state))
					       ? 0 : F_OR_FLAG)),
					    stream, start, &flags);
	    /*
	     * If we found a match then we are done, that is another message
	     * in the current thread index. Otherwise, we have to look
	     * further.
	     */
	    if(!(flags & NSF_FLAG_MATCH)){
		ret = 'n';
		while(1){

		    flags = 0;
		    new_msgno = next_sorted_flagged((F_UNDEL 
						     | F_UNSEEN
						     | ((F_ON(F_TAB_TO_NEW,
							      state))
							 ? 0 : F_OR_FLAG)),
						    stream, start, &flags);
		    /*
		     * If we got a match, new_msgno is a message in
		     * a different thread from the one we are viewing.
		     */
		    if(flags & NSF_FLAG_MATCH){
			thrd = fetch_thread(stream, mn_m2raw(msgmap,new_msgno));
			if(thrd && thrd->top)
			  topthrd = fetch_thread(stream, thrd->top);

			if(F_OFF(F_AUTO_OPEN_NEXT_UNREAD, state)){
			    static ESCKEY_S next_opt[] = {
				{'y', 'y', "Y", "Yes"},
				{'n', 'n', "N", "No"},
				{TAB, 'n', "Tab", "NextNew"},
				{-1, 0, NULL, NULL}
			    };

			    if(in_index)
			      sprintf(prompt, "View thread number %.10s? ",
				     topthrd ? comatose(topthrd->thrdno) : "?");
			    else
			      sprintf(prompt,
				     "View message in thread number %.10s? ",
				     topthrd ? comatose(topthrd->thrdno) : "?");
				    
			    ret = radio_buttons(prompt, -FOOTER_ROWS(state),
						next_opt, 'y', 'x', NO_HELP,
						RB_NORM);
			    if(ret == 'x'){
				cmd_cancelled(NULL);
				goto get_out;
			    }
			}
			else
			  ret = 'y';

			if(ret == 'y'){
			    unview_thread(state, stream, msgmap);
			    mn_set_cur(msgmap, new_msgno);
			    if(THRD_AUTO_VIEW()){

				if(count_lflags_in_thread(stream, topthrd,
				                         msgmap, MN_NONE) == 1){
				    if(view_thread(state, stream, msgmap, 1)){
					state->view_skipped_index = 1;
					command = MC_VIEW_TEXT;
					goto view_text;
				    }
				}
			    }

			    view_thread(state, stream, msgmap, 1);
			    state->next_screen = SCREEN_FUN_NULL;
			    break;
			}
			else if(ret == 'n' && topthrd){
			    /*
			     * skip to end of this thread and look starting
			     * in the next thread.
			     */
			    if(mn_get_revsort(msgmap)){
				/*
				 * if reversed, top of thread is last one
				 * before next thread
				 */
				start = mn_raw2m(msgmap, topthrd->rawno);
			    }
			    else{
				/*
				 * last msg of thread is at the ends of
				 * the branches/nexts
				 */
				thrd = topthrd;
				while(thrd){
				    start = mn_raw2m(msgmap, thrd->rawno);
				    if(thrd->branch)
				      thrd = fetch_thread(stream, thrd->branch);
				    else if(thrd->next)
				      thrd = fetch_thread(stream, thrd->next);
				    else
				      thrd = NULL;
				}
			    }
			}
			else if(ret == 'n')
			  break;
		    }
		    else
		      break;
		}
	    }
	}
	else{

	    start = mn_get_cur(msgmap);

	    if(THREADING()){
		PINETHRD_S *thrd;
		long        rawno;
		int         collapsed;

		/*
		 * If we are on a collapsed thread, start looking after the
		 * collapsed part.
		 */
		rawno = mn_m2raw(msgmap, start);
		thrd = fetch_thread(stream, rawno);
		collapsed = thrd && thrd->next
			    && get_lflag(stream, NULL, rawno, MN_COLL);

		if(collapsed){
		    if(mn_get_revsort(msgmap)){
			if(thrd && thrd->top)
			  start = mn_raw2m(msgmap, thrd->top);
		    }
		    else{
			while(thrd){
			    start = mn_raw2m(msgmap, thrd->rawno);
			    if(thrd->branch)
			      thrd = fetch_thread(stream, thrd->branch);
			    else if(thrd->next)
			      thrd = fetch_thread(stream, thrd->next);
			    else
			      thrd = NULL;
			}
		    }

		}
	    }

	    new_msgno = next_sorted_flagged((F_UNDEL 
					     | F_UNSEEN
					     | ((F_ON(F_TAB_TO_NEW,state))
						 ? 0 : F_OR_FLAG)),
					    stream, start, &flags);
	}

	/*
	 * If there weren't any unread messages left, OR there
	 * aren't any messages at all, we may want to offer to
	 * go on to the next folder...
	 */
	if(flags & NSF_FLAG_MATCH){
	    mn_set_cur(msgmap, new_msgno);
	    if(in_index != View)
	      adjust_cur_to_visible(stream, msgmap);
	}
	else{
	    int  in_inbox = !strucmp(state->cur_folder,state->inbox_name);

	    if(state->context_current
	       && ((NEWS_TEST(state->context_current)
		    && context_isambig(state->cur_folder))
		   || ((state->context_current->use & CNTXT_INCMNG)
		       && (in_inbox
			   || folder_index(state->cur_folder,
					   state->context_current,
					   FI_FOLDER) >= 0)))){
		char	    nextfolder[MAXPATH];
		MAILSTREAM *nextstream = NULL;
		long	    recent_cnt;
		int         did_cancel = 0;

		strncpy(nextfolder, state->cur_folder, sizeof(nextfolder));
		nextfolder[sizeof(nextfolder)-1] = '\0';
		while(1){
		    if(!(next_folder(&nextstream, nextfolder, nextfolder,
				     state->context_current, &recent_cnt,
				     F_ON(F_TAB_NO_CONFIRM,state)
				       ? NULL : &did_cancel))){
			if(!in_inbox){
			    static ESCKEY_S inbox_opt[] = {
				{'y', 'y', "Y", "Yes"},
				{'n', 'n', "N", "No"},
				{TAB, 'z', "Tab", "To Inbox"},
				{-1, 0, NULL, NULL}
			    };

			    if(F_ON(F_RET_INBOX_NO_CONFIRM,state))
			      ret = 'y';
			    else{
				sprintf(prompt,
					"No more %ss.  Return to \"%.*s\"? ",
					(state->context_current->use&CNTXT_INCMNG)
					  ? "incoming folder" : "news group", 
					sizeof(prompt)-40, state->inbox_name);

				ret = radio_buttons(prompt, -FOOTER_ROWS(state),
						    inbox_opt, 'y', 'x',
						    NO_HELP, RB_NORM);
			    }

			    /*
			     * 'z' is a synonym for 'y'.  It is not 'y'
			     * so that it isn't displayed as a default
			     * action with square-brackets around it
			     * in the keymenu...
			     */
			    if(ret == 'y' || ret == 'z'){
				visit_folder(state, state->inbox_name,
					     state->context_current,
					     NULL);
				a_changed = TRUE;
			    }
			}
			else if (did_cancel)
			  cmd_cancelled(NULL);			
			else
			  q_status_message1(SM_ORDER, 0, 2, "No more %.200ss",
				     (state->context_current->use&CNTXT_INCMNG)
				        ? "incoming folder" : "news group");

			break;
		    }

		    {char *front, type[80], cnt[80], fbuf[MAX_SCREEN_COLS/2+1];
		     int rbspace, avail, need, take_back;

			/*
			 * View_next_
			 * Incoming_folder_ or news_group_ or folder_ or group_
			 * "foldername"
			 * _(13 recent) or _(some recent) or nothing
			 * ?_
			 */
			front = "View next";
			strncpy(type,
				(state->context_current->use & CNTXT_INCMNG)
				    ? "Incoming folder" : "news group",
				sizeof(type));
			sprintf(cnt, " (%.*s recent)", sizeof(cnt)-20,
				recent_cnt ? long2string(recent_cnt) : "some");

			/*
			 * Space reserved for radio_buttons call.
			 * If we make this 3 then radio_buttons won't mess
			 * with the prompt. If we make it 2, then we get
			 * one more character to use but radio_buttons will
			 * cut off the last character of our prompt, which is
			 * ok because it is a space.
			 */
			rbspace = 2;
			avail = ps_global->ttyo ? ps_global->ttyo->screen_cols
						: 80;
			need = strlen(front)+1 + strlen(type)+1 +
			       + strlen(nextfolder)+2 + strlen(cnt) +
			       2 + rbspace;
			if(avail < need){
			    take_back = strlen(type);
			    strncpy(type,
				    (state->context_current->use & CNTXT_INCMNG)
					? "folder" : "group", sizeof(type));
			    take_back -= strlen(type);
			    need -= take_back;
			    if(avail < need){
				need -= strlen(cnt);
				cnt[0] = '\0';
			    }
			}

			sprintf(prompt, "%.*s %.*s \"%.*s\"%.*s? ",
				sizeof(prompt)/8, front,
				sizeof(prompt)/8, type,
				sizeof(prompt)/2,
				short_str(nextfolder, fbuf,
					  strlen(nextfolder) -
					    ((need>avail) ? (need-avail) : 0),
					  MidDots),
				sizeof(prompt)/8, cnt);
		    }

		    /*
		     * When help gets added, this'll have to become
		     * a loop like the rest...
		     */
		    if(F_OFF(F_AUTO_OPEN_NEXT_UNREAD, state)){
			static ESCKEY_S next_opt[] = {
			    {'y', 'y', "Y", "Yes"},
			    {'n', 'n', "N", "No"},
			    {TAB, 'n', "Tab", "NextNew"},
			    {-1, 0, NULL, NULL}
			};

			ret = radio_buttons(prompt, -FOOTER_ROWS(state),
					    next_opt, 'y', 'x', NO_HELP,
					    RB_NORM);
			if(ret == 'x'){
			    cmd_cancelled(NULL);
			    break;
			}
		    }
		    else
		      ret = 'y';

		    if(ret == 'y'){
			visit_folder(state, nextfolder,
				     state->context_current, nextstream);
			/* visit_folder takes care of nextstream */
			nextstream = NULL;
			a_changed = TRUE;
			break;
		    }
		}

		if(nextstream)
		  pine_mail_close(nextstream);
	    }
	    else
	      any_messages(NULL,
			   (mn_get_total(msgmap) > 0L)
			     ? IS_NEWS(stream) ? "more undeleted" : "more new"
			     : NULL,
			   NULL);
	}

get_out:

	break;


          /*------- Zoom -----------*/
      case MC_ZOOM :
	/*
	 * Right now the way zoom is implemented is sort of silly.
	 * There are two per-message flags where just one and a 
	 * global "zoom mode" flag to suppress messags from the index
	 * should suffice.
	 */
	if(any_messages(msgmap, NULL, "to Zoom on")){
	    if(unzoom_index(state, stream, msgmap)){
		dprint(4, (debugfile, "\n\n ---- Exiting ZOOM mode ----\n"));
		q_status_message(SM_ORDER,0,2, "Index Zoom Mode is now off");
	    }
	    else if(i = zoom_index(state, stream, msgmap)){
		if(any_lflagged(msgmap, MN_HIDE)){
		    dprint(4,(debugfile,"\n\n ---- Entering ZOOM mode ----\n"));
		    q_status_message4(SM_ORDER, 0, 2,
				      "In Zoomed Index of %.200s%.200s%.200s%.200s.  Use \"Z\" to restore regular Index",
				      THRD_INDX() ? "" : comatose(i),
				      THRD_INDX() ? "" : " ",
				      THRD_INDX() ? "threads" : "message",
				      THRD_INDX() ? "" : plural(i));
		}
		else
		  q_status_message(SM_ORDER, 0, 2,
		     "All messages selected, so not entering Index Zoom Mode");
	    }
	    else
	      any_messages(NULL, "selected", "to Zoom on");
	}

	break;


          /*---------- print message on paper ----------*/
      case MC_PRINTMSG :
	if(any_messages(msgmap, NULL, "to print"))
	  cmd_print(state, msgmap, 0, in_index);

	break;


          /*---------- Take Address ----------*/
      case MC_TAKE :
	if(F_ON(F_ENABLE_ROLE_TAKE, state) ||
	   any_messages(msgmap, NULL, "to Take address from"))
	  cmd_take_addr(state, msgmap, 0);

	break;


          /*---------- Save Message ----------*/
      case MC_SAVE :
	if(any_messages(msgmap, NULL, "to Save"))
	  cmd_save(state, stream, msgmap, 0, in_index);

	break;


          /*---------- Export message ----------*/
      case MC_EXPORT :
	if(any_messages(msgmap, NULL, "to Export")){
	    cmd_export(state, msgmap, question_line, 0);
	    state->mangled_footer = 1;
	}

	break;


          /*---------- Expunge ----------*/
      case MC_EXPUNGE :
	dprint(2, (debugfile, "\n - expunge -\n"));
	if(IS_NEWS(stream) && stream->rdonly){
	    if((del_count = count_flagged(stream, F_DEL)) > 0L){
		state->mangled_footer = 1;
		sprintf(prompt, "Exclude %ld message%s from %.*s", del_count,
			plural(del_count), sizeof(prompt)-40,
			pretty_fn(state->cur_folder));
		if(F_ON(F_FULL_AUTO_EXPUNGE, state)
		   || (F_ON(F_AUTO_EXPUNGE, state)
		       && (state->context_current
		           && (state->context_current->use & CNTXT_INCMNG))
		       && context_isambig(state->cur_folder))
		   || want_to(prompt, 'y', 0, NO_HELP, WT_NORM) == 'y'){
		    if(F_ON(F_NEWS_CROSS_DELETE, ps_global))
		      cross_delete_crossposts(stream);
		    msgno_exclude_deleted(stream, msgmap);
		    clear_index_cache();

		    /*
		     * This is kind of surprising at first. For most sort
		     * orders, if the whole set is sorted, then any subset
		     * is also sorted. Not so for threaded sorts.
		     */
		    if(SORT_IS_THREADED())
		      refresh_sort(msgmap, SRT_NON);

		    state->mangled_body = 1;
		    state->mangled_header = 1;
		    q_status_message2(SM_ORDER, 0, 4,
				      "%.200s message%.200s excluded",
				      long2string(del_count),
				      plural(del_count));
		}
		else
		  any_messages(NULL, NULL, "Excluded");
	    }
	    else
	      any_messages(NULL, "deleted", "to Exclude");

	    break;
	}
	else if(READONLY_FOLDER){
	    q_status_message(SM_ORDER, 0, 4,
			     "Can't expunge. Folder is read-only");
	    break;
	}

	mail_expunge_prefilter(stream);

	if(del_count = count_flagged(stream, F_DEL)){
	    int ret;

	    sprintf(prompt, "Expunge %ld message%s from %.*s", del_count,
		    plural(del_count), sizeof(prompt)-40,
		    pretty_fn(state->cur_folder));
	    state->mangled_footer = 1;
	    if(F_ON(F_FULL_AUTO_EXPUNGE, state)
	       || (F_ON(F_AUTO_EXPUNGE, state)
		   && ((!strucmp(state->cur_folder,state->inbox_name))
		       || (state->context_current->use & CNTXT_INCMNG))
		   && context_isambig(state->cur_folder))
	       || (ret=want_to(prompt, 'y', 0, NO_HELP, WT_NORM)) == 'y')
	      ret = 'y';

	    if(ret == 'x')
	      cmd_cancelled("Expunge");

	    if(ret != 'y')
	      break;
	}

	dprint(8,(debugfile, "Expunge max:%ld cur:%ld kill:%d\n",
		  mn_get_total(msgmap), mn_get_cur(msgmap), del_count));

	old_max_msgno = mn_get_total(msgmap);
	lastc = pico_set_colors(ps_global->VAR_TITLE_FORE_COLOR,
				ps_global->VAR_TITLE_BACK_COLOR,
				PSC_REV|PSC_RET);

	PutLine0(0, 0, "**");			/* indicate delay */

	if(lastc){
	    (void)pico_set_colorp(lastc, PSC_NONE);
	    free_color_pair(&lastc);
	}

	MoveCursor(state->ttyo->screen_rows -FOOTER_ROWS(state), 0);
	fflush(stdout);

	we_cancel = busy_alarm(1, "Expunging", NULL, 0);
	delete_filtered_msgs(stream);
	ps_global->expunge_in_progress = 1;
	mail_expunge(stream);
	ps_global->expunge_in_progress = 0;
	if(we_cancel)
	  cancel_busy_alarm((state->expunge_count > 0) ? 0 : -1);

	dprint(2,(debugfile,"expunge complete cur:%ld max:%ld\n",
		  mn_get_cur(msgmap), mn_get_total(msgmap)));
	/*
	 * This is only actually necessary if this causes the width of the
	 * message number field to change.  That is, it depends on the
	 * format the user is using as well as on the max_msgno.  Since it
	 * should be rare, we'll just do it whenever it happens.
	 * Also have to check for an increase in max_msgno on new mail.
	 */
	if(old_max_msgno >= 1000L && mn_get_total(msgmap) < 1000L
	   || old_max_msgno >= 10000L && mn_get_total(msgmap) < 10000L
	   || old_max_msgno >= 100000L && mn_get_total(msgmap) < 100000L){
	    clear_index_cache();
	    state->mangled_body = 1;
	}

	/*
	 * mm_exists and mm_expunge take care of updating max_msgno
	 * and selecting a new message should the selected get removed
	 */
	reset_check_point();
	lastc = pico_set_colors(ps_global->VAR_TITLE_FORE_COLOR,
				ps_global->VAR_TITLE_BACK_COLOR,
				PSC_REV|PSC_RET);
	PutLine0(0, 0, "  ");			/* indicate delay's over */

	if(lastc){
	    (void)pico_set_colorp(lastc, PSC_NONE);
	    free_color_pair(&lastc);
	}

	fflush(stdout);

	if(state->expunge_count > 0){
	    /*
	     * This is kind of surprising at first. For most sort
	     * orders, if the whole set is sorted, then any subset
	     * is also sorted. Not so for threaded sorts.
	     */
	    if(SORT_IS_THREADED())
	      refresh_sort(msgmap, SRT_NON);
	}
	else{
	    if(del_count)
	      q_status_message1(SM_ORDER, 0, 3,
			        "No messages expunged from folder \"%.200s\"",
			        pretty_fn(state->cur_folder));
	    else
	      q_status_message(SM_ORDER, 0, 3,
			 "No messages marked deleted.  No messages expunged.");
	}

	break;


          /*------- Unexclude -----------*/
      case MC_UNEXCLUDE :
	if(!(IS_NEWS(stream) && stream->rdonly)){
	    q_status_message(SM_ORDER, 0, 3,
			     "Unexclude not available for mail folders");
	}
	else if(any_lflagged(msgmap, MN_EXLD)){
	    SEARCHPGM *pgm;
	    long       i;
	    int	       exbits;

	    /*
	     * Since excluded means "hidden deleted" and "killed",
	     * the count should reflect the former.
	     */
	    pgm = mail_newsearchpgm();
	    pgm->deleted = 1;
	    mail_search_full(stream, NULL, pgm, SE_NOPREFETCH | SE_FREE);
	    for(i = 1L, del_count = 0L; i <= stream->nmsgs; i++)
	      if(mail_elt(stream, i)->searched
		 && get_lflag(stream, NULL, i, MN_EXLD)
		 && !(msgno_exceptions(stream, i, "0", &exbits, FALSE)
		      && (exbits & MSG_EX_FILTERED)))
		del_count++;

	    if(del_count > 0L){
		state->mangled_footer = 1;
		sprintf(prompt, "UNexclude %ld message%s in %.*s", del_count,
			plural(del_count), sizeof(prompt)-40,
			pretty_fn(state->cur_folder));
		if(F_ON(F_FULL_AUTO_EXPUNGE, state)
		   || (F_ON(F_AUTO_EXPUNGE, state)
		       && (state->context_current
			   && (state->context_current->use & CNTXT_INCMNG))
		       && context_isambig(state->cur_folder))
		   || want_to(prompt, 'y', 0, NO_HELP, WT_NORM) == 'y'){
		    long save_cur_rawno;
		    int  were_viewing_a_thread;

		    save_cur_rawno = mn_m2raw(msgmap, mn_get_cur(msgmap));
		    were_viewing_a_thread = (THREADING()
					     && state->viewing_a_thread);

		    msgno_include(stream, msgmap, FALSE);
		    clear_index_cache();

		    if(stream && stream->spare)
		      erase_threading_info(stream, msgmap);

		    refresh_sort(msgmap, SRT_NON);

		    if(were_viewing_a_thread){
			if(save_cur_rawno > 0L)
			  mn_set_cur(msgmap, mn_raw2m(msgmap,save_cur_rawno));

			view_thread(state, stream, msgmap, 1);
		    }

		    if(save_cur_rawno > 0L)
		      mn_set_cur(msgmap, mn_raw2m(msgmap,save_cur_rawno));

		    state->mangled_screen = 1;
		    q_status_message2(SM_ORDER, 0, 4,
				      "%.200s message%.200s UNexcluded",
				      long2string(del_count),
				      plural(del_count));

		    if(in_index != View)
		      adjust_cur_to_visible(stream, msgmap);
		}
		else
		  any_messages(NULL, NULL, "UNexcluded");
	    }
	    else
	      any_messages(NULL, "excluded", "to UNexclude");
	}
	else
	  any_messages(NULL, "excluded", "to UNexclude");

	break;


          /*------- Make Selection -----------*/
      case MC_SELECT :
	if(any_messages(msgmap, NULL, "to Select")){
	    if(THRD_INDX())
	      thread_index_select(state, msgmap, question_line, in_index);
	    else
	      aggregate_select(state, msgmap, question_line, in_index);

	    if((in_index == MsgIndx || in_index == ThrdIndx)
	       && any_lflagged(msgmap, MN_SLCT) > 0L
	       && !any_lflagged(msgmap, MN_HIDE)
	       && F_ON(F_AUTO_ZOOM, state))
	      (void) zoom_index(state, stream, msgmap);
	}

	break;


          /*------- Toggle Current Message Selection State -----------*/
      case MC_SELCUR :
	if(any_messages(msgmap, NULL, NULL)
	   && (individual_select(state, msgmap, question_line, in_index)
	       || (F_OFF(F_UNSELECT_WONT_ADVANCE, state)
	           && !any_lflagged(msgmap, MN_HIDE)))
	   && (i = mn_get_cur(msgmap)) < mn_get_total(msgmap)){
	    /* advance current */
	    mn_inc_cur(stream, msgmap,
		       (in_index == View && THREADING()
		        && state->viewing_a_thread)
			 ? MH_THISTHD
			 : (in_index == View)
			   ? MH_ANYTHD : MH_NONE);
	}

	break;


          /*------- Apply command -----------*/
      case MC_APPLY :
	if(any_messages(msgmap, NULL, NULL)){
	    if(any_lflagged(msgmap, MN_SLCT) > 0L){
		if(apply_command(state, stream, msgmap, 0,
				 AC_NONE, question_line)
		   && F_ON(F_AUTO_UNZOOM, state))
		  unzoom_index(state, stream, msgmap);
	    }
	    else
	      any_messages(NULL, NULL, "to Apply command to.  Try \"Select\"");
	}

	break;


          /*-------- Sort command -------*/
      case MC_SORT :
	{
	    int were_threading = THREADING();
	    SortOrder sort = mn_get_sort(msgmap);
	    int	      rev  = mn_get_revsort(msgmap);

	    dprint(1, (debugfile,"MAIL_CMD: sort\n"));		    
	    if(select_sort(state, question_line, &sort, &rev)){
		/* $ command reinitializes threading collapsed/expanded info */
		if(SORT_IS_THREADED() && !SEP_THRDINDX())
		  erase_threading_info(stream, msgmap);

		sort_folder(ps_global->msgmap, sort, rev, SRT_VRB|SRT_MAN);
	    }

	    state->mangled_footer = 1;

	    /*
	     * We've changed whether we are threading or not so we need to
	     * exit the index and come back in so that we switch between the
	     * thread index and the regular index. Sort_folder will have
	     * reset viewing_a_thread if necessary.
	     */
	    if(SEP_THRDINDX()
	       && ((!were_threading && THREADING())
	            || (were_threading && !THREADING()))){
		state->next_screen = mail_index_screen;
		state->mangled_screen = 1;
	    }
	}

	break;


          /*------- Toggle Full Headers -----------*/
      case MC_FULLHDR :
	state->full_header = !state->full_header;
	q_status_message3(SM_ORDER, 0, 3,
    "Display of full headers is now o%.200s.  Use %.200s to turn back o%.200s",
			    state->full_header ? "n" : "ff",
			    F_ON(F_USE_FK, state) ? "F9" : "H",
			    !state->full_header ? "n" : "ff");
	a_changed = TRUE;
	break;


          /*------- Bounce -----------*/
      case MC_BOUNCE :
	cmd_bounce(state, msgmap, 0);
	break;


          /*------- Flag -----------*/
      case MC_FLAG :
	dprint(4, (debugfile, "\n - flag message -\n"));
	cmd_flag(state, msgmap, 0);
	break;


          /*------- Pipe message -----------*/
      case MC_PIPE :
	cmd_pipe(state, msgmap, 0);
	break;


          /*--------- Default, unknown command ----------*/
      default:
	panic("Unexpected command case");
	break;
    }

    return(a_changed || mn_get_cur(msgmap) != old_msgno);
}



/*----------------------------------------------------------------------
   Complain about bogus input

  Args: ch -- input command to complain about
	help -- string indicating where to get help

 ----*/
void
bogus_command(cmd, help)
    int   cmd;
    char *help;
{
    if(cmd == ctrl('Q') || cmd == ctrl('S'))
      q_status_message1(SM_ASYNC, 0, 2,
 "%.200s char received.  Set \"preserve-start-stop\" feature in Setup/Config.",
			pretty_command(cmd));
    else if(cmd == KEY_JUNK)
      q_status_message3(SM_ORDER, 0, 2,
		      "Invalid key pressed.%.200s%.200s%.200s",
		      (help) ? " Use " : "",
		      (help) ?  help   : "",
		      (help) ? " for help" : "");
    else
      q_status_message4(SM_ORDER, 0, 2,
	  "Command \"%.200s\" not defined for this screen.%.200s%.200s%.200s",
		      pretty_command(cmd),
		      (help) ? " Use " : "",
		      (help) ?  help   : "",
		      (help) ? " for help" : "");
}


/*----------------------------------------------------------------------
   Reveal Keymenu to Pine Command mappings

  Args: 

 ----*/
int
menu_command(keystroke, menu)
    int keystroke;
    struct key_menu *menu;
{
    int i, n;

    if(!menu)
      return(MC_UNKNOWN);

    if(F_ON(F_USE_FK,ps_global)){
	/* No alpha commands permitted in function key mode */
	if(keystroke < 0x0100 && isalpha((unsigned char) keystroke))
	  return(MC_UNKNOWN);

	/* Tres simple: compute offset, and test */
	if(keystroke >= F1 && keystroke <= F12){
	    n = (menu->which * 12) + (keystroke - F1);
	    if(bitnset(n, menu->bitmap))
	      return(menu->keys[n].bind.cmd);
	}
    }
    else if(keystroke >= F1 && keystroke <= F12)
      return(MC_UNKNOWN);

    /* if ascii, coerce lower case */
    if(keystroke < 0x0100 && isupper((unsigned char) keystroke))
      keystroke = tolower((unsigned char) keystroke);

    /* keep this here for Windows port */
    if((keystroke = validatekeys(keystroke)) == KEY_JUNK)
      return(MC_UNKNOWN);

    /* Scan the list for any keystroke/command binding */
    for(i = (menu->how_many * 12) - 1;  i >= 0; i--)
      if(bitnset(i, menu->bitmap))
	for(n = menu->keys[i].bind.nch - 1; n >= 0; n--)
	  if(keystroke == menu->keys[i].bind.ch[n])
	    return(menu->keys[i].bind.cmd);

    /*
     * If explicit mapping failed, check feature mappings and
     * hardwired defaults...
     */
    if(F_ON(F_ENABLE_PRYNT,ps_global)
	&& (keystroke == 'y' || keystroke == 'Y')){
	/* SPECIAL CASE: Scan the list for print bindings */
	for(i = (menu->how_many * 12) - 1;  i >= 0; i--)
	  if(bitnset(i, menu->bitmap))
	    if(menu->keys[i].bind.cmd == MC_PRINTMSG
	       || menu->keys[i].bind.cmd == MC_PRINTTXT)
	      return(menu->keys[i].bind.cmd);
    }

    if(F_ON(F_ENABLE_LESSTHAN_EXIT,ps_global)
       && (keystroke == '<' || keystroke == ','
	   || (F_ON(F_ARROW_NAV,ps_global) && keystroke == KEY_LEFT))){
	/* SPECIAL CASE: Scan the list for MC_EXIT bindings */
	for(i = (menu->how_many * 12) - 1;  i >= 0; i--)
	  if(bitnset(i, menu->bitmap))
	    if(menu->keys[i].bind.cmd == MC_EXIT)
	      return(MC_EXIT);
    }

    /*
     * If no match after scanning bindings, try universally
     * bound keystrokes...
     */
    switch(keystroke){
      case KEY_MOUSE :
	return(MC_MOUSE);

      case ctrl('P') :
      case KEY_UP :
	return(MC_CHARUP);

      case ctrl('N') :
      case KEY_DOWN :
	return(MC_CHARDOWN);

      case ctrl('F') :
      case KEY_RIGHT :
	return(MC_CHARRIGHT);

      case ctrl('B') :
      case KEY_LEFT :
	return(MC_CHARLEFT);

      case ctrl('A') :
	return(MC_GOTOBOL);

      case ctrl('E') :
	return(MC_GOTOEOL);

      case  ctrl('L') :
	return(MC_REPAINT);

      case KEY_RESIZE :
	return(MC_RESIZE);

      case NO_OP_IDLE:
      case NO_OP_COMMAND:
	if(USER_INPUT_TIMEOUT(ps_global))
	  user_input_timeout_exit(ps_global->hours_to_timeout); /* no return */

	return(MC_NONE);

      default :
	break;
    }

    return(MC_UNKNOWN);			/* utter failure */
}



/*----------------------------------------------------------------------
   Set up a binding for cmd, with one key bound to it.
   Use menu_add_binding to add more keys to this binding.

  Args: menu   -- the keymenu
	key    -- the initial key to bind to
	cmd    -- the command to initialize to
	name   -- a pointer to the string to point name to
	label  -- a pointer to the string to point label to
	keynum -- which key in the keys array to initialize

 ----*/
void
menu_init_binding(menu, key, cmd, name, label, keynum)
    struct key_menu *menu;
    int              key, cmd;
    char            *name, *label;
    int              keynum;
{
    /* if ascii, coerce to lower case */
    if(key < 0x0100 && isupper((unsigned char)key))
      key = tolower((unsigned char)key);

    /* remove binding from any other key */
    menu_clear_cmd_binding(menu, cmd);

    menu->keys[keynum].name     = name;
    menu->keys[keynum].label    = label;
    menu->keys[keynum].bind.cmd = cmd;
    menu->keys[keynum].bind.nch = 0;
    menu->keys[keynum].bind.ch[menu->keys[keynum].bind.nch++] = key;
}


/*----------------------------------------------------------------------
   Add a key/command binding to the given keymenu structure

  Args: 

 ----*/
void
menu_add_binding(menu, key, cmd)
    struct key_menu *menu;
    int		     key, cmd;
{
    int i, n;

    /* NOTE: cmd *MUST* already have had a binding */
    for(i = (menu->how_many * 12) - 1;  i >= 0; i--)
      if(menu->keys[i].bind.cmd == cmd){
	  for(n = menu->keys[i].bind.nch - 1;
	      n >= 0 && key != menu->keys[i].bind.ch[n];
	      n--)
	    ;

	  /* if ascii, coerce to lower case */
	  if(key < 0x0100 && isupper((unsigned char)key))
	    key = tolower((unsigned char)key);

	  if(n < 0)		/* not already bound, bind it */
	    menu->keys[i].bind.ch[menu->keys[i].bind.nch++] = key;

	  break;
      }
}


/*----------------------------------------------------------------------
   REMOVE a key/command binding from the given keymenu structure

  Args: 

 ----*/
int
menu_clear_binding(menu, key)
    struct key_menu *menu;
    int		     key;
{
    int i, n;

    /* if ascii, coerce to lower case */
    if(key < 0x0100 && isupper((unsigned char)key))
      key = tolower((unsigned char)key);

    for(i = (menu->how_many * 12) - 1;  i >= 0; i--)
      for(n = menu->keys[i].bind.nch - 1; n >= 0; n--)
	if(key == menu->keys[i].bind.ch[n]){
	    int cmd = menu->keys[i].bind.cmd;

	    for(--menu->keys[i].bind.nch; n < menu->keys[i].bind.nch; n++)
	      menu->keys[i].bind.ch[n] = menu->keys[i].bind.ch[n+1];

	    return(cmd);
	}

    return(MC_UNKNOWN);
}


void
menu_clear_cmd_binding(menu, cmd)
    struct key_menu *menu;
    int		     cmd;
{
    int i;

    for(i = (menu->how_many * 12) - 1;  i >= 0; i--){
	if(cmd == menu->keys[i].bind.cmd){
	    menu->keys[i].name     = NULL;
	    menu->keys[i].label    = NULL;
	    menu->keys[i].bind.cmd = 0;
	    menu->keys[i].bind.nch = 0;
	    menu->keys[i].bind.ch[0] = 0;
	}
    }
}


int
menu_binding_index(menu, cmd)
    struct key_menu *menu;
    int		     cmd;
{
    int i;

    for(i = 0; i < menu->how_many * 12; i++)
      if(cmd == menu->keys[i].bind.cmd)
	return(i);

    return(-1);
}


/*----------------------------------------------------------------------
   Complain about command on empty folder

  Args: map -- msgmap 
	type -- type of message that's missing
	cmd -- string explaining command attempted

 ----*/
int
any_messages(map, type, cmd)
    MSGNO_S *map;
    char *type, *cmd;
{
    if(mn_get_total(map) <= 0L){
	q_status_message5(SM_ORDER, 0, 2, "No %.200s%.200s%.200s%.200s%.200s",
			  type ? type : "",
			  type ? " " : "",
			  THRD_INDX() ? "threads" : "messages",
			  (!cmd || *cmd != '.') ? " " : "",
			  cmd ? cmd : "in folder");
	return(FALSE);
    }

    return(TRUE);
}


/*----------------------------------------------------------------------
   test whether or not we have a valid stream to set flags on

  Args: state -- pine state containing vital signs
	cmd -- string explaining command attempted
	permflag -- associated permanent flag state

  Result: returns 1 if we can set flags, otw 0 and complains

 ----*/
int
can_set_flag(state, cmd, permflag)
    struct pine *state;
    char	*cmd;
{
    if((!permflag && READONLY_FOLDER) || state->dead_stream){
	q_status_message2(SM_ORDER | (state->dead_stream ? SM_DING : 0), 0, 3,
			  "Can't %.200s message.  Folder is %.200s.", cmd,
			  (state->dead_stream) ? "closed" : "read-only");
	return(FALSE);
    }

    return(TRUE);
}



/*----------------------------------------------------------------------
   Complain about command on empty folder

  Args: type -- type of message that's missing
	cmd -- string explaining command attempted

 ----*/
void
cmd_cancelled(cmd)
    char *cmd;
{
    q_status_message1(SM_INFO, 0, 2, "%.200s cancelled", cmd ? cmd : "Command");
}
	

void
mail_expunge_prefilter(stream)
    MAILSTREAM *stream;
{
    int sfdo_state = 0,		/* Some Filter Depends On or Sets State  */
	sfdo_scores = 0,	/* Some Filter Depends On Scores */
	ssdo_state = 0;		/* Some Score Depends On State   */
    
    /*
     * An Expunge causes a re-examination of the filters to
     * see if any state changes have caused new matches.
     */
    
    sfdo_scores = (scores_are_used(SCOREUSE_GET) & SCOREUSE_FILTERS);
    if(sfdo_scores)
      ssdo_state = (scores_are_used(SCOREUSE_GET) & SCOREUSE_STATEDEP);

    if(!(sfdo_scores && ssdo_state))
      sfdo_state = some_filter_depends_on_state();


    if(sfdo_state || (sfdo_scores && ssdo_state)){
	if(sfdo_scores && ssdo_state)
	  clear_folder_scores(stream);

	if(stream == ps_global->mail_stream)
	  reprocess_filter_patterns(stream, ps_global->msgmap);
	else if(stream == ps_global->inbox_stream)
	  reprocess_filter_patterns(stream, ps_global->inbox_msgmap);
    }
}


/*----------------------------------------------------------------------
   Execute DELETE message command

  Args: state --  Various satate info
        msgmap --  map of c-client to local message numbers

 Result: with side effect of "current" message delete flag set

 ----*/
void
cmd_delete(state, msgmap, agg, in_index)
     struct pine *state;
     MSGNO_S     *msgmap;
     int	  agg;
     CmdWhere	  in_index;
{
    int	  lastmsg, opts;
    long  msgno, del_count = 0L, new;
    char *sequence = NULL, prompt[128];

    dprint(4, (debugfile, "\n - delete message -\n"));
    if(!(any_messages(msgmap, NULL, "to Delete")
	 && can_set_flag(state, "delete", state->mail_stream->perm_deleted)))
      return;

    if(state->io_error_on_stream) {
	state->io_error_on_stream = 0;
	mail_check(state->mail_stream);		/* forces write */
    }

    if(agg){
	sequence = selected_sequence(state->mail_stream, msgmap, &del_count);
	sprintf(prompt, "%ld%s message%s marked for deletion",
		del_count, (agg == 2) ? "" : " selected", plural(del_count));
    }
    else{
	long rawno;
	int  exbits = 0;

	msgno	  = mn_get_cur(msgmap);
	rawno     = mn_m2raw(msgmap, msgno);
	del_count = 1L;				/* return current */
	sequence  = cpystr(long2string(rawno));
	lastmsg	  = (msgno >= mn_get_total(msgmap));
	sprintf(prompt, "%s%s marked for deletion",
		lastmsg ? "Last message" : "Message ",
		lastmsg ? "" : long2string(msgno));

	/*
	 * Mark this message manually flagged so we don't re-filter it
	 * with a filter which only sets flags.
	 */
	if(msgno_exceptions(state->mail_stream, rawno, "0", &exbits, FALSE))
	  exbits |= MSG_EX_MANFLAGGED;
	else
	  exbits = MSG_EX_MANFLAGGED;

	msgno_exceptions(state->mail_stream, rawno, "0", &exbits, TRUE);
    }

    dprint(3,(debugfile, "DELETE: msg %s\n", sequence));
    new = state->new_mail_count;
    mail_flag(state->mail_stream, sequence, "\\DELETED", ST_SET);
    fs_give((void **) &sequence);
    if(new != state->new_mail_count)
      process_filter_patterns(state->mail_stream, state->msgmap,
			      state->new_mail_count);

    if(!agg){

	advance_cur_after_delete(state, state->mail_stream, msgmap, in_index);

	if(IS_NEWS(state->mail_stream)
		|| ((state->context_current->use & CNTXT_INCMNG)
		    && context_isambig(state->cur_folder))){

	    opts = (NSF_TRUST_FLAGS | NSF_SKIP_CHID);
	    if(in_index == View)
	      opts &= ~NSF_SKIP_CHID;

	    (void)next_sorted_flagged(F_UNDEL|F_UNSEEN, state->mail_stream,
				      msgno, &opts);
	    if(!(opts & NSF_FLAG_MATCH)){
		char nextfolder[MAXPATH];

		strncpy(nextfolder, state->cur_folder, sizeof(nextfolder));
		nextfolder[sizeof(nextfolder)-1] = '\0';
		strncat(prompt, next_folder(NULL, nextfolder, nextfolder,
					   state->context_current, NULL, NULL)
				  ? ".  Press TAB for next folder."
				  : ".  No more folders to TAB to.",
			sizeof(prompt)-1-strlen(prompt));
	    }
	}
    }

    q_status_message(SM_ORDER, 0, 3, prompt);
}


void
advance_cur_after_delete(state, stream, msgmap, in_index)
    struct pine *state;
    MAILSTREAM  *stream;
    MSGNO_S     *msgmap;
    CmdWhere     in_index;
{
    long new_msgno, msgno;
    int  opts;

    new_msgno = msgno = mn_get_cur(msgmap);
    opts = NSF_TRUST_FLAGS;

    if(F_ON(F_DEL_SKIPS_DEL, state)){

	if(THREADING() && state->viewing_a_thread)
	  opts |= NSF_SKIP_CHID;

	new_msgno = next_sorted_flagged(F_UNDEL, stream, msgno, &opts);
    }
    else{
	mn_inc_cur(stream, msgmap,
		   (in_index == View && THREADING()
		    && state->viewing_a_thread)
		     ? MH_THISTHD
		     : (in_index == View)
		       ? MH_ANYTHD : MH_NONE);
	new_msgno = mn_get_cur(msgmap);
	if(new_msgno != msgno)
	  opts |= NSF_FLAG_MATCH;
    }

    /*
     * Viewing_a_thread is the complicated case because we want to ignore
     * other threads at first and then look in other threads if we have to.
     * By ignoring other threads we also ignore collapsed partial threads
     * in our own thread.
     */
    if(THREADING() && state->viewing_a_thread && !(opts & NSF_FLAG_MATCH)){
	long rawno, orig_thrdno;
	PINETHRD_S *thrd, *topthrd = NULL;

	rawno = mn_m2raw(msgmap, msgno);
	thrd  = fetch_thread(stream, rawno);
	if(thrd && thrd->top)
	  topthrd = fetch_thread(stream, thrd->top);

	orig_thrdno = topthrd ? topthrd->thrdno : -1L;

	opts = NSF_TRUST_FLAGS;
	new_msgno = next_sorted_flagged(F_UNDEL, stream, msgno, &opts);

	/*
	 * If we got a match, new_msgno may be a message in
	 * a different thread from the one we are viewing, or it could be
	 * in a collapsed part of this thread.
	 */
	if(opts & NSF_FLAG_MATCH){
	    int         ret;
	    char        pmt[128];

	    topthrd = NULL;
	    thrd = fetch_thread(stream, mn_m2raw(msgmap,new_msgno));
	    if(thrd && thrd->top)
	      topthrd = fetch_thread(stream, thrd->top);
	    
	    /*
	     * If this match is in the same thread we're already in
	     * then we're done, else we have to ask the user and maybe
	     * switch threads.
	     */
	    if(!(orig_thrdno > 0L && topthrd
		 && topthrd->thrdno == orig_thrdno)){

		if(F_OFF(F_AUTO_OPEN_NEXT_UNREAD, state)){
		    if(in_index == View)
		      sprintf(pmt,
			     "View message in thread number %.10s",
			     topthrd ? comatose(topthrd->thrdno) : "?");
		    else
		      sprintf(pmt, "View thread number %.10s",
			     topthrd ? comatose(topthrd->thrdno) : "?");
			    
		    ret = want_to(pmt, 'y', 'x', NO_HELP, WT_NORM);
		}
		else
		  ret = 'y';

		if(ret == 'y'){
		    unview_thread(state, stream, msgmap);
		    mn_set_cur(msgmap, new_msgno);
		    if(THRD_AUTO_VIEW()
		       && (count_lflags_in_thread(stream, topthrd, msgmap,
						  MN_NONE) == 1)
		       && view_thread(state, stream, msgmap, 1)){
			state->view_skipped_index = 1;
			state->next_screen = mail_view_screen;
		    }
		    else{
			view_thread(state, stream, msgmap, 1);
			state->next_screen = SCREEN_FUN_NULL;
		    }
		}
		else
		  new_msgno = msgno;	/* stick with original */
	    }
	}
    }

    mn_set_cur(msgmap, new_msgno);
    if(in_index != View)
      adjust_cur_to_visible(stream, msgmap);
}



/*----------------------------------------------------------------------
   Execute UNDELETE message command

  Args: state --  Various satate info
        msgmap --  map of c-client to local message numbers

 Result: with side effect of "current" message delete flag UNset

 ----*/
void
cmd_undelete(state, msgmap, agg)
     struct pine *state;
     MSGNO_S     *msgmap;
     int	  agg;
{
    long	  del_count;
    char	 *sequence;
    int		  wasdeleted = FALSE;
    MESSAGECACHE *mc;

    dprint(4, (debugfile, "\n - undelete -\n"));
    if(!(any_messages(msgmap, NULL, "to Undelete")
	 && can_set_flag(state, "undelete", state->mail_stream->perm_deleted)))
      return;

    if(agg){
	del_count = 0L;				/* return current */
	sequence = selected_sequence(state->mail_stream, msgmap, &del_count);
    }
    else{
	long rawno;
	int  exbits = 0;

	del_count = 1L;				/* return current */
	rawno = mn_m2raw(msgmap, mn_get_cur(msgmap));
	sequence  = cpystr(long2string(rawno));
	wasdeleted = ((mc = mail_elt(state->mail_stream, rawno))
		       && mc->valid
		       && mc->deleted);
	/*
	 * Mark this message manually flagged so we don't re-filter it
	 * with a filter which only sets flags.
	 */
	if(msgno_exceptions(state->mail_stream, rawno, "0", &exbits, FALSE))
	  exbits |= MSG_EX_MANFLAGGED;
	else
	  exbits = MSG_EX_MANFLAGGED;

	msgno_exceptions(state->mail_stream, rawno, "0", &exbits, TRUE);
    }

    dprint(3,(debugfile, "UNDELETE: msg %s\n", sequence));

    mail_flag(state->mail_stream, sequence, "\\DELETED", 0L);
    fs_give((void **) &sequence);

    if(del_count == 1L && !agg){
	update_titlebar_status();
	q_status_message(SM_ORDER, 0, 3,
			wasdeleted
			 ? "Deletion mark removed, message won't be deleted"
			 : "Message not marked for deletion; no action taken");
    }
    else
      q_status_message2(SM_ORDER, 0, 3,
			"Deletion mark removed from %.200s message%.200s",
			comatose(del_count), plural(del_count));

    if(state->io_error_on_stream) {
	state->io_error_on_stream = 0;
	mail_check(state->mail_stream);		/* forces write */
    }
}



/*----------------------------------------------------------------------
   Execute FLAG message command

  Args: state --  Various satate info
        msgmap --  map of c-client to local message numbers

 Result: with side effect of "current" message FLAG flag set or UNset

 ----*/
void
cmd_flag(state, msgmap, agg)
    struct pine *state;
    MSGNO_S     *msgmap;
    int		 agg;
{
    char	  *flagit, *seq, *screen_text[20], **exp, **p, *answer = NULL;
    long	   unflagged, flagged, flags;
    MESSAGECACHE  *mc = NULL;
    struct	   flag_table  *fp;
    struct flag_screen flag_screen;
    static char   *flag_screen_text1[] = {
	"    Set desired flags for current message below.  An 'X' means set",
	"    it, and a ' ' means to unset it.  Choose \"Exit\" when finished.",
	NULL
    };
    static char   *flag_screen_text2[] = {
	"    Set desired flags below for selected messages.  A '?' means to",
	"    leave the flag unchanged, 'X' means to set it, and a ' ' means",
	"    to unset it.  Use the \"Return\" key to toggle, and choose",
	"    \"Exit\" when finished.",
	NULL
    };
    static char   *flag_screen_boiler_plate[] = {
	"",
	"            Set        Flag Name",
	"            ---   ----------------------",
	NULL
    };
    static struct  flag_table ftbl[] = {
	/*
	 * At some point when user defined flags are possible,
	 * it should just be a simple matter of grabbing this
	 * array from the heap and explicitly filling the
	 * non-system flags in at run time...
	 *  {NULL, h_flag_user, F_USER, 0, 0},
	 */
	{"Important", h_flag_important, F_FLAG, 0, 0},
	{"New",	  h_flag_new, F_SEEN, 0, 0},
	{"Answered",  h_flag_answered, F_ANS, 0, 0},
	{"Deleted",   h_flag_deleted, F_DEL, 0, 0},
	{NULL, NO_HELP, 0, 0, 0}
    };

    /* Only check for dead stream for now.  Should check permanent flags
     * eventually
     */
    if(!(any_messages(msgmap, NULL, "to Flag")
	 && can_set_flag(state, "flag", 1)))
      return;

    if(state->io_error_on_stream) {
	state->io_error_on_stream = 0;
	mail_check(state->mail_stream); /* forces write */
	return;
    }

    flag_screen.flag_table  = ftbl;
    flag_screen.explanation = screen_text;
    if(agg){
	if(!pseudo_selected(msgmap))
	  return;

	exp = flag_screen_text2;
	for(fp = ftbl; fp->name; fp++){
	    fp->set = CMD_FLAG_UNKN;		/* set to unknown */
	    fp->ukn = TRUE;
	}
    }
    else if(mc = mail_elt(state->mail_stream,
			  mn_m2raw(msgmap, mn_get_cur(msgmap)))){
	exp = flag_screen_text1;
	for(fp = &ftbl[0]; fp->name; fp++){
	    fp->ukn = 0;
	    fp->set = ((fp->flag == F_SEEN && !mc->seen)
		       || (fp->flag == F_DEL && mc->deleted)
		       || (fp->flag == F_FLAG && mc->flagged)
		       || (fp->flag == F_ANS && mc->answered))
			? CMD_FLAG_SET : CMD_FLAG_CLEAR;
	}
    }
    else{
	q_status_message(SM_ORDER | SM_DING, 3, 4,
			 "Error accessing message data");
	return;
    }

#ifdef _WINDOWS
    if (mswin_usedialog ()) {
	if (!os_flagmsgdialog (&ftbl[0]))
	  return;
    }
    else
#endif	    
    if(F_ON(F_FLAG_SCREEN_DFLT, ps_global)
       || !cmd_flag_prompt(state, &flag_screen)){
	screen_text[0] = "";
	for(p = &screen_text[1]; *exp; p++, exp++)
	  *p = *exp;

	for(exp = flag_screen_boiler_plate; *exp; p++, exp++)
	  *p = *exp;

	*p = NULL;

	flag_maintenance_screen(state, &flag_screen);
    }

    /* reaquire the elt pointer */
    mc = mail_elt(state->mail_stream,mn_m2raw(msgmap,mn_get_cur(msgmap)));

    for(fp = ftbl; fp->name; fp++){
	flags = -1;
	switch(fp->flag){
	  case F_SEEN:
	    if((!agg && fp->set != !mc->seen)
	       || (agg && fp->set != CMD_FLAG_UNKN)){
		flagit = "\\SEEN";
		if(fp->set){
		    flags     = 0L;
		    unflagged = F_SEEN;
		}
		else{
		    flags     = ST_SET;
		    unflagged = F_UNSEEN;
		}
	    }

	    break;

	  case F_ANS:
	    if((!agg && fp->set != mc->answered)
	       || (agg && fp->set != CMD_FLAG_UNKN)){
		flagit = "\\ANSWERED";
		if(fp->set){
		    flags     = ST_SET;
		    unflagged = F_UNANS;
		}
		else{
		    flags     = 0L;
		    unflagged = F_ANS;
		}
	    }

	    break;

	  case F_DEL:
	    if((!agg && fp->set != mc->deleted)
	       || (agg && fp->set != CMD_FLAG_UNKN)){
		flagit = "\\DELETED";
		if(fp->set){
		    flags     = ST_SET;
		    unflagged = F_UNDEL;
		}
		else{
		    flags     = 0L;
		    unflagged = F_DEL;
		}
	    }

	    break;

	  case F_FLAG:
	    if((!agg && fp->set != mc->flagged)
	       || (agg && fp->set != CMD_FLAG_UNKN)){
		flagit = "\\FLAGGED";
		if(fp->set){
		    flags     = ST_SET;
		    unflagged = F_UNFLAG;
		}
		else{
		    flags     = 0L;
		    unflagged = F_FLAG;
		}
	    }

	    break;

	  default:
	    break;
	}

	flagged = 0L;
	if(flags >= 0L
	   && (seq = currentf_sequence(state->mail_stream, msgmap,
				       unflagged, &flagged, 1))){
	    mail_flag(state->mail_stream, seq, flagit, flags);
	    fs_give((void **)&seq);
	    if(flagged){
		sprintf(tmp_20k_buf, "%slagged%s%s%s%s%s message%s%s \"%s\"",
			(fp->set) ? "F" : "Unf",
			agg ? " " : "",
			agg ? long2string(flagged) : "",
			(agg && flagged != mn_total_cur(msgmap))
			  ? " (of " : "",
			(agg && flagged != mn_total_cur(msgmap))
			  ? comatose(mn_total_cur(msgmap)) : "",
			(agg && flagged != mn_total_cur(msgmap))
			  ? ")" : "",
			agg ? plural(flagged) : " ",
			agg ? "" : long2string(mn_get_cur(msgmap)),
			fp->name);
		q_status_message(SM_ORDER, 0, 2, answer = tmp_20k_buf);
	    }
	}
    }

    if(!answer)
      q_status_message(SM_ORDER, 0, 2, "No status flags changed.");

  fini:
    if(agg)
      restore_selected(msgmap);
}



/*----------------------------------------------------------------------
   Offer concise status line flag prompt 

  Args: state --  Various satate info
        flags -- flags to offer setting

 Result: TRUE if flag to set specified in flags struct or FALSE otw

 ----*/
int
cmd_flag_prompt(state, flags)
    struct pine	       *state;
    struct flag_screen *flags;
{
    int			r, setflag = 1;
    struct flag_table  *fp;
    static char *flag_text = "Flag New, Deleted, Answered, or Important ? ";
    static char *flag_text2	=
	"Flag NOT New, NOT Deleted, NOT Answered, or NOT Important ? ";
    static ESCKEY_S flag_text_opt[] = {
	{'n', 'n', "N", "New"},
	{'*', '*', "*", "Important"},
	{'d', 'd', "D", "Deleted"},
	{'a', 'a', "A", "Answered"},
	{'!', '!', "!", "Not"},
	{ctrl('T'), 10, "^T", "To Flag Details"},
	{-1, 0, NULL, NULL}
    };

    while(1){
	r = radio_buttons(setflag ? flag_text : flag_text2,
			  -FOOTER_ROWS(state), flag_text_opt, '*', 'x',
			  NO_HELP, RB_NORM | RB_SEQ_SENSITIVE);
	if(r == 'x')			/* ol'cancelrooney */
	  return(TRUE);
	else if(r == 10)		/* return and goto flag screen */
	  return(FALSE);
	else if(r == '!')		/* flip intention */
	  setflag = !setflag;
	else
	  break;
    }

    for(fp = flags->flag_table; fp->name; fp++)
      if((r == 'n' && fp->flag == F_SEEN)
	 || (r == '*' && fp->flag == F_FLAG)
	 || (r == 'd' && fp->flag == F_DEL)
	 || (r == 'a' && fp->flag == F_ANS)){
	  fp->set = setflag ? CMD_FLAG_SET : CMD_FLAG_CLEAR;
	  break;
      }

    return(TRUE);
}



/*----------------------------------------------------------------------
   Execute REPLY message command

  Args: state --  Various satate info
        msgmap --  map of c-client to local message numbers

 Result: reply sent or not

 ----*/
void
cmd_reply(state, msgmap, agg)
     struct pine *state;
     MSGNO_S     *msgmap;
     int	  agg;
{
    if(any_messages(msgmap, NULL, "to Reply to")){
#if	defined(DOS) && !defined(WIN32)
	flush_index_cache();		/* save room on PC */
#endif
	if(agg && !pseudo_selected(msgmap))
	  return;

	reply(state);

	if(agg)
	  restore_selected(msgmap);

	state->mangled_screen = 1;
    }
}



/*----------------------------------------------------------------------
   Execute FORWARD message command

  Args: state --  Various satate info
        msgmap --  map of c-client to local message numbers

 Result: selected message[s] forwarded or not

 ----*/
void
cmd_forward(state, msgmap, agg)
     struct pine *state;
     MSGNO_S     *msgmap;
     int	  agg;
{
    if(any_messages(msgmap, NULL, "to Forward")){
#if	defined(DOS) && !defined(WIN32)
	flush_index_cache();		/* save room on PC */
#endif
	if(agg && !pseudo_selected(msgmap))
	  return;

	forward(state);

	if(agg)
	  restore_selected(msgmap);

	state->mangled_screen = 1;
    }
}



/*----------------------------------------------------------------------
   Execute BOUNCE message command

  Args: state --  Various satate info
        msgmap --  map of c-client to local message numbers

 Result: selected message[s] bounced or not

 ----*/
void
cmd_bounce(state, msgmap, agg)
     struct pine *state;
     MSGNO_S     *msgmap;
     int	  agg;
{
    if(any_messages(msgmap, NULL, "to Bounce")){
#if	defined(DOS) && !defined(WIN32)
	flush_index_cache();			/* save room on PC */
#endif
	if(agg && !pseudo_selected(msgmap))
	  return;

	bounce(state);
	if(agg)
	  restore_selected(msgmap);

	state->mangled_footer = 1;
    }
}



/*----------------------------------------------------------------------
   Execute save message command: prompt for folder and call function to save

  Args: screen_line    --  Line on the screen to prompt on
        message        --  The MESSAGECACHE entry of message to save

 Result: The folder lister can be called to make selection; mangled screen set

   This does the prompting for the folder name to save to, possibly calling 
 up the folder display for selection of folder by user.                 
 ----*/
void
cmd_save(state, stream, msgmap, agg, in_index)
    struct pine *state;
    MAILSTREAM  *stream;
    MSGNO_S	*msgmap;
    int		 agg;
    CmdWhere     in_index;
{
    char	      newfolder[MAILTMPLEN], nmsgs[32];
    int		      del = 0, we_cancel = 0;
    long	      i, raw;
    CONTEXT_S	     *cntxt = NULL;
    ENVELOPE	     *e = NULL;

    dprint(4, (debugfile, "\n - saving message -\n"));

    state->ugly_consider_advancing_bit = 0;
    if(msgno_any_deletedparts(stream, msgmap)
       && want_to("Saved copy will NOT include entire message!  Continue",
		  'y', 'n', NO_HELP, WT_FLUSH_IN | WT_SEQ_SENSITIVE) != 'y'){
	cmd_cancelled("Save message");
	return;
    }

    if(agg && !pseudo_selected(msgmap))
      return;

    raw = mn_m2raw(msgmap, mn_get_cur(msgmap));

    if(mn_total_cur(msgmap) <= 1L){
	sprintf(nmsgs, "Msg #%ld ", mn_get_cur(msgmap));
	e = mail_fetchstructure(stream, raw, NULL);
	if(!e) {
	    q_status_message(SM_ORDER | SM_DING, 3, 4,
			     "Can't save message.  Error accessing folder");
	    restore_selected(msgmap);
	    return;
	}
    }
    else
      sprintf(nmsgs, "%s msgs ", comatose(mn_total_cur(msgmap)));

    if(save_prompt(state,&cntxt,newfolder,sizeof(newfolder),nmsgs,e,raw,NULL)){
	del = !READONLY_FOLDER && F_OFF(F_SAVE_WONT_DELETE, ps_global);
	we_cancel = busy_alarm(1, NULL, NULL, 0);
	i = save(state, stream, cntxt, newfolder, msgmap,
		 (del ? SV_DELETE : 0) | SV_FIX_DELS);
	if(we_cancel)
	  cancel_busy_alarm(0);

	if(i == mn_total_cur(msgmap)){
	    if(mn_total_cur(msgmap) <= 1L){
		int need, avail = ps_global->ttyo->screen_cols - 2;
		int lennick, lenfldr;

		if(cntxt
		   && ps_global->context_list->next
		   && context_isambig(newfolder)){
		    lennick = min(strlen(cntxt->nickname), 500);
		    lenfldr = min(strlen(newfolder), 500);
		    need = 27 + strlen(long2string(mn_get_cur(msgmap))) +
			   lenfldr + lennick;
		    if(need > avail){
			if(lennick > 10){
			    need -= min(lennick-10, need-avail);
			    lennick -= min(lennick-10, need-avail);
			}

			if(need > avail && lenfldr > 10)
			  lenfldr -= min(lenfldr-10, need-avail);
		    }

		    sprintf(tmp_20k_buf,
			    "Message %.10s copied to \"%.99s\" in <%.99s>",
			    long2string(mn_get_cur(msgmap)),
			    short_str(newfolder, (char *)(tmp_20k_buf+1000),
				      lenfldr, MidDots),
			    short_str(cntxt->nickname,
				      (char *)(tmp_20k_buf+2000),
				      lennick, EndDots));
		}
		else{
		    char *f = " folder";

		    lenfldr = min(strlen(newfolder), 500);
		    need = 28 + strlen(long2string(mn_get_cur(msgmap))) +
			   lenfldr;
		    if(need > avail){
			need -= strlen(f);
			f = "";
			if(need > avail && lenfldr > 10)
			  lenfldr -= min(lenfldr-10, need-avail);
		    }

		    sprintf(tmp_20k_buf,
			    "Message %.10s copied to%.10s \"%.99s\"",
			    long2string(mn_get_cur(msgmap)), f,
			    short_str(newfolder, (char *)(tmp_20k_buf+1000),
				      lenfldr, MidDots));
		}
	    }
	    else
	      sprintf(tmp_20k_buf, "%s messages saved",
		      comatose(mn_total_cur(msgmap)));

	    if(del)
	      strcat(tmp_20k_buf, " and deleted");

	    q_status_message(SM_ORDER, 0, 3, tmp_20k_buf);

	    if(!agg && F_ON(F_SAVE_ADVANCES, state)){
		if(state->new_mail_count)
		  process_filter_patterns(stream, msgmap,
					  state->new_mail_count);

		mn_inc_cur(stream, msgmap,
			   (in_index == View && THREADING()
			    && state->viewing_a_thread)
			     ? MH_THISTHD
			     : (in_index == View)
			       ? MH_ANYTHD : MH_NONE);
	    }

	    state->ugly_consider_advancing_bit = 1;
	}
    }

    if(agg)					/* straighten out fakes */
      restore_selected(msgmap);

    if(del)
      update_titlebar_status();			/* make sure they see change */
}


/*----------------------------------------------------------------------
   Do the dirty work of prompting the user for a folder name

  Args: 
        nfldr should be a buffer at least MAILTMPLEN long
        

 Result: 

 ----*/
int
save_prompt(state, cntxt, nfldr, len_nfldr, nmsgs, env, rawmsgno, section)
    struct pine  *state;
    CONTEXT_S   **cntxt;
    char	 *nfldr;
    size_t        len_nfldr;
    char	 *nmsgs;
    ENVELOPE	 *env;
    long	  rawmsgno;
    char	 *section;
{
    static char	      folder[MAILTMPLEN+1] = {'\0'};
    static CONTEXT_S *last_context = NULL;
    int		      rc, n, flags, last_rc = 0, saveable_count = 0, done = 0;
    char	      prompt[MAX_SCREEN_COLS+1], *p, expanded[MAILTMPLEN];
    char              *buf = tmp_20k_buf;
    HelpType	      help;
    CONTEXT_S	     *tc;
    ESCKEY_S	      ekey[8];

    /* start with the default save context */
    if(((*cntxt) = default_save_context(state->context_list)) == NULL)
       (*cntxt) = state->context_list;

    if(!env || ps_global->save_msg_rule == MSG_RULE_LAST
	  || ps_global->save_msg_rule == MSG_RULE_DEFLT){
	if(ps_global->save_msg_rule == MSG_RULE_LAST && last_context)
	  (*cntxt) = last_context;
	else{
	    strncpy(folder,ps_global->VAR_DEFAULT_SAVE_FOLDER,sizeof(folder)-1);
	    folder[sizeof(folder)-1] = '\0';
	}
    }
    else{
	get_save_fldr_from_env(folder, sizeof(folder), env, state,
			       rawmsgno, section);
	if(ps_global->expunge_count)	/* somebody expunged current message */
	  return(0);
    }


    /* how many context's can be saved to... */
    for(tc = state->context_list; tc; tc = tc->next)
      if(!NEWS_TEST(tc))
        saveable_count++;

    /* set up extra command option keys */
    rc = 0;
    ekey[rc].ch      = ctrl('T');
    ekey[rc].rval    = 2;
    ekey[rc].name    = "^T";
    ekey[rc++].label = "To Fldrs";

    if(saveable_count > 1){
	ekey[rc].ch      = ctrl('P');
	ekey[rc].rval    = 10;
	ekey[rc].name    = "^P";
	ekey[rc++].label = "Prev Collection";

	ekey[rc].ch      = ctrl('N');
	ekey[rc].rval    = 11;
	ekey[rc].name    = "^N";
	ekey[rc++].label = "Next Collection";
    }

    if(F_ON(F_ENABLE_TAB_COMPLETE, ps_global)){
	ekey[rc].ch      = TAB;
	ekey[rc].rval    = 12;
	ekey[rc].name    = "TAB";
	ekey[rc++].label = "Complete";
    }

    if(F_ON(F_ENABLE_SUB_LISTS, ps_global)){
	ekey[rc].ch      = ctrl('X');
	ekey[rc].rval    = 14;
	ekey[rc].name    = "^X";
	ekey[rc++].label = "ListMatches";
    }

    if(saveable_count > 1){
	ekey[rc].ch      = KEY_UP;
	ekey[rc].rval    = 10;
	ekey[rc].name    = "";
	ekey[rc++].label = "";

	ekey[rc].ch      = KEY_DOWN;
	ekey[rc].rval    = 11;
	ekey[rc].name    = "";
	ekey[rc++].label = "";
    }

    ekey[rc].ch = -1;

    *nfldr = '\0';
    help = NO_HELP;
    while(!done){
	/* only show collection number if more than one available */
	if(ps_global->context_list->next)
	  sprintf(prompt, "SAVE %sto folder in <%.16s%s> [%s] : ",
		  nmsgs, (*cntxt)->nickname, 
		  strlen((*cntxt)->nickname) > 16 ? "..." : "",
		  strsquish(buf, folder, 25));
	else
	  sprintf(prompt, "SAVE %sto folder [%s] : ", nmsgs,
		  strsquish(buf, folder, 40));

	/*
	 * If the prompt won't fit, remove the extra info contained
	 * in nmsgs.
	 */
	if(state->ttyo->screen_cols < strlen(prompt) + 6 && *nmsgs){
	    if(ps_global->context_list->next)
	      sprintf(prompt, "SAVE to folder in <%.16s%s> [%s] : ",
		      (*cntxt)->nickname, 
		      strlen((*cntxt)->nickname) > 16 ? "..." : "",
		      strsquish(buf, folder, 25));
	    else
	      sprintf(prompt, "SAVE to folder [%s] : ", 
		      strsquish(buf, folder, 25));
	}
	
	flags = OE_APPEND_CURRENT | OE_SEQ_SENSITIVE;
	rc = optionally_enter(nfldr, -FOOTER_ROWS(state), 0, len_nfldr,
			      prompt, ekey, help, &flags);

	switch(rc){
	  case -1 :
	    q_status_message(SM_ORDER | SM_DING, 3, 3,
			     "Error reading folder name");
	    done--;
	    break;

	  case 0 :
	    removing_trailing_white_space(nfldr);
	    removing_leading_white_space(nfldr);

	    if(*nfldr || *folder){
		char *p, *name, *fullname = NULL;
		int   exists, breakout = FALSE;

		if(!*nfldr){
		    strncpy(nfldr, folder, len_nfldr-1);
		    nfldr[len_nfldr-1] = '\0';
		}
		if(!(name = folder_is_nick(nfldr, FOLDERS(*cntxt))))
		    name = nfldr;

		if(update_folder_spec(expanded, name)){
		    strncpy(name = nfldr, expanded, len_nfldr-1);
		    nfldr[len_nfldr-1] = '\0';
		}

		exists = folder_name_exists(*cntxt, name, &fullname);

		if(exists == FEX_ERROR){
		    q_status_message1(SM_ORDER, 0, 3,
				      "Problem accessing folder \"%.200s\"",
				      nfldr);
		    done--;
		}
		else{
		    if(fullname){
			strncpy(name = nfldr, fullname, len_nfldr-1);
			nfldr[len_nfldr-1] = '\0';
			fs_give((void **) &fullname);
			breakout = TRUE;
		    }

		    if(exists & FEX_ISFILE){
			done++;
		    }
		    else if((exists & FEX_ISDIR)){
			tc = *cntxt;
			if(breakout){
			    CONTEXT_S *fake_context;
			    char	   tmp[MAILTMPLEN];
			    size_t	   l;

			    strncpy(tmp, name, sizeof(tmp)-2);
			    tmp[sizeof(tmp)-2-1] = '\0';
			    if(tmp[(l = strlen(tmp)) - 1] != tc->dir->delim){
				tmp[l] = tc->dir->delim;
				strcpy(&tmp[l+1], "[]");
			    }
			    else
			      strcat(tmp, "[]");

			    fake_context = new_context(tmp, 0);
			    nfldr[0] = '\0';
			    done = display_folder_list(&fake_context, nfldr,
						       1, folders_for_save);
			    free_context(&fake_context);
			}
			else if(tc->dir->delim
				&& (p = strrindex(name, tc->dir->delim))
				&& *(p+1) == '\0')
			  done = display_folder_list(cntxt, nfldr,
						     1, folders_for_save);
		    }
		    else{			/* Doesn't exist, create! */
			if(fullname = folder_as_breakout(*cntxt, name)){
			    strncpy(name = nfldr, fullname, len_nfldr-1);
			    nfldr[len_nfldr-1] = '\0';
			    fs_give((void **) &fullname);
			}

			switch(create_for_save(NULL, *cntxt, name)){
			  case 1 :		/* success */
			    done++;
			    break;
			  case 0 :		/* error */
			  case -1 :		/* declined */
			    done--;
			    break;
			}
		    }
		}

		break;
	    }
	    /* else fall thru like they cancelled */

	  case 1 :
	    cmd_cancelled("Save message");
	    done--;
	    break;

	  case 2 :
	    if(display_folder_list(cntxt, nfldr, 0, folders_for_save))
	      done++;

	    break;

	  case 3 :
            help = (help == NO_HELP) ? h_oe_save : NO_HELP;
	    break;

	  case 4 :				/* redraw */
	    break;

	  case 10 :				/* previous collection */
	    for(tc = (*cntxt)->prev; tc; tc = tc->prev)
	      if(!NEWS_TEST(tc))
		break;

	    if(!tc){
		CONTEXT_S *tc2;

		for(tc2 = (tc = (*cntxt))->next; tc2; tc2 = tc2->next)
		  if(!NEWS_TEST(tc2))
		    tc = tc2;
	    }

	    *cntxt = tc;
	    break;

	  case 11 :				/* next collection */
	    tc = (*cntxt);

	    do
	      if(((*cntxt) = (*cntxt)->next) == NULL)
		(*cntxt) = ps_global->context_list;
	    while(NEWS_TEST(*cntxt) && (*cntxt) != tc);
	    break;

	  case 12 :				/* file name completion */
	    if(!folder_complete(*cntxt, nfldr, &n)){
		if(n && last_rc == 12 && !(flags & OE_USER_MODIFIED)){
		    if(display_folder_list(cntxt, nfldr, 1, folders_for_save))
		      done++;			/* bingo! */
		    else
		      rc = 0;			/* burn last_rc */
		}
		else
		  Writechar(BELL, 0);
	    }

	    break;

	  case 14 :				/* file name completion */
	    if(display_folder_list(cntxt, nfldr, 2, folders_for_save))
	      done++;			/* bingo! */
	    else
	      rc = 0;			/* burn last_rc */

	    break;

	  default :
	    panic("Unhandled case");
	    break;
	}

	last_rc = rc;
    }

    ps_global->mangled_footer = 1;

    if(done < 0)
      return(0);

    if(*cntxt)
      last_context = *cntxt;		/* remember for next time */

    if(*nfldr){
	strncpy(folder, nfldr, sizeof(folder)-1);
	folder[sizeof(folder)-1] = '\0';
    }
    else{
	strncpy(nfldr, folder, len_nfldr-1);
	nfldr[len_nfldr-1] = '\0';
    }

    /* nickname?  Copy real name to nfldr */
    if(*cntxt
       && context_isambig(nfldr)
       && (p = folder_is_nick(nfldr, FOLDERS(*cntxt)))){
	strncpy(nfldr, p, len_nfldr-1);
	nfldr[len_nfldr-1] = '\0';
    }

    return(1);
}


/*----------------------------------------------------------------------
   Grope through envelope to find default folder name to save to

  Args: fbuf     --  Buffer to return result in
        nfbuf    --  Size of fbuf
        e        --  The envelope to look in
        state    --  Usual pine state
        rawmsgno --  Raw c-client sequence number of message
	section  --  Mime section of header data (for message/rfc822)

 Result: The appropriate default folder name is copied into fbuf.
 ----*/
void
get_save_fldr_from_env(fbuf, nfbuf, e, state, rawmsgno, section)
    char         fbuf[];
    int          nfbuf;
    ENVELOPE    *e;
    struct pine *state;
    long	 rawmsgno;
    char	*section;
{
    char     fakedomain[2];
    ADDRESS *tmp_adr = NULL;
    char     buf[max(MAXFOLDER,MAX_NICKNAME) + 1];
    char    *bufp;
    char    *folder_name = NULL;
    static char botch[] = "programmer botch, unknown message save rule";
    unsigned save_msg_rule;

    if(!e)
      return;

    /* copy this because we might change it below */
    save_msg_rule = state->save_msg_rule;

    /* first get the relevant address to base the folder name on */
    switch(save_msg_rule){
      case MSG_RULE_FROM:
      case MSG_RULE_NICK_FROM:
      case MSG_RULE_NICK_FROM_DEF:
      case MSG_RULE_FCC_FROM:
      case MSG_RULE_FCC_FROM_DEF:
      case MSG_RULE_RN_FROM:
      case MSG_RULE_RN_FROM_DEF:
        tmp_adr = e->from ? copyaddr(e->from)
			  : e->sender ? copyaddr(e->sender) : NULL;
	break;

      case MSG_RULE_SENDER:
      case MSG_RULE_NICK_SENDER:
      case MSG_RULE_NICK_SENDER_DEF:
      case MSG_RULE_FCC_SENDER:
      case MSG_RULE_FCC_SENDER_DEF:
      case MSG_RULE_RN_SENDER:
      case MSG_RULE_RN_SENDER_DEF:
        tmp_adr = e->sender ? copyaddr(e->sender)
			    : e->from ? copyaddr(e->from) : NULL;
	break;

      case MSG_RULE_REPLYTO:
      case MSG_RULE_NICK_REPLYTO:
      case MSG_RULE_NICK_REPLYTO_DEF:
      case MSG_RULE_FCC_REPLYTO:
      case MSG_RULE_FCC_REPLYTO_DEF:
      case MSG_RULE_RN_REPLYTO:
      case MSG_RULE_RN_REPLYTO_DEF:
        tmp_adr = e->reply_to ? copyaddr(e->reply_to)
			  : e->from ? copyaddr(e->from)
			  : e->sender ? copyaddr(e->sender) : NULL;
	break;

      case MSG_RULE_RECIP:
      case MSG_RULE_NICK_RECIP:
      case MSG_RULE_NICK_RECIP_DEF:
      case MSG_RULE_FCC_RECIP:
      case MSG_RULE_FCC_RECIP_DEF:
      case MSG_RULE_RN_RECIP:
      case MSG_RULE_RN_RECIP_DEF:
	/* news */
	if(state->mail_stream && IS_NEWS(state->mail_stream)){
	    char *tmp_a_string, *ng_name;

	    fakedomain[0] = '@';
	    fakedomain[1] = '\0';

	    /* find the news group name */
	    if(ng_name = strstr(state->mail_stream->mailbox,"#news"))
	      ng_name += 6;
	    else
	      ng_name = state->mail_stream->mailbox; /* shouldn't happen */

	    /* copy this string so rfc822_parse_adrlist can't blast it */
	    tmp_a_string = cpystr(ng_name);
	    /* make an adr */
	    rfc822_parse_adrlist(&tmp_adr, tmp_a_string, fakedomain);
	    fs_give((void **)&tmp_a_string);
	    if(tmp_adr && tmp_adr->host && tmp_adr->host[0] == '@')
	      tmp_adr->host[0] = '\0';
	}
	/* not news */
	else{
	    static char *fields[] = {"Resent-To", NULL};
	    char *extras, *values[sizeof(fields)/sizeof(fields[0])];

	    extras = pine_fetchheader_lines(state->mail_stream, rawmsgno,
					    section, fields);
	    if(extras){
		long i;

		memset(values, 0, sizeof(fields));
		simple_header_parse(extras, fields, values);
		fs_give((void **)&extras);

		for(i = 0; i < sizeof(fields)/sizeof(fields[0]); i++)
		  if(values[i]){
		      if(tmp_adr)		/* take last matching value */
			mail_free_address(&tmp_adr);

		      /* build a temporary address list */
		      fakedomain[0] = '@';
		      fakedomain[1] = '\0';
		      rfc822_parse_adrlist(&tmp_adr, values[i], fakedomain);
		      fs_give((void **)&values[i]);
		  }
	    }

	    if(!tmp_adr)
	      tmp_adr = e->to ? copyaddr(e->to) : NULL;
	}

	break;
      
      default:
	panic(botch);
	break;
    }

    /* For that address, lookup the fcc or nickname from address book */
    switch(save_msg_rule){
      case MSG_RULE_NICK_FROM:
      case MSG_RULE_NICK_SENDER:
      case MSG_RULE_NICK_REPLYTO:
      case MSG_RULE_NICK_RECIP:
      case MSG_RULE_FCC_FROM:
      case MSG_RULE_FCC_SENDER:
      case MSG_RULE_FCC_REPLYTO:
      case MSG_RULE_FCC_RECIP:
      case MSG_RULE_NICK_FROM_DEF:
      case MSG_RULE_NICK_SENDER_DEF:
      case MSG_RULE_NICK_REPLYTO_DEF:
      case MSG_RULE_NICK_RECIP_DEF:
      case MSG_RULE_FCC_FROM_DEF:
      case MSG_RULE_FCC_SENDER_DEF:
      case MSG_RULE_FCC_REPLYTO_DEF:
      case MSG_RULE_FCC_RECIP_DEF:
	switch(save_msg_rule){
	  case MSG_RULE_NICK_FROM:
	  case MSG_RULE_NICK_SENDER:
	  case MSG_RULE_NICK_REPLYTO:
	  case MSG_RULE_NICK_RECIP:
	  case MSG_RULE_NICK_FROM_DEF:
	  case MSG_RULE_NICK_SENDER_DEF:
	  case MSG_RULE_NICK_REPLYTO_DEF:
	  case MSG_RULE_NICK_RECIP_DEF:
	    bufp = get_nickname_from_addr(tmp_adr, buf, sizeof(buf));
	    break;

	  case MSG_RULE_FCC_FROM:
	  case MSG_RULE_FCC_SENDER:
	  case MSG_RULE_FCC_REPLYTO:
	  case MSG_RULE_FCC_RECIP:
	  case MSG_RULE_FCC_FROM_DEF:
	  case MSG_RULE_FCC_SENDER_DEF:
	  case MSG_RULE_FCC_REPLYTO_DEF:
	  case MSG_RULE_FCC_RECIP_DEF:
	    bufp = get_fcc_from_addr(tmp_adr, buf, sizeof(buf));
	    break;
	}

	if(bufp && *bufp){
	    istrncpy(fbuf, bufp, nfbuf - 1);
	    fbuf[nfbuf - 1] = '\0';
	}
	else
	  /* fall back to non-nick/non-fcc version of rule */
	  switch(save_msg_rule){
	    case MSG_RULE_NICK_FROM:
	    case MSG_RULE_FCC_FROM:
	      save_msg_rule = MSG_RULE_FROM;
	      break;

	    case MSG_RULE_NICK_SENDER:
	    case MSG_RULE_FCC_SENDER:
	      save_msg_rule = MSG_RULE_SENDER;
	      break;

	    case MSG_RULE_NICK_REPLYTO:
	    case MSG_RULE_FCC_REPLYTO:
	      save_msg_rule = MSG_RULE_REPLYTO;
	      break;

	    case MSG_RULE_NICK_RECIP:
	    case MSG_RULE_FCC_RECIP:
	      save_msg_rule = MSG_RULE_RECIP;
	      break;
	    
	    default:
	      istrncpy(fbuf, ps_global->VAR_DEFAULT_SAVE_FOLDER, nfbuf - 1);
	      fbuf[nfbuf - 1] = '\0';
	      break;
	  }

	break;
    }

    /* Realname */
    switch(save_msg_rule){
      case MSG_RULE_RN_FROM_DEF:
      case MSG_RULE_RN_FROM:
      case MSG_RULE_RN_SENDER_DEF:
      case MSG_RULE_RN_SENDER:
      case MSG_RULE_RN_RECIP_DEF:
      case MSG_RULE_RN_RECIP:
      case MSG_RULE_RN_REPLYTO_DEF:
      case MSG_RULE_RN_REPLYTO:
        /* Fish out the realname */
	if(tmp_adr && tmp_adr->personal && tmp_adr->personal[0]){
	    char *dummy = NULL;

	    folder_name = (char *)rfc1522_decode((unsigned char *)tmp_20k_buf,
						 SIZEOF_20KBUF,
						 tmp_adr->personal, &dummy);
	    if(dummy)
	      fs_give((void **)&dummy);
	}

	if(folder_name && folder_name[0]){
	    istrncpy(fbuf, folder_name, nfbuf - 1);
	    fbuf[nfbuf - 1] = '\0';
	}
	else{	/* fall back to other behaviors */
	    switch(save_msg_rule){
	      case MSG_RULE_RN_FROM:
	        save_msg_rule = MSG_RULE_FROM;
		break;

	      case MSG_RULE_RN_SENDER:
	        save_msg_rule = MSG_RULE_SENDER;
		break;

	      case MSG_RULE_RN_RECIP:
	        save_msg_rule = MSG_RULE_RECIP;
		break;

	      case MSG_RULE_RN_REPLYTO:
	        save_msg_rule = MSG_RULE_REPLYTO;
		break;

	      default:
		istrncpy(fbuf, ps_global->VAR_DEFAULT_SAVE_FOLDER, nfbuf - 1);
		fbuf[nfbuf - 1] = '\0';
		break;
	    }
	}

	break;
    }

    /* get the username out of the mailbox for this address */
    switch(save_msg_rule){
      case MSG_RULE_FROM:
      case MSG_RULE_SENDER:
      case MSG_RULE_REPLYTO:
      case MSG_RULE_RECIP:
	/*
	 * Fish out the user's name from the mailbox portion of
	 * the address and put it in folder.
	 */
	folder_name = (tmp_adr && tmp_adr->mailbox && tmp_adr->mailbox[0])
		      ? tmp_adr->mailbox : NULL;
	if(!get_uname(folder_name, fbuf, nfbuf)){
	    istrncpy(fbuf, ps_global->VAR_DEFAULT_SAVE_FOLDER, nfbuf - 1);
	    fbuf[nfbuf - 1] = '\0';
	}

	break;
    }

    if(tmp_adr)
      mail_free_address(&tmp_adr);
}



/*----------------------------------------------------------------------
        Do the work of actually saving messages to a folder

    Args: state -- pine state struct (for stream pointers)
	  context -- context to interpret name in if not fully qualified
	  folder  -- The folder to save the message in
          msgmap -- message map of currently selected messages
	  flgs -- Possible bits are
		    SV_DELETE   - delete after saving
		    SV_FOR_FILT - called from filtering function, not save
		    SV_FIX_DELS - remove Del mark before saving

  Result: Returns number of messages saved

  Note: There's a bit going on here; temporary clearing of deleted flags
	since they are *not* preserved, picking or creating the stream for
	copy or append, and dealing with errors...
 ----*/
long
save(state, stream, context, folder, msgmap, flgs)
    struct pine  *state;
    MAILSTREAM	 *stream;
    CONTEXT_S    *context;
    char         *folder;
    MSGNO_S	 *msgmap;
    int		  flgs;
{
    int		  rv, rc, j, our_stream = 0, cancelled = 0;
    int           delete, filter;
    char	 *save_folder, *seq, flags[64], date[64], tmp[MAILTMPLEN];
    long	  i, nmsgs;
    STORE_S	 *so = NULL;
    MAILSTREAM	 *save_stream = NULL;
    MESSAGECACHE *mc;
#if	defined(DOS) && !defined(WIN32)
#define	SAVE_TMP_TYPE		TmpFileStar
#else
#define	SAVE_TMP_TYPE		CharStar
#endif

    delete = flgs & SV_DELETE;
    filter = flgs & SV_FOR_FILT;

    if(strucmp(folder, state->inbox_name) == 0){
	save_folder = state->VAR_INBOX_PATH;
	context = NULL;
    }
    else
      save_folder = folder;

    /*
     * If any of the messages have exceptional attachment handling
     * we have to fall thru below to do the APPEND by hand...
     */
    if(!msgno_any_deletedparts(stream, msgmap)){
	/*
	 * Compare the current stream (the save's source) and the stream
	 * the destination folder will need...
	 */
	context_apply(tmp, context, save_folder, sizeof(tmp));
        save_stream = (stream->dtb->flags & DR_LOCAL) && !IS_REMOTE(tmp) ?
	  stream : context_same_stream(context, save_folder, stream);
    }

    /* if needed, this'll get set in mm_notify */
    ps_global->try_to_create = 0;
    rv = rc = 0;
    nmsgs = 0L;

    /*
     * At this point, if we have a save_stream, then none of the messages
     * being saved involve special handling that would require our use
     * of mail_append, so go with mail_copy since in the IMAP case it
     * means no data on the wire...
     */
    if(save_stream){
	char *dseq = NULL, *oseq;

	if((flgs & SV_FIX_DELS) &&
	   (dseq = currentf_sequence(stream, msgmap, F_DEL, NULL,0)))
	  mail_flag(stream, dseq, "\\DELETED", 0L);

	seq = currentf_sequence(stream, msgmap, 0L, &nmsgs, 0);
	if(F_ON(F_AGG_SEQ_COPY, ps_global)
	   || (mn_get_sort(msgmap) == SortArrival && !mn_get_revsort(msgmap))){
	    /*
	     * currentf_sequence() above lit all the elt "sequence"
	     * bits of the interesting messages.  Now, build a sequence
	     * that preserves sort order...
	     */
	    oseq = build_sequence(stream, msgmap, &nmsgs);
	}
	else{
	    oseq  = NULL;			/* no single sequence! */
	    nmsgs = 0L;
	    i = mn_first_cur(msgmap);		/* set first to copy */
	}

	do{
	    while(!(rv = (int) context_copy(context, save_stream,
				oseq ? oseq : long2string(mn_m2raw(msgmap, i)),
				save_folder))){
		if(rc++ || !ps_global->try_to_create)   /* abysmal failure! */
		  break;			/* c-client returned error? */

		if((context && context->use & CNTXT_INCMNG)
		   && context_isambig(save_folder)){
		    q_status_message(SM_ORDER, 3, 5,
		   "Can only save to existing folders in Incoming Collection");
		    break;
		}

		ps_global->try_to_create = 0;	/* reset for next time */
		if((j = create_for_save(save_stream,context,save_folder)) < 1){
		    if(j < 0)
		      cancelled = 1;		/* user cancels */

		    break;
		}
	    }

	    if(rv){				/* failure or finished? */
		if(oseq)			/* all done? */
		  break;
		else
		  nmsgs++;
	    }
	    else{				/* failure! */
		if(oseq)
		  nmsgs = 0L;			/* nothing copy'd */

		break;
	    }
	}
	while((i = mn_next_cur(msgmap)) > 0L);

	if(rv && delete)			/* delete those saved */
	  mail_flag(stream, seq, "\\DELETED", ST_SET);
	else if(dseq)				/* or restore previous state */
	  mail_flag(stream, dseq, "\\DELETED", ST_SET);

	if(dseq)				/* clean up */
	  fs_give((void **)&dseq);

	if(oseq)
	  fs_give((void **)&oseq);

	fs_give((void **)&seq);
    }
    else{
	/*
	 * Special handling requires mail_append.  See if we can
	 * reuse stream source messages are on...
	 */
	save_stream = context_same_stream(context, save_folder, stream);

	/*
	 * IF the destination's REMOTE, open a stream here so c-client
	 * doesn't have to open it for each aggregate save...
	 */
	if(!save_stream){
	    char tmp[MAILTMPLEN];		/* must be within context */

	    if(context_apply(tmp, context, save_folder, sizeof(tmp))[0] == '{'
	       && (save_stream = context_open(context, NULL,
					      save_folder, OP_HALFOPEN)))
	      our_stream = 1;
	}

	/*
	 * Allocate a storage object to temporarily store the message
	 * object in.  Below it'll get mapped into a c-client STRING struct
	 * in preparation for handing off to context_append...
	 */
	if(!(so = so_get(SAVE_TMP_TYPE, NULL, WRITE_ACCESS))){
	    dprint(1, (debugfile, "Can't allocate store for save: %s\n",
		       error_description(errno)));
	    q_status_message(SM_ORDER | SM_DING, 3, 4,
			     "Problem creating space for message text.");
	}

	/*
	 * get a sequence of invalid elt's so we can get thier flags...
	 */
	if(seq = invalid_elt_sequence(stream, msgmap)){
	    mail_fetch_fast(stream, seq, 0L);
	    fs_give((void **) &seq);
	}

	/*
	 * If we're supposed set the deleted flag, clear the elt bit
	 * we'll use to build the sequence later...
	 */
	if(delete)
	  for(i = 1L; i <= stream->nmsgs; i++)
	    mail_elt(stream, i)->sequence = 0;

	nmsgs = 0L;

	/* 
	 * if there is more than one message, do multiappend.
	 * otherwise, we can use our already open stream.
	 */
	if(!save_stream || !is_imap_stream(save_stream) ||
	   (LEVELMULTIAPPEND(save_stream) && mn_total_cur(msgmap) > 1)){
	    APPENDPACKAGE pkg;
	    STRING msg;

	    pkg.stream = stream;
	    pkg.flags = flags;
	    pkg.date = date;
	    pkg.msg = &msg;
	    pkg.msgmap = msgmap;

	    if ((pkg.so = so) && ((pkg.msgno = mn_first_cur(msgmap)) > 0L)) {
	        so_truncate(so, 0L);

		/* 
		 * we've gotta make sure this is a stream that we've
		 * opened ourselves.
		 */
		rc = 0;
		while(!(rv = context_append_multiple(context, 
						     our_stream ? save_stream
						     : NULL, save_folder,
						     save_fetch_append_cb,
						     (void *) &pkg))) {
		  if(rc++ || !ps_global->try_to_create)
		    break;
		  if((context && context->use & CNTXT_INCMNG)
		     && context_isambig(save_folder)){
		    q_status_message(SM_ORDER, 3, 5,
		   "Can only save to existing folders in Incoming Collection");
		    break;
		  }

		  ps_global->try_to_create = 0;
		  if((j = create_for_save(our_stream ? save_stream : NULL, 
					  context, save_folder)) < 1){
		    if(j < 0)
		      cancelled = 1;
		    break;
		  }
		}
		if(rv){
		    /*
		     * Success!  Count it, and if it's not already deleted and 
		     * it's supposed to be, mark it to get deleted later...
		     */
		  for(i = mn_first_cur(msgmap); so && i > 0L;
		      i = mn_next_cur(msgmap)){
		    nmsgs++;
		    if(delete){
		      mc = mail_elt(stream, mn_m2raw(msgmap, i));
		      if(!mc->deleted){
			mc->sequence = 1;	/* mark for later deletion */
			clear_index_cache_ent(i);
			check_point_change();
		      }
		    }
		  }
		}
	    }
	    else
	      cancelled = 1;		/* No messages to append! */

	    if(ps_global->expunge_count)
	      cancelled = 1;		/* All bets are off! */
	}
	else 
	  for(i = mn_first_cur(msgmap); so && i > 0L; i = mn_next_cur(msgmap)){
	    so_truncate(so, 0L);

	    /* store flags before the fetch so UNSEEN bit isn't flipped */
	    mc = mail_elt(stream, mn_m2raw(msgmap, i));
	    flag_string(mc, F_ANS|F_FLAG|F_SEEN, flags);
	    if(mc->day)
	      mail_date(date, mc);
	    else
	      *date = '\0';

	    rv = save_fetch_append(stream, mn_m2raw(msgmap, i),
				   NULL, save_stream, save_folder, context,
				   mc->rfc822_size, flags, date, so);

	    if(ps_global->expunge_count)
	      rv = -1;			/* All bets are off! */

	    if(rv == 1){
		/*
		 * Success!  Count it, and if it's not already deleted and 
		 * it's supposed to be, mark it to get deleted later...
		 */
		nmsgs++;
		if(delete){
		    mc = mail_elt(stream, mn_m2raw(msgmap, i));
		    if(!mc->deleted){
			mc->sequence = 1;	/* mark for later deletion */
			clear_index_cache_ent(i);
			check_point_change();
		    }
		}
	    }
	    else{
		if(rv == -1)
		  cancelled = 1;		/* else horrendous failure */

		break;
	    }
	}

	if(our_stream)
	  pine_mail_close(save_stream);

	if(so)
	  so_give(&so);

	if(delete && (seq = build_sequence(stream, NULL, NULL))){
	    mail_flag(stream, seq, "\\DELETED", ST_SET);
	    fs_give((void **)&seq);
	}
    }

    ps_global->try_to_create = 0;		/* reset for next time */
    if(!cancelled && nmsgs < mn_total_cur(msgmap)){
	dprint(1, (debugfile, "FAILED save of msg %s (c-client sequence #)\n",
		   long2string(mn_m2raw(msgmap, mn_get_cur(msgmap)))));
	if((mn_total_cur(msgmap) > 1L) && nmsgs != 0){
	  /* this shouldn't happen cause it should be all or nothing */
	    sprintf(tmp_20k_buf,
		    "%ld of %ld messages saved before error occurred",
		    nmsgs, mn_total_cur(msgmap));
	    dprint(1, (debugfile, "\t%s\n", tmp_20k_buf));
	    q_status_message(SM_ORDER, 3, 5, tmp_20k_buf);
	}
	else if(mn_total_cur(msgmap) == 1){
	  sprintf(tmp_20k_buf,
		  "%s to folder \"%s\" FAILED",
		  filter ? "Filter" : "Save", 
		  strsquish(tmp_20k_buf+500, folder, 35));
	  dprint(1, (debugfile, "\t%s\n", tmp_20k_buf));
	  q_status_message(SM_ORDER | SM_DING, 3, 5, tmp_20k_buf);
	}
	else{
	  sprintf(tmp_20k_buf,
		  "%s of %s messages to folder \"%s\" FAILED",
		  filter ? "Filter" : "Save", comatose(mn_total_cur(msgmap)),
		  strsquish(tmp_20k_buf+500, folder, 35));
	  dprint(1, (debugfile, "\t%s\n", tmp_20k_buf));
	  q_status_message(SM_ORDER | SM_DING, 3, 5, tmp_20k_buf);
	}
    }

    return(nmsgs);
}

/* Append message callback
 * Accepts: MAIL stream
 *	    append data package
 *	    pointer to return initial flags
 *	    pointer to return message internal date
 *	    pointer to return stringstruct of message or NIL to stop
 * Returns: T if success (have message or stop), NIL if error
 */

long save_fetch_append_cb (stream, data, flags, date, message)
    MAILSTREAM *stream;
    void       *data;
    char      **flags;
    char      **date;
    STRING    **message;
{
    unsigned long size = 0;
    APPENDPACKAGE *pkg = (APPENDPACKAGE *) data;
    MESSAGECACHE *mc;
    char *fetch;
    int rc;
    unsigned long raw, hlen, tlen, mlen;

    if(pkg->so && (pkg->msgno > 0L)) {
	raw = mn_m2raw(pkg->msgmap, pkg->msgno);
	mc = mail_elt(pkg->stream, raw);
	size = mc->rfc822_size;
	flag_string(mc, F_ANS|F_FLAG|F_SEEN, pkg->flags);
	if(mc->day)
	  mail_date(pkg->date, mc);
	else
	  *pkg->date = '\0';
	if(fetch = mail_fetch_header(pkg->stream, raw, NULL, NULL, &hlen,
				     FT_PEEK)){
	    if(!*pkg->date)
	      saved_date(pkg->date, fetch);
	}
	else
	  return(0);			/* fetchtext writes error */

	rc = MSG_EX_DELETE;		/* "rc" overloaded */
	if(msgno_exceptions(pkg->stream, raw, NULL, &rc, 0)){
	    char  section[64];
	    int	 failure = 0;
	    BODY *body;
	    gf_io_t  pc;

	    size = 0;			/* all bets off, abort sanity test  */
	    gf_set_so_writec(&pc, pkg->so);

	    section[0] = '\0';
	    if(!mail_fetch_structure(pkg->stream, raw, &body, 0))
	      return(0);
	    
	    if(msgno_part_deleted(pkg->stream, raw, "")){
	       tlen = 0;
	       failure = !save_ex_replace_body(fetch, &hlen, body, pc);
	     }
	    else
	      failure = !(so_nputs(pkg->so, fetch, (long) hlen)
			  && save_ex_output_body(pkg->stream, raw, section,
						 body, &tlen, pc));

	    gf_clear_so_writec(pkg->so);

	    if(failure)
	      return(0);

	    q_status_message(SM_ORDER, 3, 3,
			     "NOTE: Deleted message parts NOT included in saved copy!");

	}
	else{
	    if(!so_nputs(pkg->so, fetch, (long) hlen))
	      return(0);

#if	defined(DOS) && !defined(WIN32)
	    mail_parameters(pkg->stream, SET_GETS, (void *)dos_gets);
	    append_file = (FILE *) so_text(pkg->so);
	    mail_gc(pkg->stream, GC_TEXTS);
#endif

	    fetch = mail_fetch_text(pkg->stream, raw, NULL, &tlen, FT_PEEK);

#if	!(defined(DOS) && !defined(WIN32))
	    if(!(fetch && so_nputs(pkg->so, fetch, tlen)))
	      return(0);
#else
	    append_file = NULL;
	    mail_parameters(pkg->stream, SET_GETS, (void *)NULL);
	    mail_gc(pkg->stream, GC_TEXTS);
	    
	    if(!fetch)
	      return(0);
#endif
	}

	so_seek(pkg->so, 0L, 0);	/* rewind just in case */

#if	defined(DOS) && !defined(WIN32)
	d.fd  = fileno((FILE *)so_text(pkg->so));
	d.pos = 0L;
	mlen = filelength(d.fd);
#else
	mlen = hlen + tlen;
#endif

	if(size && mlen < size){
	    char buf[128];

	    sprintf(buf, "Message to save shrank!  (#%ld: %ld --> %ld)",
		    raw, size, mlen);
	    q_status_message(SM_ORDER | SM_DING, 3, 4, buf);
	    dprint(1, (debugfile, "BOTCH: %s\n", buf));
	    return(0);
	}

#if	defined(DOS) && !defined(WIN32)
	INIT(pkg->msg, dawz_string, (void *)&d, mlen);
#else
	INIT(pkg->msg, mail_string, (void *)so_text(pkg->so), mlen);
#endif
      *message = pkg->msg;
					/* Next message */
      pkg->msgno = mn_next_cur(pkg->msgmap);
  }
  else					/* No more messages */
    *message = NIL;

  *flags = pkg->flags;
  *date = (pkg->date && *pkg->date) ? pkg->date : NIL;
  return LONGT;				/* Return success */
}

/*----------------------------------------------------------------------
   FETCH an rfc822 message header and body and APPEND to destination folder

  Args: 
        

 Result: 

 ----*/
int
save_fetch_append(stream, raw, sect, save_stream, save_folder,
		  context, size, flags, date, so)
    MAILSTREAM	  *stream;
    long	   raw;
    char	  *sect;
    MAILSTREAM	  *save_stream;
    char	  *save_folder;
    CONTEXT_S	  *context;
    unsigned long  size;
    char	  *flags, *date;
    STORE_S	  *so;
{
    int		   rc, rv, old_imap_server = 0;
    long	   j;
    char	  *fetch;
    unsigned long  hlen, tlen, mlen;
    STRING	   msg;
#if	defined(DOS) && !defined(WIN32)
    struct {					/* hack! stolen from dawz.c */
	int fd;
	unsigned long pos;
    } d;
    extern STRINGDRIVER dawz_string;
#endif

    if(fetch = mail_fetch_header(stream, raw, sect, NULL, &hlen, FT_PEEK)){
	/*
	 * If there's no date string, then caller found the
	 * MESSAGECACHE for this message element didn't already have it.
	 * So, parse the "internal date" by hand since fetchstructure
	 * hasn't been called yet for this particular message, and
	 * we don't want to call it now just to get the date since
	 * the full header has what we want.  Likewise, don't even
	 * think about calling mail_fetchfast either since it also
	 * wants to load mc->rfc822_size (which we could actually
	 * use but...), which under some drivers is *very*
	 * expensive to acquire (can you say NNTP?)...
	 */
	if(!*date)
	  saved_date(date, fetch);
    }
    else
      return(0);			/* fetchtext writes error */

    rc = MSG_EX_DELETE;			/* "rc" overloaded */
    if(msgno_exceptions(stream, raw, NULL, &rc, 0)){
	char	 section[64];
	int	 failure = 0;
	BODY	*body;
	gf_io_t  pc;

	size = 0;			/* all bets off, abort sanity test  */
	gf_set_so_writec(&pc, so);

	if(sect && *sect){
	    sprintf(section, "%s.1", sect);
	    if(!(body = mail_body(stream, raw, section)))
	      return(0);
	}
	else{
	    section[0] = '\0';
	    if(!mail_fetch_structure(stream, raw, &body, 0))
	      return(0);
	}

	/*
	 * Walk the MIME structure looking for exceptional segments,
	 * writing them in the requested fashion.
	 *
	 * First, though, check for the easy case...
	 */
	if(msgno_part_deleted(stream, raw, sect ? sect : "")){
	    tlen = 0;
	    failure = !save_ex_replace_body(fetch, &hlen, body, pc);
	}
	else{
	    /*
	     * Otherwise, roll up your sleeves and get to work...
	     * start by writing msg header and then the processed body
	     */
	    failure = !(so_nputs(so, fetch, (long) hlen)
			&& save_ex_output_body(stream, raw, section,
					       body, &tlen, pc));
	}

	gf_clear_so_writec(so);

	if(failure)
	  return(0);

	q_status_message(SM_ORDER, 3, 3,
		    "NOTE: Deleted message parts NOT included in saved copy!");

    }
    else{
	/* First, write the header we just fetched... */
	if(!so_nputs(so, fetch, (long) hlen))
	  return(0);

#if	defined(DOS) && !defined(WIN32)
	/*
	 * Set append file and install dos_gets so message text
	 * is fetched directly to disk.
	 */
	mail_parameters(stream, SET_GETS, (void *)dos_gets);
	append_file = (FILE *) so_text(so);
	mail_gc(stream, GC_TEXTS);
#endif

	old_imap_server = is_imap_stream(stream) && !modern_imap_stream(stream);

	/* Second, go fetch the corresponding text... */
	fetch = mail_fetch_text(stream, raw, sect, &tlen,
				!old_imap_server ? FT_PEEK : 0);

	/*
	 * Special handling in case we're fetching a Message/rfc822
	 * segment and we're talking to an old server...
	 */
	if(fetch && *fetch == '\0' && sect && (hlen + tlen) != size){
	    so_seek(so, 0L, 0);
	    fetch = mail_fetch_body(stream, raw, sect, &tlen, 0L);
	}

	/*
	 * Pre IMAP4 servers can't do a non-peeking mail_fetch_text,
	 * so if the message we are saving from was originally unseen,
	 * we have to change it back to unseen. Flags contains the
	 * string "SEEN" if the original message was seen.
	 */
	if(old_imap_server && (!flags || !srchstr(flags,"SEEN"))){
	    char seq[20];

	    strcpy(seq, long2string(raw));
	    mail_flag(stream, seq, "\\SEEN", 0);
	}


#if	!(defined(DOS) && !defined(WIN32))
	/* If fetch succeeded, write the result */
	if(!(fetch && so_nputs(so, fetch, tlen)))
	   return(0);
#else
	/*
	 * Clean up after our DOS hacks...
	 */
	append_file = NULL;
	mail_parameters(stream, SET_GETS, (void *)NULL);
	mail_gc(stream, GC_TEXTS);

	if(!fetch)
	  return(0);
#endif
    }

    so_seek(so, 0L, 0);			/* rewind just in case */

    /*
     * Set up a c-client string driver so we can hand the
     * collected text down to mail_append.
     *
     * NOTE: We only test the size if and only if we already
     *	     have it.  See, in some drivers, especially under
     *	     dos, its too expensive to get the size (full
     *	     header and body text fetch plus MIME parse), so
     *	     we only verify the size if we already know it.
     */
#if	defined(DOS) && !defined(WIN32)
    d.fd  = fileno((FILE *)so_text(so));
    d.pos = 0L;
    mlen = filelength(d.fd);
#else
    mlen = hlen + tlen;
#endif

    if(size && mlen < size){
	char buf[128];

	sprintf(buf, "Message to save shrank!  (#%ld: %ld --> %ld)",
		raw, size, mlen);
	q_status_message(SM_ORDER | SM_DING, 3, 4, buf);
	dprint(1, (debugfile, "BOTCH: %s\n", buf));
	return(0);
    }

#if	defined(DOS) && !defined(WIN32)
    INIT(&msg, dawz_string, (void *)&d, mlen);
#else
    INIT(&msg, mail_string, (void *)so_text(so), mlen);
#endif

    rc = 0;
    while(!(rv = (int) context_append_full(context, save_stream,
					   save_folder, flags,
					   *date ? date : NULL,
					   &msg))){
	if(rc++ || !ps_global->try_to_create)	/* abysmal failure! */
	  break;				/* c-client returned error? */

	if(context && (context->use & CNTXT_INCMNG)
	   && context_isambig(save_folder)){
	    q_status_message(SM_ORDER, 3, 5,
	       "Can only save to existing folders in Incoming Collection");
	    break;
	}

	ps_global->try_to_create = 0;		/* reset for next time */
	if((j = create_for_save(save_stream,context,save_folder)) < 1){
	    if(j < 0)
	      rv = -1;			/* user cancelled */

	    break;
	}

	SETPOS((&msg), 0L);			/* reset string driver */
    }

    return(rv);
}


/*
 * save_ex_replace_body -
 *
 * NOTE : hlen points to a cell that has the byte count of "hdr" on entry
 *	  *BUT* which is to contain the count of written bytes on exit
 */
int
save_ex_replace_body(hdr, hlen, body, pc)
    char	   *hdr;
    unsigned long  *hlen;
    BODY	   *body;
    gf_io_t	    pc;
{
    unsigned long len;

    /*
     * "X-" out the given MIME headers unless they're
     * the same as we're going to substitute...
     */
    if(body->type == TYPETEXT
       && (!body->subtype || !strucmp(body->subtype, "plain"))
       && body->encoding == ENC7BIT){
	if(!gf_nputs(hdr, *hlen, pc))	/* write out header */
	  return(0);
    }
    else{
	int bol = 1;

	/*
	 * write header, "X-"ing out transport headers bothersome to
	 * software but potentially useful to the human recipient...
	 */
	for(len = *hlen; len; len--){
	    if(bol){
		unsigned long n;

		bol = 0;
		if(save_ex_mask_types(hdr, &n, pc))
		  *hlen += n;		/* add what we inserted */
		else
		  break;
	    }

	    if(*hdr == '\015' && *(hdr+1) == '\012'){
		bol++;			/* remember beginning of line */
		len--;			/* account for LF */
		if(gf_nputs(hdr, 2, pc))
		  hdr += 2;
		else
		  break;
	    }
	    else if(!(*pc)(*hdr++))
	      break;
	}

	if(len)				/* bytes remain! */
	  return(0);
    }

    /* Now, blat out explanatory text as the body... */
    if(save_ex_explain_body(body, &len, pc)){
	*hlen += len;
	return(1);
    }
    else
      return(0);
}



int
save_ex_output_body(stream, raw, section, body, len, pc)
    MAILSTREAM	  *stream;
    long	   raw;
    char	  *section;
    BODY	  *body;
    unsigned long *len;
    gf_io_t	   pc;
{
    char	  *txtp, newsect[128];
    unsigned long  ilen;

    txtp = mail_fetch_mime(stream, raw, section, len, FT_PEEK);

    if(msgno_part_deleted(stream, raw, section))
      return(save_ex_replace_body(txtp, len, body, pc));

    if(body->type == TYPEMULTIPART){
	char	  *subsect, boundary[128];
	int	   n, blen;
	PART	  *part = body->nested.part;
	PARAMETER *param;

	/* Locate supplied supplied multipart boundary */
	for (param = body->parameter; param; param = param->next)
	  if (!strucmp(param->attribute, "boundary")){
	      sprintf(boundary, "--%.*s\015\012", sizeof(boundary)-10,
		      param->value);
	      blen = strlen(boundary);
	      break;
	  }

	if(!param){
	    q_status_message(SM_ORDER|SM_DING, 3, 3, "Missing MIME boundary");
	    return(0);
	}

	/*
	 * BUG: if multi/digest and a message deleted (which we'll
	 * change to text/plain), we need to coerce this composite
	 * type to multi/mixed !!
	 */
	if(!gf_nputs(txtp, *len, pc))		/* write MIME header */
	  return(0);

	/* Prepare to specify sub-sections */
	strncpy(newsect, section, sizeof(newsect));
	newsect[sizeof(newsect)-1] = '\0';
	subsect = &newsect[n = strlen(newsect)];
	if(n > 2 && !strcmp(&newsect[n-2], ".0"))
	  subsect--;
	else if(n)
	  *subsect++ = '.';

	n = 1;
	do {				/* for each part */
	    strcpy(subsect, int2string(n++));
	    if(gf_puts(boundary, pc)
		 && save_ex_output_body(stream, raw, newsect,
					&part->body, &ilen, pc))
	      *len += blen + ilen;
	    else
	      return(0);
	}
	while (part = part->next);	/* until done */

	sprintf(boundary, "--%.*s--\015\012", sizeof(boundary)-10,param->value);
	*len += blen + 2;
	return(gf_puts(boundary, pc));
    }

    /* Start by writing the part's MIME header */
    if(!gf_nputs(txtp, *len, pc))
      return(0);
    
    if(body->type == TYPEMESSAGE
       && (!body->subtype || !strucmp(body->subtype, "rfc822"))){
	/* write RFC 822 message's header */
	if((txtp = mail_fetch_header(stream,raw,section,NULL,&ilen,FT_PEEK))
	     && gf_nputs(txtp, ilen, pc))
	  *len += ilen;
	else
	  return(0);

	/* then go deal with its body parts */
	sprintf(newsect, "%.10s%s%s", section, section ? "." : "",
		(body->nested.msg->body->type == TYPEMULTIPART) ? "0" : "1");
	if(save_ex_output_body(stream, raw, newsect,
			       body->nested.msg->body, &ilen, pc)){
	    *len += ilen;
	    return(1);
	}

	return(0);
    }

    /* Write corresponding body part */
    if((txtp = mail_fetch_body(stream, raw, section, &ilen, FT_PEEK))
       && gf_nputs(txtp, (long) ilen, pc) && gf_puts("\015\012", pc)){
	*len += ilen + 2;
	return(1);
    }

    return(0);
}



/*----------------------------------------------------------------------
    Mask off any header entries we don't want to show up in exceptional saves

Args:  hdr -- pointer to start of a header line
       pc -- function to write the prefix

  ----*/
int
save_ex_mask_types(hdr, len, pc)
    char	  *hdr;
    unsigned long *len;
    gf_io_t	   pc;
{
    char *s = NULL;

    if(!struncmp(hdr, "content-type:", 13))
      s = "Content-Type: Text/Plain; charset=US-ASCII\015\012X-";
    else if(!struncmp(hdr, "content-description:", 20))
      s = "Content-Description: Deleted Attachment\015\012X-";
    else if(!struncmp(hdr, "content-transfer-encoding:", 26)
	    || !struncmp(hdr, "content-disposition:", 20))
      s = "X-";

    return((*len = s ? strlen(s) : 0) ? gf_puts(s, pc) : 1);
}


int
save_ex_explain_body(body, len, pc)
    BODY	  *body;
    unsigned long *len;
    gf_io_t	   pc;
{
    unsigned long   ilen;
    char	  **blurbp;
    static char    *blurb[] = {
	"The following attachments were DELETED when this message was saved:",
	NULL
    };

    *len = 0;
    for(blurbp = blurb; *blurbp; blurbp++)
      if(save_ex_output_line(*blurbp, &ilen, pc))
	*len += ilen;
      else
	return(0);

    if(!save_ex_explain_parts(body, 0, &ilen, pc))
      return(0);

    *len += ilen;
    return(1);
}


int
save_ex_explain_parts(body, depth, len, pc)
    BODY	  *body;
    int		   depth;
    unsigned long *len;
    gf_io_t	   pc;
{
    char	  tmp[MAILTMPLEN], buftmp[MAILTMPLEN];
    unsigned long ilen;

    if(body->type == TYPEMULTIPART) {   /* multipart gets special handling */
	PART *part = body->nested.part;	/* first body part */

	*len = 0;
	if(body->description && *body->description){
	    sprintf(tmp, "%*.*sA %s/%.*s segment described",
		    depth, depth, " ", body_type_names(body->type),
		    sizeof(tmp)-100, body->subtype ? body->subtype : "Unknown");
	    if(!save_ex_output_line(tmp, len, pc))
	      return(0);

	    sprintf(buftmp, "%.75s", body->description);
	    sprintf(tmp, "%*.*s  as \"%.50s\" containing:", depth, depth, " ",
		    (char *) rfc1522_decode((unsigned char *)tmp_20k_buf,
					    SIZEOF_20KBUF, buftmp, NULL));
	}
	else{
	    sprintf(tmp, "%*.*sA %s/%.*s segment containing:",
		    depth, depth, " ",
		    body_type_names(body->type),
		    sizeof(tmp)-100, body->subtype ? body->subtype : "Unknown");
	}

	if(save_ex_output_line(tmp, &ilen, pc))
	  *len += ilen;
	else
	  return(0);

	depth++;
	do				/* for each part */
	  if(save_ex_explain_parts(&part->body, depth, &ilen, pc))
	    *len += ilen;
	  else
	    return(0);
	while (part = part->next);	/* until done */
    }
    else{
	sprintf(tmp, "%*.*sA %s/%.*s segment of about %s bytes%s",
		depth, depth, " ",
		body_type_names(body->type), 
		sizeof(tmp)-100, body->subtype ? body->subtype : "Unknown",
		comatose((body->encoding == ENCBASE64)
			   ? ((body->size.bytes * 3)/4)
			   : body->size.bytes),
		body->description ? "," : ".");
	if(!save_ex_output_line(tmp, len, pc))
	  return(0);

	if(body->description && *body->description){
	    sprintf(buftmp, "%.75s", body->description);
	    sprintf(tmp, "%*.*s   described as \"%.*s\"", depth, depth, " ",
		    sizeof(tmp)-100,
		    (char *) rfc1522_decode((unsigned char *)tmp_20k_buf,
					    SIZEOF_20KBUF, buftmp, NULL));
	    if(save_ex_output_line(tmp, &ilen, pc))
	      *len += ilen;
	    else
	      return(0);
	}
    }

    return(1);
}



int
save_ex_output_line(line, len, pc)
    char	  *line;
    unsigned long *len;
    gf_io_t	   pc;
{
    sprintf(tmp_20k_buf, "  [ %-*.*s ]\015\012", 68, 68, line);
    *len = strlen(tmp_20k_buf);
    return(gf_puts(tmp_20k_buf, pc));
}



/*----------------------------------------------------------------------
    Offer to create a non-existant folder to copy message[s] into

   Args: stream -- stream to use for creation
	 context -- context to create folder in
	 name -- name of folder to create

 Result: 0 if create failed (c-client writes error)
	 1 if create successful
	-1 if user declines to create folder
 ----*/
int
create_for_save(stream, context, folder)
    MAILSTREAM *stream;
    CONTEXT_S  *context;
    char       *folder;
{
    if(context && ps_global->context_list->next && context_isambig(folder)){
	if(context->use & CNTXT_INCMNG){
	    sprintf(tmp_20k_buf,
		"\"%.15s%s\" doesn't exist - Add it in FOLDER LIST screen",
		folder, (strlen(folder) > 15) ? "..." : "");
	    q_status_message(SM_ORDER, 3, 3, tmp_20k_buf);
	    return(0);		/* error */
	}

	sprintf(tmp_20k_buf,
		"Folder \"%.15s%s\" in <%.15s%s> doesn't exist. Create",
		folder, (strlen(folder) > 15) ? "..." : "",
		context->nickname,
		(strlen(context->nickname) > 15) ? "..." : "");
    }
    else
      sprintf(tmp_20k_buf,"Folder \"%.40s%s\" doesn't exist.  Create", 
	      folder, strlen(folder) > 40 ? "..." : "");

    if(want_to(tmp_20k_buf, 'y', 'n', NO_HELP, WT_SEQ_SENSITIVE) != 'y'){
	cmd_cancelled("Save message");
	return(-1);
    }

    return(context_create(context, NULL, folder));
}



/*----------------------------------------------------------------------
  Build flags string based on requested flags and what's set in messagecache

   Args: mc -- message cache element to dig the flags out of
	 flags -- flags to test
	 flagbuf -- place to write string representation of bits

 Result: flags represented in bits and mask written in flagbuf
 ----*/
void
flag_string(mc, flags, flagbuf)
    MESSAGECACHE *mc;
    long	  flags;
    char	 *flagbuf;
{
    char *p;

    *(p = flagbuf) = '\0';

    if((flags & F_DEL) && mc->deleted)
      sstrcpy(&p, "\\DELETED ");

    if((flags & F_ANS) && mc->answered)
      sstrcpy(&p, "\\ANSWERED ");

    if((flags & F_FLAG) && mc->flagged)
      sstrcpy(&p, "\\FLAGGED ");

    if((flags & F_SEEN) && mc->seen)
      sstrcpy(&p, "\\SEEN ");

    if(p != flagbuf)
      *--p = '\0';			/* tie off tmp_20k_buf   */
}



/*----------------------------------------------------------------------
   Save() helper function to create canonical date string from given header

   Args: date -- buf to recieve canonical date string
	 header -- rfc822 header to fish date string from

 Result: date filled with canonicalized date in header, or null string
 ----*/
void
saved_date(date, header)
    char *date, *header;
{
    char	 *d, *p, c;
    MESSAGECACHE  elt;

    *date = '\0';
    if((toupper((unsigned char)(*(d = header)))
	== 'D' && !strncmp(d, "date:", 5))
       || (d = srchstr(header, "\015\012date:"))){
	for(d += 7; *d == ' '; d++)
	  ;					/* skip white space */

	if(p = strstr(d, "\015\012")){
	    for(; p > d && *p == ' '; p--)
	      ;					/* skip white space */

	    c  = *p;
	    *p = '\0';				/* tie off internal date */
	}

	if(mail_parse_date(&elt, d))		/* let c-client normalize it */
	  mail_date(date, &elt);

	if(p)					/* restore header */
	  *p = c;
    }
}



/*----------------------------------------------------------------------
    Export a message to a plain file in users home directory

    Args: state -- pointer to struct holding a bunch of pine state
	 msgmap -- table mapping msg nums to c-client sequence nums
	  qline -- screen line to ask questions on
	    agg -- boolean indicating we're to operate on aggregate set

 Result: 
 ----*/
void
cmd_export(state, msgmap, qline, agg)
    struct pine *state;
    MSGNO_S     *msgmap;
    int          qline;
    int		 agg;
{
    char      filename[MAXPATH+1], full_filename[MAXPATH+1], *err;
    char      nmsgs[80];
    int       r, leading_nl, failure = 0, orig_errno, over = 0;
    ENVELOPE *env;
    BODY     *b;
    long      i, count = 0L, start_of_append;
    gf_io_t   pc;
    STORE_S  *store;
    struct variable *vars = ps_global->vars;
    ESCKEY_S export_opts[5];

    if(ps_global->restricted){
	q_status_message(SM_ORDER, 0, 3,
	    "Pine demo can't export messages to files");
	return;
    }

    if(agg && !pseudo_selected(msgmap))
      return;

    export_opts[i = 0].ch  = ctrl('T');
    export_opts[i].rval	   = 10;
    export_opts[i].name	   = "^T";
    export_opts[i++].label = "To Files";

#if	!defined(DOS) && !defined(MAC) && !defined(OS2)
    if(ps_global->VAR_DOWNLOAD_CMD && ps_global->VAR_DOWNLOAD_CMD[0]){
	export_opts[i].ch      = ctrl('V');
	export_opts[i].rval    = 12;
	export_opts[i].name    = "^V";
	export_opts[i++].label = "Downld Msg";
    }
#endif	/* !(DOS || MAC) */

    if(F_ON(F_ENABLE_TAB_COMPLETE,ps_global)){
	export_opts[i].ch      =  ctrl('I');
	export_opts[i].rval    = 11;
	export_opts[i].name    = "TAB";
	export_opts[i++].label = "Complete";
    }

#if	0
    /* Commented out since it's not yet support! */
    if(F_ON(F_ENABLE_SUB_LISTS,ps_global)){
	export_opts[i].ch      = ctrl('X');
	export_opts[i].rval    = 14;
	export_opts[i].name    = "^X";
	export_opts[i++].label = "ListMatches";
    }
#endif

    export_opts[i].ch = -1;
    filename[0] = '\0';

    if(mn_total_cur(msgmap) <= 1L)
      sprintf(nmsgs, "Msg #%ld", mn_get_cur(msgmap));
    else
      sprintf(nmsgs, "%.10s messages", comatose(mn_total_cur(msgmap)));

    r = get_export_filename(state, filename, full_filename, sizeof(filename),
			    nmsgs, "EXPORT", export_opts, &over, qline,
			    GE_IS_EXPORT | GE_SEQ_SENSITIVE);

    if(r < 0){
	switch(r){
	  case -1:
	    cmd_cancelled("Export message");
	    break;

	  case -2:
	    q_status_message1(SM_ORDER, 0, 2,
			      "Can't export to file outside of %.200s",
			      VAR_OPER_DIR);
	    break;
	}

	goto fini;
    }
#if	!defined(DOS) && !defined(MAC) && !defined(OS2)
    else if(r == 12){			/* Download */
	char     cmd[MAXPATH], *tfp = NULL;
	int	     next = 0;
	PIPE_S  *syspipe;
	STORE_S *so;
	gf_io_t  pc;

	if(ps_global->restricted){
	    q_status_message(SM_ORDER | SM_DING, 3, 3,
			     "Download disallowed in restricted mode");
	    goto fini;
	}

	err = NULL;
	tfp = temp_nam(NULL, "pd");
	build_updown_cmd(cmd, ps_global->VAR_DOWNLOAD_CMD_PREFIX,
			 ps_global->VAR_DOWNLOAD_CMD, tfp);
	dprint(1, (debugfile, "Download cmd called: \"%s\"\n", cmd));
	if(so = so_get(FileStar, tfp, WRITE_ACCESS|OWNER_ONLY)){
	    gf_set_so_writec(&pc, so);

	    for(i = mn_first_cur(msgmap); i > 0L; i = mn_next_cur(msgmap))
	      if(!(env = mail_fetchstructure(state->mail_stream,
					     mn_m2raw(msgmap, i), &b))
		 || !bezerk_delimiter(env, pc, next++)
		 || !format_message(mn_m2raw(msgmap, mn_get_cur(msgmap)),
				    env, b, NULL, FM_NEW_MESS | FM_NOWRAP, pc)){
		  q_status_message(SM_ORDER | SM_DING, 3, 3,
			   err = "Error writing tempfile for download");
		  break;
	      }

	    gf_clear_so_writec(so);
	    if(so_give(&so)){			/* close file */
		if(!err)
		  err = "Error writing tempfile for download";
	    }

	    if(!err){
		if(syspipe = open_system_pipe(cmd, NULL, NULL,
					      PIPE_USER | PIPE_RESET, 0))
		  (void) close_system_pipe(&syspipe);
		else
		  q_status_message(SM_ORDER | SM_DING, 3, 3,
				err = "Error running download command");
	    }
	}
	else
	  q_status_message(SM_ORDER | SM_DING, 3, 3,
			 err = "Error building temp file for download");

	if(tfp){
	    unlink(tfp);
	    fs_give((void **)&tfp);
	}

	if(!err)
	  q_status_message(SM_ORDER, 0, 3, "Download Command Completed");

	goto fini;
    }
#endif	/* !(DOS || MAC) */


    switch(over){
      case 0:
      case 1:
	leading_nl = 0;
	break;

      case -1:
	leading_nl = 1;
	break;
    }

    dprint(5, (debugfile, "Opening file \"%s\" for export\n", full_filename));

    if(!(store = so_get(FileStar, full_filename, WRITE_ACCESS))){
        q_status_message2(SM_ORDER | SM_DING, 3, 4,
		      "Error opening file \"%.200s\" to export message: %.200s",
                          full_filename, error_description(errno));
	goto fini;
    }
    else
      gf_set_so_writec(&pc, store);

    err = NULL;
    for(i = mn_first_cur(msgmap); i > 0L; i = mn_next_cur(msgmap), count++){
	env = mail_fetchstructure(state->mail_stream, mn_m2raw(msgmap, i), &b);
	if(!env) {
	    err = "Can't export message. Error accessing mail folder";
	    failure = 1;
	    break;
	}

	start_of_append = ftell((FILE *)so_text(store));
	if(!bezerk_delimiter(env, pc, leading_nl)
	   || !format_message(mn_m2raw(msgmap, i), env, b, NULL,
			      FM_NEW_MESS | FM_NOWRAP, pc)){
	    orig_errno = errno;		/* save incase things are really bad */
	    failure    = 1;		/* pop out of here */
	    break;
	}

	leading_nl = 1;
    }

    gf_clear_so_writec(store);
    if(so_give(&store))				/* release storage */
      failure++;

    if(failure){
	truncate(full_filename, start_of_append);
	if(err){
	    dprint(1, (debugfile, "FAILED Export: fetch(%ld): %s\n",
		       i, err));
	    q_status_message(SM_ORDER | SM_DING, 3, 4, err);
	}
	else{
	    dprint(1, (debugfile, "FAILED Export: file \"%s\" : %s\n",
		       full_filename,  error_description(orig_errno)));
	    q_status_message2(SM_ORDER | SM_DING, 3, 4,
			      "Error exporting to \"%.200s\" : %.200s",
			      filename, error_description(orig_errno));
	}
    }
    else{
	if(mn_total_cur(msgmap) > 1L)
	  q_status_message4(SM_ORDER,0,3,
			    "%.200s message%.200s %.200s to file \"%.200s\"",
			    long2string(count), plural(count),
			    over==0 ? "exported"
				    : over==1 ? "overwritten" : "appended",
			    filename);
	else
	  q_status_message3(SM_ORDER,0,3,
			    "Message %.200s %.200s to file \"%.200s\"",
			    long2string(mn_get_cur(msgmap)),
			    over==0 ? "exported"
				    : over==1 ? "overwritten" : "appended",
			    filename);
    }

  fini:
    if(agg)
      restore_selected(msgmap);
}



/*
 * Ask user what file to export to. Export from srcstore to that file.
 *
 * Args     ps -- pine struct
 *     srctext -- pointer to source text
 *     srctype -- type of that source text
 *  prompt_msg -- see get_export_filename
 *  lister_msg --      "
 *
 * Returns: != 0 : error
 *             0 : ok
 */
int
simple_export(ps, srctext, srctype, prompt_msg, lister_msg)
    struct pine *ps;
    void        *srctext;
    SourceType   srctype;
    char        *prompt_msg;
    char        *lister_msg;
{
    int r = 1, over;
    char     filename[MAXPATH+1], full_filename[MAXPATH+1];
    STORE_S *store = NULL;
    struct variable *vars = ps->vars;
    static ESCKEY_S simple_export_opts[] = {
	{ctrl('T'), 10, "^T", "To Files"},
	{-1, 0, NULL, NULL},
	{-1, 0, NULL, NULL}};

    if(F_ON(F_ENABLE_TAB_COMPLETE,ps)){
	simple_export_opts[r].ch    =  ctrl('I');
	simple_export_opts[r].rval  = 11;
	simple_export_opts[r].name  = "TAB";
	simple_export_opts[r].label = "Complete";
    }

    if(!srctext){
	q_status_message(SM_ORDER, 0, 2, "Error allocating space");
	r = -3;
	goto fini;
    }

    simple_export_opts[++r].ch = -1;
    filename[0] = '\0';
    full_filename[0] = '\0';

    r = get_export_filename(ps, filename, full_filename, sizeof(filename),
			    prompt_msg, lister_msg, simple_export_opts, &over,
			    -FOOTER_ROWS(ps), GE_IS_EXPORT);

    if(r < 0)
      goto fini;
    else if(!full_filename[0]){
	r = -1;
	goto fini;
    }

    dprint(5, (debugfile, "Opening file \"%s\" for export\n", full_filename));

    if((store = so_get(FileStar, full_filename, WRITE_ACCESS)) != NULL){
	char *pipe_err;
	gf_io_t pc, gc;

	gf_set_so_writec(&pc, store);
	gf_set_readc(&gc, srctext, (srctype == CharStar)
					? strlen((char *)srctext)
					: 0L,
		     srctype);
	gf_filter_init();
	if((pipe_err = gf_pipe(gc, pc)) != NULL){
	    q_status_message2(SM_ORDER | SM_DING, 3, 3,
			      "Problem saving to \"%.200s\": %.200s",
			      filename, pipe_err);
	    r = -3;
	}
	else
	  r = 0;

	gf_clear_so_writec(store);
	if(so_give(&store)){
	    q_status_message2(SM_ORDER | SM_DING, 3, 3,
			      "Problem saving to \"%.200s\": %.200s",
			      filename, error_description(errno));
	    r = -3;
	}
    }
    else{
	q_status_message2(SM_ORDER | SM_DING, 3, 4,
			  "Error opening file \"%.200s\" for export: %.200s",
			  full_filename, error_description(errno));
	r = -3;
    }

fini:
    switch(r){
      case  0:
	/* overloading full_filename */
	sprintf(full_filename, "%c%s",
		(prompt_msg && prompt_msg[0])
		  ? (islower((unsigned char)prompt_msg[0])
		    ? toupper((unsigned char)prompt_msg[0]) : prompt_msg[0])
		  : 'T',
	        (prompt_msg && prompt_msg[0]) ? prompt_msg+1 : "ext");
	q_status_message3(SM_ORDER,0,2,"%.200s %.200s to \"%.200s\"",
			  full_filename,
			  over==0 ? "exported" :
			    over==1 ? "overwritten" : "appended",
			  filename);
	break;

      case -1:
	cmd_cancelled("Export");
	break;

      case -2:
	q_status_message1(SM_ORDER, 0, 2,
	    "Can't export to file outside of %.200s", VAR_OPER_DIR);
	break;
    }

    ps->mangled_footer = 1;
    return(r);
}



/*
 * Ask user what file to export to.
 *
 *       filename -- On input, this it the filename to start with. On exit,
 *                   this is the filename chosen. (but this isn't used)
 *  full_filename -- This is the full filename on exit.
 *            len -- Minimum length of _both_ filename and full_filename.
 *     prompt_msg -- Message to insert in prompt.
 *     lister_msg -- Message to insert in file_lister.
 *           opts -- Key options.
 *           over -- This is a return value. over ==  1 => did overwrite
 *                                                         of existing file
 *                                                ==  0 => file didn't exist
 *                                                == -1 => did append
 *          qline -- Command line to prompt on.
 *          flags -- Logically OR'd flags
 *                     GE_IS_EXPORT     - The command was an Export command
 *                                        so the prompt should include
 *                                        EXPORT:.
 *                     GE_SEQ_SENSITIVE - The command that got us here is
 *                                        sensitive to sequence number changes
 *                                        caused by unsolicited expunges.
 *
 *  Returns:  -1  cancelled
 *            -2  prohibited by VAR_OPER_DIR
 *            -3  other error, already reported here
 *             0  ok
 *            12  user chose 12 command from opts
 */
int
get_export_filename(ps, filename, full_filename, len, prompt_msg,
		    lister_msg, opts, over, qline, flags)
    struct pine *ps;
    char        *filename;
    char        *full_filename;
    size_t       len;
    char        *prompt_msg;
    char        *lister_msg;
    ESCKEY_S     opts[];
    int         *over;
    int          qline;
    int          flags;
{
    HelpType  help = NO_HELP;
    char      dir[MAXPATH+1], dir2[MAXPATH+1];
    char      precolon[MAXPATH+1], postcolon[MAXPATH+1];
    char      filename2[MAXPATH+1], tmp[MAXPATH+1], *fn, *ill;
    int       l, i, r, fatal, homedir = 0, was_abs_path=0;
    char      prompt_buf[200];
    struct variable *vars = ps->vars;

    if(F_ON(F_USE_CURRENT_DIR, ps))
      dir[0] = '\0';
    else if(VAR_OPER_DIR)
      strcpy(dir, VAR_OPER_DIR);
#if	defined(DOS) || defined(OS2)
    else if(VAR_FILE_DIR)
      strcpy(dir, VAR_FILE_DIR);
#endif
    else{
	dir[0] = '~';
	dir[1] = '\0';
	homedir=1;
    }
    strcpy(precolon, dir);
    postcolon[0] = '\0';

    /*---------- Prompt the user for the file name -------------*/
    while(1){
	int oeflags;

	sprintf(prompt_buf, "%sCopy %.*s to file%s%s: ",
		(flags & GE_IS_EXPORT) ? "EXPORT: " : "SAVE: ",
		sizeof(prompt_buf)-50, prompt_msg,
		is_absolute_path(filename) ? "" : " in ",
		is_absolute_path(filename) ? "" :
		  (!dir[0] ? "current directory"
			   : (dir[0] == '~' && !dir[1]) ? "home directory"
						        : dir));
	oeflags = OE_APPEND_CURRENT |
		  ((flags & GE_SEQ_SENSITIVE) ? OE_SEQ_SENSITIVE : 0);
	r = optionally_enter(filename, qline, 0, len, prompt_buf,
			     opts, help, &oeflags);

        /*--- Help ----*/
	if(r == 3){
	    help = (help == NO_HELP) ? h_oe_export : NO_HELP;
	    continue;
        }
	else if(r == 10 || r == 11){	/* Browser or File Completion */
	    if(filename[0]=='~'){
	      if(filename[1] == C_FILESEP && filename[2]!='\0'){
		precolon[0] = '~';
		precolon[1] = '\0';
		for(i=0; filename[i+2] != '\0' && i+2 < len-1; i++)
		  filename[i] = filename[i+2];
		filename[i] = '\0';
		strncpy(dir, precolon, sizeof(dir)-1);
		dir[sizeof(dir)-1] = '\0';
	      }
	      else if(filename[1]=='\0' || 
		 (filename[1] == C_FILESEP && filename[2] == '\0')){
		precolon[0] = '~';
		precolon[1] = '\0';
		filename[0] = '\0';
		strncpy(dir, precolon, sizeof(dir)-1);
		dir[sizeof(dir)-1] = '\0';
	      }
	    }
	    else if(!dir[0] && !is_absolute_path(filename) && was_abs_path){
	      if(homedir){
		precolon[0] = '~';
		precolon[1] = '\0';
		strncpy(dir, precolon, sizeof(dir)-1);
		dir[sizeof(dir)-1] = '\0';
	      }
	      else{
		precolon[0] = '\0';
		dir[0] = '\0';
	      }
	    }
	    l = MAXPATH;
	    dir2[0] = '\0';
	    strncpy(tmp, filename, sizeof(tmp)-1);
	    tmp[sizeof(tmp)-1] = '\0';
	    if(*tmp && is_absolute_path(tmp))
	      fnexpand(tmp, sizeof(tmp));
	    if(strncmp(tmp,postcolon, strlen(postcolon)))
	      postcolon[0] = '\0';

	    if(*tmp && (fn = last_cmpnt(tmp))){
	        l -= fn - tmp;
		strncpy(filename2, fn, sizeof(filename2)-1);
		filename2[sizeof(filename2)-1] = '\0';
		if(is_absolute_path(tmp)){
		    strncpy(dir2, tmp, min(fn - tmp, sizeof(dir2)-1));
		    dir2[min(fn - tmp, sizeof(dir2)-1)] = '\0';
#ifdef _WINDOWS
		    if(tmp[1]==':' && tmp[2]=='\\' && dir2[2]=='\0'){
		      dir2[2] = '\\';
		      dir2[3] = '\0';
		    }
#endif
		    strncpy(postcolon, dir2, sizeof(postcolon)-1);
		    postcolon[sizeof(postcolon)-1] = '\0';
		    precolon[0] = '\0';
		}
		else{
		    char *p = NULL;
		    /*
		     * Just building the directory name in dir2,
		     * full_filename is overloaded.
		     */
		    sprintf(full_filename, "%.*s", min(fn-tmp,len-1), tmp);
		    strncpy(postcolon, full_filename, sizeof(postcolon)-1);
		    postcolon[sizeof(postcolon)-1] = '\0';
		    build_path(dir2, !dir[0] ? p = (char *)getcwd(NULL,MAXPATH)
					     : (dir[0] == '~' && !dir[1])
					       ? ps->home_dir
					       : dir,
			       full_filename, sizeof(dir2));
		    if(p)
		      free(p);
		}
	    }
	    else{
		if(is_absolute_path(tmp)){
		    strncpy(dir2, tmp, sizeof(dir2)-1);
		    dir2[sizeof(dir2)-1] = '\0';
#ifdef _WINDOWS
		    if(dir2[2]=='\0' && dir2[1]==':'){
		      dir2[2]='\\';
		      dir2[3]='\0';
		      strncpy(postcolon,dir2,sizeof(postcolon)-1);
		    }
#endif
		    filename2[0] = '\0';
		    precolon[0] = '\0';
		}
		else{
		    strncpy(filename2, tmp, sizeof(filename2)-1);
		    filename2[sizeof(filename2)-1] = '\0';
		    if(!dir[0])
		      (void)getcwd(dir2, sizeof(dir2));
		    else if(dir[0] == '~' && !dir[1]){
			strncpy(dir2, ps->home_dir, sizeof(dir2)-1);
			dir2[sizeof(dir2)-1] = '\0';
		    }
		    else{
			strncpy(dir2, dir, sizeof(dir2)-1);
			dir2[sizeof(dir2)-1] = '\0';
		    }
		    postcolon[0] = '\0';
		}
	    }

	    build_path(full_filename, dir2, filename2, len);
	    if(!strcmp(full_filename, dir2))
	      filename2[0] = '\0';
	    if(full_filename[strlen(full_filename)-1] == C_FILESEP 
	       && isdir(full_filename,NULL,NULL)){
	      if(strlen(full_filename) == 1)
		strncpy(postcolon, full_filename, sizeof(postcolon)-1);
	      else if(filename2[0])
		strncpy(postcolon, filename2, sizeof(postcolon)-1);
	      postcolon[sizeof(postcolon)-1] = '\0';
	      strncpy(dir2, full_filename, sizeof(dir2)-1);
	      dir2[sizeof(dir2)-1] = '\0';
	      filename2[0] = '\0';
	    }
#ifdef _WINDOWS  /* use full_filename even if not a valid directory */
	    else if(full_filename[strlen(full_filename)-1] == C_FILESEP){ 
	      strncpy(postcolon, filename2, sizeof(postcolon)-1);
	      postcolon[sizeof(postcolon)-1] = '\0';
	      strncpy(dir2, full_filename, sizeof(dir2)-1);
	      dir2[sizeof(dir2)-1] = '\0';
	      filename2[0] = '\0';
	    }
#endif
	    if(dir2[strlen(dir2)-1] == C_FILESEP && strlen(dir2)!=1
	       && strcmp(dir2+1, ":\\")) 
	      /* last condition to prevent stripping of '\\' 
		 in windows partition */
	      dir2[strlen(dir2)-1] = '\0';

	    if(r == 10){			/* File Browser */
		r = file_lister(lister_msg ? lister_msg : "EXPORT",
				dir2, MAXPATH+1, filename2, MAXPATH+1, 
                                TRUE, FB_SAVE);
#ifdef _WINDOWS
/* Windows has a special "feature" in which entering the file browser will
   change the working directory if the directory is changed at all (even
   clicking "Cancel" will change the working directory).
*/
		if(F_ON(F_USE_CURRENT_DIR, ps))
		  (void)getcwd(dir2,sizeof(dir2));
#endif
		if(isdir(dir2,NULL,NULL)){
		  strncpy(precolon, dir2, sizeof(precolon)-1);
		  precolon[sizeof(precolon)-1] = '\0';
		}
		strncpy(postcolon, filename2, sizeof(postcolon)-1);
		postcolon[sizeof(postcolon)-1] = '\0';
		if(r == 1){
		    build_path(full_filename, dir2, filename2, len);
		    if(isdir(full_filename, NULL, NULL)){
			strncpy(dir, full_filename, sizeof(dir)-1);
			dir[sizeof(dir)-1] = '\0';
			filename[0] = '\0';
		    }
		    else{
			fn = last_cmpnt(full_filename);
			strncpy(dir, full_filename,
				min(fn - full_filename, sizeof(dir)-1));
			dir[min(fn - full_filename, sizeof(dir)-1)] = '\0';
			if(fn - full_filename > 1)
			  dir[fn - full_filename - 1] = '\0';
		    }
		    
		    if(!strcmp(dir, ps->home_dir)){
			dir[0] = '~';
			dir[1] = '\0';
		    }

		    strncpy(filename, fn, len-1);
		    filename[len-1] = '\0';
		}
	    }
	    else{				/* File Completion */
	      if(!pico_fncomplete(dir2, filename2, l - 1))
		  Writechar(BELL, 0);
	      strncat(postcolon, filename2,
		      sizeof(postcolon)-1-strlen(postcolon));
	      
	      was_abs_path = is_absolute_path(filename);

	      if(!strcmp(dir, ps->home_dir)){
		dir[0] = '~';
		dir[1] = '\0';
	      }
	    }
	    strncpy(filename, postcolon, len-1);
	    filename[len-1] = '\0';
	    strncpy(dir, precolon, sizeof(dir)-1);
	    dir[sizeof(dir)-1] = '\0';

	    if(filename[0] == '~' && !filename[1]){
		dir[0] = '~';
		dir[1] = '\0';
		filename[0] = '\0';
	    }

	    continue;
	}
	else if(r == 12){	/* Download, caller handles it */
	    return(r);
	}
#if	0
	else if(r == 14){	/* List file names matching partial? */
	    continue;
	}
#endif
        else if(r == 1 || (r == 0 && filename[0] == '\0')){
	    return(-1);		/* Cancel */
        }
        else if(r == 4){
	    continue;
	}
	else if(r != 0){
	    Writechar(BELL, 0);
	    continue;
	}

        removing_trailing_white_space(filename);
        removing_leading_white_space(filename);

#if	defined(DOS) || defined(OS2)
	if(is_absolute_path(filename)){
	    fixpath(filename, len);
	}
#else
	if(filename[0] == '~'){
	    if(fnexpand(filename, len) == NULL){
		char *p = strindex(filename, '/');
		if(p != NULL)
		  *p = '\0';
		q_status_message1(SM_ORDER | SM_DING, 3, 3,
			  "Error expanding file name: \"%.200s\" unknown user",
			      filename);
		continue;
	    }
	}
#endif

	if(is_absolute_path(filename)){
	    strncpy(full_filename, filename, len-1);
	    full_filename[len-1] = '\0';
	}
	else{
	    if(!dir[0])
	      build_path(full_filename, (char *)getcwd(dir,sizeof(dir)),
			 filename, len);
	    else if(dir[0] == '~' && !dir[1])
	      build_path(full_filename, ps->home_dir, filename, len);
	    else
	      build_path(full_filename, dir, filename, len);
	}

        if((ill = filter_filename(full_filename, &fatal)) != NULL){
	    if(fatal){
		q_status_message1(SM_ORDER | SM_DING, 3, 3, "%.200s", ill);
		continue;
	    }
	    else{
/* BUG: we should beep when the key's pressed rather than bitch later */
		/* Warn and ask for confirmation. */
		sprintf(prompt_buf, "File name contains a '%s'.  %s anyway",
			ill, (flags & GE_IS_EXPORT) ? "Export" : "Save");
		if(want_to(prompt_buf, 'n', 0, NO_HELP,
		  ((flags & GE_SEQ_SENSITIVE) ? RB_SEQ_SENSITIVE : 0)) != 'y')
		  continue;
	    }
	}

	break;		/* Must have got an OK file name */
    }

    if(VAR_OPER_DIR && !in_dir(VAR_OPER_DIR, full_filename))
      return(-2);

    *over = 0;
    if(!can_access(full_filename, ACCESS_EXISTS)){
	int rbflags;

	static ESCKEY_S access_opts[] = {
	    {'o', 'o', "O", "Overwrite"},
	    {'a', 'a', "A", "Append"},
	    {-1, 0, NULL, NULL}};

	r = strlen(filename);
        sprintf(prompt_buf,
		"File \"%s%.*s\" already exists.  Overwrite or append it ? ",
		(r > 20) ? "..." : "",
		sizeof(prompt_buf)-100,
                filename + ((r > 20) ? r - 20 : 0));
	rbflags = RB_NORM | ((flags & GE_SEQ_SENSITIVE) ? RB_SEQ_SENSITIVE : 0);
	switch(radio_buttons(prompt_buf, -FOOTER_ROWS(ps_global), access_opts,
			     'a', 'x', NO_HELP, rbflags)){
	  case 'o' :
	    *over = 1;
	    if(truncate(full_filename, 0) < 0)
	      /* trouble truncating, but we'll give it a try anyway */
	      q_status_message2(SM_ORDER | SM_DING, 3, 5,
				"Warning: Cannot truncate old %.200s: %.200s",
				full_filename, error_description(errno));
	    break;

	  case 'a' :
 	    *over = -1;
	    break;

	  case 'x' :
	  default :
	    return(-1);
	}
    }

    return(0);
}


/*----------------------------------------------------------------------
  parse the config'd upload/download command

  Args: cmd -- buffer to return command fit for shellin'
	prefix --
	cfg_str --
	fname -- file name to build into the command

  Returns: pointer to cmd_str buffer or NULL on real bad error

  NOTE: One SIDE EFFECT is that any defined "prefix" string in the
	cfg_str is written to standard out right before a successful
	return of this function.  The call immediately following this
	function darn well better be the shell exec...
 ----*/
char *
build_updown_cmd(cmd, prefix, cfg_str, fname)
    char *cmd;
    char *prefix;
    char *cfg_str;
    char *fname;
{
    char *p;
    int   fname_found = 0;

    if(prefix && *prefix){
	/* loop thru replacing all occurances of _FILE_ */
	for(p = strcpy(cmd, prefix); (p = strstr(p, "_FILE_")); )
	  rplstr(p, 6, fname);

	fputs(cmd, stdout);
    }

    /* loop thru replacing all occurances of _FILE_ */
    for(p = strcpy(cmd, cfg_str); (p = strstr(p, "_FILE_")); ){
	rplstr(p, 6, fname);
	fname_found = 1;
    }

    if(!fname_found)
      sprintf(cmd + strlen(cmd), " %s", fname);

    dprint(4, (debugfile, "\n - build_updown_cmd = \"%s\" -\n", cmd));
    return(cmd);
}






/*----------------------------------------------------------------------
  Write a berzerk format message delimiter using the given putc function

    Args: e -- envelope of message to write
	  pc -- function to use 

    Returns: TRUE if we could write it, FALSE if there was a problem

    NOTE: follows delimiter with OS-dependent newline
 ----*/
int
bezerk_delimiter(env, pc, leading_newline)
    ENVELOPE *env;
    gf_io_t   pc;
    int	      leading_newline;
{
    time_t  now = time(0);
    char   *p = ctime(&now);
    
    /* write "[\n]From mailbox[@host] " */
    if(!((leading_newline ? gf_puts(NEWLINE, pc) : 1)
	 && gf_puts("From ", pc)
	 && gf_puts((env && env->from) ? env->from->mailbox
				       : "the-concourse-on-high", pc)
	 && gf_puts((env && env->from && env->from->host) ? "@" : "", pc)
	 && gf_puts((env && env->from && env->from->host) ? env->from->host
							  : "", pc)
	 && (*pc)(' ')))
      return(0);

    while(p && *p && *p != '\n')	/* write date */
      if(!(*pc)(*p++))
	return(0);

    if(!gf_puts(NEWLINE, pc))		/* write terminating newline */
      return(0);

    return(1);
}



/*----------------------------------------------------------------------
      Execute command to jump to a given message number

    Args: qline -- Line to ask question on

  Result: returns true if the use selected a new message, false otherwise

 ----*/
long
jump_to(msgmap, qline, first_num, sparms, in_index)
    MSGNO_S  *msgmap;
    int       qline, first_num;
    SCROLL_S *sparms;
    CmdWhere  in_index;
{
    char     jump_num_string[80], *j, prompt[70];
    HelpType help;
    int      rc;
    static ESCKEY_S jump_to_key[] = { {0, 0, NULL, NULL},
				      {ctrl('Y'), 10, "^Y", "First Msg"},
				      {ctrl('V'), 11, "^V", "Last Msg"},
				      {-1, 0, NULL, NULL} };

    dprint(4, (debugfile, "\n - jump_to -\n"));

#ifdef DEBUG
    if(sparms && sparms->jump_is_debug)
      return(get_level(qline, first_num, sparms));
#endif

    if(!any_messages(msgmap, NULL, "to Jump to"))
      return(0L);

    if(first_num && isdigit((unsigned char) first_num)){
	jump_num_string[0] = first_num;
	jump_num_string[1] = '\0';
    }
    else
      jump_num_string[0] = '\0';

    if(mn_total_cur(msgmap) > 1L){
	sprintf(prompt, "Unselect %.20s msgs in favor of number to be entered", 
		comatose(mn_total_cur(msgmap)));
	if((rc = want_to(prompt, 'n', 0, NO_HELP, WT_NORM)) == 'n')
	  return(0L);
    }

    sprintf(prompt, "%.10s number to jump to : ", in_index == ThrdIndx
						    ? "Thread"
						    : "Message");

    help = NO_HELP;
    while(1){
	int flags = OE_APPEND_CURRENT;

        rc = optionally_enter(jump_num_string, qline, 0,
                              sizeof(jump_num_string), prompt,
                              jump_to_key, help, &flags);
        if(rc == 3){
            help = help == NO_HELP
			? (in_index == ThrdIndx ? h_oe_jump_thd : h_oe_jump)
			: NO_HELP;
            continue;
        }
	else if(rc == 10 || rc == 11){
	    char warning[100];
	    long closest;

	    closest = closest_jump_target(rc == 10 ? 1L
					  : ((in_index == ThrdIndx)
					     ? msgmap->max_thrdno
					     : mn_get_total(msgmap)),
					  ps_global->mail_stream,
					  msgmap, 0,
					  in_index, warning);
	    /* ignore warning */
	    return(closest);
	}

	/*
	 * If we take out the *jump_num_string nonempty test in this if
	 * then the closest_jump_target routine will offer a jump to the
	 * last message. However, it is slow because you have to wait for
	 * the status message and it is annoying for people who hit J command
	 * by mistake and just want to hit return to do nothing, like has
	 * always worked. So the test is there for now. Hubert 2002-08-19
	 *
	 * Jumping to first/last message is now possible through ^Y/^V 
	 * commands above. jpf 2002-08-21
	 */
        if(rc == 0 && *jump_num_string != '\0'){
	    removing_trailing_white_space(jump_num_string);
	    removing_leading_white_space(jump_num_string);
            for(j=jump_num_string; isdigit((unsigned char)*j) || *j=='-'; j++)
	      ;

	    if(*j != '\0'){
	        q_status_message(SM_ORDER | SM_DING, 2, 2,
                           "Invalid number entered. Use only digits 0-9");
		jump_num_string[0] = '\0';
	    }
	    else{
		char warning[100];
		long closest, jump_num;

		if(*jump_num_string)
		  jump_num = atol(jump_num_string);
		else
		  jump_num = -1L;

		warning[0] = '\0';
		closest = closest_jump_target(jump_num, ps_global->mail_stream,
					      msgmap,
					      *jump_num_string ? 0 : 1,
					      in_index, warning);
		if(warning[0])
		  q_status_message(SM_ORDER | SM_DING, 2, 2, warning);

		if(closest == jump_num)
		  return(jump_num);

		if(closest == 0L)
		  jump_num_string[0] = '\0';
		else
		  strncpy(jump_num_string, long2string(closest),
			  sizeof(jump_num_string));
            }

            continue;
	}

        if(rc != 4)
          break;
    }

    return(0L);
}


#ifdef DEBUG
long
get_level(qline, first_num, sparms)
    int      qline, first_num;
    SCROLL_S *sparms;
{
    char     debug_num_string[80], *j, prompt[70];
    HelpType help;
    int      rc;
    long     debug_num;

    if(first_num && isdigit((unsigned char)first_num)){
	debug_num_string[0] = first_num;
	debug_num_string[1] = '\0';
	debug_num = atol(debug_num_string);
	*(int *)(sparms->proc.data.p) = debug_num;
	q_status_message1(SM_ORDER, 0, 3, "Show debug <= level %.200s",
			  comatose(debug_num));
	return(1L);
    }
    else
      debug_num_string[0] = '\0';

    sprintf(prompt, "Show debug <= this level (0-%d) : ", max(debug, 9));

    help = NO_HELP;
    while(1){
	int flags = OE_APPEND_CURRENT;

        rc = optionally_enter(debug_num_string, qline, 0,
                              sizeof(debug_num_string), prompt,
                              NULL, help, &flags);
        if(rc == 3){
            help = help == NO_HELP ? h_oe_debuglevel : NO_HELP;
            continue;
        }

        if(rc == 0){
	    removing_leading_and_trailing_white_space(debug_num_string);
            for(j=debug_num_string; isdigit((unsigned char)*j); j++)
	      ;

	    if(*j != '\0'){
	        q_status_message(SM_ORDER | SM_DING, 2, 2,
                           "Invalid number entered. Use only digits 0-9");
		debug_num_string[0] = '\0';
	    }
	    else{
		debug_num = atol(debug_num_string);
		if(debug_num < 0)
	          q_status_message(SM_ORDER | SM_DING, 2, 2,
				   "Number should be >= 0");
		else if(debug_num > max(debug,9))
	          q_status_message1(SM_ORDER | SM_DING, 2, 2,
				   "Maximum is %.200s", comatose(max(debug,9)));
		else{
		    *(int *)(sparms->proc.data.p) = debug_num;
		    q_status_message1(SM_ORDER, 0, 3,
				      "Show debug <= level %.200s",
				      comatose(debug_num));
		    return(1L);
		}
            }

            continue;
	}

        if(rc != 4)
          break;
    }

    return(0L);
}
#endif /* DEBUG */


/*
 * Returns the message number closest to target that isn't hidden.
 * Make warning at least 100 chars.
 * A return of 0 means there is no message to jump to.
 */
long
closest_jump_target(target, stream, msgmap, no_target, in_index, warning)
    long        target;
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    int         no_target;
    CmdWhere    in_index;
    char       *warning;
{
    long i, start, closest = 0L;
    char buf[80];
    long maxnum;

    warning[0] = '\0';
    maxnum = (in_index == ThrdIndx) ? msgmap->max_thrdno : mn_get_total(msgmap);

    if(no_target){
	target = maxnum;
	start = 1L;
	sprintf(warning, "No %.10s number entered, jump to end? ",
		(in_index == ThrdIndx) ? "thread" : "message");
    }
    else if(target < 1L)
      start = 1L - target;
    else if(target > maxnum)
      start = target - maxnum;
    else
      start = 1L;

    if(target > 0L && target <= maxnum)
      if(in_index == ThrdIndx
	 || !msgline_hidden(stream, msgmap, target, 0))
	return(target);

    for(i = start; target+i <= maxnum || target-i > 0L; i++){

	if(target+i > 0L && target+i <= maxnum &&
	   (in_index == ThrdIndx
	    || !msgline_hidden(stream, msgmap, target+i, 0))){
	    closest = target+i;
	    break;
	}

	if(target-i > 0L && target-i <= maxnum &&
	   (in_index == ThrdIndx
	    || !msgline_hidden(stream, msgmap, target-i, 0))){
	    closest = target-i;
	    break;
	}
    }

    strncpy(buf, long2string(closest), sizeof(buf));
    buf[sizeof(buf)-1] = '\0';

    if(closest == 0L)
      strcpy(warning, "Nothing to jump to");
    else if(target < 1L)
      sprintf(warning, "%.10s number (%.20s) must be at least %.20s",
	      (in_index == ThrdIndx) ? "Thread" : "Message",
	      long2string(target), buf);
    else if(target > maxnum)
      sprintf(warning, "%.10s number (%.20s) may be no more than %.20s",
	      (in_index == ThrdIndx) ? "Thread" : "Message",
	      long2string(target), buf);
    else if(!no_target)
      sprintf(warning,
	"Message number (%.20s) is not in \"Zoomed Index\" - Closest is(%.20s)",
	long2string(target), buf);

    return(closest);
}


/*----------------------------------------------------------------------
     Prompt for folder name to open, expand the name and return it

   Args: qline      -- Screen line to prompt on
         allow_list -- if 1, allow ^T to bring up collection lister

 Result: returns the folder name or NULL
         pine structure mangled_footer flag is set
         may call the collection lister in which case mangled screen will be set

 This prompts the user for the folder to open, possibly calling up
the collection lister if the user types ^T.
----------------------------------------------------------------------*/
char *
broach_folder(qline, allow_list, context)
    int qline, allow_list;
    CONTEXT_S **context;
{
    HelpType	help;
    static char newfolder[MAILTMPLEN];
    char        expanded[MAXPATH+1],
                prompt[MAX_SCREEN_COLS+1],
               *last_folder;
    CONTEXT_S  *tc, *tc2;
    ESCKEY_S    ekey[8];
    int		rc, n, flags, last_rc = 0, inbox, done = 0;

    /*
     * the idea is to provide a clue for the context the file name
     * will be saved in (if a non-imap names is typed), and to
     * only show the previous if it was also in the same context
     */
    help	   = NO_HELP;
    *expanded	   = '\0';
    *newfolder	   = '\0';
    last_folder	   = NULL;

    /*
     * There are three possibilities for the prompt's offered default.
     *  1) always the last folder visited
     *  2) if non-inbox current, inbox else last folder visited
     *  3) if non-inbox current, inbox else last folder visited in
     *     the first collection
     */
    if(ps_global->goto_default_rule == GOTO_LAST_FLDR){
	tc = (context && *context) ? *context : ps_global->context_current;
	inbox = 1;		/* fill in last_folder below */
    }
    else if(ps_global->goto_default_rule == GOTO_FIRST_CLCTN){
	tc = (ps_global->context_list->use & CNTXT_INCMNG)
	  ? ps_global->context_list->next : ps_global->context_list;
	ps_global->last_unambig_folder[0] = '\0';
    }
    else if(ps_global->goto_default_rule == GOTO_FIRST_CLCTN_DEF_INBOX){
	tc = (ps_global->context_list->use & CNTXT_INCMNG)
	  ? ps_global->context_list->next : ps_global->context_list;
	tc->last_folder[0] = '\0';
	inbox = 0;
	ps_global->last_unambig_folder[0] = '\0';
    }
    else{
	inbox = strucmp(ps_global->cur_folder,ps_global->inbox_name) == 0;
	if(!inbox)
	  tc = ps_global->context_list;		/* inbox's context */
	else if(ps_global->goto_default_rule == GOTO_INBOX_FIRST_CLCTN){
	    tc = (ps_global->context_list->use & CNTXT_INCMNG)
		  ? ps_global->context_list->next : ps_global->context_list;
	    ps_global->last_unambig_folder[0] = '\0';
	}
	else
	  tc = (context && *context) ? *context : ps_global->context_current;
    }

    /* set up extra command option keys */
    rc = 0;
    ekey[rc].ch	     = (allow_list) ? ctrl('T') : 0 ;
    ekey[rc].rval    = (allow_list) ? 2 : 0;
    ekey[rc].name    = (allow_list) ? "^T" : "";
    ekey[rc++].label = (allow_list) ? "ToFldrs" : "";

    if(ps_global->context_list->next){
	ekey[rc].ch      = ctrl('P');
	ekey[rc].rval    = 10;
	ekey[rc].name    = "^P";
	ekey[rc++].label = "Prev Collection";

	ekey[rc].ch      = ctrl('N');
	ekey[rc].rval    = 11;
	ekey[rc].name    = "^N";
	ekey[rc++].label = "Next Collection";
    }

    if(F_ON(F_ENABLE_TAB_COMPLETE,ps_global)){
	ekey[rc].ch      = TAB;
	ekey[rc].rval    = 12;
	ekey[rc].name    = "TAB";
	ekey[rc++].label = "Complete";
    }

    if(F_ON(F_ENABLE_SUB_LISTS, ps_global)){
	ekey[rc].ch      = ctrl('X');
	ekey[rc].rval    = 14;
	ekey[rc].name    = "^X";
	ekey[rc++].label = "ListMatches";
    }

    if(ps_global->context_list->next){
	ekey[rc].ch      = KEY_UP;
	ekey[rc].rval    = 10;
	ekey[rc].name    = "";
	ekey[rc++].label = "";

	ekey[rc].ch      = KEY_DOWN;
	ekey[rc].rval    = 11;
	ekey[rc].name    = "";
	ekey[rc++].label = "";
    }

    ekey[rc].ch = -1;

    while(!done) {
	/*
	 * Figure out next default value for this context.  The idea
	 * is that in each context the last folder opened is cached.
	 * It's up to pick it out and display it.  This is fine
	 * and dandy if we've currently got the inbox open, BUT
	 * if not, make the inbox the default the first time thru.
	 */
	if(!inbox){
	    last_folder = ps_global->inbox_name;
	    inbox = 1;		/* pretend we're in inbox from here on out */
	}
	else
	  last_folder = (ps_global->last_unambig_folder[0])
			  ? ps_global->last_unambig_folder
			  : ((tc->last_folder[0]) ? tc->last_folder : NULL);

	if(last_folder)
	  sprintf(expanded, " [%.*s]", sizeof(expanded)-5, last_folder);
	else
	  *expanded = '\0';

	/* only show collection number if more than one available */
	if(ps_global->context_list->next){
	    sprintf(prompt, "GOTO %s in <%.20s> %.*s%s: ",
		    NEWS_TEST(tc) ? "news group" : "folder",
		    tc->nickname, sizeof(prompt)-50, expanded,
		    *expanded ? " " : "");
	}
	else
	  sprintf(prompt, "GOTO folder %.*s%s: ", sizeof(prompt)-20, expanded,
		  *expanded ? " " : "");

	flags = OE_APPEND_CURRENT;
        rc = optionally_enter(newfolder, qline, 0, sizeof(newfolder),
			      prompt, ekey, help, &flags);

	ps_global->mangled_footer = 1;

	switch(rc){
	  case -1 :				/* o_e says error! */
	    q_status_message(SM_ORDER | SM_DING, 3, 3,
			     "Error reading folder name");
	    return(NULL);

	  case 0 :				/* o_e says normal entry */
	    removing_trailing_white_space(newfolder);
	    removing_leading_white_space(newfolder);

	    if(*newfolder){
		char *name, *fullname = NULL;
		int   exists, breakout = 0;

		if(!(name = folder_is_nick(newfolder, FOLDERS(tc))))
		  name = newfolder;

		if(update_folder_spec(expanded, name)){
		    strncpy(name = newfolder, expanded, sizeof(newfolder));
		    newfolder[sizeof(newfolder)-1] = '\0';
		}

		exists = folder_name_exists(tc, name, &fullname);

		if(fullname){
		    strncpy(name = newfolder, fullname, sizeof(newfolder));
		    newfolder[sizeof(newfolder)-1] = '\0';
		    fs_give((void **) &fullname);
		    breakout = TRUE;
		}

		/*
		 * if we know the things a folder, open it.
		 * else if we know its a directory, visit it.
		 * else we're not sure (it either doesn't really
		 * exist or its unLISTable) so try opening it anyway
		 */
		if(exists & FEX_ISFILE){
		    done++;
		    break;
		}
		else if((exists & FEX_ISDIR)){
		    if(breakout){
			CONTEXT_S *fake_context;
			char	   tmp[MAILTMPLEN];
			size_t	   l;

			strncpy(tmp, name, sizeof(tmp)-2);
			tmp[sizeof(tmp)-2-1] = '\0';
			if(tmp[(l = strlen(tmp)) - 1] != tc->dir->delim){
			    tmp[l] = tc->dir->delim;
			    strcpy(&tmp[l+1], "[]");
			}
			else
			  strcat(tmp, "[]");

			fake_context = new_context(tmp, 0);
			newfolder[0] = '\0';
			done = display_folder_list(&fake_context, newfolder,
						   1, folders_for_goto);
			free_context(&fake_context);
			break;
		    }
		    else if(!(tc->use & CNTXT_INCMNG)){
			done = display_folder_list(&tc, newfolder,
						   1, folders_for_goto);
			break;
		    }
		}
		else if((exists & FEX_ERROR)){
		    q_status_message1(SM_ORDER, 0, 3,
				      "Problem accessing folder \"%.200s\"",
				      newfolder);
		    return(NULL);
		}
		else{
		    done++;
		    break;
		}

		if(exists == FEX_ERROR)
		  q_status_message1(SM_ORDER, 0, 3,
				    "Problem accessing folder \"%.200s\"",
				    newfolder);
		else if(tc->use & CNTXT_INCMNG)
		  q_status_message1(SM_ORDER, 0, 3,
				    "Can't find Incoming Folder: %.200s",
				    newfolder);
		else if(context_isambig(newfolder))
		  q_status_message3(SM_ORDER, 0, 3,
				    "Can't find folder \"%.200s\" in %.*s",
				    newfolder, (void *) 50, tc->nickname);
		else
		  q_status_message1(SM_ORDER, 0, 3,
				    "Can't find folder \"%.200s\"",
				    newfolder);

		return(NULL);
	    }
	    else if(last_folder){
		strncpy(newfolder, last_folder, sizeof(newfolder));
		newfolder[sizeof(newfolder)-1] = '\0';
		done++;
		break;
	    }
	    /* fall thru like they cancelled */

	  case 1 :				/* o_e says user cancel */
	    cmd_cancelled("Open folder");
	    return(NULL);

	  case 2 :				/* o_e says user wants list */
	    if(display_folder_list(&tc, newfolder, 0, folders_for_goto))
	      done++;

	    break;

	  case 3 :				/* o_e says user wants help */
	    help = help == NO_HELP ? h_oe_broach : NO_HELP;
	    break;

	  case 4 :				/* redraw */
	    break;
	    
	  case 10 :				/* Previous collection */
	    tc2 = ps_global->context_list;
	    while(tc2->next && tc2->next != tc)
	      tc2 = tc2->next;

	    tc = tc2;
	    break;

	  case 11 :				/* Next collection */
	    tc = (tc->next) ? tc->next : ps_global->context_list;
	    break;

	  case 12 :				/* file name completion */
	    if(!folder_complete(tc, newfolder, &n)){
		if(n && last_rc == 12 && !(flags & OE_USER_MODIFIED)){
		    if(display_folder_list(&tc, newfolder, 1,folders_for_goto))
		      done++;			/* bingo! */
		    else
		      rc = 0;			/* burn last_rc */
		}
		else
		  Writechar(BELL, 0);
	    }

	    break;

	  case 14 :				/* file name completion */
	    if(display_folder_list(&tc, newfolder, 2, folders_for_goto))
	      done++;			/* bingo! */
	    else
	      rc = 0;			/* burn last_rc */

	    break;

	  default :
	    panic("Unhandled case");
	    break;
	}

	last_rc = rc;
    }

    dprint(2, (debugfile, "broach folder, name entered \"%s\"\n", newfolder));

    /*-- Just check that we can expand this. It gets done for real later --*/
    strncpy(expanded, newfolder, sizeof(expanded));
    expanded[sizeof(expanded)-1] = '\0';
    if (! expand_foldername(expanded, sizeof(expanded))) {
        dprint(1, (debugfile,
                    "Error: Failed on expansion of filename %s (save)\n", 
    	  expanded));
        return(NULL);
    }

    *context = tc;
    return(newfolder);
}


/*----------------------------------------------------------------------
    Check to see if user input is in form of old c-client mailbox speck

  Args: old --
	new -- 

 Result:  1 if the folder was successfully updatedn
          0 if not necessary
      
  ----*/
int
update_folder_spec(new, old)
    char *new, *old;
{
    char *p;
    int	  nntp = 0;

    if(*(p = old) == '*')		/* old form? */
      old++;

    if(*old == '{')			/* copy host spec */
      do
	switch(*new = *old++){
	  case '\0' :
	    return(FALSE);

	  case '/' :
	    if(!struncmp(old, "nntp", 4))
	      nntp++;

	    break;

	  default :
	    break;
	}
      while(*new++ != '}');

    if((*p == '*' && *old) || ((*old == '*') ? *++old : 0)){
	/*
	 * OK, some heuristics here.  If it looks like a newsgroup
	 * then we plunk it into the #news namespace else we
	 * assume that they're trying to get at a #public folder...
	 */
	for(p = old;
	    *p && (isalnum((unsigned char) *p) || strindex(".-", *p));
	    p++)
	  ;

	sstrcpy(&new, (*p && !nntp) ? "#public/" : "#news.");
	strcpy(new, old);
	return(TRUE);
    }

    return(FALSE);
}


/*----------------------------------------------------------------------
    Actually attempt to open given folder 

  Args: newfolder -- The folder name to open
        stream    -- Candidate stream for recycling. This stream will either
	             be re-used, or it will be closed.

 Result:  1 if the folder was successfully opened
          0 if the folder open failed and went back to old folder
         -1 if open failed and no folder is left open
      
  Attempt to open the folder name given. If the open of the new folder
  fails then the previously open folder will remain open, unless
  something really bad has happened. The designate inbox will always be
  kept open, and when a request to open it is made the already open
  stream will be used. Making a folder the current folder requires
  setting the following elements of struct pine: mail_stream, cur_folder,
  current_msgno, max_msgno.

  The first time the inbox folder is opened, usually as Pine starts up,
  it will be actually opened.
  ----*/

do_broach_folder(newfolder, new_context, streamp) 
     char      *newfolder;
     CONTEXT_S *new_context;
     MAILSTREAM **streamp;
{
    MAILSTREAM *m, *stream = streamp ? *streamp : NULL;
    int         open_inbox, rv, old_tros, we_cancel = 0,
                do_reopen = 0, n, was_dead = 0;
    char        expanded_file[max(MAXPATH,MAILTMPLEN)+1],
	       *old_folder, *old_path, *p;
    long        openmode;
    char        status_msg[81];
    SortOrder	old_sort;
    unsigned    perfolder_startup_rule;

#if	defined(DOS) && !defined(WIN32)
    openmode = OP_SHORTCACHE;
#else
    openmode = 0L;
#endif

    dprint(1, (debugfile, "About to open folder \"%s\"    inbox: \"%s\"\n",
	       newfolder, ps_global->inbox_name));

    was_dead = ps_global->dead_stream || ps_global->dead_inbox;
    /*----- Little to do to if reopening same folder -----*/
    if(new_context == ps_global->context_current && ps_global->mail_stream
       && strcmp(newfolder, ps_global->cur_folder) == 0){
	if(stream)
	  pine_mail_close(stream);

	stream = NULL;

	if(ps_global->dead_stream)
	  do_reopen++;
	
	/*
	 * If it is a stream which could probably discover newmail by
	 * reopening and user has YES set for those streams, or it
	 * is a stream which may discover newmail by reopening and
	 * user has YES set for those stream, then do_reopen.
	 */
	if(!do_reopen
	   &&
	   (((ps_global->mail_stream->dtb
	      && ((ps_global->mail_stream->dtb->flags & DR_NONEWMAIL)
		  || (ps_global->mail_stream->rdonly
		      && ps_global->mail_stream->dtb->flags
					      & DR_NONEWMAILRONLY)))
	     && (ps_global->reopen_rule == REOPEN_YES_YES
	         || ps_global->reopen_rule == REOPEN_YES_ASK_Y
	         || ps_global->reopen_rule == REOPEN_YES_ASK_N
	         || ps_global->reopen_rule == REOPEN_YES_NO))
	    ||
	    ((ps_global->mail_stream->dtb
	      && ps_global->mail_stream->rdonly
	      && !(ps_global->mail_stream->dtb->flags & DR_LOCAL))
	     && (ps_global->reopen_rule == REOPEN_YES_YES))))
	  do_reopen++;

	/*
	 * If it is a stream which could probably discover newmail by
	 * reopening and user has ASK set for those streams, or it
	 * is a stream which may discover newmail by reopening and
	 * user has ASK set for those stream, then ask.
	 */
	if(!do_reopen
	   &&
	   (((ps_global->mail_stream->dtb
	      && ((ps_global->mail_stream->dtb->flags & DR_NONEWMAIL)
		  || (ps_global->mail_stream->rdonly
		      && ps_global->mail_stream->dtb->flags
					      & DR_NONEWMAILRONLY)))
	     && (ps_global->reopen_rule == REOPEN_ASK_ASK_Y
	         || ps_global->reopen_rule == REOPEN_ASK_ASK_N
	         || ps_global->reopen_rule == REOPEN_ASK_NO_Y
	         || ps_global->reopen_rule == REOPEN_ASK_NO_N))
	    ||
	    ((ps_global->mail_stream->dtb
	      && ps_global->mail_stream->rdonly
	      && !(ps_global->mail_stream->dtb->flags & DR_LOCAL))
	     && (ps_global->reopen_rule == REOPEN_YES_ASK_Y
	         || ps_global->reopen_rule == REOPEN_YES_ASK_N
	         || ps_global->reopen_rule == REOPEN_ASK_ASK_Y
	         || ps_global->reopen_rule == REOPEN_ASK_ASK_N)))){
	    int deefault;

	    switch(ps_global->reopen_rule){
	      case REOPEN_YES_ASK_Y:
	      case REOPEN_ASK_ASK_Y:
	      case REOPEN_ASK_NO_Y:
		deefault = 'y';
		break;

	      default:
		deefault = 'n';
		break;
	    }

	    switch(want_to("Re-open folder to check for new messages", deefault,
			   'x', h_reopen_folder, WT_NORM)){
	      case 'y':
	        do_reopen++;
		break;
	    
	      case 'n':
		break;

	      case 'x':
		cmd_cancelled(NULL);
		return(0);
	    }
	}

	if(do_reopen){
	    /*
	     * If it's not healthy or if the user explicitly wants to
	     * do a reopen, we reset things and fall thru
	     * to actually reopen it.
	     */
	    if(ps_global->dead_stream){
		dprint(2, (debugfile, "Stream was dead, reopening \"%s\"\n",
				      newfolder));
	    }

	    pine_mail_close(ps_global->mail_stream);
	    if(ps_global->mail_stream == ps_global->inbox_stream)
	      ps_global->inbox_stream = NULL;

	    ps_global->mail_stream         = NULL;
	    ps_global->expunge_count       = 0;
	    ps_global->new_mail_count      = 0;
	    ps_global->noticed_dead_stream = 0;
	    ps_global->dead_stream         = 0;
	    ps_global->mangled_header	   = 1;
	    mn_give(&ps_global->msgmap);
	    clear_index_cache();
	    reset_check_point();
	}
	else{
	    return(1);			/* successful open of same folder! */
	}
    }

    /*--- Set flag that we're opening the inbox, a special case ---*/
    /*
     * We want to know if inbox is being opened either by name OR
     * fully qualified path...
     *
     * So, IF we're asked to open inbox AND it's already open AND
     * the only stream AND it's healthy, just return ELSE fall thru
     * and close mail_stream returning with inbox_stream as new stream...
     */
    if(open_inbox = (strucmp(newfolder, ps_global->inbox_name) == 0
		     || strcmp(newfolder, ps_global->VAR_INBOX_PATH) == 0)){
	new_context = ps_global->context_list; /* restore first context */
	if(ps_global->inbox_stream 
	   && (ps_global->inbox_stream == ps_global->mail_stream)){

	    if(stream)
	      pine_mail_close(stream);

	    return(1);
	}
    }

    /*
     * If ambiguous foldername (not fully qualified), make sure it's
     * not a nickname for a folder in the given context...
     */
    /* might get reset below */
    strncpy(expanded_file, newfolder, sizeof(expanded_file));
    expanded_file[sizeof(expanded_file)-1] = '\0';
    if(!open_inbox && new_context && context_isambig(newfolder)){
	if (p = folder_is_nick(newfolder, FOLDERS(new_context))){
	    strncpy(expanded_file, p, sizeof(expanded_file));
	    expanded_file[sizeof(expanded_file)-1] = '\0';
	    dprint(2, (debugfile, "broach_folder: nickname for %s is %s\n",
		       expanded_file, newfolder));
	}
	else if ((new_context->use & CNTXT_INCMNG)
		 && (folder_index(newfolder, new_context, FI_FOLDER) < 0)
		 && !is_absolute_path(newfolder)){
	    q_status_message1(SM_ORDER, 3, 4,
			    "Can't find Incoming Folder %.200s.", newfolder);
	    if(stream)
	      pine_mail_close(stream);

	    return(0);
	}
    }

    /*--- Opening inbox, inbox has been already opened, the easy case ---*/
    if(open_inbox && ps_global->inbox_stream != NULL
       && !ps_global->dead_inbox) {
        expunge_and_close(ps_global->mail_stream, ps_global->context_current,
			  ps_global->cur_folder, NULL);

	ps_global->viewing_a_thread = ps_global->inbox_viewing_a_thread;

	ps_global->mail_stream              = ps_global->inbox_stream;
        ps_global->new_mail_count           = 0L;
        ps_global->expunge_count            = 0L;
        ps_global->mail_box_changed         = 0;
        ps_global->noticed_dead_stream      = 0;
        ps_global->noticed_dead_inbox       = 0;
        ps_global->dead_stream              = 0;
        ps_global->dead_inbox               = 0;
	mn_give(&ps_global->msgmap);
	ps_global->msgmap		    = ps_global->inbox_msgmap;
	ps_global->inbox_msgmap		    = NULL;

	dprint(7, (debugfile, "%ld %ld %x\n",
		   mn_get_cur(ps_global->msgmap),
                   mn_get_total(ps_global->msgmap),
		   ps_global->mail_stream));
	/*
	 * remember last context and folder
	 */
	if(context_isambig(ps_global->cur_folder)){
	    ps_global->context_last = ps_global->context_current;
	    strncpy(ps_global->context_current->last_folder,
		    ps_global->cur_folder,
		    sizeof(ps_global->context_current->last_folder)-1);
	    ps_global->context_current->last_folder[sizeof(ps_global->context_current->last_folder)-1] = '\0';
	    ps_global->last_unambig_folder[0] = '\0';
	}
	else{
	    ps_global->context_last = NULL;
	    strncpy(ps_global->last_unambig_folder, ps_global->cur_folder,
		    sizeof(ps_global->last_unambig_folder)-1);
	    ps_global->last_unambig_folder[sizeof(ps_global->last_unambig_folder)-1] = '\0';
	}

	strncpy(ps_global->cur_folder, ps_global->inbox_name,
		sizeof(ps_global->cur_folder)-1);
	ps_global->cur_folder[sizeof(ps_global->cur_folder)-1] = '\0';
	ps_global->context_current = ps_global->context_list;
	reset_index_format();
	clear_index_cache();
        /* MUST sort before restoring msgno! */
	refresh_sort(ps_global->msgmap, SRT_NON);
        q_status_message3(SM_ORDER, 0, 3,
			  "Opened folder \"%.200s\" with %.200s message%.200s",
			  ps_global->inbox_name, 
                          long2string(mn_get_total(ps_global->msgmap)),
			  plural(mn_get_total(ps_global->msgmap)));
#ifdef	_WINDOWS
	mswin_settitle(ps_global->inbox_name);
#endif
	if(stream)
	  pine_mail_close(stream);

	return(1);
    }
    else if(open_inbox && ps_global->inbox_stream != NULL
	    && ps_global->dead_inbox){
	/* 
	 * if dead INBOX, just close it and let it be reopened.
	 * This is actually different from the do_reopen case above,
	 * because we're going from another open mail folder to the
	 * dead INBOX.
	 */
	dprint(2, (debugfile, "INBOX was dead, closing before reopening\n"));
	pine_mail_close(ps_global->inbox_stream);
	ps_global->inbox_stream = NULL;
	if(ps_global->inbox_msgmap){
	    mn_give(&ps_global->inbox_msgmap);
	    ps_global->inbox_msgmap = NULL;
	}
	ps_global->inbox_expunge_count = 0L;
	ps_global->inbox_new_mail_count = 0L;
	
    }

    if(!new_context && !expand_foldername(expanded_file,sizeof(expanded_file))){
	if(stream)
	  pine_mail_close(stream);

	return(0);
    }

    old_folder = NULL;
    old_path   = NULL;
    old_sort   = SortArrival;			/* old sort */
    old_tros   = 0;				/* old reverse sort ? */
    /*---- now close the old one we had open if there was one ----*/
    if(ps_global->mail_stream != NULL){
        old_folder   = cpystr(ps_global->cur_folder);
        old_path     = cpystr(ps_global->mail_stream->original_mailbox
	                        ? ps_global->mail_stream->original_mailbox
				: ps_global->mail_stream->mailbox);
	old_sort     = mn_get_sort(ps_global->msgmap);
	old_tros     = mn_get_revsort(ps_global->msgmap);
	if(strcmp(ps_global->cur_folder, ps_global->inbox_name) == 0){
	    /*-- don't close the inbox stream, save a bit of state --*/
	    if(ps_global->inbox_msgmap)
	      mn_give(&ps_global->inbox_msgmap);

	    ps_global->inbox_msgmap = ps_global->msgmap;
	    ps_global->msgmap       = NULL;

	    dprint(2, (debugfile,
		       "Close - saved inbox state: max %ld\n",
		       mn_get_total(ps_global->inbox_msgmap)));
	}
	else{
	    expunge_and_close(ps_global->mail_stream,
			      ps_global->context_current,
			      ps_global->cur_folder, NULL);
	    ps_global->mail_stream = NULL;
	}
    }

    sprintf(status_msg, "%.3sOpening \"", do_reopen ? "Re-" : "");
    strncat(status_msg, pretty_fn(newfolder),
	    sizeof(status_msg)-strlen(status_msg) - 2);
    status_msg[sizeof(status_msg)-2] = '\0';
    strncat(status_msg, "\"", 1);
    status_msg[sizeof(status_msg)-1] = '\0';
    we_cancel = busy_alarm(1, status_msg, NULL, 1);

    /* 
     * if requested, make access to folder readonly (only once)
     */
    if (ps_global->open_readonly_on_startup) {
	openmode |= OP_READONLY ;
	ps_global->open_readonly_on_startup = 0 ;
    }

    ps_global->first_unseen = 0L;

    m = context_open((new_context && !open_inbox) ? new_context : NULL, stream, 
		     open_inbox ? ps_global->VAR_INBOX_PATH : expanded_file,
		     openmode);
    if(streamp)
      *streamp = m;


    dprint(8, (debugfile, "Opened folder %p \"%s\" (context: \"%s\")\n",
               m, (m) ? m->mailbox : "nil",
	       (new_context) ? new_context->context : "nil"));


    /* Can get m != NULL if correct passwd for remote, but wrong name */
    if(m == NULL || ((p = strindex(m->mailbox, '<')) != NULL &&
                      strcmp(p + 1, "no_mailbox>") == 0)) {
	/*-- non-existent local mailbox, or wrong passwd for remote mailbox--*/
        /* fall back to currently open mailbox */
	if(we_cancel)
	  cancel_busy_alarm(-1);

        rv = 0;
        dprint(8, (debugfile, "Old folder: \"%s\"\n",
               old_folder == NULL ? "" : old_folder));
        if(old_folder != NULL) {
            if(strcmp(old_folder, ps_global->inbox_name) == 0){
                ps_global->mail_stream = ps_global->inbox_stream;
		if(ps_global->msgmap)
		  mn_give(&ps_global->msgmap);

		ps_global->msgmap       = ps_global->inbox_msgmap;
		ps_global->inbox_msgmap = NULL;

                dprint(8, (debugfile, "Reactivate inbox %ld %ld %p\n",
                           mn_get_cur(ps_global->msgmap),
                           mn_get_total(ps_global->msgmap),
                           ps_global->mail_stream));
		strncpy(ps_global->cur_folder, ps_global->inbox_name,
			sizeof(ps_global->cur_folder)-1);
		ps_global->cur_folder[sizeof(ps_global->cur_folder)-1] = '\0';
            } else {
                ps_global->mail_stream = pine_mail_open(NULL, old_path,
							openmode);
                /* mm_log will take care of error message here */
                if(ps_global->mail_stream == NULL) {
                    rv = -1;
                } else {
		    mn_init(&(ps_global->msgmap),
			    ps_global->mail_stream->nmsgs);
		    mn_set_sort(ps_global->msgmap, old_sort);
		    mn_set_revsort(ps_global->msgmap, old_tros);
                    ps_global->expunge_count       = 0;
                    ps_global->new_mail_count      = 0;
                    ps_global->noticed_dead_stream = 0;
                    ps_global->dead_stream         = 0;
		    ps_global->mangled_header	   = 1;

		    reset_index_format();
		    clear_index_cache();
                    reset_check_point();
		    if(IS_NEWS(ps_global->mail_stream)
		       && ps_global->mail_stream->rdonly)
		      msgno_exclude_deleted(ps_global->mail_stream,
					    ps_global->msgmap);

		    if(mn_get_total(ps_global->msgmap) > 0)
		      mn_set_cur(ps_global->msgmap,
				 first_sorted_flagged(F_NONE,
						      ps_global->mail_stream,
						      0L,
						      THREADING()
							  ? 0 : FSF_SKIP_CHID));

		    if(!(mn_get_sort(ps_global->msgmap) == SortArrival
			 && !mn_get_revsort(ps_global->msgmap)))
		      refresh_sort(ps_global->msgmap, SRT_NON);

                    q_status_message1(SM_ORDER, 0, 3,
				      "Folder \"%.200s\" reopened", old_folder);
                }
            }

	    if(rv == 0)
	      mn_set_cur(ps_global->msgmap,
			 min(mn_get_cur(ps_global->msgmap), 
			     mn_get_total(ps_global->msgmap)));

            fs_give((void **)&old_folder);
            fs_give((void **)&old_path);
        } else {
            rv = -1;
        }
        if(rv == -1) {
            q_status_message(SM_ORDER | SM_DING, 0, 4, "No folder opened");
	    mn_set_total(ps_global->msgmap, 0L);
	    mn_set_nmsgs(ps_global->msgmap, 0L);
	    mn_set_cur(ps_global->msgmap, -1L);
            strcpy(ps_global->cur_folder, "");
        }
        return(rv);
    } else {
        if(old_folder != NULL) {
            fs_give((void **)&old_folder);
            fs_give((void **)&old_path);
        }
    }

    /*----- success in opening the new folder ----*/
    dprint(2, (debugfile, "Opened folder \"%s\" with %ld messages\n",
	       m->mailbox, m->nmsgs));


    /*--- A Little house keeping ---*/
    ps_global->mail_stream	    = m;
    ps_global->expunge_count	    = 0L;
    ps_global->new_mail_count	    = 0L;
    ps_global->noticed_dead_stream  = 0;
    ps_global->dead_stream          = 0;
    if(open_inbox){
	ps_global->noticed_dead_inbox   = 0;
	ps_global->dead_inbox           = 0;
    }
    mn_init(&(ps_global->msgmap), m->nmsgs);
    if(was_dead && ps_global->dead_stream == 0
       && ps_global->dead_inbox == 0)
      icon_text(NULL, IT_MCLOSED);

    ps_global->last_unambig_folder[0] = '\0';

    /*
     * remember old folder and context...
     */
    if(context_isambig(ps_global->cur_folder)
       || strucmp(ps_global->cur_folder, ps_global->inbox_name) == 0){
	strncpy(ps_global->context_current->last_folder,
		ps_global->cur_folder,
		sizeof(ps_global->context_current->last_folder)-1);
	ps_global->context_current->last_folder[sizeof(ps_global->context_current->last_folder)-1] = '\0';
    }
    else{
	strncpy(ps_global->last_unambig_folder, ps_global->cur_folder,
		sizeof(ps_global->last_unambig_folder)-1);
	ps_global->last_unambig_folder[sizeof(ps_global->last_unambig_folder)-1] = '\0';
    }

    /* folder in a subdir of context? */
    if(ps_global->context_current->dir->prev)
      sprintf(ps_global->cur_folder, "%.*s%.*s",
		(sizeof(ps_global->cur_folder)-1)/2,
		ps_global->context_current->dir->ref,
		(sizeof(ps_global->cur_folder)-1)/2,
		newfolder);
    else{
	strncpy(ps_global->cur_folder,
		(open_inbox) ? ps_global->inbox_name : newfolder,
		sizeof(ps_global->cur_folder)-1);
	ps_global->cur_folder[sizeof(ps_global->cur_folder)-1] = '\0';
    }

    if(new_context){
	ps_global->context_last    = ps_global->context_current;
	ps_global->context_current = new_context;
    }

    reset_check_point();
    clear_index_cache();
    ps_global->mail_box_changed = 0;

    /*
     * Start news reading with messages the user's marked deleted
     * hidden from view...
     */
    if(IS_NEWS(ps_global->mail_stream) && ps_global->mail_stream->rdonly)
      msgno_exclude_deleted(ps_global->mail_stream, ps_global->msgmap);

    /*--- If we just opened the inbox remember it's special stream ---*/
    if(open_inbox && ps_global->inbox_stream == NULL)
      ps_global->inbox_stream = ps_global->mail_stream;
	
    process_filter_patterns(ps_global->mail_stream, ps_global->msgmap, 0L);

    if(we_cancel)
      cancel_busy_alarm(0);

    q_status_message5(SM_ORDER, 0, 4,
		    "%.200s \"%.200s\" opened with %.200s message%.200s%.200s",
			IS_NEWS(ps_global->mail_stream)
			  ? "News group" : "Folder",
			pretty_fn(newfolder),
			comatose(mn_get_total(ps_global->msgmap)),
			plural(mn_get_total(ps_global->msgmap)),
			READONLY_FOLDER ? " READONLY" : "");

#ifdef	_WINDOWS
    mswin_settitle(pretty_fn(newfolder));
#endif

    reset_sort_order(SRT_VRB);
    ps_global->viewing_a_thread = 0;
    reset_index_format();
    perfolder_startup_rule = reset_startup_rule(ps_global->mail_stream);

    if(mn_get_total(ps_global->msgmap) > 0L) {
	if(ps_global->start_entry > 0) {
	    mn_set_cur(ps_global->msgmap, mn_get_revsort(ps_global->msgmap)
			? first_sorted_flagged(F_NONE, m,
					       ps_global->start_entry,
					       THREADING() ? 0 : FSF_SKIP_CHID)
			: first_sorted_flagged(F_SRCHBACK, m,
					       ps_global->start_entry,
					       THREADING() ? 0 : FSF_SKIP_CHID));
	    ps_global->start_entry = 0;
        }
	else if(perfolder_startup_rule != IS_NOTSET ||
	        open_inbox ||
		ps_global->context_current->use & CNTXT_INCMNG){
	    unsigned use_this_startup_rule;

	    if(perfolder_startup_rule != IS_NOTSET)
	      use_this_startup_rule = perfolder_startup_rule;
	    else
	      use_this_startup_rule = ps_global->inc_startup_rule;

	    switch(use_this_startup_rule){
	      /*
	       * For news in incoming collection we're doing the same thing
	       * for first-unseen and first-recent. In both those cases you
	       * get first-unseen if FAKE_NEW is off and first-recent if
	       * FAKE_NEW is on. If FAKE_NEW is on, first unseen is the
	       * same as first recent because all recent msgs are unseen
	       * and all unrecent msgs are seen (see pine_mail_open).
	       */
	      case IS_FIRST_UNSEEN:
first_unseen:
		mn_set_cur(ps_global->msgmap,
			(ps_global->first_unseen
			 && mn_get_sort(ps_global->msgmap) == SortArrival
			 && !mn_get_revsort(ps_global->msgmap)
			 && !get_lflag(ps_global->mail_stream, NULL,
				       ps_global->first_unseen, MN_EXLD)
			 && (n = mn_raw2m(ps_global->msgmap, 
					  ps_global->first_unseen)))
			   ? n
			   : first_sorted_flagged(F_UNSEEN | F_UNDEL, m, 0L,
					      THREADING() ? 0 : FSF_SKIP_CHID));
		break;

	      case IS_FIRST_RECENT:
first_recent:
		/*
		 * We could really use recent for news but this is the way
		 * it has always worked, so we'll leave it. That is, if
		 * the FAKE_NEW feature is on, recent and unseen are
		 * equivalent, so it doesn't matter. If the feature isn't
		 * on, all the undeleted messages are unseen and we start
		 * at the first one. User controls with the FAKE_NEW feature.
		 */
		if(IS_NEWS(ps_global->mail_stream)){
		    mn_set_cur(ps_global->msgmap,
			       first_sorted_flagged(F_UNSEEN|F_UNDEL, m, 0L,
					       THREADING() ? 0 : FSF_SKIP_CHID));
		}
		else{
		    mn_set_cur(ps_global->msgmap,
			       first_sorted_flagged(F_RECENT|F_UNDEL, m, 0L,
					      THREADING() ? 0 : FSF_SKIP_CHID));
		}
		break;

	      case IS_FIRST_IMPORTANT:
		mn_set_cur(ps_global->msgmap,
			   first_sorted_flagged(F_FLAG|F_UNDEL, m, 0L,
					      THREADING() ? 0 : FSF_SKIP_CHID));
		break;

	      case IS_FIRST_IMPORTANT_OR_UNSEEN:

		if(IS_NEWS(ps_global->mail_stream))
		  goto first_unseen;

		{
		    MsgNo flagged, first_unseen;

		    flagged = first_sorted_flagged(F_FLAG|F_UNDEL, m, 0L,
					       THREADING() ? 0 : FSF_SKIP_CHID);
		    first_unseen = (ps_global->first_unseen
			     && mn_get_sort(ps_global->msgmap) == SortArrival
			     && !mn_get_revsort(ps_global->msgmap)
			     && !get_lflag(ps_global->mail_stream, NULL,
					   ps_global->first_unseen, MN_EXLD)
			     && (n = mn_raw2m(ps_global->msgmap, 
					      ps_global->first_unseen)))
				? n
				: first_sorted_flagged(F_UNSEEN|F_UNDEL, m, 0L,
					       THREADING() ? 0 : FSF_SKIP_CHID);
		    mn_set_cur(ps_global->msgmap,
			      (MsgNo) min((int) flagged, (int) first_unseen));

		}

		break;

	      case IS_FIRST_IMPORTANT_OR_RECENT:

		if(IS_NEWS(ps_global->mail_stream))
		  goto first_recent;

		{
		    MsgNo flagged, first_recent;

		    flagged = first_sorted_flagged(F_FLAG|F_UNDEL, m, 0L,
					       THREADING() ? 0 : FSF_SKIP_CHID);
		    first_recent = first_sorted_flagged(F_RECENT|F_UNDEL,
							m, 0L,
					       THREADING() ? 0 : FSF_SKIP_CHID);
		    mn_set_cur(ps_global->msgmap,
			      (MsgNo) min((int) flagged, (int) first_recent));
		}

		break;

	      case IS_FIRST:
		mn_set_cur(ps_global->msgmap,
			   first_sorted_flagged(F_UNDEL, m, 0L,
					      THREADING() ? 0 : FSF_SKIP_CHID));
		break;

	      case IS_LAST:
		mn_set_cur(ps_global->msgmap,
			   first_sorted_flagged(F_UNDEL, m, 0L,
			         FSF_LAST | (THREADING() ? 0 : FSF_SKIP_CHID)));
		break;

	      default:
		panic("Unexpected incoming startup case");
		break;

	    }
	}
	else if(IS_NEWS(ps_global->mail_stream)){
	    /*
	     * This will go to two different places depending on the FAKE_NEW
	     * feature (see pine_mail_open).
	     */
	    mn_set_cur(ps_global->msgmap,
		       first_sorted_flagged(F_UNSEEN|F_UNDEL, m, 0L,
					      THREADING() ? 0 : FSF_SKIP_CHID));
	}
        else{
	    mn_set_cur(ps_global->msgmap,
		       mn_get_revsort(ps_global->msgmap)
		         ? 1L
			 : mn_get_total(ps_global->msgmap));
	}

	adjust_cur_to_visible(ps_global->mail_stream, ps_global->msgmap);
    }
    else{
	mn_set_cur(ps_global->msgmap, -1L);
    }

    return(1);
}


void
reset_index_format()
{
    long rflags = ROLE_DO_OTHER;
    PAT_STATE     pstate;
    PAT_S        *pat;
    int           we_set_it = 0;

    if(ps_global->mail_stream && nonempty_patterns(rflags, &pstate)){
	for(pat = first_pattern(&pstate); pat; pat = next_pattern(&pstate)){
	    if(match_pattern(pat->patgrp, ps_global->mail_stream, NULL,
			     NULL, NULL, 0))
	      break;
	}

	if(pat && pat->action && !pat->action->bogus
	   && pat->action->index_format){
	    we_set_it++;
	    init_index_format(pat->action->index_format,
			      &ps_global->index_disp_format);
	}
    }

    if(!we_set_it)
      init_index_format(ps_global->VAR_INDEX_FORMAT,
		        &ps_global->index_disp_format);
}


void
reset_sort_order(flags)
    unsigned flags;
{
    long rflags = ROLE_DO_OTHER;
    PAT_STATE     pstate;
    PAT_S        *pat;
    SortOrder	  the_sort_order;
    int           sort_is_rev;

    /* set default order */
    the_sort_order = ps_global->def_sort;
    sort_is_rev    = ps_global->def_sort_rev;

    if(ps_global->mail_stream && nonempty_patterns(rflags, &pstate)){
	for(pat = first_pattern(&pstate); pat; pat = next_pattern(&pstate)){
	    if(match_pattern(pat->patgrp, ps_global->mail_stream, NULL,
			     NULL, NULL, 0))
	      break;
	}

	if(pat && pat->action && !pat->action->bogus
	   && pat->action->sort_is_set){
	    the_sort_order = pat->action->sortorder;
	    sort_is_rev    = pat->action->revsort;
	}
    }

    sort_folder(ps_global->msgmap, the_sort_order, sort_is_rev, flags);
}


unsigned
reset_startup_rule(stream)
    MAILSTREAM *stream;
{
    long rflags = ROLE_DO_OTHER;
    PAT_STATE     pstate;
    PAT_S        *pat;
    unsigned      startup_rule;

    startup_rule = IS_NOTSET;

    if(stream && nonempty_patterns(rflags, &pstate)){
	for(pat = first_pattern(&pstate); pat; pat = next_pattern(&pstate)){
	    if(match_pattern(pat->patgrp, stream, NULL, NULL, NULL, 0))
	      break;
	}

	if(pat && pat->action && !pat->action->bogus)
	  startup_rule = pat->action->startup_rule;
    }

    return(startup_rule);
}


/*----------------------------------------------------------------------
    Open the requested folder in the requested context

    Args: state -- usual pine state struct
	  newfolder -- folder to open
	  new_context -- folder context might live in
	  stream -- candidate for recycling

   Result: New folder open or not (if error), and we're set to
	   enter the index screen.
 ----*/
void
visit_folder(state, newfolder, new_context, stream)
    struct pine *state;
    char	*newfolder;
    CONTEXT_S	*new_context;
    MAILSTREAM  *stream;
{
    dprint(9, (debugfile, "do_broach_folder (%s, %s)\n",
	       newfolder, new_context ? new_context->context : "(NULL)"));
    if(do_broach_folder(newfolder, new_context, stream ? &stream : NULL) > 0
       || state->mail_stream != state->inbox_stream)
      state->next_screen = mail_index_screen;
}



/*----------------------------------------------------------------------
      Expunge (if confirmed) and close a mail stream

    Args: stream   -- The MAILSTREAM * to close
	  context  -- The contest to interpret the following name in
          folder   -- The name the folder is know by 
        final_msg  -- If non-null, this should be set to point to a
		      message to print out in the caller, it is allocated
		      here and freed by the caller.

   Result:  Mail box is expunged and closed. A message is displayed to
             say what happened
 ----*/
void
expunge_and_close(stream, context, folder, final_msg)
    MAILSTREAM *stream;
    CONTEXT_S  *context;
    char       *folder;
    char      **final_msg;
{
    long  i, delete_count, max_folder, seen_not_del;
    char  prompt_b[MAX_SCREEN_COLS+1], *short_folder_name,
          temp[MAILTMPLEN+1], buff1[MAX_SCREEN_COLS+1], *moved_msg = NULL,
	  buff2[MAX_SCREEN_COLS+1];
    struct variable *vars = ps_global->vars;
    int ret, expunge = FALSE;
    char ing[4];

    /* check for dead stream */
    if(stream && stream == ps_global->mail_stream && ps_global->dead_stream){
	pine_mail_close(stream);
	if(ps_global->mail_stream == ps_global->inbox_stream){
	    ps_global->inbox_stream = NULL;
	    ps_global->noticed_dead_inbox = ps_global->dead_inbox = 0;
	}

	stream = ps_global->mail_stream = NULL;
	ps_global->noticed_dead_stream = ps_global->dead_stream = 0;
    }

    if(stream && stream == ps_global->inbox_stream && ps_global->dead_inbox){
	pine_mail_close(stream);
	stream = ps_global->inbox_stream = NULL;
	ps_global->noticed_dead_inbox = ps_global->dead_inbox = 0;
    }

    if(stream != NULL){
        dprint(2, (debugfile, "expunge and close mail stream \"%s\"\n",
                   stream->mailbox));
	if(final_msg)
	  strcpy(ing, "ed");
	else
	  strcpy(ing, "ing");

	buff1[0] = '\0';
	buff2[0] = '\0';

        if(!stream->rdonly){

            q_status_message1(SM_INFO, 0, 1, "Closing \"%.200s\"...", folder);
	    flush_status_messages(1);

	    mail_expunge_prefilter(stream);

	    /*
	     * Be sure to expunge any excluded (filtered) msgs
	     * Do it here so they're not copyied into read/archived
	     * folders *AND* to be sure we don't refilter them
	     * next time the folder's opened.
	     */
	    for(i = 1L; i <= stream->nmsgs; i++)
	      if(get_lflag(stream, NULL, i, MN_EXLD)){
		  delete_filtered_msgs(stream);
		  expunge = TRUE;
		  break;
	      }

	    /* Save read messages? */
	    if(VAR_READ_MESSAGE_FOLDER && VAR_READ_MESSAGE_FOLDER[0]
	       && stream == ps_global->inbox_stream
	       && (seen_not_del = count_flagged(stream, F_SEEN | F_UNDEL))){

		if(F_ON(F_AUTO_READ_MSGS,ps_global)
		   || read_msg_prompt(seen_not_del, VAR_READ_MESSAGE_FOLDER))
		  /* move inbox's read messages */
		  moved_msg = move_read_msgs(stream, VAR_READ_MESSAGE_FOLDER,
					     buff1, -1L);
	    }
	    else if(VAR_ARCHIVED_FOLDERS)
	      moved_msg = move_read_incoming(stream, context, folder,
					     VAR_ARCHIVED_FOLDERS,
					     buff1);

	    /*
	     * We need the count_flagged to be executed not only to set
	     * delete_count, but also to set the searched bits in all of
	     * the deleted messages. The searched bit is used in the monkey
	     * business section below which undeletes deleted messages
	     * before expunging. It determines which messages are deleted
	     * by examining the searched bit, which had better be set or not
	     * based on this count_flagged call rather than some random
	     * search that happened earlier.
	     */
            delete_count = count_flagged(stream, F_DEL);
	    if(F_ON(F_EXPUNGE_MANUALLY,ps_global))
              delete_count = 0L;

	    ret = 'n';
	    if(delete_count){
		int charcnt = 0;

		if(delete_count == 1)
		  charcnt = 1;
		else{
		    sprintf(temp, "%ld", delete_count);
		    charcnt = strlen(temp)+1;
		}

		max_folder = max(1,MAXPROMPT - (36+charcnt));
		strncpy(temp, pretty_fn(folder), sizeof(temp));
		temp[sizeof(temp)-1] = '\0';
		short_folder_name = short_str(temp,buff2,max_folder,FrontDots);

		if(F_ON(F_FULL_AUTO_EXPUNGE,ps_global)
		   || (F_ON(F_AUTO_EXPUNGE, ps_global)
		       && ((!strucmp(folder,ps_global->inbox_name))
			   || (context && (context->use & CNTXT_INCMNG)))
		       && context_isambig(folder))){
		    ret = 'y';
		}
		else{
		    sprintf(prompt_b,
			    "Expunge the %ld deleted message%s from \"%s\"",
			    delete_count,
			    delete_count == 1 ? "" : "s",
			    short_folder_name);
		    ret = want_to(prompt_b, 'y', 0, NO_HELP, WT_NORM);
		}

		/* get this message back in queue */
		if(moved_msg)
		  q_status_message(SM_ORDER,
		      F_ON(F_AUTO_READ_MSGS,ps_global) ? 0 : 3, 5, moved_msg);

		if(ret == 'y'){
		    long filtered;

		    filtered = any_lflagged(stream == ps_global->mail_stream
					     ? ps_global->msgmap :
					     stream == ps_global->inbox_stream
					      ? ps_global->inbox_msgmap : NULL,
					    MN_EXLD);
		    sprintf(buff2,
		      "Clos%s \"%.30s\". %s %s message%s and remov%s %s.",
			ing,
	 		pretty_fn(folder),
			final_msg ? "Kept" : "Keeping",
			comatose(stream->nmsgs - filtered - delete_count),
			plural(stream->nmsgs - filtered - delete_count),
			ing,
			long2string(delete_count));
		    if(final_msg)
		      *final_msg = cpystr(buff2);
		    else
		      q_status_message(SM_ORDER,
				       (F_ON(F_AUTO_EXPUNGE,ps_global)
					|| F_ON(F_FULL_AUTO_EXPUNGE,ps_global))
					    ? 0 : 3,
				       5, buff2);
		      
		    flush_status_messages(1);
		    ps_global->mm_log_error = 0;
		    ps_global->expunge_in_progress = 1;
		    mail_expunge(stream);
		    ps_global->expunge_in_progress = 0;
		    if(ps_global->mm_log_error && final_msg && *final_msg){
			fs_give((void **)final_msg);
			*final_msg = NULL;
		    }
		}
	    }

	    if(ret != 'y'){
		if(expunge){
		    MESSAGECACHE *mc;
		    char	 *seq;
		    int		  expbits;

		    /*
		     * filtered message monkey business.
		     * The Plan:
		     *   1) light sequence bits for legit deleted msgs
		     *      and store marker in local extension
		     *   2) clear their deleted flag
		     *   3) perform expunge to removed filtered msgs
		     *   4) restore deleted flags for legit msgs
		     *      based on local extension bit
		     */
		    for(i = 1L; i <= stream->nmsgs; i++)
		      if(!get_lflag(stream, NULL, i, MN_EXLD)
			 && (((mc = mail_elt(stream, i))->valid && mc->deleted)
			     || (!mc->valid && mc->searched))){
			  mc->sequence = 1;
			  expbits      = MSG_EX_DELETE;
			  msgno_exceptions(stream, i, "0", &expbits, TRUE);
		      }
		      else
			mail_elt(stream, i)->sequence = 0;

		    if(seq = build_sequence(stream, NULL, NULL)){
			mail_flag(stream, seq, "\\DELETED", ST_SILENT);
			fs_give((void **) &seq);
		    }

		    ps_global->mm_log_error = 0;
		    ps_global->expunge_in_progress = 1;
		    mail_expunge(stream);
		    ps_global->expunge_in_progress = 0;

		    for(i = 1L; i <= stream->nmsgs; i++)
		      mail_elt(stream, i)->sequence 
			   = (msgno_exceptions(stream, i, "0", &expbits, FALSE)
			      && (expbits & MSG_EX_DELETE));

		    if(seq = build_sequence(stream, NULL, NULL)){
			mail_flag(stream, seq, "\\DELETED", ST_SET|ST_SILENT);
			fs_give((void **) &seq);
		    }
		}

		if(stream->nmsgs){
		    sprintf(buff2,
		        "Clos%s folder \"%.*s\". %s%s%s message%s.",
			ing,
			sizeof(buff2)-50, pretty_fn(folder), 
			final_msg ? "Kept" : "Keeping",
			(stream->nmsgs == 1L) ? " single" : " all ",
			(stream->nmsgs > 1L)
			  ? comatose(stream->nmsgs) : "",
			plural(stream->nmsgs));
		}
		else{
		    sprintf(buff2, "Clos%s empty folder \"%.*s\"",
			ing, sizeof(buff2)-50, pretty_fn(folder));
		}

		if(final_msg)
		  *final_msg = cpystr(buff2);
		else
		  q_status_message(SM_ORDER, 0, 3, buff2);
	    }
        }
	else{
            if(IS_NEWS(stream)){
		/*
		 * Mark the filtered messages deleted so they aren't
		 * filtered next time.
		 */
	        for(i = 1L; i <= stream->nmsgs; i++){
		  int exbits;
		  if(msgno_exceptions(stream, i, "0" , &exbits, FALSE)
		     && (exbits & MSG_EX_FILTERED)){
		    delete_filtered_msgs(stream);
		    break;
		  }
		}
		/* first, look to archive read messages */
		if(moved_msg = move_read_incoming(stream, context, folder,
						  VAR_ARCHIVED_FOLDERS,
						  buff1))
		  q_status_message(SM_ORDER,
		      F_ON(F_AUTO_READ_MSGS,ps_global) ? 0 : 3, 5, moved_msg);

		sprintf(buff2, "Clos%s news group \"%.*s\"",
			ing, sizeof(buff2)-50, pretty_fn(folder));

		if(F_ON(F_NEWS_CATCHUP, ps_global)){
		    MESSAGECACHE *mc;

		    /* count visible messages */
		    (void) count_flagged(stream, F_DEL);
		    for(i = 1L, delete_count = 0L; i <= stream->nmsgs; i++)
		      if(!(get_lflag(stream, NULL, i, MN_EXLD)
			   || ((mc = mail_elt(stream, i))->valid
				&& mc->deleted)
			   || (!mc->valid && mc->searched)))
			delete_count++;

		    if(delete_count){
			sprintf(prompt_b,
				"Delete %s%ld message%s from \"%s\"",
				(delete_count > 1L) ? "all " : "",
				delete_count, plural(delete_count),
				pretty_fn(folder));
			if(want_to(prompt_b, 'y', 0,
				   NO_HELP, WT_NORM) == 'y'){
			    char seq[64];

			    sprintf(seq, "1:%ld", stream->nmsgs);
			    mail_flag(stream, seq,
				      "\\DELETED", ST_SET|ST_SILENT);
			}
		    }
		}

		if(F_ON(F_NEWS_CROSS_DELETE, ps_global))
		  cross_delete_crossposts(stream);
	    }
            else
	      sprintf(buff2,
			"Clos%s read-only folder \"%.*s\". No changes to save",
			ing, sizeof(buff2)-60, pretty_fn(folder));

	    if(final_msg)
	      *final_msg = cpystr(buff2);
	    else
	      q_status_message(SM_ORDER, 0, 2, buff2);
        }

	/*
	 * Make darn sure any mm_log fallout caused above get's seen...
	 */
	flush_status_messages(1);

	pine_mail_close(stream);
    }
}



/*----------------------------------------------------------------------
  Dispatch messages matching FILTER patterns.

  Args:
	stream -- mail stream serving messages
	msgmap -- sequence to msgno mapping table
	recent -- number of recent messages to check

 When we're done, any filtered messages are filtered and the message
 mapping table has any filtered messages removed.
  ---*/
void
process_filter_patterns(stream, msgmap, recent)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    long	recent;
{
    long	  i, n, raw, orig_nmsgs;
    unsigned long uid;
    int		  exbits, nt = 0, pending_actions = 0, for_debugging = 0;
    long          rflags = ROLE_DO_FILTER;
    char	 *charset = NULL;
    MSGNO_S      *tmpmap = NULL;
    MESSAGECACHE *mc;
    PAT_S        *pat, *nextpat = NULL;
    SEARCHPGM	 *pgm = NULL;
    SEARCHSET	 *srchset = NULL;
    long          flags = (SE_NOPREFETCH|SE_FREE);
    PAT_STATE     pstate;

    dprint(5, (debugfile, "process_filter_patterns(stream=%s, recent=%ld)\n",
	    !stream                            ? "<null>"                   :
	     stream == ps_global->inbox_stream  ? "inbox"                   :
	      stream->original_mailbox           ? stream->original_mailbox :
	       stream->mailbox                    ? stream->mailbox         :
				                    "?",
	    recent));

    while(stream && stream->nmsgs && nonempty_patterns(rflags, &pstate)){

	for_debugging++;
	pending_actions = 0;
	nextpat = NULL;

	uid = mail_uid(stream, stream->nmsgs);

	/*
	 * Some of the search stuff won't work on old servers so we
	 * get the data and search locally. Big performance hit.
	 */
	if(is_imap_stream(stream) && !modern_imap_stream(stream))
	  flags |= SO_NOSERVER;
	else if(stream->dtb && !strcmp(stream->dtb->name, "nntp"))
	  flags |= SO_OVERVIEW;

	/*
	 * ignore all previously filtered messages
	 * and, if requested, anything not a recent
	 * arrival...
	 *
	 * Here we're using spare6 (MN_STMP), meaning we'll only
	 * search the ones with spare6 marked, new messages coming 
	 * in will not be considered.  There used to be orig_nmsgs,
	 * which kept track of this, but if a message gets expunged,
	 * then a new message could be lower than orig_nmsgs.
	 */
	for(i = 1; i <= stream->nmsgs; i++)
	  if(msgno_exceptions(stream, i, "0", &exbits, FALSE)){
	      if(exbits & MSG_EX_FILTERED)
		mail_elt(stream, i)->spare6 = 0;
	      else if(!recent || (exbits & MSG_EX_RECENT))
		mail_elt(stream, i)->spare6 = 1;
	      else
		mail_elt(stream, i)->spare6 = 0;
	  }
	  else 
	    mail_elt(stream, i)->spare6 = !recent;

	/* Then start searching */
	for(pat = first_pattern(&pstate); pat; pat = nextpat){
	    nextpat = next_pattern(&pstate);
	    if(pat->patgrp && !pat->patgrp->bogus
	       && pat->action && !pat->action->bogus
	       && !trivial_patgrp(pat->patgrp)
	       && match_pattern_folder(pat->patgrp, stream)
	       && !match_pattern_folder_specific(pat->action->folder,
						 stream, 0)){

		/*
		 * We could just keep track of spare6 accurately when
		 * we change the msgno_exceptions flags, but...
		 */
		for(i = 1; i <= stream->nmsgs; i++){
		    if((mc=mail_elt(stream, i))->spare6){
			if(msgno_exceptions(stream, i, "0", &exbits, FALSE)){
			    if(exbits & MSG_EX_FILTERED)
			      mc->sequence = 0;
			    else if(!recent || (exbits & MSG_EX_RECENT))
			      mc->sequence = 1;
			    else
			      mc->sequence = 0;
			}
			else 
			  mc->sequence = !recent;
		    }
		    else
		      mc->sequence = 0;
		}

		if(!(srchset = build_searchset(stream)))
		  continue;		/* nothing to search, move on */

		charset = NULL;
		pgm = match_pattern_srchpgm(pat->patgrp, stream,
					    &charset, srchset);

		mail_search_full(stream, NULL, pgm, flags);

		/* check scores */
		if(scores_are_used(SCOREUSE_GET) & SCOREUSE_FILTERS &&
		   pat->patgrp->do_score){
		    SEARCHSET *s, *ss;

		    /*
		     * Build a searchset so we can get all the scores we
		     * need and only the scores we need efficiently.
		     */

		    for(i = 1; i <= stream->nmsgs; i++)
		      mail_elt(stream, i)->sequence = 0;

		    for(s = srchset; s; s = s->next)
		      for(i = s->first; i <= s->last; i++)
			if((mc=mail_elt(stream, i))->searched &&
			   get_msg_score(stream, i) == SCORE_UNDEF)
			  mc->sequence = 1;

		    if((ss = build_searchset(stream)) != NULL){
			(void)calculate_some_scores(stream, ss, 0);
			mail_free_searchset(&ss);
		    }

		    /*
		     * Now check the patterns which have matched so far
		     * to see if their score is in the score interval.
		     */
		    for(s = srchset; s; s = s->next)
		      for(i = s->first; i <= s->last; i++)
			if(i <= stream->nmsgs
			   && (mc=mail_elt(stream, i))->searched){
			    int score;

			    score = get_msg_score(stream, i);

			    /*
			     * If the score is outside all of the intervals,
			     * turn off the searched bit.
			     * So that means we check each interval and if
			     * it is inside any interval we stop and leave
			     * the bit set. If it is outside we keep checking.
			     */
			    if(score != SCORE_UNDEF){
				INTVL_S *iv;

				for(iv = pat->patgrp->score; iv; iv = iv->next)
				  if(score >= iv->imin && score <= iv->imax)
				    break;
				
				if(!iv)
				  mc->searched = NIL;
			    }
			}
		}

		/* check for 8bit subject match or not */
		if(pat->patgrp->stat_8bitsubj != PAT_STAT_EITHER){
		    SEARCHSET *s, *ss = NULL;

		    /*
		     * Build a searchset so we can look at all the envelopes
		     * we need to look at but only those we need to look at.
		     * Everything with the searched bit set is still a
		     * possibility, so restrict to that set.
		     */

		    for(i = 1; i <= stream->nmsgs; i++)
		      mail_elt(stream, i)->sequence = 0;

		    for(s = srchset; s; s = s->next)
		      for(i = s->first; i <= s->last; i++)
			if(i <= stream->nmsgs
			   && (mc=mail_elt(stream, i))->searched)
			  mc->sequence = 1;

		    ss = build_searchset(stream);

		    for(s = ss; s; s = s->next)
		      for(i = s->first; i <= s->last; i++){
			  ENVELOPE   *e;
			  SEARCHSET **sset;

			  if(i > stream->nmsgs)
			    continue;

			  /*
			   * This causes the lookahead to fetch precisely
			   * the messages we want (in the searchset) instead
			   * of just fetching the next 20 sequential
			   * messages. If the searching so far has caused
			   * a sparse searchset in a large mailbox, the
			   * difference can be substantial.
			   */
			  sset = (SEARCHSET **) mail_parameters(stream,
							     GET_FETCHLOOKAHEAD,
							     (void *) stream);
			  if(sset)
			    *sset = s;

			  e = mail_fetchenvelope(stream, i);
			  if(pat->patgrp->stat_8bitsubj == PAT_STAT_YES){
			      if(e && e->subject){
				  char *p;

				  for(p = e->subject; *p; p++)
				    if(*p & 0x80)
				      break;

				  if(!*p)
				    mail_elt(stream,i)->searched = NIL;
			      }
			      else
				mail_elt(stream,i)->searched = NIL;
			  }
			  else if(pat->patgrp->stat_8bitsubj == PAT_STAT_NO){
			      if(e && e->subject){
				  char *p;

				  for(p = e->subject; *p; p++)
				    if(*p & 0x80)
				      break;

				  if(*p)
				    mail_elt(stream,i)->searched = NIL;
			      }
			  }
		      }

		    if(ss)
		      mail_free_searchset(&ss);
		}

		if(pat->patgrp->abookfrom != AFRM_EITHER)
		  from_or_replyto_in_abook(stream, srchset,
					   pat->patgrp->abookfrom,
					   pat->patgrp->abooks);

		nt = pat->action->non_terminating;
		pending_actions = max(nt, pending_actions);

		/* change some state bits */
		if(!pat->action->kill && pat->action->state_setting_bits){
		    tmpmap = NULL;
		    mn_init(&tmpmap, stream->nmsgs);

		    for(i = 1L, n = 0L; i <= stream->nmsgs; i++)
		      if(mail_elt(stream, i)->searched
			 && !(msgno_exceptions(stream, i, "0", &exbits, FALSE)
			      && (exbits & MSG_EX_FILTERED)))
			if(!n++){
			    mn_set_cur(tmpmap, i);
			}
			else{
			    mn_add_cur(tmpmap, i);
			}

		    if(n)
		      set_some_flags(stream, tmpmap,
				     pat->action->state_setting_bits, 1);

		    mn_give(&tmpmap);
		}

		/*
		 * The two halves of the if-else are almost the same and
		 * could probably be combined cleverly. The if clause
		 * is simply setting the MSG_EX_FILTERED bit, and leaving
		 * n set to zero. The msgno_exclude is not done in this case.
		 * The else clause excludes each message (because it is
		 * either filtered into nothing or moved to folder). The
		 * exclude messes with the msgmap and that changes max_msgno,
		 * so the loop control is a little tricky.
		 */
		if(!(pat->action->kill || pat->action->folder)){
		  n = 0L;
		  for(i = 1L; i <= mn_get_total(msgmap); i++)
		    if((raw = mn_m2raw(msgmap, i))
		       && (mail_elt(stream, raw))->searched){
		        dprint(5, (debugfile,
			    "FILTER matching \"%s\": msg %ld\n",
			    pat->patgrp->nick ? pat->patgrp->nick : "unnamed",
			    raw));
		        if(msgno_exceptions(stream, raw, "0", &exbits, FALSE))
			  exbits |= (nt ? MSG_EX_FILTONCE : MSG_EX_FILTERED);
		        else
			  exbits = (nt ? MSG_EX_FILTONCE : MSG_EX_FILTERED);

		        msgno_exceptions(stream, raw, "0", &exbits, TRUE);
		    }
		}
		else{
		  for(i = 1L, n = 0L; i <= mn_get_total(msgmap); )
		    if((raw = mn_m2raw(msgmap, i))
		       && (mail_elt(stream, raw))->searched){
		        dprint(5, (debugfile,
			      "FILTER matching \"%s\": msg %ld %s\n",
			      pat->patgrp->nick ? pat->patgrp->nick : "unnamed",
			      raw, pat->action->folder ? "filed" : "killed"));
			if(nt)
			  i++;
			else{
			    msgno_exclude(stream, msgmap, i);
			    /* 
			     * if this message is new, decrement new_mail_count.
			     * previously, the caller would do this by counting
			     * MN_EXCLUDE before and after, but the results weren't
			     * accurate in the case where new messages arrived
			     * while filtering, or the filtered message could have
			     * gotten expunged.
			     */
			    if(msgno_exceptions(stream, raw, "0", &exbits, FALSE)
			       && (exbits & MSG_EX_RECENT)){
				if(stream == ps_global->mail_stream){
				    dprint(5, (debugfile,
		          "New message being filtered from mail_stream. new_mail_count: %d (old)\n",
					       ps_global->new_mail_count));
				   if(ps_global->new_mail_count > 0L)
				    ps_global->new_mail_count--;
				}
				else if(stream == ps_global->inbox_stream){
				    dprint(5, (debugfile,
	       "New message being filtered from inbox_stream. New inbox_new_mail_count: %d (old)\n",
					       ps_global->inbox_new_mail_count));
				    if(ps_global->inbox_new_mail_count > 0L)
				      ps_global->inbox_new_mail_count--;
				}
			    }
			}

		        if(msgno_exceptions(stream, raw, "0", &exbits, FALSE))
			  exbits |= (nt ? MSG_EX_FILTONCE : MSG_EX_FILTERED);
		        else
			  exbits = (nt ? MSG_EX_FILTONCE : MSG_EX_FILTERED);

		        msgno_exceptions(stream, raw, "0", &exbits, TRUE);
		        n++;
		    }
		    else
		      i++;
		}

		if(n && pat->action->folder){
		    PATTERN_S *p;
		    int	       err = 0;

		    tmpmap = NULL;
		    mn_init(&tmpmap, stream->nmsgs);

		    /*
		     * For everything matching msg that hasn't
		     * already been saved somewhere, do it...
		     */
		    for(i = 1L, n = 0L; i <= stream->nmsgs; i++)
		      if(mail_elt(stream, i)->searched
			 && !(msgno_exceptions(stream, i, "0", &exbits, FALSE)
			      && (exbits & MSG_EX_FILED)))
			if(!n++){
			    mn_set_cur(tmpmap, i);
			}
			else{
			    mn_add_cur(tmpmap, i);
			}

		    /*
		     * Remove already deleted messages from the tmp
		     * message map.
		     */
		    if(n && pat->action->move_only_if_not_deleted){
			char         *seq;
			MSGNO_S      *tmpmap2 = NULL;
			long          nn = 0L;
			MESSAGECACHE *mc;

			mn_init(&tmpmap2, stream->nmsgs);

			/*
			 * First, make sure elts are valid for all the
			 * interesting messages.
			 */
			if(seq = invalid_elt_sequence(stream, tmpmap)){
			    mail_fetchflags(stream, seq);
			    fs_give((void **) &seq);
			}

			for(i = mn_first_cur(tmpmap); i > 0L;
			    i = mn_next_cur(tmpmap)){
			    mc = mail_elt(stream, mn_m2raw(tmpmap, i));
			    if(!mc->deleted){
				if(!nn++){
				    mn_set_cur(tmpmap2, i);
				}
				else{
				    mn_add_cur(tmpmap2, i);
				}
			    }
			}

			mn_give(&tmpmap);
			tmpmap = tmpmap2;
			n = nn;
		    }

		    if(n){
			for(p = pat->action->folder; p; p = p->next){
			  int dval;
			  int flags_for_save;

			  /* does this filter set delete bit? ... */
			  convert_statebits_to_vals(pat->action->state_setting_bits, &dval, NULL, NULL, NULL);
			  /* ... if so, tell save not to fix it before copy */
			  flags_for_save = SV_FOR_FILT |
				  (nt ? 0 : SV_DELETE) |
				  ((dval != ACT_STAT_SET) ? SV_FIX_DELS : 0);
			  if(move_filtered_msgs(stream, tmpmap,
						p->substring,
						flags_for_save)){

			      /*
			       * If we filtered into the current
			       * folder, chuck a ping down the
			       * stream so the user can notice it
			       * before the next new mail check...
			       */
			      if(ps_global->mail_stream
				 && ps_global->mail_stream != stream
				 && ps_global->inbox_stream
				 && ps_global->mail_stream
						   != ps_global->inbox_stream
				 && match_pattern_folder_specific(
				     pat->action->folder,
				     ps_global->mail_stream, 0)){
				  mail_ping(ps_global->mail_stream);
			      }				
			  }
			  else{
			      err = 1;
			      break;
			  }
			}

			if(!err)
			  for(n = mn_first_cur(tmpmap);
			      n > 0L;
			      n = mn_next_cur(tmpmap)){

			      if(msgno_exceptions(stream, mn_m2raw(tmpmap, n),
						  "0", &exbits, FALSE))
				exbits |= (nt ? MSG_EX_FILEONCE : MSG_EX_FILED);
			      else
				exbits = (nt ? MSG_EX_FILEONCE : MSG_EX_FILED);

			      msgno_exceptions(stream, mn_m2raw(tmpmap, n),
					       "0", &exbits, TRUE);
			  }
		    }

		    mn_give(&tmpmap);
		}

		mail_free_searchset(&srchset);
		if(charset)
		  fs_give((void **) &charset);
	    }

	    /*
	     * If this is a terminating rule or the last rule,
	     * we make sure we delete messages that we delayed deleting
	     * in the save. We delayed so that the deletion wouldn't have
	     * an effect on later rules. We convert any temporary
	     * FILED (FILEONCE) and FILTERED (FILTONCE) flags
	     * (which were set by an earlier non-terminating rule)
	     * to permanent. We also exclude those messages from the view.
	     */
	    if(pending_actions && (!nt || !nextpat)){

		pending_actions = 0;
		tmpmap = NULL;
		mn_init(&tmpmap, stream->nmsgs);

		for(i = 1L, n = 0L; i <= mn_get_total(msgmap); i++){

		    raw = mn_m2raw(msgmap, i);
		    if(msgno_exceptions(stream, raw, "0", &exbits, FALSE)){
			if(exbits & MSG_EX_FILEONCE){
			    if(!n++){
				mn_set_cur(tmpmap, raw);
			    }
			    else{
				mn_add_cur(tmpmap, raw);
			    }
			}
		    }
		}

		if(n)
		  set_some_flags(stream, tmpmap, F_DEL, 0);

		mn_give(&tmpmap);

		for(i = 1L; i <= mn_get_total(msgmap); i++){
		    raw = mn_m2raw(msgmap, i);
		    if(msgno_exceptions(stream, raw, "0", &exbits, FALSE)){
			if(exbits & (MSG_EX_FILTONCE | MSG_EX_FILEONCE)){
			    if(exbits & MSG_EX_FILTONCE){
				/* exclude message from view */
				if(pat->action->kill || pat->action->folder){
				    msgno_exclude(stream, msgmap, i);
				    if(msgno_exceptions(stream, raw, "0", &exbits, FALSE)
				       && (exbits & MSG_EX_RECENT)){
					/* 
					 * if this message is new, decrement new_mail_count.
					 * see the above call to msgno_exclude.
					 */
					if(stream == ps_global->mail_stream){
					    dprint(5, (debugfile,
	    "New message being filtered from mail_stream. New new_mail_count: %d (old)\n",
						       ps_global->new_mail_count));
					   if(ps_global->new_mail_count > 0L)
					     ps_global->new_mail_count--;
					}
					else if(stream == ps_global->inbox_stream){
					    dprint(5, (debugfile,
     "New message being filtered from inbox_stream. New inbox_new_mail_count: %d (old)\n",
						       ps_global->inbox_new_mail_count));
					    if(ps_global->inbox_new_mail_count > 0L)
					      ps_global->inbox_new_mail_count--;
					}
				    }
				    i--;   /* to compensate for loop's i++ */
				}

				exbits ^= MSG_EX_FILTONCE;
				exbits |= MSG_EX_FILTERED;
			    }
			}

			if(exbits & MSG_EX_FILEONCE){
			    exbits ^= MSG_EX_FILEONCE;
			    exbits |= MSG_EX_FILED;
			}

			msgno_exceptions(stream, raw, "0", &exbits,TRUE);
		    }
		}
	    }
	}

	/* New mail arrival means start over */
	if(mail_uid(stream, stream->nmsgs) == uid)
	  break;
	/* else, go again */

	recent = 1; /* only check recent ones now */
    }

    /* clear any private "recent" flags */
    for(i = 1; i <= stream->nmsgs; i++){
	if(msgno_exceptions(stream, i, "0", &exbits, FALSE))
	  if(exbits & MSG_EX_RECENT){
	      exbits ^= MSG_EX_RECENT;
	      msgno_exceptions(stream, i, "0", &exbits,TRUE);
	  }
	/* clear any stmp flags just in case */
	mail_elt(stream, i)->spare6 = 0;
    }
    msgmap->flagged_stmp = 0L;
}


/*
 * Re-check the filters for matches because a change of message state may
 * have changed the results.
 */
void
reprocess_filter_patterns(stream, msgmap)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
{
    if(stream){
	msgno_include(stream, msgmap, TRUE);

	if(stream == ps_global->mail_stream){
	    clear_index_cache();
	    refresh_sort(msgmap, SRT_NON);
	    ps_global->mangled_header = 1;
	}

	process_filter_patterns(stream, msgmap, 0L);
    }
}


/*
 * When killing or filtering we don't want to match by mistake. So if
 * a pattern has nothing set except the Current Folder Type (which is always
 * set to something) we'll consider it to be trivial and not a match.
 * match_pattern uses this to determine if there is a match, where it is
 * just triggered on the Current Folder Type.
 */
int
trivial_patgrp(patgrp)
    PATGRP_S *patgrp;
{
    int ret = 1;

    if(patgrp){
	if(patgrp->subj || patgrp->cc || patgrp->from || patgrp->to ||
	   patgrp->sender || patgrp->news || patgrp->recip || patgrp->partic ||
	   patgrp->alltext || patgrp->bodytext)
	  ret = 0;

	if(ret && patgrp->do_age)
	  ret = 0;

	if(ret && patgrp->do_score)
	  ret = 0;

	if(ret && patgrp_depends_on_state(patgrp))
	  ret = 0;

	if(ret && patgrp->stat_8bitsubj != PAT_STAT_EITHER)
	  ret = 0;

	if(ret && patgrp->abookfrom != AFRM_EITHER)
	  ret = 0;

	if(ret && patgrp->arbhdr){
	    ARBHDR_S *a;

	    for(a = patgrp->arbhdr; a && ret; a = a->next)
	      if(a->field && a->field[0] && a->p)
		ret = 0;
	}
    }

    return(ret);
}


int
some_filter_depends_on_state()
{
    long          rflags = ROLE_DO_FILTER;
    PAT_S        *pat;
    PAT_STATE     pstate;
    int           ret = 0;

    if(nonempty_patterns(rflags, &pstate)){

	for(pat = first_pattern(&pstate);
	    pat && !ret;
	    pat = next_pattern(&pstate))
	  if(patgrp_depends_on_active_state(pat->patgrp))
	    ret++;
    }

    return(ret);
}


/*----------------------------------------------------------------------
  Move all messages with sequence bit lit to dstfldr
 
  Args: stream -- stream to use
	msgmap -- map of messages to be moved
	dstfldr -- folder to receive moved messages
	flags_for_save

  Returns: nonzeron on success
  ----*/
int
move_filtered_msgs(stream, msgmap, dstfldr, flags_for_save)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    char       *dstfldr;
    int         flags_for_save;
{
    long	  n;
    int           we_cancel = 0, i;
    CONTEXT_S	 *save_context = NULL;
    char	  buf[MAX_SCREEN_COLS+1], *ellipsis, *dstfldrp;
#define	FILTMSG_MAX	30

    if(!is_absolute_path(dstfldr)
       && !(save_context = default_save_context(ps_global->context_list)))
      save_context = ps_global->context_list;

    if((i = strlen(dstfldr)) > FILTMSG_MAX){
	dstfldrp = dstfldr + (i - FILTMSG_MAX);
	ellipsis = "...";
    }
    else{
	dstfldrp = dstfldr;
	ellipsis = "";
    }

    sprintf(buf, "Moving %s filtered message%s to \"%s%s\"",
	    comatose(mn_total_cur(msgmap)), plural(mn_total_cur(msgmap)),
	    ellipsis, dstfldrp);

    dprint(5, (debugfile, "%s\n", buf));

    we_cancel = busy_alarm(1, buf, NULL, 1);

    n = save(ps_global, stream, save_context, dstfldr, msgmap, flags_for_save);
    if(n != mn_total_cur(msgmap)){
	int   exbits;
	long  x;

	buf[0] = '\0';

	/* Clear "filtered" flags for failed messages */
	for(x = mn_first_cur(msgmap); x > 0L; x = mn_next_cur(msgmap))
	  if(n-- <= 0 && msgno_exceptions(stream, mn_m2raw(msgmap, x),
					  "0", &exbits, FALSE)){
	      exbits &= ~(MSG_EX_FILTONCE | MSG_EX_FILEONCE |
			  MSG_EX_FILTERED | MSG_EX_FILED);
	      msgno_exceptions(stream, mn_m2raw(msgmap, x),
			       "0", &exbits, TRUE);
	  }

	/* then re-incorporate them into folder they belong */
	msgno_include(stream,
		      (stream == ps_global->mail_stream)
			? ps_global->msgmap
			: (stream == ps_global->inbox_stream)
			    ? ps_global->inbox_msgmap : msgmap,
		      FALSE);
	clear_index_cache();
	refresh_sort(msgmap, SRT_NON);
	ps_global->mangled_header = 1;
    }
    else{
	sprintf(buf, "Filtered all %s message%s to \"%.45s\"",
		comatose(n), plural(n), dstfldr);
	dprint(5, (debugfile, "%s\n", buf));
    }

    if(we_cancel)
      cancel_busy_alarm(buf[0] ? 0 : -1);

    return(buf[0] != '\0');
}


/*----------------------------------------------------------------------
  Move all messages with sequence bit lit to dstfldr
 
  Args: stream -- stream to use
	msgmap -- which messages to set
	flagbits -- which flags to set or clear

  Returns: nonzero on on success
  ----*/
void
set_some_flags(stream, msgmap, flagbits, verbose)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    long        flagbits;
    int         verbose;
{
    long	  count = 0L, flipped_flags;
    int           we_cancel = 0;
    char          buf[100], *seq;

    /* use this to determine if anything needs to be done */
    flipped_flags = ((flagbits & F_ANS)    ? F_UNANS : 0)       |
		    ((flagbits & F_UNANS)  ? F_ANS : 0)         |
		    ((flagbits & F_FLAG)   ? F_UNFLAG : 0)      |
		    ((flagbits & F_UNFLAG) ? F_FLAG : 0)        |
		    ((flagbits & F_DEL)    ? F_UNDEL : 0)       |
		    ((flagbits & F_UNDEL)  ? F_DEL : 0)         |
		    ((flagbits & F_SEEN)   ? F_UNSEEN : 0)      |
		    ((flagbits & F_UNSEEN) ? F_SEEN : 0);
    if(seq = currentf_sequence(stream, msgmap, flipped_flags, &count, 0)){
	char sets[200], clears[200];
	char *ps, *pc;

	/* don't use those fixed-size buffers if we add user-defined flags */
	sets[0] = clears[0] = '\0';
	ps = sets;
	pc = clears;

	sprintf(buf, "Setting flags in %.10s message%.10s",
		comatose(count), plural(count));

	we_cancel = busy_alarm(1, buf, NULL, verbose ? 1 : 0);

	/*
	 * What's going on here? If we want to set more than one flag
	 * we can do it with a single roundtrip by combining the arguments
	 * into a single call and separating them with spaces.
	 */
	if(flagbits & F_ANS)
	  sstrcpy(&ps, "\\ANSWERED");
	if(flagbits & F_FLAG){
	    if(ps > sets)
	      sstrcpy(&ps, " ");

	    sstrcpy(&ps, "\\FLAGGED");
	}
	if(flagbits & F_DEL){
	    if(ps > sets)
	      sstrcpy(&ps, " ");

	    sstrcpy(&ps, "\\DELETED");
	}
	if(flagbits & F_SEEN){
	    if(ps > sets)
	      sstrcpy(&ps, " ");

	    sstrcpy(&ps, "\\SEEN");
	}

	/* need a separate call for the clears */
	if(flagbits & F_UNANS)
	  sstrcpy(&pc, "\\ANSWERED");
	if(flagbits & F_UNFLAG){
	    if(pc > sets)
	      sstrcpy(&pc, " ");

	    sstrcpy(&pc, "\\FLAGGED");
	}
	if(flagbits & F_UNDEL){
	    if(pc > sets)
	      sstrcpy(&pc, " ");

	    sstrcpy(&pc, "\\DELETED");
	}
	if(flagbits & F_UNSEEN){
	    if(pc > sets)
	      sstrcpy(&pc, " ");

	    sstrcpy(&pc, "\\SEEN");
	}


	if(sets[0])
	  mail_flag(stream, seq, sets, ST_SET);

	if(clears[0])
	  mail_flag(stream, seq, clears, 0L);

	fs_give((void **)&seq);

	if(we_cancel)
	  cancel_busy_alarm(buf[0] ? 0 : -1);
    }
}



/*
 * Delete messages which are marked FILTERED and excluded.
 * Messages which are FILTERED but not excluded are those that have had
 * their state set by a filter pattern, but are to remain in the same
 * folder.
 */
void
delete_filtered_msgs(stream)
    MAILSTREAM *stream;
{
    int	  exbits;
    long  i;
    char *seq;

    for(i = 1L; i <= stream->nmsgs; i++)
      if(msgno_exceptions(stream, i, "0", &exbits, FALSE)
	 && (exbits & MSG_EX_FILTERED)
	 && get_lflag(stream, NULL, i, MN_EXLD))
	mail_elt(stream, i)->sequence = 1;
      else
	mail_elt(stream, i)->sequence = 0;

    if(seq = build_sequence(stream, NULL, NULL)){
	mail_flag(stream, seq, "\\DELETED", ST_SET | ST_SILENT);
	fs_give((void **) &seq);
    }
}



/*----------------------------------------------------------------------
  Move all read messages from srcfldr to dstfldr
 
  Args: stream -- stream to usr
	dstfldr -- folder to receive moved messages
	buf -- place to write success message

  Returns: success message or NULL for failure
  ----*/
char *
move_read_msgs(stream, dstfldr, buf, searched)
    MAILSTREAM *stream;
    char       *dstfldr, *buf;
    long	searched;
{
    long	  i;
    int           we_cancel = 0;
    MSGNO_S	 *msgmap = NULL;
    CONTEXT_S	 *save_context = NULL;
    char	  *bufp = NULL;
    MESSAGECACHE *mc;

    if(!is_absolute_path(dstfldr)
       && !(save_context = default_save_context(ps_global->context_list)))
      save_context = ps_global->context_list;

    /*
     * Use the "sequence" bit to select the set of messages
     * we want to save.  If searched is non-neg, the message
     * cache already has the necessary "sequence" bits set.
     */
    if(searched < 0L)
      searched = count_flagged(stream, F_SEEN | F_UNDEL);

    if(searched){
	mn_init(&msgmap, stream->nmsgs);
	for(i = 1L; i <= mn_get_total(msgmap); i++)
	  set_lflag(stream, msgmap, i, MN_SLCT, 0);

	/*
	 * re-init msgmap to fix the MN_SLCT count, "flagged_tmp", in
	 * case there were any flagged such before we got here.
	 *
	 * BUG: this means the count of MN_SLCT'd msgs in the
	 * folder's real msgmap is instantly bogus.  Until Cancel
	 * after "Really quit?" is allowed, this isn't a problem since
	 * that mapping table is either gone or about to get nuked...
	 */
	mn_init(&msgmap, stream->nmsgs);

	/* select search results */
	for(i = 1L; i <= mn_get_total(msgmap); i++)
	  if(((mc = mail_elt(stream,mn_m2raw(msgmap, i)))->valid
	      && mc->seen && !mc->deleted)
	     || (!mc->valid && mc->searched))
	    set_lflag(stream, msgmap, i, MN_SLCT, 1);

	pseudo_selected(msgmap);
	sprintf(buf, "Moving %s read message%s to \"%.45s\"",
		comatose(searched), plural(searched), dstfldr);
	we_cancel = busy_alarm(1, buf, NULL, 1);
	if(save(ps_global, stream, save_context, dstfldr, msgmap,
		SV_DELETE | SV_FIX_DELS) == searched)
	  strncpy(bufp = buf + 1, "Moved", 5); /* change Moving to Moved */

	mn_give(&msgmap);
	if(we_cancel)
	  cancel_busy_alarm(bufp ? 0 : -1);
    }

    return(bufp);
}



/*----------------------------------------------------------------------
  Move read messages from folder if listed in archive
 
  Args: 

  ----*/
int
read_msg_prompt(n, f)
    long  n;
    char *f;
{
    char buf[MAX_SCREEN_COLS+1];

    sprintf(buf, "Save the %ld read message%s in \"%.*s\"", n, plural(n),
	    sizeof(buf)-50, f);
    return(want_to(buf, 'y', 0, NO_HELP, WT_NORM) == 'y');
}



/*----------------------------------------------------------------------
  Move read messages from folder if listed in archive
 
  Args: 

  ----*/
char *
move_read_incoming(stream, context, folder, archive, buf)
    MAILSTREAM *stream;
    CONTEXT_S  *context;
    char       *folder;
    char      **archive;
    char       *buf;
{
    char *s, *d, *f = folder;
    long  seen_undel;

    buf[0] = '\0';

    if(archive && stream != ps_global->inbox_stream
       && context && (context->use & CNTXT_INCMNG)
       && ((context_isambig(folder)
	    && folder_is_nick(folder, FOLDERS(context)))
	   || folder_index(folder, context, FI_FOLDER) > 0)
       && (seen_undel = count_flagged(stream, F_SEEN | F_UNDEL))){

	for(; f && *archive; archive++){
	    char *p;

	    get_pair(*archive, &s, &d, 1, 0);
	    if(s && d
	       && (!strcmp(s, folder)
		   || (context_isambig(folder)
		       && (p = folder_is_nick(folder, FOLDERS(context)))
		       && !strcmp(s, p)))){
		if(F_ON(F_AUTO_READ_MSGS,ps_global)
		   || read_msg_prompt(seen_undel, d))
		  buf = move_read_msgs(stream, d, buf, seen_undel);

		f = NULL;		/* bust out after cleaning up */
	    }

	    fs_give((void **)&s);
	    fs_give((void **)&d);
	}
    }

    return((buf && *buf) ? buf : NULL);
}


/*----------------------------------------------------------------------
    Delete all references to a deleted news posting

 
  ---*/
void
cross_delete_crossposts(stream)
    MAILSTREAM *stream;
{
    if(count_flagged(stream, F_DEL)){
	static char *fields[] = {"Xref", NULL};
	MAILSTREAM  *tstream;
	CONTEXT_S   *fake_context;
	char	    *xref, *p, *group, *uidp,
		    *newgrp, newfolder[MAILTMPLEN];
	long	     i, uid, hostlatch = 0L;
	int	     we_cancel = 0;

	strncpy(newfolder, stream->mailbox, sizeof(newfolder));
	newfolder[sizeof(newfolder)-1] = '\0';
	if(!(newgrp = strstr(newfolder, "#news.")))
	  return;				/* weird mailbox */

	newgrp += 6;

	we_cancel = busy_alarm(1, "Busy deleting crosspostings", NULL, 0);

	/* build subscribed list */
	strcpy(newgrp, "[]");
	fake_context = new_context(newfolder, 0);
	build_folder_list(NULL, fake_context, "*", NULL, BFL_LSUB);

	for(i = 1L; i <= stream->nmsgs; i++)
	  if(!get_lflag(stream, NULL, i, MN_EXLD)
	     && mail_elt(stream, i)->deleted){

	      if(xref = pine_fetchheader_lines(stream, i, NULL, fields)){
		  if(p = strstr(xref, ": ")){
		      p	     += 2;
		      hostlatch = 0L;
		      while(*p){
			  group = p;
			  uidp  = NULL;

			  /* get server */
			  while(*++p && !isspace((unsigned char) *p))
			    if(*p == ':'){
				*p   = '\0';
				uidp = p + 1;
			    }

			  /* tie off uid/host */
			  if(*p)
			    *p++ = '\0';

			  if(uidp){
			      /*
			       * For the nonce, we're only deleting valid
			       * uid's from outside the current newsgroup
			       * and inside only subscribed newsgroups
			       */
			      if(strcmp(group, stream->mailbox
							+ (newgrp - newfolder))
				 && folder_index(group, fake_context,
						 FI_FOLDER) >= 0){
				  if(uid = atol(uidp)){
				      strcpy(newgrp, group);
				      if(tstream = pine_mail_open(NULL,
							     newfolder, 0L)){
					  mail_flag(tstream, long2string(uid),
						    "\\DELETED",
						    ST_SET | ST_UID);
					  pine_mail_close(tstream);
				      }
				  }
				  else
				    break;		/* bogus uid */
			      }
			  }
			  else if(!hostlatch++){
			      char *p, *q;

			      if(stream->mailbox[0] == '{'
				 && !((p = strpbrk(stream->mailbox+1, "}:/"))
				      && !struncmp(stream->mailbox + 1,
						   q = canonical_name(group),
						   p - (stream->mailbox + 1))
				      && q[p - (stream->mailbox + 1)] == '\0'))
				break;		/* different server? */
			  }
			  else
			    break;		/* bogus field! */
		      }
		  }
		  
		  fs_give((void **) &xref);
	      }
	  }

	free_context(&fake_context);

	if(we_cancel)
	  cancel_busy_alarm(0);
    }
}



/*----------------------------------------------------------------------
    Print current message[s] or folder index

    Args: state -- pointer to struct holding a bunch of pine state
	 msgmap -- table mapping msg nums to c-client sequence nums
	    agg -- boolean indicating we're to operate on aggregate set
       in_index -- boolean indicating we're called from Index Screen

 Filters the original header and sends stuff to printer
  ---*/
void
cmd_print(state, msgmap, agg, in_index)
     struct pine *state;
     MSGNO_S     *msgmap;
     int	  agg;
     CmdWhere	  in_index;
{
    char      prompt[250];
    long      i, msgs;
    int	      next = 0, do_index = 0;
    ENVELOPE *e;
    BODY     *b;

    if(agg && !pseudo_selected(msgmap))
      return;

    msgs = mn_total_cur(msgmap);

    if((in_index != View) && F_ON(F_PRINT_INDEX, state)){
	char m[10];
	int  ans;
	static ESCKEY_S prt_opts[] = {
	    {'i', 'i', "I", "Index"},
	    {'m', 'm', "M", NULL},
	    {-1, 0, NULL, NULL}};

	if(in_index == ThrdIndx){
	    if(want_to("Print Index", 'y', 'x', NO_HELP, WT_NORM) == 'y')
	      ans = 'i';
	    else
	      ans = 'x';
	}
	else{
	    sprintf(m, "Message%s", (msgs>1L) ? "s" : "");
	    prt_opts[1].label = m;
	    sprintf(prompt, "Print %sFolder Index or %s %s? ",
		(agg==2) ? "thread " : agg ? "selected " : "",
		(agg==2) ? "thread" : agg ? "selected" : "current", m);

	    ans = radio_buttons(prompt, -FOOTER_ROWS(state), prt_opts, 'm', 'x',
				NO_HELP, RB_NORM|RB_SEQ_SENSITIVE);
	}

	switch(ans){
	  case 'x' :
	    cmd_cancelled("Print");
	    if(agg)
	      restore_selected(msgmap);

	    return;

	  case 'i':
	    do_index = 1;
	    break;

	  default :
	  case 'm':
	    break;
	}
    }

    if(do_index)
      sprintf(prompt, "%sFolder Index ",
	      (agg==2) ? "Thread " : agg ? "Selected " : "");
    else if(msgs > 1L)
      sprintf(prompt, "%s messages ", long2string(msgs));
    else
      sprintf(prompt, "Message %s ", long2string(mn_get_cur(msgmap)));

    if(open_printer(prompt) < 0){
	if(agg)
	  restore_selected(msgmap);

	return;
    }
    
    if(do_index){
	TITLE_S *tc;

	tc = format_titlebar();

	/* Print titlebar... */
	print_text1("%s\n\n", tc ? tc->titlebar_line : "");
	/* then all the index members... */
	if(!print_index(state, msgmap, agg))
	  q_status_message(SM_ORDER | SM_DING, 3, 3,
			   "Error printing folder index");
    }
    else{
        for(i = mn_first_cur(msgmap); i > 0L; i = mn_next_cur(msgmap), next++){
	    if(next && F_ON(F_AGG_PRINT_FF, state))
	      if(!print_char(FORMFEED))
	        break;

	    if(!(e=mail_fetchstructure(state->mail_stream, mn_m2raw(msgmap,i),
				       &b))
	       || (F_ON(F_FROM_DELIM_IN_PRINT, ps_global)
		   && !bezerk_delimiter(e, print_char, next))
	       || !format_message(mn_m2raw(msgmap, mn_get_cur(msgmap)),
				  e, b, NULL, FM_NEW_MESS, print_char)){
	        q_status_message(SM_ORDER | SM_DING, 3, 3,
			       "Error printing message");
	        break;
	    }
        }
    }

    close_printer();

    if(agg)
      restore_selected(msgmap);
}



/*
 * Support structure and functions to support piping raw message texts...
 */
static struct raw_pipe_data {
    MAILSTREAM *stream;
    long	msgno;
    char       *cur, *body;
} raw_pipe;


int
raw_pipe_getc(c)
     unsigned char *c;
{
    if((!raw_pipe.cur
	&& !(raw_pipe.cur = mail_fetchheader(raw_pipe.stream, raw_pipe.msgno)))
       || (!*raw_pipe.cur && !raw_pipe.body
	   && !(raw_pipe.cur = raw_pipe.body = mail_fetchtext(raw_pipe.stream,
							      raw_pipe.msgno)))
       || (!*raw_pipe.cur && raw_pipe.body))
      return(0);

    *c = (unsigned char) *raw_pipe.cur++;
    return(1);
}


void
prime_raw_text_getc(stream, msgno)
    MAILSTREAM *stream;
    long	msgno;
{
    raw_pipe.stream = stream;
    raw_pipe.msgno  = msgno;
    raw_pipe.cur    = raw_pipe.body = NULL;
}



/*----------------------------------------------------------------------
    Pipe message text

   Args: state -- various pine state bits
	 msgmap -- Message number mapping table
	 agg -- whether or not to aggregate the command on selected msgs

   Filters the original header and sends stuff to specified command
  ---*/
void
cmd_pipe(state, msgmap, agg)
     struct pine *state;
     MSGNO_S *msgmap;
     int	  agg;
{
    ENVELOPE      *e;
    BODY	  *b;
    PIPE_S	  *syspipe;
    char          *resultfilename = NULL, prompt[80];
    int            done = 0;
    gf_io_t	   pc;
    int		   next = 0;
    long           i;
    HelpType       help;
    static int	   capture = 1, raw = 0, delimit = 0, newpipe = 0;
    static char    pipe_command[MAXPATH];
    static ESCKEY_S pipe_opt[] = {
	{0, 0, "", ""},
	{ctrl('W'), 10, "^W", NULL},
	{ctrl('Y'), 11, "^Y", NULL},
	{ctrl('R'), 12, "^R", NULL},
	{0, 13, "^T", NULL},
	{-1, 0, NULL, NULL}
    };

    if(ps_global->restricted){
	q_status_message(SM_ORDER | SM_DING, 0, 4,
			 "Pine demo can't pipe messages");
	return;
    }
    else if(!any_messages(msgmap, NULL, "to Pipe"))
      return;

    if(agg){
	if(!pseudo_selected(msgmap))
	  return;
	else
	  pipe_opt[4].ch = ctrl('T');
    }
    else
      pipe_opt[4].ch = -1;

    help = NO_HELP;
    while (!done) {
	int flags;

	sprintf(prompt, "Pipe %smessage%s%s to %s%s%s%s%s%s%s: ",
		raw ? "RAW " : "",
		agg ? "s" : " ",
		agg ? "" : comatose(mn_get_cur(msgmap)),
		(!capture || delimit || (newpipe && agg)) ? "(" : "",
		capture ? "" : "uncaptured",
		(!capture && delimit) ? "," : "",
		delimit ? "delimited" : "",
		((!capture || delimit) && newpipe && agg) ? "," : "",
		(newpipe && agg) ? "new pipe" : "",
		(!capture || delimit || (newpipe && agg)) ? ") " : "");
	pipe_opt[1].label = raw ? "Shown Text" : "Raw Text";
	pipe_opt[2].label = capture ? "Free Output" : "Capture Output";
	pipe_opt[3].label = delimit ? "No Delimiter" : "With Delimiter";
	pipe_opt[4].label = newpipe ? "To Same Pipe" : "To Individual Pipes";
	flags = OE_APPEND_CURRENT | OE_SEQ_SENSITIVE;
	switch(optionally_enter(pipe_command, -FOOTER_ROWS(state), 0,
				sizeof(pipe_command), prompt,
				pipe_opt, help, &flags)){
	  case -1 :
	    q_status_message(SM_ORDER | SM_DING, 3, 4,
			     "Internal problem encountered");
	    done++;
	    break;
      
	  case 10 :			/* flip raw bit */
	    raw = !raw;
	    break;

	  case 11 :			/* flip capture bit */
	    capture = !capture;
	    break;

	  case 12 :			/* flip delimit bit */
	    delimit = !delimit;
	    break;

	  case 13 :			/* flip newpipe bit */
	    newpipe = !newpipe;
	    break;

	  case 0 :
	    if(pipe_command[0]){
		flags = PIPE_USER | PIPE_WRITE | PIPE_STDERR;
		if(!capture){
#ifndef	_WINDOWS
		    ClearScreen();
		    fflush(stdout);
		    clear_cursor_pos();
		    ps_global->mangled_screen = 1;
		    ps_global->in_init_seq = 1;
#endif
		    flags |= PIPE_RESET;
		}

		if(!newpipe && !(syspipe = cmd_pipe_open(pipe_command,
							 (flags & PIPE_RESET)
							   ? NULL
							   : &resultfilename,
							 flags, &pc)))
		  done++;

		for(i = mn_first_cur(msgmap);
		    i > 0L && !done;
		    i = mn_next_cur(msgmap)){
		    e = mail_fetchstructure(ps_global->mail_stream,
					    mn_m2raw(msgmap, i), &b);

		    if((newpipe
			&& !(syspipe = cmd_pipe_open(pipe_command,
						     (flags & PIPE_RESET)
						       ? NULL
						       : &resultfilename,
						     flags, &pc)))
		       || (delimit && !bezerk_delimiter(e, pc, next++)))
		      done++;

		    if(!done){
			if(raw){
			    char    *pipe_err;

			    prime_raw_text_getc(ps_global->mail_stream,
						mn_m2raw(msgmap, i));
			    gf_filter_init();
			    gf_link_filter(gf_nvtnl_local, NULL);
			    if(pipe_err = gf_pipe(raw_pipe_getc, pc)){
				q_status_message1(SM_ORDER|SM_DING,
						  3, 3,
						  "Internal Error: %.200s",
						  pipe_err);
				done++;
			    }
			}
			else if(!format_message(mn_m2raw(msgmap, i), e, b,
						NULL,
						FM_NEW_MESS | FM_NOWRAP, pc))
			  done++;
		    }

		    if(newpipe)
		      (void) close_system_pipe(&syspipe);
		}

		if(!capture)
		  ps_global->in_init_seq = 0;

		if(!newpipe)
		  (void) close_system_pipe(&syspipe);

		if(done)		/* say we had a problem */
		  q_status_message(SM_ORDER | SM_DING, 3, 3,
				   "Error piping message");
		else if(resultfilename){
		    /* only display if no error */
		    display_output_file(resultfilename, "PIPE MESSAGE",
					NULL, DOF_EMPTY);
		    fs_give((void **)&resultfilename);
		}
		else
		  q_status_message(SM_ORDER, 0, 2, "Pipe command completed");

		done++;
		break;
	    }
	    /* else fall thru as if cancelled */

	  case 1 :
	    cmd_cancelled("Pipe command");
	    done++;
	    break;

	  case 3 :
            help = (help == NO_HELP) ? h_pipe_msg : NO_HELP;
	    break;

	  case 2 :                              /* no place to escape to */
	  case 4 :                              /* can't suspend */
	  default :
	    break;   
	}
    }

    ps_global->mangled_footer = 1;
    if(agg)
      restore_selected(msgmap);
}



/*----------------------------------------------------------------------
  Actually open the pipe used to write piped data down

   Args: 
   Returns: TRUE if success, otherwise FALSE

  ----*/
PIPE_S *
cmd_pipe_open(cmd, result, flags, pc)
    char     *cmd;
    char    **result;
    int       flags;
    gf_io_t  *pc;
{
    PIPE_S *pipe;

    if(pipe = open_system_pipe(cmd, result, NULL, flags, 0))
      gf_set_writec(pc, pipe->out.f, 0L, FileStar);
    else
      q_status_message(SM_ORDER | SM_DING, 3, 3, "Error opening pipe") ;

    return(pipe);
}



/*----------------------------------------------------------------------
  Screen to offer list management commands contained in message

    Args: state -- pointer to struct holding a bunch of pine state
	 msgmap -- table mapping msg nums to c-client sequence nums
	    agg -- boolean indicating we're to operate on aggregate set

 Result: 

   NOTE: Inspired by contrib from Jeremy Blackman <loki@maison-otaku.net>
 ----*/
void
rfc2369_display(stream, msgmap, msgno)
     MAILSTREAM *stream;
     MSGNO_S	*msgmap;
     long	 msgno;
{
    int	       winner = 0;
    char      *h, *hdrs[MLCMD_COUNT + 1];
    long       index_no = mn_raw2m(msgmap, msgno);
    RFC2369_S  data[MLCMD_COUNT];

    /* for each header field */
    if(h = pine_fetchheader_lines(stream, msgno, NULL, rfc2369_hdrs(hdrs))){
	memset(&data[0], 0, sizeof(RFC2369_S) * MLCMD_COUNT);
	if(rfc2369_parse_fields(h, &data[0])){
	    STORE_S *explain;

	    if(explain = list_mgmt_text(data, index_no)){
		list_mgmt_screen(explain);
		ps_global->mangled_screen = 1;
		so_give(&explain);
		winner++;
	    }
	}

	fs_give((void **) &h);
    }

    if(!winner)
      q_status_message1(SM_ORDER, 0, 3,
		    "Message %.200s contains no list management information",
			comatose(index_no));
}


STORE_S *
list_mgmt_text(data, msgno)
    RFC2369_S *data;
    long       msgno;
{
    STORE_S	  *store;
    int		   i, j, n, fields = 0;
    static  char  *rfc2369_intro1 =
      "<HTML><HEAD></HEAD><BODY><H1>Mail List Commands</H1>Message ";
    static char *rfc2369_intro2[] = {
	" has information associated with it ",
	"that explains how to participate in an email list.  An ",
	"email list is represented by a single email address that ",
	"users sharing a common interest can send messages to (known ",
	"as posting) which are then redistributed to all members ",
	"of the list (sometimes after review by a moderator).",
	"<P>List participation commands in this message include:",
	NULL
    };

    if(store = so_get(CharStar, NULL, EDIT_ACCESS)){

	/* Insert introductory text */
	so_puts(store, rfc2369_intro1);

	so_puts(store, comatose(msgno));

	for(i = 0; rfc2369_intro2[i]; i++)
	  so_puts(store, rfc2369_intro2[i]);

	so_puts(store, "<P>");
	for(i = 0; i < MLCMD_COUNT; i++)
	  if(data[i].data[0].value
	     || data[i].data[0].comment
	     || data[i].data[0].error){
	      if(!fields++)
		so_puts(store, "<UL>");

	      so_puts(store, "<LI>");
	      so_puts(store,
		      (n = (data[i].data[1].value || data[i].data[1].comment))
			? "Methods to "
			: "A method to ");

	      so_puts(store, data[i].field.description);
	      so_puts(store, ". ");

	      if(n)
		so_puts(store, "<OL>");

	      for(j = 0;
		  j < MLCMD_MAXDATA
		  && (data[i].data[j].comment
		      || data[i].data[j].value
		      || data[i].data[j].error);
		  j++){

		  so_puts(store, n ? "<P><LI>" : "<P>");

		  if(data[i].data[j].comment){
		      so_puts(store,
			      "With the provided comment:<P><BLOCKQUOTE>");
		      so_puts(store, data[i].data[j].comment);
		      so_puts(store, "</BLOCKQUOTE><P>");
		  }

		  if(data[i].data[j].value){
		      if(i == MLCMD_POST
			 && !strucmp(data[i].data[j].value, "NO")){
			  so_puts(store,
			   "Posting is <EM>not</EM> allowed on this list");
		      }
		      else{
			  so_puts(store, "Select <A HREF=\"");
			  so_puts(store, data[i].data[j].value);
			  so_puts(store, "\">HERE</A> to ");
			  so_puts(store, (data[i].field.action)
					   ? data[i].field.action
					   : "try it");
		      }

		      so_puts(store, ".");
		  }

		  if(data[i].data[j].error){
		      so_puts(store, "<P>Unfortunately, Pine can not offer");
		      so_puts(store, " to take direct action based upon it");
		      so_puts(store, " because it was improperly formatted.");
		      so_puts(store, " The unrecognized data associated with");
		      so_puts(store, " the \"");
		      so_puts(store, data[i].field.name);
		      so_puts(store, "\" header field was:<P><BLOCKQUOTE>");
		      so_puts(store, data[i].data[j].error);
		      so_puts(store, "</BLOCKQUOTE>");
		  }

		  so_puts(store, "<P>");
	      }

	      if(n)
		so_puts(store, "</OL>");
	  }

	if(fields)
	  so_puts(store, "</UL>");

	so_puts(store, "</BODY></HTML>");
    }

    return(store);
}



static struct key listmgr_keys[] =
       {HELP_MENU,
	NULL_MENU,
	{"E","Exit CmdList",{MC_EXIT,1,{'e'}},KS_EXITMODE},
	{"Ret","[Try Command]",{MC_VIEW_HANDLE,3,
				{ctrl('m'),ctrl('j'),KEY_RIGHT}},KS_NONE},
	{"^B","Prev Cmd",{MC_PREV_HANDLE,1,{ctrl('B')}},KS_NONE},
	{"^F","Next Cmd",{MC_NEXT_HANDLE,1,{ctrl('F')}},KS_NONE},
	PREVPAGE_MENU,
	NEXTPAGE_MENU,
	PRYNTTXT_MENU,
	WHEREIS_MENU,
	NULL_MENU,
	NULL_MENU};
INST_KEY_MENU(listmgr_keymenu, listmgr_keys);
#define	LM_TRY_KEY	3
#define	LM_PREV_KEY	4
#define	LM_NEXT_KEY	5


void
list_mgmt_screen(html)
    STORE_S *html;
{
    int		    cmd = MC_NONE;
    long	    offset = 0L;
    char	   *error = NULL;
    STORE_S	   *store;
    HANDLE_S	   *handles = NULL;
    gf_io_t	    gc, pc;

    do{
	so_seek(html, 0L, 0);
	gf_set_so_readc(&gc, html);

	init_handles(&handles);

	if(store = so_get(CharStar, NULL, EDIT_ACCESS)){
	    gf_set_so_writec(&pc, store);
	    gf_filter_init();

	    gf_link_filter(gf_html2plain,
			   gf_html2plain_opt(NULL,
					     ps_global->ttyo->screen_cols,
					     &handles,
					     0));

	    error = gf_pipe(gc, pc);

	    gf_clear_so_writec(store);

	    if(!error){
		SCROLL_S	sargs;

		memset(&sargs, 0, sizeof(SCROLL_S));
		sargs.text.text	   = so_text(store);
		sargs.text.src	   = CharStar;
		sargs.text.desc	   = "list commands";
		sargs.text.handles = handles;
		if(offset){
		    sargs.start.on	   = Offset;
		    sargs.start.loc.offset = offset;
		}

		sargs.bar.title	   = "MAIL LIST COMMANDS";
		sargs.bar.style	   = MessageNumber;
		sargs.resize_exit  = 1;
		sargs.help.text	   = h_special_list_commands;
		sargs.help.title   = "HELP FOR LIST COMMANDS";
		sargs.keys.menu	   = &listmgr_keymenu;
		setbitmap(sargs.keys.bitmap);
		if(!handles){
		    clrbitn(LM_TRY_KEY, sargs.keys.bitmap);
		    clrbitn(LM_PREV_KEY, sargs.keys.bitmap);
		    clrbitn(LM_NEXT_KEY, sargs.keys.bitmap);
		}

		cmd = scrolltool(&sargs);
		offset = sargs.start.loc.offset;
	    }

	    so_give(&store);
	}

	free_handles(&handles);
	gf_clear_so_readc(html);
    }
    while(cmd == MC_RESIZE);
}



/*----------------------------------------------------------------------
 Prompt the user for the type of select desired

   NOTE: any and all functions that successfully exit the second
	 switch() statement below (currently "select_*() functions"),
	 *MUST* update the folder's MESSAGECACHE element's "searched"
	 bits to reflect the search result.  Functions using
	 mail_search() get this for free, the others must update 'em
	 by hand.
  ----*/
void
aggregate_select(state, msgmap, q_line, in_index)
    struct pine *state;
    MSGNO_S     *msgmap;
    int	         q_line;
    CmdWhere	 in_index;
{
    long       i, diff, old_tot, msgno;
    int        q = 0, rv = 0, narrow = 0, hidden;
    HelpType   help = NO_HELP;
    ESCKEY_S  *sel_opts;
    extern     MAILSTREAM *mm_search_stream;
    extern     long	   mm_search_count;

    hidden           = any_lflagged(msgmap, MN_HIDE) > 0L;
    mm_search_stream = state->mail_stream;
    mm_search_count  = 0L;

    sel_opts = sel_opts2;
    if(old_tot = any_lflagged(msgmap, MN_SLCT)){
	i = get_lflag(state->mail_stream, msgmap, mn_get_cur(msgmap), MN_SLCT);
	sel_opts1[1].label = "unselect Cur" + (i ? 0 : 2);
	sel_opts += 2;			/* disable extra options */
	switch(q = radio_buttons(sel_pmt1, q_line, sel_opts1, 'c', 'x', help,
				 RB_NORM)){
	  case 'f' :			/* flip selection */
	    msgno = 0L;
	    for(i = 1L; i <= mn_get_total(msgmap); i++){
		q = !get_lflag(state->mail_stream, msgmap, i, MN_SLCT);
		set_lflag(state->mail_stream, msgmap, i, MN_SLCT, q);
		if(hidden){
		    set_lflag(state->mail_stream, msgmap, i, MN_HIDE, !q);
		    if(!msgno && q)
		      mn_reset_cur(msgmap, msgno = i);
		}
	    }

	    return;

	  case 'n' :			/* narrow selection */
	    narrow++;
	  case 'b' :			/* broaden selection */
	    q = 0;			/* offer criteria prompt */
	    break;

	  case 'c' :			/* Un/Select Current */
	  case 'a' :			/* Unselect All */
	  case 'x' :			/* cancel */
	    break;

	  default :
	    q_status_message(SM_ORDER | SM_DING, 3, 3,
			     "Unsupported Select option");
	    return;
	}
    }

    if(!q)
      q = radio_buttons(sel_pmt2, q_line, sel_opts, 'c', 'x', help, RB_NORM);

    /*
     * NOTE: See note about MESSAGECACHE "searched" bits above!
     */
    switch(q){
      case 'x':				/* cancel */
	cmd_cancelled("Select command");
	return;

      case 'c' :			/* select/unselect current */
	(void) individual_select(state, msgmap, q_line, in_index);
	return;

      case 'a' :			/* select/unselect all */
	msgno = any_lflagged(msgmap, MN_SLCT);
	diff    = (!msgno) ? mn_get_total(msgmap) : 0L;

	for(i = 1L; i <= mn_get_total(msgmap); i++){
	    if(msgno){		/* unmark 'em all */
		if(get_lflag(state->mail_stream, msgmap, i, MN_SLCT)){
		    diff++;
		    set_lflag(state->mail_stream, msgmap, i, MN_SLCT, 0);
		}
		else if(hidden)
		  set_lflag(state->mail_stream, msgmap, i, MN_HIDE, 0);
	    }
	    else			/* mark 'em all */
	      set_lflag(state->mail_stream, msgmap, i, MN_SLCT, 1);
	}

	q_status_message4(SM_ORDER,0,2,
			  "%.200s%.200s message%.200s %.200sselected",
			  msgno ? "" : "All ", comatose(diff), 
			  plural(diff), msgno ? "UN" : "");
	return;

      case 'n' :			/* Select by Number */
	rv = select_number(state->mail_stream, msgmap, mn_get_cur(msgmap));
	break;

      case 'd' :			/* Select by Date */
	rv = select_date(state->mail_stream, msgmap, mn_get_cur(msgmap));
	break;

      case 't' :			/* Text */
	rv = select_text(state->mail_stream, msgmap, mn_get_cur(msgmap));
	break;

      case 'z' :			/* Size */
	rv = select_size(state->mail_stream, msgmap, mn_get_cur(msgmap));
	break;

      case 's' :			/* Status */
	rv = select_flagged(state->mail_stream, msgmap, mn_get_cur(msgmap));
	break;

      default :
	q_status_message(SM_ORDER | SM_DING, 3, 3,
			 "Unsupported Select option");
	return;
    }

    if(rv)				/* bad return value.. */
      return;				/* error already displayed */

    if(narrow)				/* make sure something was selected */
      for(i = 1L; i <= mn_get_total(msgmap); i++)
	if(mail_elt(state->mail_stream, mn_m2raw(msgmap, i))->searched){
	    if(get_lflag(state->mail_stream, msgmap, i, MN_SLCT))
	      break;
	    else
	      mm_search_count--;
	}

    diff = 0L;
    if(mm_search_count){
	/*
	 * loop thru all the messages, adjusting local flag bits
	 * based on their "searched" bit...
	 */
	for(i = 1L, msgno = 0L; i <= mn_get_total(msgmap); i++)
	  if(narrow){
	      /* turning OFF selectedness if the "searched" bit isn't lit. */
	      if(get_lflag(state->mail_stream, msgmap, i, MN_SLCT)){
		  if(!mail_elt(state->mail_stream,
			       mn_m2raw(msgmap, i))->searched){
		      diff--;
		      set_lflag(state->mail_stream, msgmap, i, MN_SLCT, 0);
		      if(hidden)
			set_lflag(state->mail_stream, msgmap, i, MN_HIDE, 1);
		  }
		  else if(msgno < mn_get_cur(msgmap))
		    msgno = i;
	      }
	  }
	  else if(mail_elt(state->mail_stream,mn_m2raw(msgmap,i))->searched){
	      /* turn ON selectedness if "searched" bit is lit. */
	      if(!get_lflag(state->mail_stream, msgmap, i, MN_SLCT)){
		  diff++;
		  set_lflag(state->mail_stream, msgmap, i, MN_SLCT, 1);
		  if(hidden)
		    set_lflag(state->mail_stream, msgmap, i, MN_HIDE, 0);
	      }
	  }

	/* if we're zoomed and the current message was unselected */
	if(narrow && msgno
	   && get_lflag(state->mail_stream,msgmap,mn_get_cur(msgmap),MN_HIDE))
	  mn_reset_cur(msgmap, msgno);
    }

    if(!diff){
	if(narrow)
	  q_status_message4(SM_ORDER, 0, 2,
			"%.200s.  %.200s message%.200s remain%.200s selected.",
			mm_search_count ? "No change resulted"
					: "No messages in intersection",
			comatose(old_tot), plural(old_tot),
			(old_tot == 1L) ? "s" : "");
	else if(old_tot && mm_search_count)
	  q_status_message(SM_ORDER, 0, 2,
		   "No change resulted.  Matching messages already selected.");
	else
	  q_status_message1(SM_ORDER | SM_DING, 0, 2,
			    "Select failed.  No %.200smessages selected.",
			    old_tot ? "additional " : "");
    }
    else if(old_tot){
	sprintf(tmp_20k_buf,
		"Select matched %ld message%s.  %s %smessage%s %sselected.",
		(diff > 0) ? diff : old_tot + diff,
		plural((diff > 0) ? diff : old_tot + diff),
		comatose((diff > 0) ? any_lflagged(msgmap, MN_SLCT) : -diff),
		(diff > 0) ? "total " : "",
		plural((diff > 0) ? any_lflagged(msgmap, MN_SLCT) : -diff),
		(diff > 0) ? "" : "UN");
	q_status_message(SM_ORDER, 0, 2, tmp_20k_buf);
    }
    else
      q_status_message2(SM_ORDER, 0, 2, "Select matched %.200s message%.200s!",
			comatose(diff), plural(diff));
}


/*
 * This is like aggregate_select but user is in the Thread Index.
 */
void
thread_index_select(state, msgmap, q_line, in_index)
    struct pine *state;
    MSGNO_S     *msgmap;
    int	         q_line;
    CmdWhere	 in_index;
{
    long        i, diff, old_tot, msgno;
    int         q = 0, rv = 0, narrow = 0, hidden;
    HelpType    help = NO_HELP;
    ESCKEY_S   *sel_opts;
    PINETHRD_S *thrd;
    extern      MAILSTREAM *mm_search_stream;
    extern      long	    mm_search_count;

    hidden           = any_lflagged(msgmap, MN_HIDE) > 0L;
    mm_search_stream = state->mail_stream;
    mm_search_count  = 0L;

    sel_opts = sel_opts4;
    if(old_tot = any_lflagged(msgmap, MN_SLCT)){
	i = 0;
	thrd = fetch_thread(state->mail_stream,
			    mn_m2raw(msgmap, mn_get_cur(msgmap)));
	if(thrd &&
	   count_lflags_in_thread(state->mail_stream, thrd, msgmap, MN_SLCT) ==
	   count_lflags_in_thread(state->mail_stream, thrd, msgmap, MN_NONE))
	  i = 1;

	sel_opts1[1].label = "unselect Curthrd" + (i ? 0 : 2);
	sel_opts += 2;			/* disable extra options */
	switch(q = radio_buttons(sel_pmt1, q_line, sel_opts1, 'c', 'x', help,
				 RB_NORM)){
	  case 'f' :			/* flip selection */
	    msgno = 0L;
	    for(i = 1L; i <= mn_get_total(msgmap); i++){
		q = !get_lflag(state->mail_stream, msgmap, i, MN_SLCT);
		set_lflag(state->mail_stream, msgmap, i, MN_SLCT, q);
		if(hidden){
		    set_lflag(state->mail_stream, msgmap, i, MN_HIDE, !q);
		    if(!msgno && q)
		      mn_reset_cur(msgmap, msgno = i);
		}
	    }

	    return;

	  case 'n' :			/* narrow selection */
	    narrow++;
	  case 'b' :			/* broaden selection */
	    q = 0;			/* offer criteria prompt */
	    break;

	  case 'c' :			/* Un/Select Current */
	  case 'a' :			/* Unselect All */
	  case 'x' :			/* cancel */
	    break;

	  default :
	    q_status_message(SM_ORDER | SM_DING, 3, 3,
			     "Unsupported Select option");
	    return;
	}
    }

    if(!q)
      q = radio_buttons(sel_pmt2, q_line, sel_opts, 'c', 'x', help, RB_NORM);

    /*
     * NOTE: See note about MESSAGECACHE "searched" bits above!
     */
    switch(q){
      case 'x':				/* cancel */
	cmd_cancelled("Select command");
	return;

      case 'c' :			/* select/unselect current */
	(void) individual_select(state, msgmap, q_line, in_index);
	return;

      case 'a' :			/* select/unselect all */
	msgno = any_lflagged(msgmap, MN_SLCT);
	diff    = (!msgno) ? mn_get_total(msgmap) : 0L;

	for(i = 1L; i <= mn_get_total(msgmap); i++){
	    if(msgno){		/* unmark 'em all */
		if(get_lflag(state->mail_stream, msgmap, i, MN_SLCT)){
		    diff++;
		    set_lflag(state->mail_stream, msgmap, i, MN_SLCT, 0);
		}
		else if(hidden)
		  set_lflag(state->mail_stream, msgmap, i, MN_HIDE, 0);
	    }
	    else			/* mark 'em all */
	      set_lflag(state->mail_stream, msgmap, i, MN_SLCT, 1);
	}

	q_status_message4(SM_ORDER,0,2,
			  "%.200s%.200s message%.200s %.200sselected",
			  msgno ? "" : "All ", comatose(diff), 
			  plural(diff), msgno ? "UN" : "");
	return;

      case 'n' :			/* Select by Number */
	rv = select_thrd_number(state->mail_stream, msgmap, mn_get_cur(msgmap));
	break;

      case 'd' :			/* Select by Date */
	rv = select_date(state->mail_stream, msgmap, mn_get_cur(msgmap));
	break;

      case 't' :			/* Text */
	rv = select_text(state->mail_stream, msgmap, mn_get_cur(msgmap));
	break;

      case 'z' :			/* Size */
	rv = select_size(state->mail_stream, msgmap, mn_get_cur(msgmap));
	break;

      case 's' :			/* Status */
	rv = select_flagged(state->mail_stream, msgmap, mn_get_cur(msgmap));
	break;

      default :
	q_status_message(SM_ORDER | SM_DING, 3, 3,
			 "Unsupported Select option");
	return;
    }

    if(rv)				/* bad return value.. */
      return;				/* error already displayed */

    if(narrow)				/* make sure something was selected */
      for(i = 1L; i <= mn_get_total(msgmap); i++)
	if(mail_elt(state->mail_stream, mn_m2raw(msgmap, i))->searched){
	    if(get_lflag(state->mail_stream, msgmap, i, MN_SLCT))
	      break;
	    else
	      mm_search_count--;
	}

    diff = 0L;
    if(mm_search_count){
	/*
	 * loop thru all the messages, adjusting local flag bits
	 * based on their "searched" bit...
	 */
	for(i = 1L, msgno = 0L; i <= mn_get_total(msgmap); i++)
	  if(narrow){
	      /* turning OFF selectedness if the "searched" bit isn't lit. */
	      if(get_lflag(state->mail_stream, msgmap, i, MN_SLCT)){
		  if(!mail_elt(state->mail_stream,
			       mn_m2raw(msgmap, i))->searched){
		      diff--;
		      set_lflag(state->mail_stream, msgmap, i, MN_SLCT, 0);
		      if(hidden)
			set_lflag(state->mail_stream, msgmap, i, MN_HIDE, 1);
		  }
		  else if(msgno < mn_get_cur(msgmap)
			  && !get_lflag(state->mail_stream, msgmap, i, MN_CHID))
		    msgno = i;
	      }
	  }
	  else if(mail_elt(state->mail_stream,mn_m2raw(msgmap,i))->searched){
	      /* turn ON selectedness if "searched" bit is lit. */
	      if(!get_lflag(state->mail_stream, msgmap, i, MN_SLCT)){
		  diff++;
		  set_lflag(state->mail_stream, msgmap, i, MN_SLCT, 1);
		  if(hidden)
		    set_lflag(state->mail_stream, msgmap, i, MN_HIDE, 0);
	      }
	  }

	/* if we're zoomed and the current message was unselected */
	if(narrow && msgno
	   && get_lflag(state->mail_stream,msgmap,mn_get_cur(msgmap),MN_HIDE))
	  mn_reset_cur(msgmap, msgno);
    }

    if(!diff){
	if(narrow)
	  q_status_message4(SM_ORDER, 0, 2,
			"%.200s.  %.200s message%.200s remain%.200s selected.",
			mm_search_count ? "No change resulted"
					: "No messages in intersection",
			comatose(old_tot), plural(old_tot),
			(old_tot == 1L) ? "s" : "");
	else if(old_tot && mm_search_count)
	  q_status_message(SM_ORDER, 0, 2,
		   "No change resulted.  Matching messages already selected.");
	else
	  q_status_message1(SM_ORDER | SM_DING, 0, 2,
			    "Select failed.  No %.200smessages selected.",
			    old_tot ? "additional " : "");
    }
    else if(old_tot){
	sprintf(tmp_20k_buf,
		"Select matched %ld message%s.  %s %smessage%s %sselected.",
		(diff > 0) ? diff : old_tot + diff,
		plural((diff > 0) ? diff : old_tot + diff),
		comatose((diff > 0) ? any_lflagged(msgmap, MN_SLCT) : -diff),
		(diff > 0) ? "total " : "",
		plural((diff > 0) ? any_lflagged(msgmap, MN_SLCT) : -diff),
		(diff > 0) ? "" : "UN");
	q_status_message(SM_ORDER, 0, 2, tmp_20k_buf);
    }
    else
      q_status_message2(SM_ORDER, 0, 2, "Select matched %.200s message%.200s!",
			comatose(diff), plural(diff));
}



/*----------------------------------------------------------------------
 Toggle the state of the current message

   Args: state -- pointer pine's state variables
	 msgmap -- message collection to operate on
	 q_line -- line on display to write prompts
	 in_index -- in the message index view
   Returns: TRUE if current marked selected, FALSE otw
  ----*/
int
individual_select(state, msgmap, q_line, in_index)
     struct pine *state;
     MSGNO_S     *msgmap;
     int	  q_line;
     CmdWhere	  in_index;
{
    long cur;
    int  all_selected = 0;
    unsigned long was, is, tot;

    cur = mn_get_cur(msgmap);

    if(THRD_INDX()){
	PINETHRD_S *thrd;

	thrd = fetch_thread(state->mail_stream, mn_m2raw(msgmap, cur));
	if(!thrd)
	  return 0;

	was = count_lflags_in_thread(state->mail_stream, thrd, msgmap, MN_SLCT);
	tot = count_lflags_in_thread(state->mail_stream, thrd, msgmap, MN_NONE);
	if(was == tot)
	  all_selected++;

	if(all_selected){
	    set_thread_lflags(state->mail_stream, thrd, msgmap, MN_SLCT, 0);
	    if(any_lflagged(msgmap, MN_HIDE) > 0L){
		set_thread_lflags(state->mail_stream, thrd, msgmap, MN_HIDE, 1);
		/*
		 * See if there's anything left to zoom on.  If so, 
		 * pick an adjacent one for highlighting, else make
		 * sure nothing is left hidden...
		 */
		if(any_lflagged(msgmap, MN_SLCT)){
		    mn_inc_cur(state->mail_stream, msgmap,
			       (in_index == View && THREADING()
				&& state->viewing_a_thread)
				 ? MH_THISTHD
				 : (in_index == View)
				   ? MH_ANYTHD : MH_NONE);
		    if(mn_get_cur(msgmap) == cur)
		      mn_dec_cur(state->mail_stream, msgmap,
			         (in_index == View && THREADING()
				  && state->viewing_a_thread)
				   ? MH_THISTHD
				   : (in_index == View)
				     ? MH_ANYTHD : MH_NONE);
		}
		else			/* clear all hidden flags */
		  (void) unzoom_index(state, state->mail_stream, msgmap);
	    }
	}
	else
	  set_thread_lflags(state->mail_stream, thrd, msgmap, MN_SLCT, 1);

	q_status_message3(SM_ORDER, 0, 2, "%.200s message%.200s %.200sselected",
			  comatose(all_selected ? was : tot-was),
			  plural(all_selected ? was : tot-was),
			  all_selected ? "UN" : "");
    }
    else{
	if(all_selected =
	   get_lflag(state->mail_stream, msgmap, cur, MN_SLCT)){ /* set? */
	    set_lflag(state->mail_stream, msgmap, cur, MN_SLCT, 0);
	    if(any_lflagged(msgmap, MN_HIDE) > 0L){
		set_lflag(state->mail_stream, msgmap, cur, MN_HIDE, 1);
		/*
		 * See if there's anything left to zoom on.  If so, 
		 * pick an adjacent one for highlighting, else make
		 * sure nothing is left hidden...
		 */
		if(any_lflagged(msgmap, MN_SLCT)){
		    mn_inc_cur(state->mail_stream, msgmap,
			       (in_index == View && THREADING()
				&& state->viewing_a_thread)
				 ? MH_THISTHD
				 : (in_index == View)
				   ? MH_ANYTHD : MH_NONE);
		    if(mn_get_cur(msgmap) == cur)
		      mn_dec_cur(state->mail_stream, msgmap,
			         (in_index == View && THREADING()
				  && state->viewing_a_thread)
				   ? MH_THISTHD
				   : (in_index == View)
				     ? MH_ANYTHD : MH_NONE);
		}
		else			/* clear all hidden flags */
		  (void) unzoom_index(state, state->mail_stream, msgmap);
	    }
	}
	else
	  set_lflag(state->mail_stream, msgmap, cur, MN_SLCT, 1);

	q_status_message2(SM_ORDER, 0, 2, "Message %.200s %.200sselected",
			  long2string(cur), all_selected ? "UN" : "");
    }


    return(!all_selected);
}



/*----------------------------------------------------------------------
 Prompt the user for the command to perform on selected messages

   Args: state -- pointer pine's state variables
	 msgmap -- message collection to operate on
	 q_line -- line on display to write prompts
   Returns: 1 if the selected messages are suitably commanded,
	    0 if the choice to pick the command was declined

  ----*/
int
apply_command(state, stream, msgmap, preloadkeystroke, flags, q_line)
     struct pine *state;
     MAILSTREAM	 *stream;
     MSGNO_S     *msgmap;
     int	  preloadkeystroke;
     int	  flags;
     int	  q_line;
{
    int i = 8,			/* number of static entries in sel_opts3 */
        rv = 1,
	cmd,
        we_cancel = 0,
	agg = (flags & AC_FROM_THREAD) ? 2 : 1;
    char prompt[80];

    if(!preloadkeystroke){
	if(F_ON(F_ENABLE_FLAG,state)){ /* flag? */
	    sel_opts3[i].ch      = '*';
	    sel_opts3[i].rval    = '*';
	    sel_opts3[i].name    = "*";
	    sel_opts3[i++].label = "Flag";
	}

	if(F_ON(F_ENABLE_PIPE,state)){ /* pipe? */
	    sel_opts3[i].ch      = '|';
	    sel_opts3[i].rval    = '|';
	    sel_opts3[i].name    = "|";
	    sel_opts3[i++].label = "Pipe";
	}

	if(F_ON(F_ENABLE_BOUNCE,state)){ /* bounce? */
	    sel_opts3[i].ch      = 'b';
	    sel_opts3[i].rval    = 'b';
	    sel_opts3[i].name    = "B";
	    sel_opts3[i++].label = "Bounce";
	}

	if(flags & AC_FROM_THREAD){
	    if(flags & (AC_COLL | AC_EXPN)){
		sel_opts3[i].ch      = '/';
		sel_opts3[i].rval    = '/';
		sel_opts3[i].name    = "/";
		sel_opts3[i++].label = (flags & AC_COLL) ? "Collapse"
							 : "Expand";
	    }

	    sel_opts3[i].ch      = ';';
	    sel_opts3[i].rval    = ';';
	    sel_opts3[i].name    = ";";
	    if(flags & AC_UNSEL)
	      sel_opts3[i++].label = "UnSelect";
	    else
	      sel_opts3[i++].label = "Select";
	}

	if(F_ON(F_ENABLE_PRYNT, state)){	/* this one is invisible */
	    sel_opts3[i].ch      = 'y';
	    sel_opts3[i].rval    = '%';
	    sel_opts3[i].name    = "";
	    sel_opts3[i++].label = "";
	}

	sel_opts3[i].ch = -1;

	sprintf(prompt, "%.20s command : ",
		(flags & AC_FROM_THREAD) ? "THREAD" : "APPLY");
	cmd = double_radio_buttons(prompt, q_line, sel_opts3, 0, 'x', NO_HELP,
				   RB_SEQ_SENSITIVE);
    }
    else
      cmd = preloadkeystroke;
    
    if(isupper(cmd))
      cmd = tolower(cmd);

    switch(cmd){
      case 'd' :			/* delete */
	we_cancel = busy_alarm(1, NULL, NULL, 0);
	cmd_delete(state, msgmap, agg, MsgIndx);
	if(we_cancel)
	  cancel_busy_alarm(0);
	break;

      case 'u' :			/* undelete */
	we_cancel = busy_alarm(1, NULL, NULL, 0);
	cmd_undelete(state, msgmap, agg);
	if(we_cancel)
	  cancel_busy_alarm(0);
	break;

      case 'r' :			/* reply */
	cmd_reply(state, msgmap, agg);
	break;

      case 'f' :			/* Forward */
	cmd_forward(state, msgmap, agg);
	break;

      case '%' :			/* print */
	cmd_print(state, msgmap, agg, MsgIndx);
	break;

      case 't' :			/* take address */
	cmd_take_addr(state, msgmap, agg);
	break;

      case 's' :			/* save */
	cmd_save(state, stream, msgmap, agg, MsgIndx);
	break;

      case 'e' :			/* export */
	cmd_export(state, msgmap, q_line, agg);
	break;

      case '|' :			/* pipe */
	cmd_pipe(state, msgmap, agg);
	break;

      case '*' :			/* flag */
	we_cancel = busy_alarm(1, NULL, NULL, 0);
	cmd_flag(state, msgmap, agg);
	if(we_cancel)
	  cancel_busy_alarm(0);
	break;

      case 'b' :			/* bounce */
	cmd_bounce(state, msgmap, agg);
	break;

      case '/' :
	collapse_or_expand(state, stream, msgmap,
			   F_ON(F_SLASH_COLL_ENTIRE, ps_global)
			     ? 0L
			     : mn_get_cur(msgmap));
	break;

      case ':' :
	select_thread_stmp(state, stream, msgmap);
	break;

      case 'x' :			/* cancel */
	cmd_cancelled((flags & AC_FROM_THREAD) ? "Thread command"
					       : "Apply command");
	rv = 0;
	break;

      default:
	break;
    }

    return(rv);
}


/*----------------------------------------------------------------------
  ZOOM the message index (set any and all necessary hidden flag bits)

   Args: state -- usual pine state
	 msgmap -- usual message mapping
   Returns: number of messages zoomed in on

  ----*/
long
zoom_index(state, stream, msgmap)
    struct pine *state;
    MAILSTREAM  *stream;
    MSGNO_S	*msgmap;
{
    long        i, count = 0L, first = 0L, msgno;
    PINETHRD_S *thrd = NULL, *topthrd = NULL, *nthrd;

    if(any_lflagged(msgmap, MN_SLCT)){

	if(THREADING() && state->viewing_a_thread){
	    /* get top of current thread */
	    thrd = fetch_thread(stream, mn_m2raw(msgmap, mn_get_cur(msgmap)));
	    if(thrd && thrd->top)
	      topthrd = fetch_thread(stream, thrd->top);
	}

	for(i = 1L; i <= mn_get_total(msgmap); i++){
	    if(!get_lflag(stream, msgmap, i, MN_SLCT)){
		set_lflag(stream, msgmap, i, MN_HIDE, 1);
	    }
	    else{
		/*
		 * If a selected message is hidden beneath a collapsed
		 * thread (not beneath a thread index line, but a collapsed
		 * thread or subthread) then we make it visible. The user
		 * should be able to see the selected messages when they
		 * Zoom. We could get a bit fancier and re-collapse the
		 * thread when the user unzooms, but we don't do that
		 * for now.
		 */
		if(THREADING() && !THRD_INDX()
		   && get_lflag(stream, msgmap, i, MN_CHID)){

		    /*
		     * What we need to do is to unhide this message and
		     * uncollapse any parent above us.
		     * Also, when we uncollapse a parent, we need to
		     * trace back down the tree and unhide until we get
		     * to a collapse point or the end. That's what
		     * set_thread_subtree does.
		     */

		    thrd = fetch_thread(stream, mn_m2raw(msgmap, i));

		    if(thrd && thrd->parent)
		      thrd = fetch_thread(stream, thrd->parent);
		    else
		      thrd = NULL;

		    /* unhide and uncollapse its parents */
		    while(thrd){
			/* if this parent is collapsed */
			if(get_lflag(stream, NULL, thrd->rawno, MN_COLL)){
			    /* uncollapse this parent and unhide its subtree */
			    msgno = mn_raw2m(msgmap, thrd->rawno);
			    if(msgno > 0L && msgno <= mn_get_total(msgmap)){
				set_lflag(stream, msgmap, msgno,
					  MN_COLL | MN_CHID, 0);
				if(thrd->next &&
				   (nthrd = fetch_thread(stream, thrd->next)))
				  set_thread_subtree(stream, nthrd, msgmap,
						     0, MN_CHID);
			    }

			    /* collapse symbol will be wrong */
			    clear_index_cache_ent(msgno);
			}

			/*
			 * Continue up tree to next parent looking for
			 * more collapse points.
			 */
			if(thrd->parent)
			  thrd = fetch_thread(stream, thrd->parent);
			else
			  thrd = NULL;
		    }
		}

		count++;
		if(!first){
		    if(THRD_INDX()){
			/* find msgno of top of thread for msg i */
			if((thrd=fetch_thread(stream, mn_m2raw(msgmap, i)))
			    && thrd->top)
			  first = mn_raw2m(msgmap, thrd->top);
		    }
		    else if(THREADING() && state->viewing_a_thread){
			/* want first selected message in this thread */
			if(topthrd
			   && (thrd=fetch_thread(stream, mn_m2raw(msgmap, i)))
			   && thrd->top
			   && topthrd->rawno == thrd->top)
			  first = i;
		    }
		    else
		      first = i;
		}
	    }
	}

	if(THRD_INDX()){
	    thrd = fetch_thread(stream, mn_m2raw(msgmap, mn_get_cur(msgmap)));
	    if(count_lflags_in_thread(stream, thrd, msgmap, MN_SLCT) == 0)
	      mn_set_cur(msgmap, first);
	}
	else if((THREADING() && state->viewing_a_thread)
	        || !get_lflag(stream, msgmap, mn_get_cur(msgmap), MN_SLCT)){
	    if(!first){
		int flags = 0;

		/*
		 * Nothing was selected in the thread we were in, so
		 * drop back to the Thread Index instead. Set the current
		 * thread to the first one that has a selection in it.
		 */

		unview_thread(state, stream, msgmap);

		i = next_sorted_flagged(F_UNDEL, stream, 1L, &flags);
		
		if(flags & NSF_FLAG_MATCH
		   && (thrd=fetch_thread(stream, mn_m2raw(msgmap, i)))
		    && thrd->top)
		  first = mn_raw2m(msgmap, thrd->top);
		else
		  first = 1L;	/* can't happen */

		mn_set_cur(msgmap, first);
	    }
	    else{
		if(msgline_hidden(stream, msgmap, mn_get_cur(msgmap), 0))
		  mn_set_cur(msgmap, first);
	    }
	}
    }

    return(count);
}



/*----------------------------------------------------------------------
  UnZOOM the message index (clear any and all hidden flag bits)

   Args: state -- usual pine state
	 msgmap -- usual message mapping
   Returns: 1 if hidden bits to clear and they were, 0 if none to clear

  ----*/
int
unzoom_index(state, stream, msgmap)
    struct pine *state;
    MAILSTREAM  *stream;
    MSGNO_S	*msgmap;
{
    register long i;

    if(!any_lflagged(msgmap, MN_HIDE))
      return(0);

    for(i = 1L; i <= mn_get_total(msgmap); i++)
      set_lflag(stream, msgmap, i, MN_HIDE, 0);

    return(1);
}



/*----------------------------------------------------------------------
 Prompt the user for the type of sort he desires

   Args: none
   Returns: 0 if search OK (matching numbers selected by side effect)
            1 if there's a problem

  ----*/
int
select_number(stream, msgmap, msgno)
     MAILSTREAM *stream;
     MSGNO_S    *msgmap;
     long	 msgno;
{
    int r;
    long n1, n2;
    char number1[16], number2[16], numbers[80], *p, *t;
    HelpType help;

    numbers[0] = '\0';
    ps_global->mangled_footer = 1;
    help = NO_HELP;
    while(1){
	int flags = OE_APPEND_CURRENT;

        r = optionally_enter(numbers, -FOOTER_ROWS(ps_global), 0,
			     sizeof(numbers), select_num, NULL, help, &flags);
        if(r == 4)
	  continue;

        if(r == 3){
            help = (help == NO_HELP) ? h_select_by_num : NO_HELP;
	    continue;
	}

	for(t = p = numbers; *p ; p++)	/* strip whitespace */
	  if(!isspace((unsigned char)*p))
	    *t++ = *p;

	*t = '\0';

        if(r == 1 || numbers[0] == '\0'){
	    cmd_cancelled("Selection by number");
	    return(1);
        }
	else
	  break;
    }

    for(n1 = 1; n1 <= stream->nmsgs; n1++)
      mail_elt(stream, n1)->searched = 0;	/* clear searched bits */

    for(p = numbers; *p ; p++){
	t = number1;
	while(*p && isdigit((unsigned char)*p))
	  *t++ = *p++;

	*t = '\0';

	if(number1[0] == '\0'){
	    if(*p == '-')
	      q_status_message1(SM_ORDER | SM_DING, 0, 2,
	   "Invalid message number range, missing number before \"-\": %.200s",
	       numbers);
	    else
	      q_status_message1(SM_ORDER | SM_DING, 0, 2,
			        "Invalid message number: %.200s", numbers);
	    return(1);
	}

	if((n1 = atol(number1)) < 1L || n1 > mn_get_total(msgmap)){
	    q_status_message1(SM_ORDER | SM_DING, 0, 2,
			      "\"%.200s\" out of message number range",
			      long2string(n1));
	    return(1);
	}

	t = number2;
	if(*p == '-'){
	    while(*++p && isdigit((unsigned char)*p))
	      *t++ = *p;

	    *t = '\0';

	    if(number2[0] == '\0'){
		q_status_message1(SM_ORDER | SM_DING, 0, 2,
	     "Invalid message number range, missing number after \"-\": %.200s",
		 numbers);
		return(1);
	    }

	    if((n2 = atol(number2)) < 1L 
	       || n2 > mn_get_total(msgmap)){
		q_status_message1(SM_ORDER | SM_DING, 0, 2,
				  "\"%.200s\" out of message number range",
				  long2string(n2));
		return(1);
	    }

	    if(n2 <= n1){
		char t[20];

		strcpy(t, long2string(n1));
		q_status_message2(SM_ORDER | SM_DING, 0, 2,
			  "Invalid reverse message number range: %.200s-%.200s",
				  t, long2string(n2));
		return(1);
	    }

	    for(;n1 <= n2; n1++)
	      mm_searched(stream, mn_m2raw(msgmap, n1));
	}
	else
	  mm_searched(stream, mn_m2raw(msgmap, n1));

	if(*p == '\0')
	  break;
    }
    
    return(0);
}


int
select_thrd_number(stream, msgmap, msgno)
     MAILSTREAM *stream;
     MSGNO_S    *msgmap;
     long	 msgno;
{
    int r;
    long n1, n2;
    char number1[16], number2[16], numbers[80], *p, *t;
    HelpType help;
    PINETHRD_S   *thrd = NULL;

    numbers[0] = '\0';
    ps_global->mangled_footer = 1;
    help = NO_HELP;
    while(1){
	int flags = OE_APPEND_CURRENT;

        r = optionally_enter(numbers, -FOOTER_ROWS(ps_global), 0,
			     sizeof(numbers), select_num, NULL, help, &flags);
        if(r == 4)
	  continue;

        if(r == 3){
            help = (help == NO_HELP) ? h_select_by_thrdnum : NO_HELP;
	    continue;
	}

	for(t = p = numbers; *p ; p++)	/* strip whitespace */
	  if(!isspace((unsigned char)*p))
	    *t++ = *p;

	*t = '\0';

        if(r == 1 || numbers[0] == '\0'){
	    cmd_cancelled("Selection by number");
	    return(1);
        }
	else
	  break;
    }

    for(n1 = 1; n1 <= stream->nmsgs; n1++)
      mail_elt(stream, n1)->searched = 0;	/* clear searched bits */

    for(p = numbers; *p ; p++){
	t = number1;
	while(*p && isdigit((unsigned char)*p))
	  *t++ = *p++;

	*t = '\0';

	if(number1[0] == '\0'){
	    if(*p == '-')
	      q_status_message1(SM_ORDER | SM_DING, 0, 2,
	       "Invalid number range, missing number before \"-\": %.200s",
	       numbers);
	    else
	      q_status_message1(SM_ORDER | SM_DING, 0, 2,
			        "Invalid thread number: %.200s", numbers);
	    return(1);
	}

	if((n1 = atol(number1)) < 1L || n1 > msgmap->max_thrdno){
	    q_status_message1(SM_ORDER | SM_DING, 0, 2,
			      "\"%.200s\" out of thread number range",
			      long2string(n1));
	    return(1);
	}

	t = number2;
	if(*p == '-'){
	    while(*++p && isdigit((unsigned char)*p))
	      *t++ = *p;

	    *t = '\0';

	    if(number2[0] == '\0'){
		q_status_message1(SM_ORDER | SM_DING, 0, 2,
		 "Invalid number range, missing number after \"-\": %.200s",
		 numbers);
		return(1);
	    }

	    if((n2 = atol(number2)) < 1L 
	       || n2 > mn_get_total(msgmap)){
		q_status_message1(SM_ORDER | SM_DING, 0, 2,
				  "\"%.200s\" out of thread number range",
				  long2string(n2));
		return(1);
	    }

	    if(n2 <= n1){
		char t[20];

		strcpy(t, long2string(n1));
		q_status_message2(SM_ORDER | SM_DING, 0, 2,
			  "Invalid reverse message number range: %.200s-%.200s",
				  t, long2string(n2));
		return(1);
	    }

	    for(;n1 <= n2; n1++){
		thrd = find_thread_by_number(stream, msgmap, n1, thrd);

		if(thrd)
		  set_search_bit_for_thread(stream, thrd);
	    }
	}
	else{
	    thrd = find_thread_by_number(stream, msgmap, n1, NULL);

	    if(thrd)
	      set_search_bit_for_thread(stream, thrd);
	}

	if(*p == '\0')
	  break;
    }
    
    return(0);
}



/*----------------------------------------------------------------------
 Prompt the user for the type of sort he desires

   Args: none
   Returns: 0 if search OK (matching numbers selected by side effect)
            1 if there's a problem

  ----*/
int
select_date(stream, msgmap, msgno)
     MAILSTREAM *stream;
     MSGNO_S    *msgmap;
     long	 msgno;
{
    int	       r, we_cancel = 0, when = 0;
    char       date[100], defdate[100], prompt[128];
    time_t     seldate = time(0);
    struct tm *seldate_tm;
    SEARCHPGM *pgm;
    HelpType   help;
    static struct _tense {
	char *preamble,
	     *range,
	     *scope;
    } tense[] = {
	{"were ", "SENT SINCE",     " (inclusive)"},
	{"were ", "SENT BEFORE",    " (exclusive)"},
	{"were ", "SENT ON",        ""            },
	{"",      "ARRIVED SINCE",  " (inclusive)"},
	{"",      "ARRIVED BEFORE", " (exclusive)"},
	{"",      "ARRIVED ON",     ""            }
    };

    date[0]		      = '\0';
    ps_global->mangled_footer = 1;
    help		      = NO_HELP;

    /*
     * If talking to an old server, default to SINCE instead of
     * SENTSINCE, which was added later.
     */
    if(is_imap_stream(stream) && !modern_imap_stream(stream))
      when = 3;

    while(1){
	int flags = OE_APPEND_CURRENT;

	seldate_tm = localtime(&seldate);
	sprintf(defdate, "%.2d-%.4s-%.4d", seldate_tm->tm_mday,
		month_abbrev(seldate_tm->tm_mon + 1),
		seldate_tm->tm_year + 1900);
	sprintf(prompt,"Select messages which %s%s%s [%s]: ",
		tense[when].preamble, tense[when].range,
		tense[when].scope, defdate);
	r = optionally_enter(date,-FOOTER_ROWS(ps_global), 0, sizeof(date),
			     prompt, sel_date_opt, help, &flags);
	switch (r){
	  case 1 :
	    cmd_cancelled("Selection by date");
	    return(1);

	  case 3 :
	    help = (help == NO_HELP) ? h_select_date : NO_HELP;
	    continue;

	  case 4 :
	    continue;

	  case 11 :
	    {
		MESSAGECACHE *mc;

		if(stream && (mc = mail_elt(stream, mn_m2raw(msgmap, msgno)))){

		    /* cache not filled in yet? */
		    if(mc->day == 0){
			char seq[20];

			if(stream->dtb->flags & DR_NEWS){
			    strncpy(seq,
				    long2string(mail_uid(stream,
							 mn_m2raw(msgmap,
								  msgno))),
				    sizeof(seq));
			    seq[sizeof(seq)-1] = '\0';
			    mail_fetch_overview(stream, seq, NULL);
			}
			else{
			    strncpy(seq, long2string(mn_m2raw(msgmap, msgno)),
				    sizeof(seq));
			    seq[sizeof(seq)-1] = '\0';
			    mail_fetch_fast(stream, seq, 0L);
			}
		    }

		    /* mail_date returns fixed field width date */
		    mail_date(date, mc);
		    date[11] = '\0';
		}
	    }

	    continue;

	  case 12 :			/* set default to PREVIOUS day */
	    seldate -= 86400;
	    continue;

	  case 13 :			/* set default to NEXT day */
	    seldate += 86400;
	    continue;

	  case 14 :
	    when = (when+1) % (sizeof(tense) / sizeof(struct _tense));
	    continue;

	  default:
	    break;
	}

	removing_leading_white_space(date);
	removing_trailing_white_space(date);
	if(!*date){
	    strncpy(date, defdate, sizeof(date));
	    date[sizeof(date)-1] = '\0';
	}

	break;
    }

    we_cancel = busy_alarm(1, "Busy Selecting", NULL, 0);

    if((pgm = mail_newsearchpgm()) != NULL){
	MESSAGECACHE elt;
	int          converted_date;

	if(mail_parse_date(&elt, date))
	  converted_date = mail_shortdate(elt.year, elt.month, elt.day);

	switch(when){
	  case 0:
	    pgm->sentsince = converted_date;
	    break;
	  case 1:
	    pgm->sentbefore = converted_date;
	    break;
	  case 2:
	    pgm->senton = converted_date;
	    break;
	  case 3:
	    pgm->since = converted_date;
	    break;
	  case 4:
	    pgm->before = converted_date;
	    break;
	  case 5:
	    pgm->on = converted_date;
	    break;
	}

	mail_search_full(stream, NULL, pgm, SE_NOPREFETCH | SE_FREE);
    }

    if(we_cancel)
      cancel_busy_alarm(0);

    return(0);
}



/*----------------------------------------------------------------------
 Prompt the user for the type of sort he desires

   Args: none
   Returns: 0 if search OK (matching numbers selected by side effect)
            1 if there's a problem

  ----*/
int
select_text(stream, msgmap, msgno)
     MAILSTREAM *stream;
     MSGNO_S    *msgmap;
     long	 msgno;
{
    int          r, type, we_cancel = 0, not = 0, flags, old_imap;
    char         sstring[80], savedsstring[80], origcharset[16], tmp[128];
    char        *sval = NULL, *cset = NULL, *charset = NULL;
    char         buftmp[MAILTMPLEN];
    ESCKEY_S     ekey[4];
    ENVELOPE    *env = NULL;
    HelpType     help;
    static char *recip = "RECIPIENTS";
    static char *partic = "PARTICIPANTS";
    long         searchflags;
    SEARCHPGM   *srchpgm, *pgm, *secondpgm = NULL, *thirdpgm = NULL;

    ps_global->mangled_footer = 1;
    origcharset[0] = '\0';
    savedsstring[0] = '\0';
    ekey[0].ch = ekey[1].ch = ekey[2].ch = ekey[3].ch = -1;

    while(1){
	type = radio_buttons(not ? sel_not_text : sel_text,
			     -FOOTER_ROWS(ps_global), sel_text_opt,
			     's', 'x', NO_HELP, RB_NORM);
	
	if(type == '!')
	  not = !not;
	else
	  break;
    }

    /*
     * prepare some friendly defaults...
     */
    switch(type){
      case 't' :			/* address fields, offer To or From */
      case 'f' :
      case 'c' :
      case 'r' :
      case 'p' :
	sval          = (type == 't') ? "TO" :
			  (type == 'f') ? "FROM" :
			    (type == 'c') ? "CC" :
			      (type == 'r') ? recip : partic;
	ekey[0].ch    = ctrl('T');
	ekey[0].name  = "^T";
	ekey[0].rval  = 10;
	ekey[0].label = "Cur To";
	ekey[1].ch    = ctrl('R');
	ekey[1].name  = "^R";
	ekey[1].rval  = 11;
	ekey[1].label = "Cur From";
	ekey[2].ch    = ctrl('W');
	ekey[2].name  = "^W";
	ekey[2].rval  = 12;
	ekey[2].label = "Cur Cc";
	break;

      case 's' :
	sval          = "SUBJECT";
	ekey[0].ch    = ctrl('X');
	ekey[0].name  = "^X";
	ekey[0].rval  = 13;
	ekey[0].label = "Cur Subject";
	break;

      case 'a' :
	sval = "TEXT";
	break;

      case 'b' :
	sval = "BODYTEXT";
	break;

      case 'x':
	break;

      default:
	dprint(1, (debugfile,"\n - BOTCH: select_text unrecognized option\n"));
	return(1);
    }

    if(type != 'x'){
	if(ekey[0].ch > -1 && msgno > 0L
	   && !(env=mail_fetchstructure(stream,mn_m2raw(msgmap,msgno),NULL)))
	  ekey[0].ch = -1;

	sstring[0] = '\0';
	help = NO_HELP;
	r = type;
	while(r != 'x'){
	    sprintf(tmp, "String in message %s to %smatch : ", sval,
		    not ? "NOT " : "");
	    flags = OE_APPEND_CURRENT | OE_KEEP_TRAILING_SPACE;
	    r = optionally_enter(sstring, -FOOTER_ROWS(ps_global), 0,
				 sizeof(sstring), tmp, ekey, help, &flags);

	    switch(r){
	      case 3 :
		help = (help == NO_HELP)
			? (not
			    ? ((type == 'f') ? h_select_txt_not_from
			      : (type == 't') ? h_select_txt_not_to
			       : (type == 'c') ? h_select_txt_not_cc
				: (type == 's') ? h_select_txt_not_subj
				 : (type == 'a') ? h_select_txt_not_all
				  : (type == 'r') ? h_select_txt_not_recip
				   : (type == 'p') ? h_select_txt_not_partic
				    : (type == 'b') ? h_select_txt_not_body
				     :                 NO_HELP)
			    : ((type == 'f') ? h_select_txt_from
			      : (type == 't') ? h_select_txt_to
			       : (type == 'c') ? h_select_txt_cc
				: (type == 's') ? h_select_txt_subj
				 : (type == 'a') ? h_select_txt_all
				  : (type == 'r') ? h_select_txt_recip
				   : (type == 'p') ? h_select_txt_partic
				    : (type == 'b') ? h_select_txt_body
				     :                 NO_HELP))
			: NO_HELP;

	      case 4 :
		continue;

	      case 10 :			/* To: default */
		if(env && env->to && env->to->mailbox)
		  sprintf(sstring, "%.30s%s%.40s", env->to->mailbox,
			  env->to->host ? "@" : "",
			  env->to->host ? env->to->host : "");
		continue;

	      case 11 :			/* From: default */
		if(env && env->from && env->from->mailbox)
		  sprintf(sstring, "%.30s%s%.40s", env->from->mailbox,
			  env->from->host ? "@" : "",
			  env->from->host ? env->from->host : "");
		continue;

	      case 12 :			/* Cc: default */
		if(env && env->cc && env->cc->mailbox)
		  sprintf(sstring, "%.30s%s%.40s", env->cc->mailbox,
			  env->cc->host ? "@" : "",
			  env->cc->host ? env->cc->host : "");
		continue;

	      case 13 :			/* Subject: default */
		if(env && env->subject && env->subject[0]){
		    char *q = NULL;
		    if(cset)
		      fs_give((void **) &cset);

		    sprintf(buftmp, "%.75s", env->subject);
		    q = (char *)rfc1522_decode((unsigned char *)tmp_20k_buf,
					       SIZEOF_20KBUF, buftmp, &cset);
		    /*
		     * If decoding was done, and the charset of the decoded
		     * subject is different from ours (cset != NULL) then
		     * we save that charset information for the search.
		     */
		    if(q != env->subject && cset && cset[0]){
			charset = cset;
			sprintf(savedsstring, "%.70s", q);
		    }

		    sprintf(sstring, "%.70s", q);
		}

		continue;

	      default :
		break;
	    }

	    if(r == 1 || sstring[0] == '\0')
	      r = 'x';

	    break;
	}
    }

    if(type == 'x' || r == 'x'){
	cmd_cancelled("Selection by text");
	return(1);
    }

    old_imap = (is_imap_stream(stream) && !modern_imap_stream(stream));

    /* create a search program and fill it in */
    srchpgm = pgm = mail_newsearchpgm();
    if(not && !old_imap){
	srchpgm->not = mail_newsearchpgmlist();
	srchpgm->not->pgm = mail_newsearchpgm();
	pgm = srchpgm->not->pgm;
    }

    switch(type){
      case 'r' :				/* TO or CC */
	if(old_imap){
	    /* No OR on old servers */
	    pgm->to = mail_newstringlist();
	    pgm->to->text.data = (unsigned char *) cpystr(sstring);
	    pgm->to->text.size = strlen(sstring);
	    secondpgm = mail_newsearchpgm();
	    secondpgm->cc = mail_newstringlist();
	    secondpgm->cc->text.data = (unsigned char *) cpystr(sstring);
	    secondpgm->cc->text.size = strlen(sstring);
	}
	else{
	    pgm->or = mail_newsearchor();
	    pgm->or->first->to = mail_newstringlist();
	    pgm->or->first->to->text.data = (unsigned char *) cpystr(sstring);
	    pgm->or->first->to->text.size = strlen(sstring);
	    pgm->or->second->cc = mail_newstringlist();
	    pgm->or->second->cc->text.data = (unsigned char *) cpystr(sstring);
	    pgm->or->second->cc->text.size = strlen(sstring);
	}

	break;

      case 'p' :				/* TO or CC or FROM */
	if(old_imap){
	    /* No OR on old servers */
	    pgm->to = mail_newstringlist();
	    pgm->to->text.data = (unsigned char *) cpystr(sstring);
	    pgm->to->text.size = strlen(sstring);
	    secondpgm = mail_newsearchpgm();
	    secondpgm->cc = mail_newstringlist();
	    secondpgm->cc->text.data = (unsigned char *) cpystr(sstring);
	    secondpgm->cc->text.size = strlen(sstring);
	    thirdpgm = mail_newsearchpgm();
	    thirdpgm->from = mail_newstringlist();
	    thirdpgm->from->text.data = (unsigned char *) cpystr(sstring);
	    thirdpgm->from->text.size = strlen(sstring);
	}
	else{
	    pgm->or = mail_newsearchor();
	    pgm->or->first->to = mail_newstringlist();
	    pgm->or->first->to->text.data = (unsigned char *) cpystr(sstring);
	    pgm->or->first->to->text.size = strlen(sstring);

	    pgm->or->second->or = mail_newsearchor();
	    pgm->or->second->or->first->cc = mail_newstringlist();
	    pgm->or->second->or->first->cc->text.data =
					    (unsigned char *) cpystr(sstring);
	    pgm->or->second->or->first->cc->text.size = strlen(sstring);
	    pgm->or->second->or->second->from = mail_newstringlist();
	    pgm->or->second->or->second->from->text.data =
					    (unsigned char *) cpystr(sstring);
	    pgm->or->second->or->second->from->text.size = strlen(sstring);
	}

	break;

      case 'f' :				/* FROM */
	pgm->from = mail_newstringlist();
	pgm->from->text.data = (unsigned char *) cpystr(sstring);
	pgm->from->text.size = strlen(sstring);
	break;

      case 'c' :				/* CC */
	pgm->cc = mail_newstringlist();
	pgm->cc->text.data = (unsigned char *) cpystr(sstring);
	pgm->cc->text.size = strlen(sstring);
	break;

      case 't' :				/* TO */
	pgm->to = mail_newstringlist();
	pgm->to->text.data = (unsigned char *) cpystr(sstring);
	pgm->to->text.size = strlen(sstring);
	break;

      case 's' :				/* SUBJECT */
	pgm->subject = mail_newstringlist();
	pgm->subject->text.data = (unsigned char *) cpystr(sstring);
	pgm->subject->text.size = strlen(sstring);
	break;

      case 'a' :				/* ALL TEXT */
	pgm->text = mail_newstringlist();
	pgm->text->text.data = (unsigned char *) cpystr(sstring);
	pgm->text->text.size = strlen(sstring);
	break;

      case 'b' :				/* ALL BODY TEXT */
	pgm->body = mail_newstringlist();
	pgm->body->text.data = (unsigned char *) cpystr(sstring);
	pgm->body->text.size = strlen(sstring);
	break;

      default :
	dprint(1, (debugfile,"\n - BOTCH: select_text unrecognized type\n"));
	return(1);
    }

    /*
     * If the user gets the current subject with the ^X command, and
     * that subject has a different charset than what the user uses, and
     * what is left after editing by the user is still a substring of
     * the original subject, and it still has non-ascii characters in it;
     * then use that charset from the original subject in the search.
     */
    if(charset && strstr(savedsstring, sstring) == NULL){
	strncpy(origcharset, charset, sizeof(origcharset));
	origcharset[sizeof(origcharset)-1] = '\0';
	charset = NULL;
    }

    /* set the charset */
    if(!charset){
	for(sval = sstring; *sval && isascii(*sval); sval++)
	  ;

	/* if it's ascii, don't warn user about charset change */
	if(!*sval)
	  origcharset[0] = '\0';

	/* if it isn't ascii, use user's charset */
	charset = (*sval &&
		   ps_global->VAR_CHAR_SET &&
		   ps_global->VAR_CHAR_SET[0])
		     ? ps_global->VAR_CHAR_SET
		     : "US-ASCII";
    }

    if(*origcharset)
      q_status_message2(SM_ORDER, 5, 5,
	    "Warning: character set used for search changed (%.200s -> %.200s)",
		    origcharset, charset);

    /*
     * If we happen to have any messages excluded, make sure we
     * don't waste time searching their text...
     */
    srchpgm->msgno = visible_searchset(stream, msgmap);

    we_cancel = busy_alarm(1, "Busy Selecting", NULL, 0);

    searchflags = (SE_NOPREFETCH | SE_FREE);

    mail_search_full(stream, !old_imap ? charset : NULL, srchpgm, searchflags);
    
    /* search for To or Cc; or To or Cc or From on old imap server */
    if(secondpgm){
	searchflags |= SE_RETAIN;
	mail_search_full(stream, NULL, secondpgm, searchflags);

	if(thirdpgm)
	  mail_search_full(stream, NULL, thirdpgm, searchflags);
    }

    if(old_imap && not){
	MESSAGECACHE *mc;

	/* 
	 * Old imap server doesn't have a NOT, so we actually searched for
	 * the subject (or whatever) instead of !subject. Flip the searched
	 * bits.
	 */
	for(msgno = 1L; msgno <= mn_get_total(msgmap); msgno++)
	    if((mc=mail_elt(stream, msgno))->searched)
	      mc->searched = NIL;
	    else
	      mc->searched = T;
    }

    if(we_cancel)
      cancel_busy_alarm(0);

    if(cset)
      fs_give((void **)&cset);

    return(0);
}


int
select_size(stream, msgmap, msgno)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    long	 msgno;
{
    int        r, large = 1;
    unsigned long n, mult = 1L, numerator = 0L, divisor = 1L;
    char       size[16], numbers[80], *p, *t;
    HelpType   help;
    SEARCHPGM *pgm;
    long       flags = (SE_NOPREFETCH | SE_FREE);

    numbers[0] = '\0';
    ps_global->mangled_footer = 1;

    help = NO_HELP;
    while(1){
	int flgs = OE_APPEND_CURRENT;

	sel_size_opt[1].label = large ? sel_size_smaller : sel_size_larger;

        r = optionally_enter(numbers, -FOOTER_ROWS(ps_global), 0,
			     sizeof(numbers), large ? select_size_larger_msg
						    : select_size_smaller_msg,
			     sel_size_opt, help, &flgs);
        if(r == 4)
	  continue;

        if(r == 14){
	    large = 1 - large;
	    continue;
	}

        if(r == 3){
            help = (help == NO_HELP) ? (large ? h_select_by_larger_size
					      : h_select_by_smaller_size)
				     : NO_HELP;
	    continue;
	}

	for(t = p = numbers; *p ; p++)	/* strip whitespace */
	  if(!isspace((unsigned char)*p))
	    *t++ = *p;

	*t = '\0';

        if(r == 1 || numbers[0] == '\0'){
	    cmd_cancelled("Selection by size");
	    return(1);
        }
	else
	  break;
    }

    if(numbers[0] == '-'){
	q_status_message1(SM_ORDER | SM_DING, 0, 2,
			  "Invalid size entered: %.200s", numbers);
	return(1);
    }

    t = size;
    p = numbers;

    while(*p && isdigit((unsigned char)*p))
      *t++ = *p++;

    *t = '\0';

    if(size[0] == '\0' && *p == '.' && isdigit(*(p+1))){
	size[0] = '0';
	size[1] = '\0';
    }

    if(size[0] == '\0'){
	q_status_message1(SM_ORDER | SM_DING, 0, 2,
			  "Invalid size entered: %.200s", numbers);
	return(1);
    }

    n = strtoul(size, (char **)NULL, 10); 

    size[0] = '\0';
    if(*p == '.'){
	/*
	 * We probably ought to just use atof() to convert 1.1 into a
	 * double, but since we haven't used atof() anywhere else I'm
	 * reluctant to use it because of portability concerns.
	 */
	p++;
	t = size;
	while(*p && isdigit((unsigned char)*p)){
	    *t++ = *p++;
	    divisor *= 10;
	}

	*t = '\0';

	if(size[0])
	  numerator = strtoul(size, (char **)NULL, 10); 
    }

    switch(*p){
      case 'g':
      case 'G':
        mult *= 1000;
	/* fall through */

      case 'm':
      case 'M':
        mult *= 1000;
	/* fall through */

      case 'k':
      case 'K':
        mult *= 1000;
	break;
    }

    n = n * mult + (numerator * mult) / divisor;

    pgm = mail_newsearchpgm();
    if(large)
	pgm->larger = n;
    else
	pgm->smaller = n;

    if(is_imap_stream(stream) && !modern_imap_stream(stream))
      flags |= SO_NOSERVER;

    mail_search_full(stream, NULL, pgm, SE_NOPREFETCH | SE_FREE);

    return(0);
}


/*
 * visible_searchset -- return c-client search set unEXLDed
 *			sequence numbers
 */
SEARCHSET *
visible_searchset(stream, msgmap)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
{
    long       n, run;
    SEARCHSET *full_set = NULL, **set;

    /*
     * If we're talking to anything other than a server older than
     * imap 4rev1, build a searchset otherwise it'll choke.
     */
    if(!(is_imap_stream(stream) && !modern_imap_stream(stream))){
	if(any_lflagged(msgmap, MN_EXLD)){
	    for(n = 1L, set = &full_set, run = 0L; n <= stream->nmsgs; n++)
	      if(get_lflag(stream, NULL, n, MN_EXLD)){
		  if(run){		/* previous NOT excluded? */
		      if(run > 1L)
			(*set)->last = n - 1L;

		      set = &(*set)->next;
		      run = 0L;
		  }
	      }
	      else if(run++){		/* next in run */
		  (*set)->last = n;
	      }
	      else{				/* start of run */
		  *set = mail_newsearchset();
		  (*set)->first = n;
	      }
	}
	else{
	    full_set = mail_newsearchset();
	    full_set->first = 1L;
	    full_set->last  = stream->nmsgs;
	}
    }

    return(full_set);
}


/*----------------------------------------------------------------------
 Prompt the user for the type of sort he desires

   Args: none
   Returns: 0 if search OK (matching numbers selected by side effect)
            1 if there's a problem

  ----*/
int
select_flagged(stream, msgmap, msgno)
     MAILSTREAM *stream;
     MSGNO_S    *msgmap;
     long	 msgno;
{
    int	       s, not = 0, we_cancel = 0;
    SEARCHPGM *pgm;

    while(1){
	s = radio_buttons((not) ? sel_flag_not : sel_flag,
			  -FOOTER_ROWS(ps_global), sel_flag_opt, '*', 'x',
			  NO_HELP, RB_NORM);
			  
	if(s == 'x'){
	    cmd_cancelled("Selection by status");
	    return(1);
	}
	else if(s == '!')
	  not = !not;
	else
	  break;
    }

    pgm = mail_newsearchpgm();
    switch(s){
      case 'n' :
	if(not){
	    SEARCHPGM *notpgm;

	    /* this is the same as seen or deleted or answered */
	    pgm->not = mail_newsearchpgmlist();
	    notpgm = pgm->not->pgm = mail_newsearchpgm();
	    notpgm->unseen = notpgm->undeleted = notpgm->unanswered = 1;
	}
	else
	  pgm->unseen = pgm->undeleted = pgm->unanswered = 1;
	  
	break;

      case 'd' :
	if(not)
	  pgm->undeleted = 1;
	else
	  pgm->deleted = 1;

	break;

      case 'a':
	/*
	 * Not a true "not", we are implicitly only interested in undeleted.
	 */
	if(not)
	  pgm->unanswered = pgm->undeleted = 1;
	else
	  pgm->answered = pgm->undeleted = 1;
	break;

      default :
	if(not)
	  pgm->unflagged = 1;
	else
	  pgm->flagged = 1;
	  
	break;
    }

    we_cancel = busy_alarm(1, "Busy Selecting", NULL, 0);
    mail_search_full(stream, NULL, pgm, SE_NOPREFETCH | SE_FREE);
    if(we_cancel)
      cancel_busy_alarm(0);

    return(0);
}



/*----------------------------------------------------------------------
   Prompt the user for the type of sort he desires

Args: state -- pine state pointer
      q1 -- Line to prompt on

      Returns 0 if it was cancelled, 1 otherwise.
  ----*/
int
select_sort(state, ql, sort, rev)
     struct pine *state;
     int	  ql;
     SortOrder	 *sort;
     int	 *rev;
{
    char      prompt[200], tmp[3], *p;
    int       s, i;
    int       deefault = 'a', retval = 1;
    HelpType  help;
    ESCKEY_S  sorts[14];

#ifdef _WINDOWS
    DLG_SORTPARAM	sortsel;

    if (mswin_usedialog ()) {

	sortsel.reverse = mn_get_revsort (state->msgmap);
	sortsel.cursort = mn_get_sort (state->msgmap);
	sortsel.helptext = get_help_text (h_select_sort);
	sortsel.rval = 0;

	if ((retval = os_sortdialog (&sortsel))) {
	    *sort = sortsel.cursort;
	    *rev  = sortsel.reverse;
        }

	free_list_array(&sortsel.helptext);

	return (retval);
    }
#endif

    /*----- String together the prompt ------*/
    tmp[1] = '\0';
    strcpy(prompt, "Choose type of sort, or 'R' to reverse current sort : ");
    for(i = 0; state->sort_types[i] != EndofList; i++) {
	sorts[i].rval	   = i;
	p = sorts[i].label = sort_name(state->sort_types[i]);
	while(*(p+1) && islower((unsigned char)*p))
	  p++;

	sorts[i].ch   = tolower((unsigned char)(tmp[0] = *p));
	sorts[i].name = cpystr(tmp);

        if(mn_get_sort(state->msgmap) == state->sort_types[i])
	  deefault = sorts[i].rval;
    }

    sorts[i].ch     = 'r';
    sorts[i].rval   = 'r';
    sorts[i].name   = cpystr("R");
    sorts[i].label  = "";
    sorts[++i].ch   = -1;
    help = h_select_sort;

    if((s = radio_buttons(prompt,ql,sorts,deefault,'x',help,RB_NORM)) != 'x'){
	state->mangled_body = 1;		/* signal screen's changed */
	if(s == 'r')
	  *rev = !mn_get_revsort(state->msgmap);
	else
	  *sort = state->sort_types[s];
    }
    else{
	retval = 0;
	cmd_cancelled("Sort");
    }

    while(--i >= 0)
      fs_give((void **)&sorts[i].name);

    blank_keymenu(ps_global->ttyo->screen_rows - 2, 0);
    return(retval);
}


/*---------------------------------------------------------------------
  Build list of folders in the given context for user selection

  Args: c -- pointer to pointer to folder's context context 
	f -- folder prefix to display
	sublist -- whether or not to use 'f's contents as prefix
	lister -- function used to do the actual display

  Returns: malloc'd string containing sequence, else NULL if
	   no messages in msgmap with local "selected" flag.
  ----*/
int
display_folder_list(c, f, sublist, lister)
    CONTEXT_S **c;
    char       *f;
    int	        sublist;
    int	      (*lister) PROTO((struct pine *, CONTEXT_S **, char *, int));
{
    int	       rc;
    CONTEXT_S *tc;
    void (*redraw)() = ps_global->redrawer;

    push_titlebar_state();
    tc = *c;
    if(rc = (*lister)(ps_global, &tc, f, sublist))
      *c = tc;

    ClearScreen();
    pop_titlebar_state();
    redraw_titlebar();
    if(ps_global->redrawer = redraw) /* reset old value, and test */
      (*ps_global->redrawer)();

    if(rc == 1 && F_ON(F_SELECT_WO_CONFIRM, ps_global))
      return(1);

    return(0);
}



/*----------------------------------------------------------------------
  Build comma delimited list of selected messages

  Args: stream -- mail stream to use for flag testing
	msgmap -- message number struct of to build selected messages in
	count -- pointer to place to write number of comma delimited

  Returns: malloc'd string containing sequence, else NULL if
	   no messages in msgmap with local "selected" flag.
  ----*/
char *
selected_sequence(stream, msgmap, count)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    long       *count;
{
    long  i;

    /*
     * The plan here is to use the c-client elt's "sequence" bit
     * to work around any orderings or exclusions in pine's internal
     * mapping that might cause the sequence to be artificially
     * lengthy.  It's probably cheaper to run down the elt list
     * twice rather than call nm_raw2m() for each message as
     * we run down the elt list once...
     */
    for(i = 1L; i <= stream->nmsgs; i++)
      mail_elt(stream, i)->sequence = 0;

    for(i = 1L; i <= mn_get_total(msgmap); i++)
      if(get_lflag(stream, msgmap, i, MN_SLCT)){
	  long rawno;
	  int  exbits = 0;

	  /*
	   * Forget we knew about it, and set "add to sequence"
	   * bit...
	   */
	  clear_index_cache_ent(i);
	  mail_elt(stream, (rawno=mn_m2raw(msgmap, i)))->sequence = 1;

	  /*
	   * Mark this message manually flagged so we don't re-filter it
	   * with a filter which only sets flags.
	   */
	  if(msgno_exceptions(stream, rawno, "0", &exbits, FALSE))
	    exbits |= MSG_EX_MANFLAGGED;
	  else
	    exbits = MSG_EX_MANFLAGGED;

	  msgno_exceptions(stream, rawno, "0", &exbits, TRUE);
      }

    return(build_sequence(stream, NULL, count));
}


/*----------------------------------------------------------------------
  Build comma delimited list of current, flagged messages

  Args: stream -- mail stream to use for flag testing
	msgmap -- message number struct of to build selected messages in
	flag -- system flags to match against
	count -- pointer to place to return number of comma delimited
	mark -- mark index cache entry changed, and count state change

  Returns: malloc'd string containing sequence, else NULL if
	   no messages in msgmap with local "selected" flag (a flag
	   of zero means all current msgs).
  ----*/
char *
currentf_sequence(stream, msgmap, flag, count, mark)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    long	flag;
    long       *count;
    int		mark;
{
    char	 *seq;
    long	  i, rawno;
    int           exbits;
    MESSAGECACHE *mc;

    /* First, make sure elts are valid for all the interesting messages */
    if(seq = invalid_elt_sequence(stream, msgmap)){
	mail_fetchflags(stream, seq);
	fs_give((void **) &seq);
    }

    for(i = 1L; i <= stream->nmsgs; i++)
      mail_elt(stream, i)->sequence = 0;	/* clear "sequence" bits */

    for(i = mn_first_cur(msgmap); i > 0L; i = mn_next_cur(msgmap)){
	/* if not already set, go on... */
	mc = mail_elt(stream, (rawno=mn_m2raw(msgmap, i)));
	if((flag == 0)
	   || ((flag & F_DEL) && mc->deleted)
	   || ((flag & F_UNDEL) && !mc->deleted)
	   || ((flag & F_SEEN) && mc->seen)
	   || ((flag & F_UNSEEN) && !mc->seen)
	   || ((flag & F_ANS) && mc->answered)
	   || ((flag & F_UNANS) && !mc->answered)
	   || ((flag & F_FLAG) && mc->flagged)
	   || ((flag & F_UNFLAG) && !mc->flagged)){

	    mc->sequence = 1;			/* set "sequence" flag */
	    if(mark){
		if(THRD_INDX()){
		    PINETHRD_S *thrd;
		    long        t;

		    /* clear thread index line instead of index index line */
		    thrd = fetch_thread(stream, mn_m2raw(msgmap, i));
		    if(thrd && thrd->top
		       && (thrd=fetch_thread(stream,thrd->top))
		       && (t = mn_raw2m(msgmap, thrd->rawno)))
		      clear_index_cache_ent(t);
		}
		else
		  clear_index_cache_ent(i);	/* force new index line */

		check_point_change();		/* count state change */

		/*
		 * Mark this message manually flagged so we don't re-filter it
		 * with a filter which only sets flags.
		 */
		exbits = 0;
		if(msgno_exceptions(stream, rawno, "0", &exbits, FALSE))
		  exbits |= MSG_EX_MANFLAGGED;
		else
		  exbits = MSG_EX_MANFLAGGED;

		msgno_exceptions(stream, rawno, "0", &exbits, TRUE);
	    }
	}
    }

    return(build_sequence(stream, NULL, count));
}


/*----------------------------------------------------------------------
  Return sequence numbers of messages with invalid MESSAGECACHEs

  Args: stream -- mail stream to use for flag testing
	msgmap -- message number struct of to build selected messages in

  Returns: malloc'd string containing sequence, else NULL if
	   no messages in msgmap with local "selected" flag (a flag
	   of zero means all current msgs).
  ----*/
char *
invalid_elt_sequence(stream, msgmap)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
{
    long	  i;
    MESSAGECACHE *mc;

    for(i = 1L; i <= stream->nmsgs; i++)
      mail_elt(stream, i)->sequence = 0;	/* clear "sequence" bits */

    for(i = mn_first_cur(msgmap); i > 0L; i = mn_next_cur(msgmap))
      if(!(mc = mail_elt(stream, mn_m2raw(msgmap, i)))->valid)
	mc->sequence = 1;

    return(build_sequence(stream, NULL, NULL));
}


/*----------------------------------------------------------------------
  Build comma delimited list of messages with elt "sequence" bit set

  Args: stream -- mail stream to use for flag testing
	msgmap -- struct containing sort to build sequence in
	count -- pointer to place to write number of comma delimited
		 NOTE: if non-zero, it's a clue as to how many messages
		       have the sequence bit lit.

  Returns: malloc'd string containing sequence, else NULL if
	   no messages in msgmap with elt's "sequence" bit set
  ----*/
char *
build_sequence(stream, msgmap, count)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    long       *count;
{
#define	SEQ_INCREMENT	128
    long    n = 0L, i, x, lastn = 0L, runstart = 0L;
    size_t  size = SEQ_INCREMENT;
    char   *seq = NULL, *p;

    if(count){
	if(*count > 0L)
	  size = max(size, min((*count) * 4, 16384));

	*count = 0L;
    }

    for(x = 1L; x <= stream->nmsgs; x++){
	if(msgmap){
	    if((i = mn_m2raw(msgmap, x)) == 0L)
	      continue;
	}
	else
	  i = x;

	if(mail_elt(stream, i)->sequence){
	    n++;
	    if(!seq)				/* initialize if needed */
	      seq = p = fs_get(size);

	    /*
	     * This code will coalesce the ascending runs of
	     * sequence numbers, but fails to break sequences
	     * into a reasonably sensible length for imapd's to
	     * swallow (reasonable addtition to c-client?)...
	     */
	    if(lastn){				/* if may be in a run */
		if(lastn + 1L == i){		/* and its the next raw num */
		    lastn = i;			/* skip writing anything... */
		    continue;
		}
		else if(runstart != lastn){
		    *p++ = (runstart + 1L == lastn) ? ',' : ':';
		    sstrcpy(&p, long2string(lastn));
		}				/* wrote end of run */
	    }

	    runstart = lastn = i;		/* remember last raw num */

	    if(n > 1L)				/* !first num, write delim */
	      *p++ = ',';

	    if(size - (p - seq) < 16){	/* room for two more nums? */
		size_t offset = p - seq;	/* grow the sequence array */
		size += SEQ_INCREMENT;
		fs_resize((void **)&seq, size);
		p = seq + offset;
	    }

	    sstrcpy(&p, long2string(i));	/* write raw number */
	}
    }

    if(lastn && runstart != lastn){		/* were in a run? */
	*p++ = (runstart + 1L == lastn) ? ',' : ':';
	sstrcpy(&p, long2string(lastn));	/* write the trailing num */
    }

    if(seq)					/* if sequence, tie it off */
      *p  = '\0';

    if(count)
      *count = n;

    return(seq);
}



/*----------------------------------------------------------------------
  If any messages flagged "selected", fake the "currently selected" array

  Args: map -- message number struct of to build selected messages in

  OK folks, here's the tradeoff: either all the functions have to
  know if the user want's to deal with the "current" hilited message
  or the list of currently "selected" messages, *or* we just
  wrap the call to these functions with some glue that tweeks
  what these functions see as the "current" message list, and let them
  do their thing.
  ----*/
int
pseudo_selected(map)
    MSGNO_S *map;
{
    long i, later = 0L;

    if(any_lflagged(map, MN_SLCT)){
	map->hilited = mn_m2raw(map, mn_get_cur(map));

	for(i = 1L; i <= mn_get_total(map); i++)
	  /* BUG: using the global mail_stream is kind of bogus since
	   * everybody that calls us get's a pine stuct passed it.
	   * perhaps a stream pointer in the message struct makes 
	   * sense?
	   */
	  if(get_lflag(ps_global->mail_stream, map, i, MN_SLCT)){
	      if(!later++){
		  mn_set_cur(map, i);
	      }
	      else{
		  mn_add_cur(map, i);
	      }
	  }

	return(1);
    }

    return(0);
}


/*----------------------------------------------------------------------
  Antidote for the monkey business committed above

  Args: map -- message number struct of to build selected messages in

  ----*/
void
restore_selected(map)
    MSGNO_S *map;
{
    if(map->hilited){
	mn_reset_cur(map, mn_raw2m(map, map->hilited));
	map->hilited = 0L;
    }
}


/*
 * Get the user name from the mailbox portion of an address.
 *
 * Args: mailbox -- the mailbox portion of an address (lhs of address)
 *       target  -- a buffer to put the result in
 *       len     -- length of the target buffer
 *
 * Returns the left most portion up to the first '%', ':' or '@',
 * and to the right of any '!' (as if c-client would give us such a mailbox).
 * Returns NULL if it can't find a username to point to.
 */
char *
get_uname(mailbox, target, len)
    char  *mailbox,
	  *target;
    int    len;
{
    int i, start, end;

    if(!mailbox || !*mailbox)
      return(NULL);

    end = strlen(mailbox) - 1;
    for(start = end; start > -1 && mailbox[start] != '!'; start--)
        if(strindex("%:@", mailbox[start]))
	    end = start - 1;

    start++;			/* compensate for either case above */

    for(i = start; i <= end && (i-start) < (len-1); i++) /* copy name */
      target[i-start] = isupper((unsigned char)mailbox[i])
					  ? tolower((unsigned char)mailbox[i])
					  : mailbox[i];

    target[i-start] = '\0';	/* tie it off */

    return(*target ? target : NULL);
}


/*
 * file_lister - call pico library's file lister
 */
int
file_lister(title, path, pathlen, file, filelen, newmail, flags)
    char *title, *path, *file;
    int   pathlen, filelen, newmail, flags;
{
    PICO   pbf;
    int	   rv;
    void (*redraw)() = ps_global->redrawer;

    push_titlebar_state();
    standard_picobuf_setup(&pbf);
    if(!newmail)
      pbf.newmail = NULL;

/* BUG: what about help command and text? */
    pbf.pine_anchor   = title;

    rv = pico_file_browse(&pbf, path, pathlen, file, filelen, NULL, flags);
    standard_picobuf_teardown(&pbf);
    fix_windsize(ps_global);
    init_signals();		/* has it's own signal stuff */

    /* Restore display's titlebar and body */
    pop_titlebar_state();
    redraw_titlebar();
    if(ps_global->redrawer = redraw)
      (*ps_global->redrawer)();

    return(rv);
}


#ifdef	_WINDOWS


/*
 * windows callback to get/set header mode state
 */
int
header_mode_callback(set, args)
    int  set;
    long args;
{
    return(ps_global->full_header);
}


/*
 * windows callback to get/set zoom mode state
 */
int
zoom_mode_callback(set, args)
    int  set;
    long args;
{
    return(any_lflagged(ps_global->msgmap, MN_HIDE) != 0);
}


/*
 * windows callback to get/set zoom mode state
 */
int
any_selected_callback(set, args)
    int  set;
    long args;
{
    return(any_lflagged(ps_global->msgmap, MN_SLCT) != 0);
}


/*
 *
 */
int
flag_callback(set, flags)
    int  set;
    long flags;
{
    MESSAGECACHE *mc;
    int		  newflags = 0;
    long	  msgno;
    int		  permflag = 0;

    switch (set) {
      case 1:			/* Important */
        permflag = ps_global->mail_stream->perm_flagged;
	break;

      case 2:			/* New */
        permflag = ps_global->mail_stream->perm_seen;
	break;

      case 3:			/* Answered */
        permflag = ps_global->mail_stream->perm_answered;
	break;

      case 4:			/* Deleted */
        permflag = ps_global->mail_stream->perm_deleted;
	break;

    }

    if(!(any_messages(ps_global->msgmap, NULL, "to Flag")
	 && can_set_flag(ps_global, "flag", permflag)))
      return(0);

    if(ps_global->io_error_on_stream) {
	ps_global->io_error_on_stream = 0;
	mail_check(ps_global->mail_stream); /* forces write */
	return(0);
    }

    msgno = mn_m2raw(ps_global->msgmap, mn_get_cur(ps_global->msgmap));
    if((mc = mail_elt(ps_global->mail_stream, msgno)) && mc->valid){
	/*
	 * NOTE: code below is *VERY* sensitive to the order of
	 * the messages defined in resource.h for flag handling.
	 * Don't change it unless you know what you're doing.
	 */
	if(set){
	    char *flagstr;
	    long  ourflag, mflag;

	    switch(set){
	      case 1 :			/* Important */
		flagstr = "\\FLAGGED";
		mflag   = (mc->flagged) ? 0L : ST_SET;
		break;

	      case 2 :			/* New */
		flagstr = "\\SEEN";
		mflag   = (mc->seen) ? 0L : ST_SET;
		break;

	      case 3 :			/* Answered */
		flagstr = "\\ANSWERED";
		mflag   = (mc->answered) ? 0L : ST_SET;
		break;

	      case 4 :		/* Deleted */
		flagstr = "\\DELETED";
		mflag   = (mc->deleted) ? 0L : ST_SET;
		break;

	      default :			/* bogus */
		return(0);
	    }

	    mail_flag(ps_global->mail_stream, long2string(msgno),
		      flagstr, mflag);

	    if(ps_global->redrawer)
	      (*ps_global->redrawer)();
	}
	else{
	    /* Important */
	    if(mc->flagged)
	      newflags |= 0x0001;

	    /* New */
	    if(!mc->seen)
	      newflags |= 0x0002;

	    /* Answered */
	    if(mc->answered)
	      newflags |= 0x0004;

	    /* Deleted */
	    if(mc->deleted)
	      newflags |= 0x0008;
	}
    }

    return(newflags);
}



MPopup *
flag_submenu(mc)
    MESSAGECACHE *mc;
{
    static MPopup flag_submenu[] = {
	{tMessage, {"Important", lNormal}, {IDM_MI_FLAGIMPORTANT}},
	{tMessage, {"New", lNormal}, {IDM_MI_FLAGNEW}},
	{tMessage, {"Answered", lNormal}, {IDM_MI_FLAGANSWERED}},
	{tMessage , {"Deleted", lNormal}, {IDM_MI_FLAGDELETED}},
	{tTail}
    };

    /* Important */
    flag_submenu[0].label.style = (mc->flagged) ? lChecked : lNormal;

    /* New */
    flag_submenu[1].label.style = (mc->seen) ? lNormal : lChecked;

    /* Answered */
    flag_submenu[2].label.style = (mc->answered) ? lChecked : lNormal;

    /* Deleted */
    flag_submenu[3].label.style = (mc->deleted) ? lChecked : lNormal;

    return(flag_submenu);
}
#endif	/* _WINDOWS */
