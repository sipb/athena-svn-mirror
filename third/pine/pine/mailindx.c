#if !defined(lint) && !defined(DOS)
static char rcsid[] = "$Id: mailindx.c,v 1.1.1.1 2001-02-19 07:12:03 ghudson Exp $";
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
    mailindx.c
    Implements the mail index screen
     - most code here builds the header list and displays it

 ====*/
 
#include "headers.h"
#include "../c-client/imap4r1.h"

/*
 * Some common Command Bindings
 */
#define	VIEWMSG_MENU	{">", "[ViewMsg]", \
			 {MC_VIEW_TEXT, 5,{'v','.','>',ctrl('M'),ctrl('J')}}, \
			 KS_VIEW}
#define	FLDRSORT_MENU	{"$", "SortIndex", {MC_SORT,1,{'$'}}, KS_SORT}


/*
 * Arrays containing all key-binding knowledge
 */
static struct key index_keys[] = 
       {HELP_MENU,
	OTHER_MENU,
	{"<", "FldrList", {MC_FOLDERS,2,{'<',','}}, KS_NONE},
	VIEWMSG_MENU,
	PREVMSG_MENU,
	NEXTMSG_MENU,
	PREVPAGE_MENU,
	NEXTPAGE_MENU,
	DELETE_MENU,
	UNDELETE_MENU,
	REPLY_MENU,
	FORWARD_MENU,

	HELP_MENU,
	OTHER_MENU,
	MAIN_MENU,
	QUIT_MENU,
	COMPOSE_MENU,
	GOTO_MENU,
	TAB_MENU,
	WHEREIS_MENU,
	PRYNTMSG_MENU,
	TAKE_MENU,
	SAVE_MENU,
	EXPORT_MENU,

	HELP_MENU,
	OTHER_MENU,
	{"X",NULL,{MC_EXPUNGE,1,{'x'}},KS_NONE},
	{"&","unXclude",{MC_UNEXCLUDE,1,{'&'}},KS_NONE},
	{";","Select",{MC_SELECT,1,{';'}},KS_SELECT},
	{"A","Apply",{MC_APPLY,1,{'a'}},KS_APPLY},
	FLDRSORT_MENU,
	JUMP_MENU,
	HDRMODE_MENU,
	BOUNCE_MENU,
	FLAG_MENU,
	PIPE_MENU,

	HELP_MENU,
	OTHER_MENU,
	{":","SelectCur",{MC_SELCUR,1,{':'}},KS_SELECTCUR},
	{"Z","ZoomMode",{MC_ZOOM,1,{'z'}},KS_ZOOM},
	LISTFLD_MENU,
	RCOMPOSE_MENU,
	NULL_MENU,
	NULL_MENU,
	NULL_MENU,
	NULL_MENU,
	NULL_MENU,
	NULL_MENU};
INST_KEY_MENU(index_keymenu, index_keys);
#define PREVM_KEY 4
#define NEXTM_KEY 5
#define EXCLUDE_KEY 26
#define UNEXCLUDE_KEY 27
#define SELECT_KEY 28
#define APPLY_KEY 29
#define VIEW_FULL_HEADERS_KEY 32
#define BOUNCE_KEY 33
#define FLAG_KEY 34
#define VIEW_PIPE_KEY 35
#define SELCUR_KEY 38
#define ZOOM_KEY 39

static struct key nr_anon_index_keys[] = 
       {HELP_MENU,
	WHEREIS_MENU,
	QUIT_MENU,
	VIEWMSG_MENU,
	PREVMSG_MENU,
	NEXTMSG_MENU,
	PREVPAGE_MENU,
	NEXTPAGE_MENU,
	FWDEMAIL_MENU,
	JUMP_MENU,
	FLDRSORT_MENU,
	NULL_MENU};
INST_KEY_MENU(nr_anon_index_keymenu, nr_anon_index_keys);

static struct key nr_index_keys[] = 
       {HELP_MENU,
	OTHER_MENU,
	QUIT_MENU,
	VIEWMSG_MENU,
	PREVMSG_MENU,
	NEXTMSG_MENU,
	PREVPAGE_MENU,
	NEXTPAGE_MENU,
	FWDEMAIL_MENU,
	JUMP_MENU,
	PRYNTMSG_MENU,
	SAVE_MENU,

	HELP_MENU,
	OTHER_MENU,
	EXPORT_MENU,
	COMPOSE_MENU,
	FLDRSORT_MENU,
	RCOMPOSE_MENU,
	NULL_MENU,
	WHEREIS_MENU,
	NULL_MENU,
	NULL_MENU,
	NULL_MENU,
	NULL_MENU};
INST_KEY_MENU(nr_index_keymenu, nr_index_keys);
  
static struct key simple_index_keys[] = 
       {HELP_MENU,
	{"E","ExitSelect",{MC_EXIT,1,{'e'}},KS_EXITMODE},
	NULL_MENU,
	{"S","[Select]",{MC_SELECT,3,{'s',ctrl('M'),ctrl('J')}},KS_SELECT},
	PREVMSG_MENU,
	NEXTMSG_MENU,
	PREVPAGE_MENU,
	NEXTPAGE_MENU,
	DELETE_MENU,
	UNDELETE_MENU,
	WHEREIS_MENU,
	NULL_MENU};
INST_KEY_MENU(simple_index_keymenu, simple_index_keys);


static OtherMenu what_keymenu = FirstMenu;

/*
 * structures to maintain index line text
 */

typedef struct _offsets {
    int        offset;		/* the offset into the line */
    COLOR_PAIR color;		/* color for this offset    */
} OFFCOLOR_S;

#define OFFS 5

typedef struct header_line {
    unsigned long id;				/* header line's uid   */
    unsigned      color_lookup_done:1;
    COLOR_PAIR	  linecolor;
    OFFCOLOR_S    offs[OFFS];
    char          line[1];			/* header line data    */
} HLINE_S;



/*
 * locally global place to store mail_sort and mail_thread
 * results.
 */
static struct global_sort_data {
    MSGNO_S *msgmap;
    SORTPGM *prog;
} g_sort;






/*-----------
  Saved state to redraw message index body 
  ----*/
struct entry_state {
    unsigned hilite:1;
    unsigned bolded:1;
    long     id;
};


#define MAXIFLDS 20  /* max number of fields in index format */
static struct index_state {
    long        msg_at_top,
	        lines_per_page;
    struct      entry_state *entry_state;
    MSGNO_S    *msgmap;
    MAILSTREAM *stream;
    int         status_col;		/* column for select X's */
} *current_index_state = NULL;


/*
 * Pieces needed to construct a valid folder index entry, and to
 * control what can be fetched when (due to callbacks and such)
 */
typedef struct index_data {
    MAILSTREAM *stream;
    ADDRESS    *from,			/* always valid */
	       *to,			/* check valid bit, fetch as req'd */
	       *cc,			/* check valid bit, fetch as req'd */
	       *sender;			/* check valid bit, fetch as req'd */
    char       *newsgroups,		/* check valid bit, fetch as req'd */
	       *subject,		/* always valid */
	       *date;			/* always valid */
    long	msgno,			/* tells us what we're looking at */
		rawno,
		uid,
		size;			/* always valid */
    unsigned	no_fetch:1,		/* lit when we're in a callback */
		bogus:2,		/* lit when there were problems */
		valid_to:1,		/* trust struct's "to" pointer */
		valid_cc:1,		/* trust struct's "cc" pointer */
		valid_sender:1,		/* trust struct's "sender" pointer */
		valid_news:1,		/* trust struct's "news" pointer */
		valid_resent_to:1,	/* trust struct's "resent-to" ptr */
		resent_to_us:1;		/* lit when we know its true */
} INDEXDATA_S;



/*
 * Binary tree to accumulate runs of subject groups (poor man's threads)
 */
typedef struct _subject_run_s {
    long	start;			/* raw msgno of run start */
    long	run;			/* run's length */
    struct _subject_run_s *left, *right;
} SUBR_S;



/*
 * Internal prototypes
 */
int		update_index PROTO((struct pine *, struct index_state *));
int		index_scroll_up PROTO((long));
int		index_scroll_down PROTO((long));
int		index_scroll_to_pos PROTO((long));
long		top_ent_calc PROTO((MAILSTREAM *, MSGNO_S *, long, long));
void		reset_index_border PROTO((void));
int		pine_compare_long PROTO((const QSType *, const QSType *));
int		pine_compare_long_rev PROTO((const QSType *, const QSType *));
int		pine_compare_scores PROTO((const QSType *, const QSType *));
void		build_score_array PROTO((MAILSTREAM *, MSGNO_S *));
void		free_score_array PROTO((void));
HLINE_S	       *get_index_cache PROTO((long));
int		fetch_sort_data PROTO((MAILSTREAM *, long, int, char *));
HLINE_S	       *build_header_line PROTO((struct pine *, MAILSTREAM *,
					 MSGNO_S *, long));
HLINE_S	       *format_index_line PROTO((INDEXDATA_S *));
void		index_data_env PROTO((INDEXDATA_S *, ENVELOPE *));
int		set_index_addr PROTO((INDEXDATA_S *, char *, ADDRESS *,
				      char *, int, char *));
int		i_cache_size PROTO((long));
int		i_cache_width PROTO(());
void		setup_header_widths PROTO((void));
int		parse_index_format PROTO((char *, INDEX_COL_S **));
void		clear_icache_flags PROTO(());
void		set_need_format_setup PROTO(());
int		need_format_setup PROTO(());
void		set_format_includes_msgno PROTO(());
int		format_includes_msgno PROTO(());
void		set_format_includes_smartdate PROTO(());
int		format_includes_smartdate PROTO(());
int		index_in_overview PROTO((MAILSTREAM *));
int		resent_to_us PROTO((INDEXDATA_S *));
int		parsed_resent_to_us PROTO((char *));
ADDRESS	       *fetch_from PROTO((INDEXDATA_S *));
ADDRESS	       *fetch_to PROTO((INDEXDATA_S *));
ADDRESS	       *fetch_cc PROTO((INDEXDATA_S *));
ADDRESS	       *fetch_sender PROTO((INDEXDATA_S *));
char	       *fetch_newsgroups PROTO((INDEXDATA_S *));
char	       *fetch_subject PROTO((INDEXDATA_S *));
char	       *fetch_date PROTO((INDEXDATA_S *));
long		fetch_size PROTO((INDEXDATA_S *));
BODY	       *fetch_body PROTO((INDEXDATA_S *));
void            set_msg_score PROTO((MAILSTREAM *, long, int));
COLOR_PAIR     *get_index_line_color PROTO((MAILSTREAM *, SEARCHSET *,
					    PAT_STATE **));
void		load_overview PROTO((MAILSTREAM *, unsigned long, OVERVIEW *));
void		paint_index_hline PROTO((MAILSTREAM *, long, HLINE_S *));
void		index_search PROTO((struct pine *,MAILSTREAM *,int,MSGNO_S *));
void		msgno_flush_selected PROTO((MSGNO_S *, long));
void		sort_sort_callback PROTO((MAILSTREAM *, unsigned long *,
					  unsigned long));
void		sort_thread_callback PROTO((MAILSTREAM *, THREADNODE *));
long	       *sort_thread_flatten PROTO((THREADNODE *, long *));
int             day_of_week PROTO((struct date *));
int             day_of_year PROTO((struct date *));
#if defined(DOS) && !defined(_WINDOWS)
void		i_cache_hit PROTO((long));
void		icread PROTO((void));
void		icwrite PROTO((void));
#endif
#ifdef	_WINDOWS
int		index_scroll_callback PROTO((int,long));
int		index_gettext_callback PROTO((char *, void **, long *, int *));
void		index_popup PROTO((MAILSTREAM *, MSGNO_S *, int));
char	       *pcpine_help_index PROTO((char *));
char	       *pcpine_help_index_simple PROTO((char *));
int		pcpine_resize_index PROTO((void));
#endif




/*----------------------------------------------------------------------


  ----*/
struct key_menu *
do_index_border(cntxt, folder, stream, msgmap, style, which_keys, flags)
     CONTEXT_S   *cntxt;
     char        *folder;
     MAILSTREAM  *stream;
     MSGNO_S     *msgmap;
     IndexType    style;
     int         *which_keys, flags;
{
    struct key_menu *km = (ps_global->anonymous)
			    ? &nr_anon_index_keymenu
			    : (ps_global->nr_mode)
			        ? &nr_index_keymenu
				: (ps_global->mail_stream != stream)
				    ? &simple_index_keymenu
				    : &index_keymenu;

    if(flags & INDX_CLEAR)
      ClearScreen();

    if(flags & INDX_HEADER)
      set_titlebar((stream == ps_global->mail_stream)
		     ? (style == MsgIndex || style == MultiMsgIndex)
		         ? "MESSAGE INDEX"
			 : "ZOOMED MESSAGE INDEX"
		     : (!strcmp(folder, INTERRUPTED_MAIL))
			 ? "COMPOSE: SELECT INTERRUPTED"
			 : (ps_global->VAR_FORM_FOLDER
			    && !strcmp(ps_global->VAR_FORM_FOLDER, folder))
			     ? "COMPOSE: SELECT FORM LETTER"
			     : "COMPOSE: SELECT POSTPONED",
		   stream, cntxt, folder, msgmap, 1, MessageNumber, 0, 0);

    if(flags & INDX_FOOTER) {
	bitmap_t bitmap;
	int	 cmd;

	setbitmap(bitmap);

	if(km == &index_keymenu){
#ifndef DOS
	    if(F_OFF(F_ENABLE_PIPE,ps_global))
#endif
	      clrbitn(VIEW_PIPE_KEY, bitmap);  /* always clear for DOS */
	    if(F_OFF(F_ENABLE_FULL_HDR,ps_global))
	      clrbitn(VIEW_FULL_HEADERS_KEY, bitmap);
	    if(F_OFF(F_ENABLE_BOUNCE,ps_global))
	      clrbitn(BOUNCE_KEY, bitmap);
	    if(F_OFF(F_ENABLE_FLAG,ps_global))
	      clrbitn(FLAG_KEY, bitmap);
	    if(F_OFF(F_ENABLE_AGG_OPS,ps_global)){
		clrbitn(SELECT_KEY, bitmap);
		clrbitn(APPLY_KEY, bitmap);
		clrbitn(SELCUR_KEY, bitmap);
		if(style != ZoomIndex)
		  clrbitn(ZOOM_KEY, bitmap);

	    }

	    if(IS_NEWS(stream)){
		index_keys[EXCLUDE_KEY].label = "eXclude";
		KS_OSDATASET(&index_keys[EXCLUDE_KEY], KS_NONE);
	    }
	    else {
		clrbitn(UNEXCLUDE_KEY, bitmap);
		index_keys[EXCLUDE_KEY].label = "eXpunge";
		KS_OSDATASET(&index_keys[EXCLUDE_KEY], KS_EXPUNGE);
	    }

	    if(style == MultiMsgIndex){
		clrbitn(PREVM_KEY, bitmap);
		clrbitn(NEXTM_KEY, bitmap);
	    }
	}

	menu_clear_binding(km, KEY_LEFT);
	menu_clear_binding(km, KEY_RIGHT);
	if(F_ON(F_ARROW_NAV, ps_global)){
	    if((cmd = menu_clear_binding(km, '<')) != MC_UNKNOWN){
		menu_add_binding(km, '<', cmd);
		menu_add_binding(km, KEY_LEFT, cmd);
	    }

	    if((cmd = menu_clear_binding(km, '>')) != MC_UNKNOWN){
		menu_add_binding(km, '>', cmd);
		menu_add_binding(km, KEY_RIGHT, cmd);
	    }
	}

	if(menu_binding_index(km, MC_JUMP) >= 0){
	    for(cmd = 0; cmd < 10; cmd++)
	      if(F_ON(F_ENABLE_JUMP, ps_global))
		(void) menu_add_binding(km, '0' + cmd, MC_JUMP);
	      else
		(void) menu_clear_binding(km, '0' + cmd);
	}

        draw_keymenu(km, bitmap, ps_global->ttyo->screen_cols,
		     1-FOOTER_ROWS(ps_global), 0, what_keymenu);
	what_keymenu = SameMenu;
	if(which_keys)
	  *which_keys = km->which;  /* pass back to caller */
    }

    return(km);
}

      
    
/*----------------------------------------------------------------------
        Main loop executing commands for the mail index screen

   Args: state -- the pine_state structure for next/prev screen pointers
                  and to pass to the index manager...
 ----*/

void
mail_index_screen(state)
     struct pine *state;
{
    dprint(1, (debugfile, "\n\n ---- MAIL INDEX ----\n"));
    if(!state->mail_stream) {
	q_status_message(SM_ORDER, 0, 3, "No folder is currently open");
        state->prev_screen = mail_index_screen;
	state->next_screen = main_menu_screen;
	return;
    }

    state->prev_screen = mail_index_screen;
    state->next_screen = SCREEN_FUN_NULL;
    index_lister(state, state->context_current, state->cur_folder,
		 state->mail_stream, state->msgmap);
}



/*----------------------------------------------------------------------
        Main loop executing commands for the mail index screen

   Args: state -- pine_state structure for display flags and such
         msgmap -- c-client/pine message number mapping struct
 ----*/

int
index_lister(state, cntxt, folder, stream, msgmap)
     struct pine *state;
     CONTEXT_S   *cntxt;
     char        *folder;
     MAILSTREAM  *stream;
     MSGNO_S     *msgmap;
{
    int		 ch, cmd, which_keys, force,
		 cur_row, cur_col, km_popped, paint_status;
    int          old_day = -1;
    long	 i, j, k, old_max_msgno;
    IndexType    style, old_style = MsgIndex;
    struct index_state id;
    struct key_menu *km = NULL;
#if defined(DOS) || defined(OS2)
    extern void (*while_waiting)();
#endif

    dprint(1, (debugfile, "\n\n ---- INDEX MANAGER ----\n"));
    
    ch                    = 'x';	/* For displaying msg 1st time thru */
    force                 = 0;
    km_popped             = 0;
    state->mangled_screen = 1;
    what_keymenu          = FirstMenu;
    memset((void *)&id, 0, sizeof(struct index_state));
    current_index_state   = &id;
    id.msgmap		  = msgmap;
    if(msgmap->top > 0L)
      id.msg_at_top = msgmap->top;

    if((id.stream = stream) != state->mail_stream)
      clear_index_cache();	/* BUG: should better tie stream to cache */

    set_need_format_setup();

    while (1) {
	if(km_popped){
	    km_popped--;
	    if(km_popped == 0){
		clearfooter(state);
		if(!state->mangled_body
		   && id.entry_state
		   && id.lines_per_page > 1){
		    id.entry_state[id.lines_per_page-2].id = -1;
		    id.entry_state[id.lines_per_page-1].id = -1;
		}
		else
		  state->mangled_body = 1;
	    }
	}

	old_max_msgno = mn_get_total(msgmap);

	/*------- Check for new mail -------*/
        new_mail(force, NM_TIMING(ch), NM_STATUS_MSG);
	force = 0;			/* may not need to next time around */

	/*
	 * If the width of the message number field in the display changes
	 * we need to flush the cache and redraw. When the cache is cleared
	 * the widths are recalculated, taking into account the max msgno.
	 */

	if(format_includes_msgno() &&
	   (old_max_msgno < 1000L && mn_get_total(msgmap) >= 1000L
	    || old_max_msgno < 10000L && mn_get_total(msgmap) >= 10000L
	    || old_max_msgno < 100000L && mn_get_total(msgmap) >= 100000L)){
	    clear_index_cache();
	    state->mangled_body = 1;
        }

	/*
	 * If the display includes the SMARTDATE ("Today", "Yesterday", ...)
	 * then when the day changes the date column will change. All of the
	 * Today's will become Yesterday's at midnight. So we have to
	 * clear the cache at midnight.
	 */
	if(format_includes_smartdate()){
	    char        db[200];
	    struct date nnow;

	    rfc822_date(db);
	    parse_date(db, &nnow);
	    if(old_day != -1 && nnow.day != old_day){
		clear_index_cache();
		state->mangled_body = 1;
	    }

	    old_day = nnow.day;
	}

        if(streams_died())
          state->mangled_header = 1;

        if(state->mangled_screen){
            state->mangled_header = 1;
            state->mangled_body   = 1;
            state->mangled_footer = 1;
            state->mangled_screen = 0;
        }

	/*
	 * events may have occured that require us to shift from
	 * mode to another...
	 */
	style = (any_lflagged(msgmap, MN_HIDE))
		  ? ZoomIndex
		  : (mn_total_cur(msgmap) > 1L) ? MultiMsgIndex : MsgIndex;
	if(style != old_style){
            state->mangled_header = 1;
            state->mangled_footer = 1;
	    old_style = style;
	    id.msg_at_top = 0L;
	}

        /*------------ Update the title bar -----------*/
	if(state->mangled_header) {
	    km = do_index_border(cntxt, folder, stream, msgmap,
				 style, NULL, INDX_HEADER);
	    state->mangled_header = 0;
	    paint_status = 0;
	} 
	else if(mn_get_total(msgmap) > 0) {
	    update_titlebar_message();
	    /*
	     * If flags aren't available to update the status,
	     * defer it until after all the fetches associated
	     * with building index lines are done (no extra rtts!)...
	     */
	    paint_status = !update_titlebar_status();
	}

	current_index_state = &id;

        /*------------ draw the index body ---------------*/
	cur_row = update_index(state, &id);
	if(F_OFF(F_SHOW_CURSOR, state)){
	    cur_row = state->ttyo->screen_rows - FOOTER_ROWS(state);
	    cur_col = 0;
	}
	else if(id.status_col >= 0)
	  cur_col = min(id.status_col, state->ttyo->screen_cols-1);

        ps_global->redrawer = redraw_index_body;

	if(paint_status)
	  (void) update_titlebar_status();

        /*------------ draw the footer/key menus ---------------*/
	if(state->mangled_footer) {
            if(!state->painted_footer_on_startup){
		if(km_popped){
		    FOOTER_ROWS(state) = 3;
		    clearfooter(state);
		}

		km = do_index_border(cntxt, folder, stream, msgmap, style,
				     &which_keys, INDX_FOOTER);
		if(km_popped){
		    FOOTER_ROWS(state) = 1;
		    mark_keymenu_dirty();
		}
	    }

	    state->mangled_footer = 0;
	}

        state->painted_body_on_startup   = 0;
        state->painted_footer_on_startup = 0;

	/*-- Display any queued message (eg, new mail, command result --*/
	if(km_popped){
	    FOOTER_ROWS(state) = 3;
	    mark_status_unknown();
	}

        display_message(ch);
	if(km_popped){
	    FOOTER_ROWS(state) = 1;
	    mark_status_unknown();
	}

	if(F_ON(F_SHOW_CURSOR, state) && cur_row < 0){
	    q_status_message(SM_ORDER,
		(ch==NO_OP_IDLE || ch==NO_OP_COMMAND) ? 0 : 3, 5,
		"No messages in folder");
	    cur_row = state->ttyo->screen_rows - FOOTER_ROWS(state);
	    display_message(ch);
	}

	MoveCursor(cur_row, cur_col);

        /* Let read_command do the fflush(stdout) */

        /*---------- Read command and validate it ----------------*/
#ifdef	MOUSE
	mouse_in_content(KEY_MOUSE, -1, -1, 0x5, 0);
	register_mfunc(mouse_in_content, HEADER_ROWS(ps_global), 0,
		       state->ttyo->screen_rows-(FOOTER_ROWS(ps_global)+1),
		       state->ttyo->screen_cols);
#endif
#if defined(DOS) || defined(OS2)
	/*
	 * AND pre-build header lines.  This works just fine under
	 * DOS since we wait for characters in a loop. Something will
         * will have to change under UNIX if we want to do the same.
	 */
	while_waiting = build_header_cache;
#ifdef	_WINDOWS
	while_waiting = NULL;
	mswin_setscrollcallback (index_scroll_callback);
	mswin_sethelptextcallback((stream == state->mail_stream)
				    ? pcpine_help_index
				    : pcpine_help_index_simple);
	mswin_setresizecallback(pcpine_resize_index);
#endif
#endif
	ch = read_command();
#ifdef	MOUSE
	clear_mfunc(mouse_in_content);
#endif
#if defined(DOS) || defined(OS2)
	while_waiting = NULL;
#ifdef	_WINDOWS
	mswin_setscrollcallback(NULL);
	mswin_sethelptextcallback(NULL);
	mswin_clearresizecallback(pcpine_resize_index);
#endif
#endif

	cmd = menu_command(ch, km);

	if(km_popped)
	  switch(cmd){
	    case MC_NONE :
	    case MC_OTHER :
	    case MC_RESIZE :
	    case MC_REPAINT :
	      km_popped++;
	      break;

	    default:
	      clearfooter(state);
	      break;
	  }

	/*----------- Execute the command ------------------*/
	switch(cmd){

            /*---------- Roll keymenu ----------*/
	  case MC_OTHER :
	    if(F_OFF(F_USE_FK, ps_global))
	      warn_other_cmds();

	    what_keymenu = NextMenu;
	    state->mangled_footer = 1;
	    break;


            /*---------- Scroll line up ----------*/
	  case MC_CHARUP :
	    (void) process_cmd(state, stream, msgmap, MC_PREVITEM,
			       (style == MsgIndex
				|| style == MultiMsgIndex
				|| style == ZoomIndex),
			       &force);
	    if(mn_get_cur(msgmap) < (id.msg_at_top + HS_MARGIN(state)))
	      index_scroll_up(1L);

	    break;


            /*---------- Scroll line down ----------*/
	  case MC_CHARDOWN :
	    /*
	     * Special Page framing handling here.  If we
	     * did something that should scroll-by-a-line, frame
	     * the page by hand here rather than leave it to the
	     * page-by-page framing in update_index()...
	     */
	    (void) process_cmd(state, stream, msgmap, MC_NEXTITEM,
			       (style == MsgIndex
				|| style == MultiMsgIndex
				|| style == ZoomIndex),
			       &force);
	    for(j = 0L, k = i = id.msg_at_top; ; i++){
		if(!get_lflag(stream, msgmap, i, MN_HIDE)){
		    k = i;
		    if(j++ >= id.lines_per_page)
		      break;
		}

		if(i >= mn_get_total(msgmap)){
		    k = 0L;		/* don't scroll */
		    break;
		}
	    }

	    if(k && (mn_get_cur(msgmap) + HS_MARGIN(state)) >= k)
	      index_scroll_down(1L);

	    break;


            /*---------- Scroll page up ----------*/
	  case MC_PAGEUP :
	    j = -1L;
	    for(k = i = id.msg_at_top; ; i--){
		if(!get_lflag(stream, msgmap, i, MN_HIDE)){
		    k = i;
		    if(++j >= id.lines_per_page){
			if((id.msg_at_top = i) == 1L)
			  q_status_message(SM_ORDER, 0, 1, "First Index page");

			break;
		    }
	       }

		if(i <= 1L){
		    if(mn_get_cur(msgmap) == 1L)
		      q_status_message(SM_ORDER, 0, 1,
			  "Already at start of Index");

		    break;
		}
	    }

	    if(mn_get_total(msgmap) > 0L && mn_total_cur(msgmap) == 1L)
	      mn_set_cur(msgmap, k);

	    break;


            /*---------- Scroll page forward ----------*/
	  case MC_PAGEDN :
	    j = -1L;
	    for(k = i = id.msg_at_top; ; i++){
		if(!get_lflag(stream, msgmap, i, MN_HIDE)){
		    k = i;
		    if(++j >= id.lines_per_page){
			if(i+id.lines_per_page >= mn_get_total(msgmap))
			  q_status_message(SM_ORDER, 0, 1, "Last Index page");

			id.msg_at_top = i;
			break;
		    }
		}

		if(i >= mn_get_total(msgmap)){
		    if(mn_get_cur(msgmap) == k)
		      q_status_message(SM_ORDER,0,1,"Already at end of Index");

		    break;
		}
	    }

	    if(mn_get_total(msgmap) > 0L && mn_total_cur(msgmap) == 1L)
	      mn_set_cur(msgmap, k);

	    break;


	    /*---------- Search (where is command) ----------*/
	  case MC_WHEREIS :
	    index_search(state, stream, -FOOTER_ROWS(ps_global), msgmap);
	    state->mangled_footer = 1;
	    break;


            /*-------------- jump command -------------*/
	    /* NOTE: preempt the process_cmd() version because
	     *	     we need to get at the number..
	     */
	  case MC_JUMP :
	    (void) jump_to(msgmap, -FOOTER_ROWS(ps_global), ch);
	    state->mangled_footer = 1;
	    break;


#ifdef MOUSE	    
	  case MC_MOUSE:
	    {
	      MOUSEPRESS mp;
	      int	 new_cur;

	      mouse_get_last (NULL, &mp);
	      mp.row -= 2;

	      for(i = id.msg_at_top;
		  mp.row >= 0 && i <= mn_get_total(msgmap);
		  i++)
		if(!get_lflag(stream, msgmap, i, MN_HIDE)){
		    mp.row--;
		    new_cur = i;
		}

	      if(mn_get_total(msgmap) && mp.row < 0){
		  switch(mp.button){
		    case M_BUTTON_LEFT :
		      if(mn_total_cur(msgmap) == 1L)
			mn_set_cur(msgmap, new_cur);

		      if(mp.flags & M_KEY_CONTROL){
			  if(F_ON(F_ENABLE_AGG_OPS, ps_global)){
			      (void) individual_select(state, msgmap,
						       -FOOTER_ROWS(state),
						       TRUE);
			  }
		      }
		      else if(!(mp.flags & M_KEY_SHIFT)){
			  if (mp.doubleclick){
			      if(mp.button == M_BUTTON_LEFT){
				  if(stream == state->mail_stream){
				      msgmap->top = id.msg_at_top;
				      process_cmd(state, stream, msgmap,
						  MC_VIEW_TEXT,
						  (style == MsgIndex
						   || style == MultiMsgIndex
						   || style == ZoomIndex),
						  &force);
				  }

				  ps_global->redrawer = NULL;
				  current_index_state = NULL;
				  if(id.entry_state)
				    fs_give((void **)&(id.entry_state));

				  return(0);
			      }
			  }
		      }

		      break;

		    case M_BUTTON_MIDDLE:
		      break;

		    case M_BUTTON_RIGHT :
#ifdef _WINDOWS
		      if (!mp.doubleclick){
			  if(mn_total_cur(msgmap) == 1L)
			    mn_set_cur(msgmap, new_cur);

			  cur_row = update_index(state, &id);

			  index_popup(stream, msgmap, TRUE);
		      }
#endif
		      break;
		  }
	      }
	      else{
		  switch(mp.button){
		    case M_BUTTON_LEFT :
		      break;

		    case M_BUTTON_MIDDLE :
		      break;

		    case M_BUTTON_RIGHT :
#ifdef	_WINDOWS
		      index_popup(stream, msgmap, FALSE);
#endif
		      break;
		  }
	      }
	    }

	    break;
#endif	/* MOUSE */

            /*---------- Resize ----------*/
          case MC_RESIZE:
	    clear_index_cache();
	    reset_index_border();
	    break;


            /*---------- Redraw ----------*/
	  case MC_REPAINT :
	    force = 1;			/* check for new mail! */
	    reset_index_border();
	    break;


            /*---------- No op command ----------*/
          case MC_NONE :
            break;	/* no op check for new mail */


	    /*--------- keystroke not bound to command --------*/
	  case MC_CHARRIGHT :
	  case MC_CHARLEFT :
	  case MC_GOTOBOL :
	  case MC_GOTOEOL :
	  case MC_UNKNOWN :
	    bogus_command(ch, F_ON(F_USE_FK,state) ? "F1" : "?");
	    break;


            /*---------- First HELP command with menu hidden ----------*/
	  case MC_HELP :
	    if(FOOTER_ROWS(state) == 1 && km_popped == 0){
		km_popped = 2;
		mark_status_unknown();
		mark_keymenu_dirty();
		state->mangled_footer = 1;
		break;
	    }
	    /* else fall thru to normal default */


            /*---------- Default -- all other command ----------*/
          default:
	    if(stream == state->mail_stream){
		msgmap->top = id.msg_at_top;
		process_cmd(state, stream, msgmap, cmd,
			    (style == MsgIndex
			     || style == MultiMsgIndex
			     || style == ZoomIndex),
			    &force);
		if(state->next_screen != SCREEN_FUN_NULL){
		    ps_global->redrawer = NULL;
		    current_index_state = NULL;
		    if(id.entry_state)
		      fs_give((void **)&(id.entry_state));

		    return(0);
		}
		else{
		    if(stream != state->mail_stream){
			/*
			 * Must have had an failed open.  repair our
			 * pointers...
			 */
			id.stream = stream = state->mail_stream;
			id.msgmap = msgmap = state->msgmap;
		    }

		    current_index_state = &id;
		}
	    }
	    else{			/* special processing */
		switch(cmd){
		  case MC_HELP :
		    helper(h_simple_index,
			   (!strcmp(folder, INTERRUPTED_MAIL))
			     ? "HELP FOR SELECTING INTERRUPTED MSG"
			     : "HELP FOR SELECTING POSTPONED MSG",
			   HLPD_SIMPLE);
		    state->mangled_screen = 1;
		    break;

		  case MC_DELETE :	/* delete */
		    dprint(3,(debugfile, "Special delete: msg %s\n",
			      long2string(mn_get_cur(msgmap))));
		    {
			long	      raw;
			int	      del = 0;

			raw = mn_m2raw(msgmap, mn_get_cur(msgmap));
			if(!mail_elt(stream, raw)->deleted){
			    clear_index_cache_ent(mn_get_cur(msgmap));
			    mail_setflag(stream,long2string(raw),"\\DELETED");
			    update_titlebar_status();
			    del++;
			}

			q_status_message2(SM_ORDER, 0, 1,
					  "Message %s %sdeleted",
					  long2string(mn_get_cur(msgmap)),
					  (del) ? "" : "already ");
		    }

		    break;

		  case MC_UNDELETE :	/* UNdelete */
		    dprint(3,(debugfile, "Special UNdelete: msg %s\n",
			      long2string(mn_get_cur(msgmap))));
		    {
			long	      raw;
			int	      del = 0;

			raw = mn_m2raw(msgmap, mn_get_cur(msgmap));
			if(mail_elt(stream, raw)->deleted){
			    clear_index_cache_ent(mn_get_cur(msgmap));
			    mail_clearflag(stream, long2string(raw),
					   "\\DELETED");
			    update_titlebar_status();
			    del++;
			}

			q_status_message2(SM_ORDER, 0, 1,
					  "Message %s %sdeleted",
					  long2string(mn_get_cur(msgmap)),
					  (del) ? "UN" : "NOT ");
		    }

		    break;

		  case MC_EXIT :	/* exit */
		    ps_global->redrawer = NULL;
		    current_index_state = NULL;
		    if(id.entry_state)
		      fs_give((void **)&(id.entry_state));

		    return(1);

		  case MC_SELECT :	/* select */
		    ps_global->redrawer = NULL;
		    current_index_state = NULL;
		    if(id.entry_state)
		      fs_give((void **)&(id.entry_state));

		    return(0);

		  case MC_PREVITEM :		/* previous */
		    mn_dec_cur(stream, msgmap);
		    break;

		  case MC_NEXTITEM :		/* next */
		    mn_inc_cur(stream, msgmap);
		    break;

		  default :
		    bogus_command(ch, NULL);
		    break;
		}
	    }
	}				/* The big switch */
    }					/* the BIG while loop! */
}



/*----------------------------------------------------------------------
  Manage index body painting

  Args: state - pine struct containing selected message data
	index_state - struct describing what's currently displayed

  Returns: screen row number of first highlighted message

  The idea is pretty simple.  Maintain an array of index line id's that
  are displayed and their hilited state.  Decide what's to be displayed
  and update the screen appropriately.  All index screen painting
  is done here.  Pretty simple, huh?
 ----*/
int
update_index(state, screen)
    struct pine         *state;
    struct index_state  *screen;
{
    int  i, retval = -1, row;
    long n;

    if(!screen)
      return(-1);

#ifdef _WINDOWS
    mswin_beginupdate();
#endif

    /*---- reset the works if necessary ----*/
    if(state->mangled_body){
	ClearBody();
	if(screen->entry_state){
	    fs_give((void **)&(screen->entry_state));
	    screen->lines_per_page = 0;
	}
    }

    state->mangled_body = 0;

    /*---- make sure we have a place to write state ----*/
    if(screen->lines_per_page
	!= max(0, state->ttyo->screen_rows - FOOTER_ROWS(state)
					   - HEADER_ROWS(state))){
	i = screen->lines_per_page;
	screen->lines_per_page
	    = max(0, state->ttyo->screen_rows - FOOTER_ROWS(state)
					      - HEADER_ROWS(state));
	if(!i){
	    size_t len = screen->lines_per_page * sizeof(struct entry_state);
	    screen->entry_state = (struct entry_state *) fs_get(len);
	}
	else
	  fs_resize((void **)&(screen->entry_state),
		    (size_t)screen->lines_per_page);

	for(; i < screen->lines_per_page; i++)	/* init new entries */
	  screen->entry_state[i].id = -1;
    }

    /*---- figure out the first message on the display ----*/
    if(screen->msg_at_top < 1L
       || (any_lflagged(screen->msgmap, MN_HIDE) > 0L
	   && get_lflag(screen->stream, screen->msgmap,
			screen->msg_at_top, MN_HIDE))){
	screen->msg_at_top = top_ent_calc(screen->stream, screen->msgmap,
					  screen->msg_at_top,
					  screen->lines_per_page);
    }
    else if(mn_get_cur(screen->msgmap) < screen->msg_at_top){
	long i, j, k;

	/* scroll back a page at a time until current is displayed */
	while(mn_get_cur(screen->msgmap) < screen->msg_at_top){
	    for(i = screen->lines_per_page, j = screen->msg_at_top-1L, k = 0L;
		i > 0L && j > 0L;
		j--)
	      if(!get_lflag(screen->stream, screen->msgmap, j, MN_HIDE)){
		  k = j;
		  i--;
	      }

	    if(i == screen->lines_per_page)
	      break;				/* can't scroll back ? */
	    else
	      screen->msg_at_top = k;
	}
    }
    else if(mn_get_cur(screen->msgmap) >= screen->msg_at_top
						     + screen->lines_per_page){
	long i, j, k;

	while(1){
	    for(i = screen->lines_per_page, j = k = screen->msg_at_top;
		j <= mn_get_total(screen->msgmap) && i > 0L;
		j++)
	      if(!get_lflag(screen->stream, screen->msgmap, j, MN_HIDE)){
		  k = j;
		  i--;
	      }

	    if(mn_get_cur(screen->msgmap) <= k)
	      break;
	    else{
		/* set msg_at_top to next displayed message */
		for(i = k + 1L; i <= mn_get_total(screen->msgmap); i++)
		  if(!get_lflag(screen->stream, screen->msgmap, i, MN_HIDE)){
		      k = i;
		      break;
		  }

		screen->msg_at_top = k;
	    }
	}
    }

#ifdef	_WINDOWS
    /* Set scroll range and position.  Note that message numbers start at 1
     * while scroll position starts at 0. */
    if(n = any_lflagged(screen->msgmap, MN_HIDE)){
	long x;

	scroll_setrange(screen->lines_per_page,
			mn_get_total(screen->msgmap) - n - 1);

	for(n = 1, x = 0; n != screen->msg_at_top; n++)
	  if(!get_lflag(screen->stream, screen->msgmap, n, MN_HIDE))
	    x++;

	scroll_setpos(x);
    }
    else{
	scroll_setrange(screen->lines_per_page,
			mn_get_total(screen->msgmap) - 1L);

	scroll_setpos(screen->msg_at_top - 1L);
    }
#endif

    /*
     * Set up c-client call back to tell us about IMAP envelope arrivals
     */
    if(F_OFF(F_QUELL_IMAP_ENV_CB, ps_global))
      mail_parameters(NULL, SET_IMAPENVELOPE, (void *) pine_imap_envelope);

    /*---- march thru display lines, painting whatever is needed ----*/
    for(i = 0, n = screen->msg_at_top; i < (int) screen->lines_per_page; i++){
	if(n < 1 || n > mn_get_total(screen->msgmap)){
	    if(screen->entry_state[i].id){
		screen->entry_state[i].hilite = 0;
		screen->entry_state[i].bolded = 0;
		screen->entry_state[i].id     = 0L;
		ClearLine(HEADER_ROWS(state) + i);
	    }
	}
	else{
	    row = paint_index_line(n, build_header_line(state, screen->stream,
							screen->msgmap, n),
				   i, screen->status_col,
				   &screen->entry_state[i],
				   mn_is_cur(screen->msgmap, n),
				   get_lflag(screen->stream, screen->msgmap,
					     n, MN_SLCT));
	    if(row && retval < 0)
	      retval = row;
	}

	/*--- increment n ---*/
	while(++n <= mn_get_total(screen->msgmap)
	      && get_lflag(screen->stream, screen->msgmap, n, MN_HIDE))
	  ;

    }

    if(F_OFF(F_QUELL_IMAP_ENV_CB, ps_global))
      mail_parameters(NULL, SET_IMAPENVELOPE, (void *) NULL);

#ifdef _WINDOWS
    mswin_endupdate();
#endif
    fflush(stdout);
    return(retval);
}



/*----------------------------------------------------------------------
     Paint the given index line


  Args: h -- structure describing the header line to paint
	n -- message number to paint
	screen -- structure describing current screen state

  Returns: 0 or the row number if the message is "current"
 ----*/
int
paint_index_line(msg, h, line, col, entry, cur, sel)
    long		msg;
    HLINE_S	       *h;
    int			line, col;
    struct entry_state *entry;
    int			cur, sel;
{
    COLOR_PAIR *lastc = NULL, *base_color = NULL;
    int inverse_hack = 0;

    if(h->id != entry->id || (cur != entry->hilite) || (sel != entry->bolded)){

	if(F_ON(F_FORCE_LOW_SPEED,ps_global) || ps_global->low_speed){
	    MoveCursor(HEADER_ROWS(ps_global) + line, col);
	    Writechar((sel)
		      ? 'X'
		      : (cur && h->line[col] == ' ')
			  ? '-'
			  : h->line[col], 0);
	    Writechar((cur) ? '>' : h->line[col+1], 0);

	    if(h->id != entry->id){
		if(col == 0)
		  PutLine0(HEADER_ROWS(ps_global) + line, 2, &h->line[2]);
		else{ /* this will rarely be set up this way */
		    char save_char1, save_char2;

		    save_char1 = h->line[col];
		    save_char2 = h->line[col+1];
		    h->line[col] = (sel) ? 'X' :
		      (cur && save_char1 == ' ') ?
		      '-' : save_char1;
		    h->line[col+1] = (cur) ? '>' : save_char2;
		    PutLine0(HEADER_ROWS(ps_global) + line, 0, &h->line[0]);
		    h->line[col]   = save_char1;
		    h->line[col+1] = save_char2;
		}
	    }
	}
	else{
	    char *draw = h->line, *p, save_char, save;
	    int   uc, i, drew_X = 0, cols = ps_global->ttyo->screen_cols;

	    if(uc=pico_usingcolor())
	      lastc = pico_get_cur_color();

	    MoveCursor(HEADER_ROWS(ps_global) + line, 0);

	    if(cur){
		/*
		 * If the current line has a linecolor, we're going to use
		 * the reverse of that to show it is current.
		 */
		if(uc && h->linecolor.fg[0] && h->linecolor.bg[0] &&
		   pico_is_good_colorpair(&h->linecolor)){
		    base_color = new_color_pair(h->linecolor.bg,
						h->linecolor.fg); /* reverse */
		    (void)pico_set_colorp(base_color, PSC_NONE);
		}
		else{
		    inverse_hack++;
		    if(uc)
		      base_color = lastc;
		}
	    }
	    else if(uc && h->linecolor.fg[0] && h->linecolor.bg[0] &&
		    pico_is_good_colorpair(&h->linecolor)){
		(void)pico_set_colorp(&h->linecolor, PSC_NONE);
		base_color = &h->linecolor;
	    }
	    else
	      base_color = lastc;

	    save_char = draw[col];

	    if(sel && (F_OFF(F_SELECTED_SHOWN_BOLD, ps_global)
		       || !StartBold())){
		draw[col] = 'X';
		drew_X++;
	    }

	    if(h->offs[0].offset < 0 || h->offs[0].offset >= cols){
		/* no special color, draw from 0 to end */
		if(inverse_hack)
		  StartInverse();

		Write_to_screen(draw);
		if(inverse_hack)
		  EndInverse();

		goto done_drawing;
	    }
	    /* draw possible first piece */
	    else if(h->offs[0].offset > 0){
		save = draw[h->offs[0].offset];
		draw[h->offs[0].offset] = '\0';
		if(inverse_hack)
		  StartInverse();

		Write_to_screen(draw);
		if(inverse_hack)
		  EndInverse();

		draw[h->offs[0].offset] = save;
	    }
	    /* else, no first section */

	    for(i = 0; i < OFFS-1; i++){
		/*
		 * Switch to color for i, which shouldn't be NULL.
		 * But don't switch if we drew an X in this column.
		 */
		if(h->offs[i].color.fg[0] && (!drew_X ||
					      col != h->offs[i].offset ||
					      save_char != '+')){
		    (void)pico_set_colorp(&h->offs[i].color, PSC_NORM);
		}

		if(h->offs[i+1].offset < 0 || h->offs[i+1].offset >= cols){
		    /* draw single colored character */
		    save = draw[h->offs[i].offset + 1];
		    draw[h->offs[i].offset + 1] = '\0';
		    Write_to_screen(draw + h->offs[i].offset);
		    draw[h->offs[i].offset + 1] = save;
		    (void)pico_set_colorp(base_color, PSC_NORM);
		    /* draw rest of line */
		    if(inverse_hack)
		      StartInverse();

		    Write_to_screen(draw + h->offs[i].offset + 1);
		    if(inverse_hack)
		      EndInverse();

		    goto done_drawing;
		}
		else if(h->offs[i+1].offset > h->offs[i].offset){
		    /*
		     * draw offs[i].offset to offs[i+1].offset - 1
		     */

		    /* draw single colored character */
		    save = draw[h->offs[i].offset + 1];
		    draw[h->offs[i].offset + 1] = '\0';
		    Write_to_screen(draw + h->offs[i].offset);
		    draw[h->offs[i].offset + 1] = save;
		    if(h->offs[i+1].offset > h->offs[i].offset + 1){
			/* draw to next offset */
			pico_set_colorp(base_color, PSC_NORM);
			save = draw[h->offs[i+1].offset];
			draw[h->offs[i+1].offset] = '\0';
			if(inverse_hack)
			  StartInverse();

			Write_to_screen(draw + h->offs[i].offset + 1);
			if(inverse_hack)
			  EndInverse();

			draw[h->offs[i+1].offset] = save;
		    }
		}
	    }

	    /* switch to color for 4, which shouldn't be NULL */
	    if(h->offs[4].color.fg[0] && (!drew_X ||
					  col != h->offs[4].offset ||
					  save_char != '+')){
		(void)pico_set_colorp(&h->offs[4].color, PSC_NORM);
	    }

	    if(h->offs[4].offset < cols){
		/* draw single colored character */
		save = draw[h->offs[4].offset + 1];
		draw[h->offs[4].offset + 1] = '\0';
		Write_to_screen(draw + h->offs[4].offset);
		draw[h->offs[4].offset + 1] = save;
		(void)pico_set_colorp(base_color, PSC_NORM);
		/* draw rest of line */
		if(h->offs[4].offset+1 < cols){
		    if(inverse_hack)
		      StartInverse();

		    Write_to_screen(draw + h->offs[4].offset + 1);
		    if(inverse_hack)
		      EndInverse();
		}
	    }

done_drawing:
	    if(drew_X)
	      draw[col] = save_char;

	    if(sel && !drew_X)
	      EndBold();

	    if(cur)
	      EndInverse();
	}

	if(base_color && base_color != lastc && base_color != &h->linecolor)
	  free_color_pair(&base_color);

	if(lastc){
	    (void)pico_set_colorp(lastc, PSC_NORM);
	    free_color_pair(&lastc);
	}
    }

    entry->hilite = cur;
    entry->bolded = sel;
    entry->id     = h->id;

    if(!h->color_lookup_done && pico_usingcolor())
      entry->id = -1;

    return(cur ? (line + HEADER_ROWS(ps_global)) : 0);
}




/*
 * pine_imap_env -- C-client's telling us an envelope just arrived
 *		    from the server.  Use it if we can...
 */
void
pine_imap_envelope(stream, rawno, env)
    MAILSTREAM	  *stream;
    unsigned long  rawno;
    ENVELOPE	  *env;
{
    MESSAGECACHE *mc;

    dprint(5, (debugfile, "imap_env(%ld)\n", rawno));
    if(!ps_global->mail_box_changed
       && stream == ps_global->mail_stream
       && (mc = mail_elt(stream,rawno))->valid
       && mc->rfc822_size
       && !get_lflag(stream, NULL, rawno, MN_HIDE | MN_EXLD)){
	INDEXDATA_S  idata;
	HLINE_S	    *hline;

	memset(&idata, 0, sizeof(INDEXDATA_S));
	idata.no_fetch = 1;
	idata.size     = mc->rfc822_size;
	idata.rawno    = rawno;
	idata.msgno    = mn_raw2m(ps_global->msgmap, rawno);
	idata.stream   = stream;

	index_data_env(&idata, env);

	/*
	 * Look for resent-to already in MAILCACHE data 
	 */
	if(mc->private.msg.header.text.data){
	    char       *p, *q;
	    STRINGLIST *lines;
	    SIZEDTEXT	szt;
	    static char *linelist[] = {"resent-to" , NULL};

	    if(mail_match_lines(lines = new_strlst(linelist),
				mc->private.msg.lines, 0L)){
		idata.valid_resent_to = 1;
		memset(&szt, 0, sizeof(SIZEDTEXT));
		textcpy(&szt, &mc->private.msg.header.text);
		mail_filter((char *) szt.data, szt.size, lines, 0L);
		idata.resent_to_us = parsed_resent_to_us((char *) szt.data);
		fs_give((void **) &szt.data);
	    }

	    free_strlst(&lines);
	}

	hline = format_index_line(&idata);
	if(idata.bogus)
	  hline->line[0] = '\0';
	else
	  paint_index_hline(stream, idata.msgno, hline);
    }
}



/*
 *
 */
void
paint_index_hline(stream, msgno, hline)
    MAILSTREAM *stream;
    long	msgno;
    HLINE_S    *hline;
{
    /*
     * Trust only what we get back that isn't bogus since
     * we were prevented from doing any fetches and such...
     */
    if((ps_global->redrawer == redraw_index_body
	|| ps_global->prev_screen == mail_index_screen)
       && current_index_state
       && current_index_state->stream == stream
       && !ps_global->msgmap->hilited){
	int line;

	if((line = (int)(msgno - current_index_state->msg_at_top)) >= 0
	   && line < current_index_state->lines_per_page){
	    if(any_lflagged(ps_global->msgmap, MN_HIDE)){
		long n;

		for(line = 0, n = current_index_state->msg_at_top;
		    n != msgno;
		    n++)
		  if(get_lflag(current_index_state->stream,
			       current_index_state->msgmap, n, MN_SLCT))
		    line++;
	    }

	    paint_index_line(msgno, hline, line,
			     current_index_state->status_col,
			     &current_index_state->entry_state[line],
			     mn_is_cur(current_index_state->msgmap, msgno),
			     get_lflag(current_index_state->stream,
				       current_index_state->msgmap,
				       msgno, MN_SLCT));
	    fflush(stdout);
	}
    }
}




/*----------------------------------------------------------------------
     Scroll to specified postion.


  Args: pos - position to scroll to.

  Returns: TRUE - did the scroll operation.
	   FALSE - was not able to do the scroll operation.
 ----*/
int
index_scroll_to_pos (pos)
long	pos;
{
    static short bad_timing = 0;
    long	i, j, k;
    
    if(bad_timing)
      return (FALSE);

    /*
     * Put the requested line at the top of the screen...
     */
#if 1
    /*
     * Starting at msg 'pos' find next visable message.
     */
    for(i=pos; i <= mn_get_total(current_index_state->msgmap); i++) {
      if(!get_lflag(current_index_state->stream, 
	            current_index_state->msgmap, i, MN_HIDE)){
	  current_index_state->msg_at_top = i;
	  break;
      }
    }
#else
    for(i=1L, j=pos; i <= mn_get_total(current_index_state->msgmap); i++) {
      if(!get_lflag(current_index_state->stream, 
	            current_index_state->msgmap, i, MN_HIDE)){
	  if((current_index_state->msg_at_top = i) > 
		  mn_get_cur(current_index_state->msgmap))
	    mn_set_cur(current_index_state->msgmap, i);

	  if(--j <= 0L)
	    break;
      }
    }
#endif

    /*
     * If single selection, move selected message to be on the sceen.
     */
    if (mn_total_cur(current_index_state->msgmap) == 1L) {
      if (current_index_state->msg_at_top > 
			      mn_get_cur (current_index_state->msgmap)) {
	/* Selection was above screen, move to top of screen. */
	mn_set_cur (current_index_state->msgmap, 
					current_index_state->msg_at_top);
      }
      else {
	/* Scan through the screen.  If selection found, leave where is.
	 * Otherwise, move to end of screen */
        for(  i = current_index_state->msg_at_top, 
	        j = current_index_state->lines_per_page;
	      i != mn_get_cur(current_index_state->msgmap) && 
		i <= mn_get_total(current_index_state->msgmap) && 
		j > 0L;
	      i++) {
	    if(!get_lflag(current_index_state->stream, 
	            current_index_state->msgmap, i, MN_HIDE)){
	        j--;
	        k = i;
            }
        }
	if(j <= 0L)
	    /* Move to end of screen. */
	    mn_set_cur(current_index_state->msgmap, k);
      }
    }

    bad_timing = 0;
    return (TRUE);
}



/*----------------------------------------------------------------------
     Adjust the index display state down a line

  Args: scroll_count -- number of lines to scroll

  Returns: TRUE - did the scroll operation.
	   FALSE - was not able to do the scroll operation.
 ----*/
int
index_scroll_down(scroll_count)
    long scroll_count;
{
    static short bad_timing = 0;
    long i, j, k;
    long cur, total;

    if(bad_timing)
      return (FALSE);

    bad_timing = 1;
    
    
    j = -1L;
    total = mn_get_total (current_index_state->msgmap);
    for(k = i = current_index_state->msg_at_top; ; i++){

	/* Only examine non-hidden messages. */
        if(!get_lflag(current_index_state->stream, 
		      current_index_state->msgmap, i, MN_HIDE)){
	    /* Remember this message */
	    k = i;
	    /* Increment count of lines.  */
	    if (++j >= scroll_count) {
		/* Counted enough lines, stop. */
		current_index_state->msg_at_top = k;
		break;
	    }
	}
	    
	/* If at last message, stop. */
	if (i >= total){
	    current_index_state->msg_at_top = k;
	    break;
	}
    }

    /*
     * If not multiple selection, see if selected message visable.  if not
     * set it to last visable message. 
     */
    if(mn_total_cur(current_index_state->msgmap) == 1L) {
	j = 0L;
	cur = mn_get_cur (current_index_state->msgmap);
	for (i = current_index_state->msg_at_top; i <= total; ++i) {
	    if(!get_lflag(current_index_state->stream, 
		          current_index_state->msgmap, i, MN_HIDE)) {
	        if (++j >= current_index_state->lines_per_page) {
		    break;
	        }
		if (i == cur) 
		    break;
	    }
        }
	if (i != cur) 
	    mn_set_cur(current_index_state->msgmap,
		       current_index_state->msg_at_top);
    }

    bad_timing = 0;
    return (TRUE);
}



/*----------------------------------------------------------------------
     Adjust the index display state up a line

  Args: scroll_count -- number of lines to scroll

  Returns: TRUE - did the scroll operation.
	   FALSE - was not able to do the scroll operation.

 ----*/
int
index_scroll_up(scroll_count)
    long scroll_count;
{
    static short bad_timing = 0;
    long i, j, k;
    long cur;

    if(bad_timing)
      return(FALSE);

    bad_timing = 1;
    
    j = -1L;
    for(k = i = current_index_state->msg_at_top; ; i--){

	/* Only examine non-hidden messages. */
        if(!get_lflag(current_index_state->stream, 
		      current_index_state->msgmap, i, MN_HIDE)){
	    /* Remember this message */
	    k = i;
	    /* Increment count of lines.  */
	    if (++j >= scroll_count) {
		/* Counted enough lines, stop. */
		current_index_state->msg_at_top = k;
		break;
	    }
	}
	    
	/* If at first message, stop */
	if (i <= 1L){
	    current_index_state->msg_at_top = k;
	    break;
	}
    }

    
    /*
     * If not multiple selection, see if selected message visable.  if not
     * set it to last visable message. 
     */
    if(mn_total_cur(current_index_state->msgmap) == 1L) {
	j = 0L;
	cur = mn_get_cur (current_index_state->msgmap);
	for (	i = current_index_state->msg_at_top; 
		i <= mn_get_total(current_index_state->msgmap);
		++i) {
	    if(!get_lflag(current_index_state->stream, 
		          current_index_state->msgmap, i, MN_HIDE)) {
	        if (++j >= current_index_state->lines_per_page) {
		    k = i;
		    break;
	        }
		if (i == cur) 
		    break;
	    }
        }
	if (i != cur) 
	    mn_set_cur(current_index_state->msgmap, k);
    }


    bad_timing = 0;
    return (TRUE);
}



/*----------------------------------------------------------------------
     Calculate the message number that should be at the top of the display

  Args: current - the current message number
        lines_per_page - the number of lines for the body of the index only

  Returns: -1 if the current message is -1 
           the message entry for the first message at the top of the screen.

When paging in the index it is always on even page boundies, and the
current message is always on the page thus the top of the page is
completely determined by the current message and the number of lines
on the page. 
 ----*/
long
top_ent_calc(stream, msgs, at_top, lines_per_page)
     MAILSTREAM *stream;
     MSGNO_S *msgs;
     long     at_top, lines_per_page;
{
    long current;

    current = (mn_total_cur(msgs) <= 1L) ? mn_get_cur(msgs) : at_top;

    if(current < 0L)
      return(-1);

    if(lines_per_page == 0L)
      return(current);

    if(any_lflagged(msgs, (MN_HIDE|MN_EXLD))){
	long n, m = 0L, t = 1L;

	for(n = 1L; n <= mn_get_total(msgs); n++)
	  if(!get_lflag(stream, msgs, n, MN_HIDE)
	     && (++m % lines_per_page) == 1L){
	      if(n > current)
		break;

	      t = n;
	  }

	return(t);
    }
    else
      return(lines_per_page * ((current - 1L)/ lines_per_page) + 1L);
}


/*----------------------------------------------------------------------
      Clear various bits that make up a healthy display

 ----*/
void
reset_index_border()
{
    mark_status_dirty();
    mark_keymenu_dirty();
    mark_titlebar_dirty();
    ps_global->mangled_screen = 1;	/* signal FULL repaint */
}


/*----------------------------------------------------------------------
      Initialize the index_disp_format array in ps_global from this
      format string.

   Args: format -- the string containing the format tokens
	 answer -- put the answer here, free first if there was a previous
		    value here
 ----*/
void
init_index_format(format, answer)
char         *format;
INDEX_COL_S **answer;
{
    int column = 0;

    set_need_format_setup();
    /* if custom format is specified, try it, else go with default */
    if(!(format && *format && parse_index_format(format, answer))){
	static INDEX_COL_S answer_default[] = {
	    {iStatus, Fixed, 3},
	    {iMessNo, WeCalculate },
	    {iDate, Fixed, 6},
	    {iFromTo, Percent, 33}, /* percent of rest */
	    {iSize, WeCalculate},
	    {iSubject, Percent, 67},
	    {iNothing}
	};

	if(*answer)
	  fs_give((void **)answer);

	*answer = (INDEX_COL_S *)fs_get(sizeof(answer_default));
	memcpy(*answer, answer_default, sizeof(answer_default));
    }

    /*
     * Fill in req_width's for WeCalculate items.
     */
    for(column = 0; (*answer)[column].ctype != iNothing; column++){
	if((*answer)[column].wtype == WeCalculate){
	    switch((*answer)[column].ctype){
	      case iAtt:
		(*answer)[column].req_width = 1;
		break;
	      case iStatus:
	      case iMessNo:
	      case iMonAbb:
		(*answer)[column].req_width = 3;
		break;
	      case iTime24:
	      case iTimezone:
		(*answer)[column].req_width = 5;
		break;
	      case iFStatus:
	      case iIStatus:
	      case iDate:
		(*answer)[column].req_width = 6;
		break;
	      case iTime12:
		(*answer)[column].req_width = 7;
		break;
	      case iS1Date:
	      case iS2Date:
	      case iS3Date:
	      case iS4Date:
	      case iSize:
	      case iDateIsoS:
		(*answer)[column].req_width = 8;
		break;
	      case iDescripSize:
	      case iSDate:
		(*answer)[column].req_width = 9;
		break;
	      case iDateIso:
		(*answer)[column].req_width = 10;
		break;
	      case iLDate:
		(*answer)[column].req_width = 12;
		break;
	    }
	}
    }
}


/* popular ones first to make it slightly faster */
static INDEX_PARSE_T itokens[] = {
    {"STATUS",		iStatus,	FOR_INDEX},
    {"MSGNO",		iMessNo,	FOR_INDEX},
    {"DATE",		iDate,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"FROMORTO",	iFromTo,	FOR_INDEX},
    {"FROMORTONOTNEWS",	iFromToNotNews,	FOR_INDEX},
    {"SIZE",		iSize,		FOR_INDEX},
    {"SUBJECT",		iSubject,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"FULLSTATUS",	iFStatus,	FOR_INDEX},
    {"IMAPSTATUS",	iIStatus,	FOR_INDEX},
    {"DESCRIPSIZE",	iDescripSize,	FOR_INDEX},
    {"ATT",		iAtt,		FOR_INDEX},
    {"LONGDATE",	iLDate,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SHORTDATE1",	iS1Date,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SHORTDATE2",	iS2Date,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SHORTDATE3",	iS3Date,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SHORTDATE4",	iS4Date,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"DATEISO",		iDateIso,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SHORTDATEISO",	iDateIsoS,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SMARTDATE",	iSDate,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"TIME24",		iTime24,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"TIME12",		iTime12,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"TIMEZONE",	iTimezone,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"MONTHABBREV",	iMonAbb,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"FROM",		iFrom,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"TO",		iTo,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SENDER",		iSender,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"CC",		iCc,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"RECIPS",		iRecips,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"NEWS",		iNews,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"TOANDNEWS",	iToAndNews,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"NEWSANDTO",	iNewsAndTo,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"RECIPSANDNEWS",	iRecipsAndNews,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"NEWSANDRECIPS",	iNewsAndRecips,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"MSGID",		iMsgID,		FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"DAYDATE",		iRDate,		FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"DAY",		iDay,		FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"DAYORDINAL",	iDayOrdinal,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"DAY2DIGIT",	iDay2Digit,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"MONTHLONG",	iMonLong,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"MONTH",		iMon,		FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"MONTH2DIGIT",	iMon2Digit,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"YEAR",		iYear,		FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"YEAR2DIGIT",	iYear2Digit,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"ADDRESS",		iAddress,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"MAILBOX",		iMailbox,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"CURDATE",		iCurDate,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"CURDATEISO",	iCurDateIso,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"CURDATEISOS",	iCurDateIsoS,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"CURTIME24",	iCurTime24,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"CURTIME12",	iCurTime12,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"CURSORPOS",	iCursorPos,	FOR_TEMPLATE},
    {NULL,		iNothing,	FOR_NOTHING}
};


/*
 * Args  txt -- The token being checked begins at the beginning
 *              of txt. The end of the token is delimited by a null, or
 *              white space, or an underscore if DELIM_USCORE is set,
 *              or a left paren if DELIM_PAREN is set.
 *     flags -- Flags contains the what_for value, and DELIM_ values.
 *
 * Returns  A ptr to an INDEX_PARSE_T from itokens above, else NULL.
 */
INDEX_PARSE_T *
itoktype(txt, flags)
    char *txt;
    int   flags;
{
    INDEX_PARSE_T *pt;
    char           token[100 + 1];
    char          *v, *w;

    /*
     * Separate a copy of the possible token out of txt.
     */
    v = txt;
    w = token;
    while(w < token+100 &&
	  *v &&
	  !isspace((unsigned char)*v) &&
	  !(flags & DELIM_USCORE && *v == '_') &&
	  !(flags & DELIM_PAREN && *v == '('))
      *w++ = *v++;
    
    *w = '\0';

    for(pt = itokens; pt->name; pt++)
      if(pt->what_for & flags && !strucmp(pt->name, token))
        break;
    
    return(pt->name ? pt : NULL);
}


int
parse_index_format(format_str, answer)
char         *format_str;
INDEX_COL_S **answer;
{
    int            i, column = 0;
    char          *p, *q;
    INDEX_PARSE_T *pt;
    INDEX_COL_S    cdesc[MAXIFLDS]; /* temp storage for answer */

    memset((void *)cdesc, 0, sizeof(cdesc));

    p = format_str;
    while(p && *p && column < MAXIFLDS-1){
	/* skip leading white space for next word */
	p = skip_white_space(p);
	pt = itoktype(p, FOR_INDEX | DELIM_PAREN);
	
	/* ignore unrecognized word */
	if(!pt){
	    for(q = p; *p && !isspace((unsigned char)*p); p++)
	      ;

	    if(*p)
	      *p++ = '\0';

	    dprint(1, (debugfile,
		       "parse_index_format: unrecognized token: %s\n", q));
	    q_status_message1(SM_ORDER | SM_DING, 0, 3,
			      "Unrecognized string in index-format: %s", q);
	    continue;
	}

	cdesc[column].ctype = pt->ctype;

	/* skip over name and look for parens */
	p += strlen(pt->name);
	if(*p == '('){
	    p++;
	    q = p;
	    while(p && *p && isdigit((unsigned char)*p))
	      p++;
	    
	    if(p && *p && *p == ')' && p > q){
		cdesc[column].wtype = Fixed;
		cdesc[column].req_width = atoi(q);
	    }
	    else if(p && *p && *p == '%' && p > q){
		cdesc[column].wtype = Percent;
		cdesc[column].req_width = atoi(q);
	    }
	    else{
		cdesc[column].wtype = WeCalculate;
		cdesc[column].req_width = 0;
	    }
	}
	else{
	    cdesc[column].wtype     = WeCalculate;
	    cdesc[column].req_width = 0;
	}

	column++;
	/* skip text at end of word */
	while(p && *p && !isspace((unsigned char)*p))
	  p++;
    }

    /* if, after all that, we didn't find anything recognizable, bitch */
    if(!column){
	dprint(1, (debugfile, "Completely unrecognizable index-format\n"));
	q_status_message(SM_ORDER | SM_DING, 0, 3,
		 "Configured \"index-format\" unrecognizable. Using default.");
	return(0);
    }

    /* Finish with Nothing column */
    cdesc[column].ctype = iNothing;

    /* free up old answer */
    if(*answer)
      fs_give((void **)answer);

    /* allocate space for new answer */
    *answer = (INDEX_COL_S *)fs_get((column+1)*sizeof(INDEX_COL_S));
    memset((void *)(*answer), 0, (column+1)*sizeof(INDEX_COL_S));
    /* copy answer to real place */
    for(i = 0; i <= column; i++)
      (*answer)[i] = cdesc[i];

    return(1);
}


/*----------------------------------------------------------------------
    This redraws the body of the index screen, taking into
account any change in the size of the screen. All the state needed to
repaint is in the static variables so this can be called from
anywhere.
 ----*/
void
redraw_index_body()
{
    int agg;

    if(agg = (mn_total_cur(current_index_state->msgmap) > 1L))
      restore_selected(current_index_state->msgmap);

    ps_global->mangled_body = 1;

    (void) update_index(ps_global, current_index_state);
    if(agg)
      pseudo_selected(current_index_state->msgmap);
}



/*----------------------------------------------------------------------
      Setup the widths of the various columns in the index display

   Args: news      -- mail stream is news
	 max_msgno -- max message number in mail stream
 ----*/
void
setup_header_widths()
{
    int		 j, columns, some_to_calculate;
    int		 space_left, screen_width, width, fix, col, scol, altcol;
    int		 keep_going, tot_pct, was_sl;
    long         max_msgno;
    WidthType	 wtype;
    INDEX_COL_S *cdesc;

    max_msgno = mn_get_total(ps_global->msgmap);

    dprint(8, (debugfile, "=== setup_header_widths(%ld) ===\n",max_msgno));

    clear_icache_flags();
    screen_width = ps_global->ttyo->screen_cols;
    space_left	 = screen_width;
    columns	 = some_to_calculate = 0;

    /*
     * Calculate how many fields there are so we know how many spaces
     * between columns to reserve.  Fill in Fixed widths now.  Reserve
     * special case WeCalculate with non-zero req_widths before doing
     * Percent cases below.
     */
    for(cdesc = ps_global->index_disp_format;
	cdesc->ctype != iNothing;
	cdesc++){

	/* These aren't included in nr mode */
	if(ps_global->nr_mode && (cdesc->ctype == iFromTo ||
				  cdesc->ctype == iFromToNotNews ||
				  cdesc->ctype == iFrom ||
				  cdesc->ctype == iSender ||
				  cdesc->ctype == iCc ||
				  cdesc->ctype == iRecips ||
				  cdesc->ctype == iNews ||
				  cdesc->ctype == iToAndNews ||
				  cdesc->ctype == iNewsAndTo ||
				  cdesc->ctype == iRecipsAndNews ||
				  cdesc->ctype == iNewsAndRecips ||
				  cdesc->ctype == iTo)){
	    cdesc->req_width = 0;
	    cdesc->width = 0;
	    cdesc->wtype = Fixed;
	}
	else if(cdesc->wtype == Fixed){
	  cdesc->width = cdesc->req_width;
	  if(cdesc->width > 0)
	    columns++;
	}
	else if(cdesc->wtype == Percent){
	    cdesc->width = 0; /* calculated later */
	    columns++;
	}
	else{ /* WeCalculate */
	    cdesc->width = cdesc->req_width; /* reserve this for now */
	    some_to_calculate++;
	    columns++;
	}

	space_left -= cdesc->width;
    }

    space_left -= (columns - 1); /* space between columns */

    for(cdesc = ps_global->index_disp_format;
	cdesc->ctype != iNothing;
	cdesc++){
	wtype = cdesc->wtype;
	if(wtype != WeCalculate && wtype != Percent && cdesc->width == 0)
	  continue;

	switch(cdesc->ctype){
	  case iStatus:
	  case iMonAbb:
	    cdesc->actual_length = 3;
	    cdesc->adjustment = Left;
	    break;

	  case iTime24:
	  case iTimezone:
	    cdesc->actual_length = 5;
	    cdesc->adjustment = Left;
	    break;

	  case iFStatus:
	  case iIStatus:
	  case iDate:
	    cdesc->actual_length = 6;
	    cdesc->adjustment = Left;
	    break;

	  case iTime12:
	    cdesc->actual_length = 7;
	    cdesc->adjustment = Right;
	    break;

	  case iLDate:
	    cdesc->actual_length = 12;
	    cdesc->adjustment = Left;
	    break;

	  case iS1Date:
	  case iS2Date:
	  case iS3Date:
	  case iS4Date:
	  case iDateIsoS:
	    cdesc->actual_length = 8;
	    cdesc->adjustment = Left;
	    break;

	  case iSDate:
	    set_format_includes_smartdate();
	    cdesc->actual_length = 9;
	    cdesc->adjustment = Left;
	    break;

	  case iDateIso:
	    cdesc->actual_length = 10;
	    cdesc->adjustment = Left;
	    break;

	  case iAtt:
	    cdesc->actual_length = 1;
	    cdesc->adjustment = Left;
	    break;

	  case iFromTo:
	  case iFromToNotNews:
	  case iFrom:
	  case iSender:
	  case iTo:
	  case iCc:
	  case iRecips:
	  case iNews:
	  case iToAndNews:
	  case iNewsAndTo:
	  case iRecipsAndNews:
	  case iNewsAndRecips:
	  case iSubject:
	    cdesc->adjustment = Left;
	    break;

	  case iMessNo:
	    set_format_includes_msgno();
	    if(max_msgno < 1000)
	      cdesc->actual_length = 3;
	    else if(max_msgno < 10000)
	      cdesc->actual_length = 4;
	    else if(max_msgno < 100000)
	      cdesc->actual_length = 5;
	    else
	      cdesc->actual_length = 6;

	    cdesc->adjustment = Right;
	    break;

	  case iSize:
	    cdesc->actual_length = 8;
	    cdesc->adjustment = Right;
	    break;

	  case iDescripSize:
	    cdesc->actual_length = 9;
	    cdesc->adjustment = Right;
	    break;
	}
    }

    /* if have reserved unneeded space for size, give it back */
    for(cdesc = ps_global->index_disp_format;
	cdesc->ctype != iNothing;
	cdesc++)
      if(cdesc->ctype == iSize || cdesc->ctype == iDescripSize){
	  if(cdesc->actual_length == 0){
	      if((fix=cdesc->width) > 0){ /* had this reserved */
		  cdesc->width = 0;
		  space_left += fix;
	      }

	      space_left++;  /* +1 for space between columns */
	  }
      }

    /*
     * Calculate the field widths that are basically fixed in width.
     * Do them in this order in case we don't have enough space to go around.
     */
    for(j = 0; space_left > 0 && some_to_calculate; j++){
      static IndexColType targetctype[] = {
	iMessNo, iStatus, iFStatus, iIStatus, iDate, iSDate, iLDate,
	iS1Date, iS2Date, iS3Date, iS4Date, iDateIso, iDateIsoS,
	iSize, iDescripSize, iAtt, iTime24, iTime12, iTimezone, iMonAbb
      };

      if(j >= sizeof(targetctype)/sizeof(*targetctype))
	break;

      for(cdesc = ps_global->index_disp_format;
	  cdesc->ctype != iNothing && space_left > 0 && some_to_calculate;
	  cdesc++)
	if(cdesc->ctype == targetctype[j] && cdesc->wtype == WeCalculate){
	    some_to_calculate--;
	    fix = min(cdesc->actual_length - cdesc->width, space_left);
	    cdesc->width += fix;
	    space_left -= fix;
	}
    }

    /*
     * Fill in widths for Percent cases.  If there are no more to calculate,
     * use the percentages as relative numbers and use the rest of the space,
     * else treat them as absolute percentages of the original avail screen.
     */
    if(space_left > 0){
      if(some_to_calculate){
	for(cdesc = ps_global->index_disp_format;
	    cdesc->ctype != iNothing && space_left > 0;
	    cdesc++){
	  if(cdesc->wtype == Percent){
	      /* The 2, 200, and +100 are because we're rounding */
	      fix = ((2*cdesc->req_width *
		      (screen_width-(columns-1)))+100) / 200;
	      fix = min(fix, space_left);
	      cdesc->width += fix;
	      space_left -= fix;
	  }
	}
      }
      else{
	tot_pct = 0;
	was_sl = space_left;
	/* add up total percentages requested */
	for(cdesc = ps_global->index_disp_format;
	    cdesc->ctype != iNothing;
	    cdesc++)
	  if(cdesc->wtype == Percent)
	    tot_pct += cdesc->req_width;

	/* give relative weight to requests */
	for(cdesc = ps_global->index_disp_format;
	    cdesc->ctype != iNothing && space_left > 0 && tot_pct > 0;
	    cdesc++){
	    if(cdesc->wtype == Percent){
		fix = ((2*cdesc->req_width*was_sl)+tot_pct) / (2*tot_pct);
	        fix = min(fix, space_left);
	        cdesc->width += fix;
	        space_left -= fix;
	    }
	}
      }
    }

    /* split up rest, give twice as much to Subject */
    keep_going = 1;
    while(space_left > 0 && keep_going){
      keep_going = 0;
      for(cdesc = ps_global->index_disp_format;
	  cdesc->ctype != iNothing && space_left > 0;
	  cdesc++){
	if(cdesc->wtype == WeCalculate &&
	  (cdesc->ctype == iFromTo ||
	   cdesc->ctype == iFromToNotNews ||
	   cdesc->ctype == iFrom ||
	   cdesc->ctype == iSender ||
	   cdesc->ctype == iTo ||
	   cdesc->ctype == iCc ||
	   cdesc->ctype == iRecips ||
	   cdesc->ctype == iNews ||
	   cdesc->ctype == iToAndNews ||
	   cdesc->ctype == iNewsAndTo ||
	   cdesc->ctype == iRecipsAndNews ||
	   cdesc->ctype == iNewsAndRecips ||
	   cdesc->ctype == iSubject)){
	  keep_going++;
	  cdesc->width++;
	  space_left--;
	  if(space_left > 0 && cdesc->ctype == iSubject){
	      cdesc->width++;
	      space_left--;
	  }
	}
      }
    }

    /* if still more, pad out percent's */
    keep_going = 1;
    while(space_left > 0 && keep_going){
      keep_going = 0;
      for(cdesc = ps_global->index_disp_format;
	  cdesc->ctype != iNothing && space_left > 0;
	  cdesc++){
	if(cdesc->wtype == Percent &&
	  (cdesc->ctype == iFromTo ||
	   cdesc->ctype == iFromToNotNews ||
	   cdesc->ctype == iFrom ||
	   cdesc->ctype == iSender ||
	   cdesc->ctype == iTo ||
	   cdesc->ctype == iCc ||
	   cdesc->ctype == iRecips ||
	   cdesc->ctype == iNews ||
	   cdesc->ctype == iToAndNews ||
	   cdesc->ctype == iNewsAndTo ||
	   cdesc->ctype == iRecipsAndNews ||
	   cdesc->ctype == iNewsAndRecips ||
	   cdesc->ctype == iSubject)){
	  keep_going++;
	  cdesc->width++;
	  space_left--;
	}
      }
    }

    /* if user made Fixed fields too big, give back space */
    keep_going = 1;
    while(space_left < 0 && keep_going){
      keep_going = 0;
      for(cdesc = ps_global->index_disp_format;
	  cdesc->ctype != iNothing && space_left < 0;
	  cdesc++){
	if(cdesc->wtype == Fixed && cdesc->width > 0){
	  keep_going++;
	  cdesc->width--;
	  space_left++;
	}
      }
    }

    col = 0;
    scol = -1;
    altcol = -1;
    /* figure out what column is start of status field */
    for(cdesc = ps_global->index_disp_format;
	cdesc->ctype != iNothing;
	cdesc++){
	width = cdesc->width;
	if(width == 0)
	  continue;

	/* space between columns */
	if(col > 0)
	  col++;

	if(cdesc->ctype == iStatus){
	    scol = col;
	    break;
	}

	if(cdesc->ctype == iFStatus || cdesc->ctype == iIStatus){
	    scol = col;
	    break;
	}

	if(cdesc->ctype == iMessNo)
	  altcol = col;

	col += width;
    }

    if(scol == -1){
	if(altcol == -1)
	  scol = 0;
	else
	  scol = altcol;
    }

    if(current_index_state)
      current_index_state->status_col = scol;
}


/*----------------------------------------------------------------------
      Create a string summarizing the message header for index on screen

   Args: stream -- mail stream to fetch envelope info from
	 msgmap -- message number to pine sort mapping
	 msgno  -- Message number to create line for

  Result: returns a malloced string
          saves string in a cache for next call for same header
 ----*/
HLINE_S *
build_header_line(state, stream, msgmap, msgno)
    struct pine *state;
    MAILSTREAM  *stream;
    MSGNO_S     *msgmap;
    long         msgno;
{
    HLINE_S	 *hline;
    MESSAGECACHE *mc;
    long          n, i, cnt;

    /* cache hit? */
    if(*(hline = get_index_cache(msgno))->line && hline->color_lookup_done){
        dprint(9, (debugfile, "Hit: Returning %p -> <%s (%d), %ld>\n",
		   hline, hline->line, strlen(hline->line), hline->id));
	return(hline);
    }

    /*
     * Fetch everything we need to start filling in the index line
     * explicitly via mail_fetch_overview.  On an nntp stream
     * this has the effect of building the index lines in the
     * load_overview callback.  Under IMAP we're either getting
     * the envelope data via the imap_envelope callback or
     * preloading the cache.  Either way, we're getting exactly
     * what we want rather than relying on linear lookahead sort
     * of prefetch...
     */
    if(!*hline->line && index_in_overview(stream)){
	char	     *uidseq, *p;
	long	      uid, next;
	int	      count;
	MESSAGECACHE *mc;

	/* clear sequence bits */
	for(n = 1L; n <= stream->nmsgs; n++)
	  mail_elt(stream, n)->sequence = 0;

	/*
	 * Light interesting bits
	 * NOTE: not set above because m2raw's cheaper
	 * than raw2m for every message
	 */
	for(count=0, i=0, n = current_index_state->msg_at_top;
	    i < current_index_state->lines_per_page;
	    i++){

	    if(n >= msgno
	       && n <= mn_get_total(msgmap)
	       && !*get_index_cache(n)->line
	       && !(mc = mail_elt(stream,mn_m2raw(msgmap,n)))->private.msg.env){
		mc->sequence = 1;
		count++;
	    }

	    /* find next n which is visible */
	    while(++n <=  mn_get_total(msgmap) &&
		  get_lflag(stream, msgmap, n, MN_HIDE))
	      ;
	}

	if(count){
	    /* How big a buf do we need?  How about eleven per -- 10 for
	     * decimal representation of 32bit value plus a trailing
	     * comma or terminating null...
	     */
	    uidseq = (char *) fs_get((count * 11) * sizeof(char));

	    *(p = uidseq) = '\0';
	    next = 0L;
	    for(n = 1L; n <= stream->nmsgs; n++)
	      if(mail_elt(stream, n)->sequence){
		  if((uid = mail_uid(stream, n)) == next){
		      *p = ':';
		      strcpy(p + 1, long2string(uid));
		      next++;			/* run! */
		  }
		  else{
		      if(p != uidseq){
			  p += strlen(p);
			  *p++ = ',';
		      }

		      sstrcpy(&p, long2string(uid));
		      next = ++uid;
		  }
	      }

	    mail_fetch_overview(stream, uidseq,
				(!strcmp(stream->dtb->name, "imap"))
				  ? NULL : load_overview);
	    fs_give((void **) &uidseq);
	}

	/*
	 * reassign hline from the cache as it may've been built
	 * within the overview callback or it may have become stale
	 * in the prior sequence bit setting loop ...
	 */
	hline = get_index_cache(msgno);
    }

    if(!*(hline->line)){
	INDEXDATA_S idata;

	/*
	 * With pre-fetching/callback-formatting done and no success,
	 * fall into formatting the requested line...
	 */
	memset(&idata, 0, sizeof(INDEXDATA_S));
	idata.stream   = stream;
	idata.msgno    = msgno;
	idata.rawno    = mn_m2raw(msgmap, msgno);
	idata.uid	   = mail_uid(stream, idata.rawno);
	if(mc = mail_elt(stream, idata.rawno)){
	    idata.size = mc->rfc822_size;
	    index_data_env(&idata, mail_fetchenvelope(stream, idata.rawno));
	}
	else
	  idata.bogus = 2;

	hline = format_index_line(&idata);
    }

#if defined(PRECALCULATE_SCORES)
Instead of calculating the scores of all the visible messages, we may
defer the calculation until we actually need it in match_pattern while
looking up index line colors. That way we will only calculate those that we
need. It could be less efficient if it turns out we needed them all
afterall and we calculate them bits at a time instead of all at once.
    /*
     * Get message scores.
     */
    cnt = 0L;
    if(scores_are_used(SCOREUSE_GET) & SCOREUSE_INCOLS){
	/* see if any of the visible messages are missing a score */
	for(i=0, n = current_index_state->msg_at_top;
	    i < current_index_state->lines_per_page;
	    i++){
	    
	    if(n >= msgno && n <= mn_get_total(msgmap) &&
	       get_msg_score(stream, mn_m2raw(msgmap,n)) == SCORE_UNDEF){
		cnt++;
		break;
	    }

	    /* find next n which is visible */
	    while(++n <=  mn_get_total(msgmap) &&
		  get_lflag(stream, msgmap, n, MN_HIDE))
	      ;
	}
    }

    /* if any scores need to be calculated */
    if(cnt){
	SEARCHSET  *ss;
	HLINE_S    *h;

	/* clear sequence bits */
	for(n = 1L; n <= stream->nmsgs; n++)
	  mail_elt(stream, n)->sequence = 0;

	/* build searchset of messages we need to find the score for */
	for(cnt=0, i=0, n = current_index_state->msg_at_top;
	    i < current_index_state->lines_per_page;
	    i++){
	    
	    if(n >= msgno && n <= mn_get_total(msgmap) &&
	       get_msg_score(stream, mn_m2raw(msgmap,n)) == SCORE_UNDEF){
		mail_elt(stream,mn_m2raw(msgmap,n))->sequence = 1;
		cnt++;
	    }

	    /* find next n which is visible */
	    while(++n <=  mn_get_total(msgmap) &&
		  get_lflag(stream, msgmap, n, MN_HIDE))
	      ;
	}

	if((ss = build_searchset(stream)) != NULL){
	    calculate_some_scores(stream, ss);
	    mail_free_searchset(&ss);
	}
    }
#endif

    /*
     * Look for a color for this line (and other lines in the current
     * view). This does a SEARCH for each role which has a color until
     * it finds a match. This will be satisfied by the c-client
     * cache created by the mail_fetch_overview above if it is a header
     * search.
     */
    if(!hline->color_lookup_done){
	COLOR_PAIR *linecolor;
	long        cnt;
	SEARCHSET  *ss, *s;
	HLINE_S    *h;
	PAT_STATE  *pstate = NULL;

	if(pico_usingcolor()){
	    /* clear sequence bits */
	    for(n = 1L; n <= stream->nmsgs; n++)
	      mail_elt(stream, n)->sequence = 0;

	    for(cnt=0, i=0, n = current_index_state->msg_at_top;
		i < current_index_state->lines_per_page;
		i++){
		
		if(n >= msgno
		   && n <= mn_get_total(msgmap)
		   && !(h=get_index_cache(n))->color_lookup_done){
		    mail_elt(stream,mn_m2raw(msgmap,n))->sequence = 1;
		    cnt++;
		}

		/* find next n which is visible */
		while(++n <=  mn_get_total(msgmap) &&
		      get_lflag(stream, msgmap, n, MN_HIDE))
		  ;
	    }

	    /*
	     * Why is there a loop here? The first call to get_index_line_color
	     * will return a set of messages which match one of the roles.
	     * Then, we eliminate those messages from the search set and try
	     * again. This time we'd get past that role and into a different
	     * role. Because of that, we hang onto the state and don't reset
	     * to the first_pattern on the second and subsequent times
	     * through the loop, avoiding fruitless match_pattern calls in
	     * get_index_line_color.
	     * Before the first call, pstate should be set to NULL.
	     */
	    while(cnt > 0L){
		ss = build_searchset(stream);
		if(ss){
		    linecolor = get_index_line_color(stream, ss, &pstate);

		    /*
		     * Assign this color to all matched msgno's and
		     * turn off the sequence bit so we won't check
		     * for them again.
		     */
		    if(linecolor){
			for(s = ss; s; s = s->next){
			  for(n = s->first; n <= s->last; n++){
			    if(mail_elt(stream, n)->searched){
				cnt--;
				mail_elt(stream, n)->sequence = 0;

				/*
				 * n is the raw msgno we want to assign
				 * the color to. We need to find the
				 * sorted msgno that goes with that
				 * raw number.
				 */
				for(i = current_index_state->msg_at_top;
				    i <= mn_get_total(msgmap);
				    i++)
				  if(mn_m2raw(msgmap,i) == n)
				    break;

				if(i >= msgno){	/* it has to be */
				    h = get_index_cache(i);
				    h->color_lookup_done = 1;
				    strcpy(h->linecolor.fg, linecolor->fg);
				    strcpy(h->linecolor.bg, linecolor->bg);
				}
			    }
			  }
			}

			free_color_pair(&linecolor);
		    }
		    else{
			/* have to mark the rest of the lookups done */
			for(s = ss; s && cnt > 0; s = s->next){
			  for(n = s->first; n <= s->last && cnt > 0; n++){
			    if(mail_elt(stream, n)->sequence){
				cnt--;

				for(i = current_index_state->msg_at_top;
				    i <= mn_get_total(msgmap);
				    i++)
				  if(mn_m2raw(msgmap,i) == n)
				    break;

				if(i >= msgno){	/* it has to be */
				    h = get_index_cache(i);
				    h->color_lookup_done = 1;
				}
			    }
			  }
			}

			/* just making sure */
			cnt = 0L;
		    }

		    mail_free_searchset(&ss);
		}
		else
		  cnt = 0L;
	    }

	    hline = get_index_cache(msgno);
	}
	else
	  hline->color_lookup_done = 1;
    }

    return(hline); /* Return formatted index data */
}


/* 
 * Set the score for a message to score, which can be anything including
 * SCORE_UNDEF.
 */
void
set_msg_score(stream, rawmsgno, score)
    MAILSTREAM *stream;
    long rawmsgno;
    int  score;
{
    (void)msgno_exceptions(stream, rawmsgno, "S", &score, TRUE);
}


/*
 * Returns the score for a message. If that score is undefined the value
 * returned will be SCORE_UNDEF, so the caller has to be prepared for that.
 * The caller should calculate the undefined scores before calling this.
 */
int
get_msg_score(stream, rawmsgno)
    MAILSTREAM *stream;
    long        rawmsgno;
{
    int score;

    if(!msgno_exceptions(stream, rawmsgno, "S", &score, FALSE))
      score = SCORE_UNDEF;

    return(score);
}


/*
 * Set all the score values to undefined.
 */
void
clear_msg_scores(stream)
    MAILSTREAM *stream;
{
    int  score = SCORE_UNDEF;
    long n;

    for(n = 1L; n <= stream->nmsgs; n++)
      set_msg_score(stream, n, score);
}


SEARCHSET *
build_searchset(stream)
    MAILSTREAM *stream;
{
    long       i;
    SEARCHSET *ret_s = NULL, *s;

    for(i = 1L; i <= stream->nmsgs; i++){
	if(mail_elt(stream, i)->sequence){
	    /*
	     * Add i to searchset. First check if it can be coalesced into
	     * an existing set.
	     */
	    s = ret_s;
	    while(s){
		if(s->first == (unsigned long)(i+1)){
		    s->first--;
		    break;
		}
		else if(s->last + 1 == (unsigned long)i){
		    s->last++;
		    break;
		}

		s = s->next;
	    }

	    if(!s){	/* have to add a new range to searchset */
		s = mail_newsearchset();
		s->next = ret_s ? ret_s->next : NULL;
		if(ret_s)
		  ret_s->next = s;
		else
		  ret_s = s;

		s->first = s->last = (unsigned long)i;
	    }
	}
    }

    return(ret_s);
}


int
day_of_week(d)
    struct date *d;
{
    int m, y;

    m = d->month;
    y = d->year;
    if(m <= 2){
	m += 9;
	y--;
    }
    else
      m -= 3;	/* March is month 0 */
    
    return((d->day+2+((7+31*m)/12)+y+(y/4)+(y/400)-(y/100))%7);
}


static int daytab[2][13] = {
    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static char *day_name[] = {"Sunday","Monday","Tuesday","Wednesday",
			   "Thursday","Friday","Saturday"};

int
day_of_year(d)
    struct date *d;
{
    int i, leap, doy;

    if(d->year <= 0 || d->month < 1 || d->month > 12)
      return(-1);

    doy = d->day;
    leap = d->year%4 == 0 && d->year%100 != 0 || d->year%400 == 0;
    for(i = 1; i < d->month; i++)
      doy += daytab[leap][i];
    
    return(doy);
}


/*----------------------------------------------------------------------
   Format a string summarizing the message header for index on screen

   Args: buffer -- buffer to place formatted line
	 idata -- snot it takes to format the line

  Result: returns pointer given buffer IF entry formatted
	  else NULL if there was a problem (but the buffer is
	  still suitable for display)
 ----*/
HLINE_S *
format_index_line(idata)
    INDEXDATA_S	*idata;
{
    char          str_buf[MAXIFLDS][MAX_SCREEN_COLS+1], to_us, *field,
		 *buffer, *s_tmp, *p, *str, *newsgroups;
    int		  width, offsets_set = 0, i, j, smallest, which_array = 0;
    int           plus_off = -1, imp_off = -1, del_off = -1, ans_off = -1,
		  new_off = -1, rec_off = -1, uns_off = -1, status_offset = 0;
    long	  l;
    HLINE_S	 *hline;
    BODY	 *body = NULL;
    MESSAGECACHE *cache;
    ADDRESS      *addr, *toaddr, *ccaddr, *last_to;
    INDEX_COL_S	 *cdesc = NULL;
    struct variable *vars = ps_global->vars;

    dprint(8, (debugfile, "=== format_index_line(%ld,%ld) ===\n",
	       idata ? idata->msgno : -1, idata ? idata->rawno : -1));

    hline = get_index_cache(idata->msgno);

    /* calculate contents of the required fields */
    for(cdesc = ps_global->index_disp_format;
	cdesc->ctype != iNothing && which_array < MAXIFLDS;
	cdesc++)
      if(width = cdesc->width){
	  str	    = str_buf[which_array++];
	  str[0]    = '\0';
	  cdesc->string = str;
	  status_offset += (width + 1);
	  if(idata->bogus){
	      if(cdesc->ctype == iMessNo)
		sprintf(str, "%ld", idata->msgno);
	      else if(idata->bogus < 2 && cdesc->ctype == iSubject)
		sprintf(str, "%-*.*s", width, width,
			"[ No Message Text Available ]");
	  }
	  else
	    switch(cdesc->ctype){
	      case iStatus:
		to_us = ' ';
		if(mail_elt(idata->stream, idata->rawno)->flagged)
		  to_us = '*';		/* simple */
		else if(!IS_NEWS(idata->stream)){
		    for(addr = fetch_to(idata); addr; addr = addr->next)
		      if(address_is_us(addr, ps_global)){
			  to_us = '+';
			  break;
		      }
		  
		    if(to_us == ' ' && resent_to_us(idata))
		      to_us = '+';

		    if(to_us == ' ' && F_ON(F_MARK_FOR_CC,ps_global))
		      for(addr = fetch_cc(idata); addr; addr = addr->next)
			if(address_is_us(addr, ps_global)){
			    to_us = '-';
			    break;
			}
		}

		if(idata->bogus)
		  break;

		sprintf(str, "%c %s", to_us,
			status_string(idata->stream,
				      mail_elt(idata->stream, idata->rawno)));
		if(!offsets_set && pico_usingcolor()){
		    offsets_set = 1;
		    status_offset -= (width + 1);
		    if(str[0] == '*')
		      imp_off = status_offset;
		    else if(str[0] == '+' || str[0] == '-')
		      plus_off = status_offset;

		    if(str[2] == 'D')
		      del_off = status_offset + 2;
		    else if(str[2] == 'A')
		      ans_off = status_offset + 2;
		    else if(str[2] == 'N')
		      new_off = status_offset + 2;
		}

		break;

	      case iFStatus:
	      case iIStatus:
	      {
		  char new, answered, deleted, flagged;

		  to_us = ' ';
		  if(!IS_NEWS(idata->stream)){
		    for(addr = fetch_to(idata); addr; addr = addr->next)
		      if(address_is_us(addr, ps_global)){
			  to_us = '+';
			  break;
		      }
		  
		    if(to_us == ' ' && resent_to_us(idata))
		      to_us = '+';

		    if(to_us == ' ' && F_ON(F_MARK_FOR_CC,ps_global))
		      for(addr = fetch_cc(idata); addr; addr = addr->next)
			if(address_is_us(addr, ps_global)){
			    to_us = '-';
			    break;
			}
		  }

		  if(idata->bogus)
		    break;

		  new   = answered = deleted = flagged = ' ';
		  cache = mail_elt(idata->stream, idata->rawno);
		  if(!ps_global->nr_mode && cache->valid){
		      if(cdesc->ctype == iIStatus){
			  if(cache->recent)
			    new = cache->seen ? 'R' : 'N';
			  else if (!cache->seen)
			    new = 'U';
		      }
		      else if(!cache->seen
			      && (!IS_NEWS(idata->stream)
			          || F_ON(F_FAKE_NEW_IN_NEWS, ps_global)))
			new = 'N';

		      if(cache->answered)
			answered = 'A';

		      if(cache->deleted)
			deleted = 'D';

		      if(cache->flagged)
			flagged = '*';
		  }

		  sprintf(str, "%c %c%c%c%c", to_us, flagged, new,
			  answered, deleted);
		  if(!offsets_set && pico_usingcolor()){
		      offsets_set = 1;
		      status_offset -= (width + 1);
		      if(str[0] == '+' || str[0] == '-')
			plus_off = status_offset;

		      if(str[2] == '*')
			imp_off = status_offset + 2;

		      if(str[3] == 'N')
			new_off = status_offset + 3;
		      else if(str[3] == 'R')
			rec_off = status_offset + 3;
		      else if(str[3] == 'U')
			uns_off = status_offset + 3;

		      if(str[4] == 'A')
			ans_off = status_offset + 4;

		      if(str[5] == 'D')
			del_off = status_offset + 5;
		  }
	      }

	      break;

	      case iMessNo:
		sprintf(str, "%ld", idata->msgno);
		break;

	      case iDate:
	      case iMonAbb:
	      case iLDate:
	      case iSDate:
	      case iS1Date:
	      case iS2Date:
	      case iS3Date:
	      case iS4Date:
	      case iDateIso:
	      case iDateIsoS:
	      case iTime24:
	      case iTime12:
	      case iTimezone:
		date_str(fetch_date(idata), cdesc->ctype, 0, str);
		break;

	      case iFromTo:
	      case iFromToNotNews:
		if(!(addr = fetch_from(idata))
		     || address_is_us(addr, ps_global)){
		    if(width <= 4){
			strcpy(str, "To: ");
			str[width] = '\0';
			break;
		    }
		    else{
			if((field = ((addr = fetch_to(idata))
				     ? "To"
				     : (addr = fetch_cc(idata))
				     ? "Cc"
				     : NULL))
			   && set_index_addr(idata, field, addr, "To: ",
					     width, str))
			  break;

			if(cdesc->ctype == iFromTo &&
			   (newsgroups = fetch_newsgroups(idata)) &&
			   *newsgroups){
			    sprintf(str, "To: %-*.*s", width-4, width-4,
				    newsgroups);
			    break;
			}

			/* else fall thru to From: */
		    }
		}
		/* else fall thru to From: */

		if(idata->bogus)
		  break;

	      case iFrom:
		set_index_addr(idata, "From", fetch_from(idata),
			       NULL, width, str);
		break;

	      case iTo:
		if(((field = ((addr = fetch_to(idata))
			      ? "To"
			      : (addr = fetch_cc(idata))
			      ? "Cc"
			      : NULL))
		    && !set_index_addr(idata, field, addr, NULL, width, str))
		   || !field)
		  if(newsgroups = fetch_newsgroups(idata))
		    sprintf(str, "%-*.*s", width, width, newsgroups);

		break;

	      case iCc:
		set_index_addr(idata, "Cc", fetch_cc(idata),
			       NULL, width, str);
		break;

	      case iRecips:
		toaddr = fetch_to(idata);
		ccaddr = fetch_cc(idata);
		for(last_to = toaddr;
		    last_to && last_to->next;
		    last_to = last_to->next)
		  ;
		 
		/* point end of to list temporarily at cc list */
		if(last_to)
		  last_to->next = ccaddr;

		set_index_addr(idata, "To", toaddr, NULL, width, str);

		if(last_to)
		  last_to->next = NULL;

		break;

	      case iSender:
		if(addr = fetch_sender(idata))
		  set_index_addr(idata, "Sender", addr, NULL, width, str);

		break;

	      case iSize:
		if((l = fetch_size(idata)) < 100000)
		  sprintf(str, "(%s)", comatose(l));
		else if(l < 10000000)
		  sprintf(str, "(%sK)", comatose(l/1000));
		else
		  strcpy(str, "(BIG!)");

		break;

	      case iDescripSize:
		if(body = fetch_body(idata))
		  switch(body->type){
		    case TYPETEXT:
		    {
			cache = mail_elt(idata->stream, idata->rawno);
			if(cache->rfc822_size < 6000)
			  strcpy(str, "(short  )");
			else if(cache->rfc822_size < 25000)
			  strcpy(str, "(medium )");
			else if(cache->rfc822_size < 100000)
			  strcpy(str, "(long   )");
			else
			  strcpy(str, "(huge   )");
		    }

		    break;

		    case TYPEMULTIPART:
		      if(strucmp(body->subtype, "MIXED") == 0){
			  int x;

			  x = body->nested.part
			    ? body->nested.part->body.type
			    : TYPETEXT + 1000;
			  switch(x){
			    case TYPETEXT:
			      if(body->nested.part->body.size.bytes < 6000)
				strcpy(str, "(short+ )");
			      else if(body->nested.part->body.size.bytes
				      < 25000)
				strcpy(str, "(medium+)");
			      else if(body->nested.part->body.size.bytes
				      < 100000)
				strcpy(str, "(long+  )");
			      else
				strcpy(str, "(huge+  )");
			      break;

			    default:
			      strcpy(str, "(multi  )");
			      break;
			  }
		      }
		      else if(strucmp(body->subtype, "DIGEST") == 0)
			strcpy(str, "(digest )");
		      else if(strucmp(body->subtype, "ALTERNATIVE") == 0)
			strcpy(str, "(mul/alt)");
		      else if(strucmp(body->subtype, "PARALLEL") == 0)
			strcpy(str, "(mul/par)");
		      else
			strcpy(str, "(multi  )");

		      break;

		    case TYPEMESSAGE:
		      strcpy(str, "(message)");
		      break;

		    case TYPEAPPLICATION:
		      strcpy(str, "(applica)");
		      break;

		    case TYPEAUDIO:
		      strcpy(str, "(audio  )");
		      break;

		    case TYPEIMAGE:
		      strcpy(str, "(image  )");
		      break;

		    case TYPEVIDEO:
		      strcpy(str, "(video  )");
		      break;

		    default:
		      strcpy(str, "(other  )");
		      break;
		  }

		break;

	      case iAtt:
		str[0] = SPACE;
		str[1] = '\0';
		if((body = fetch_body(idata)) &&
		   body->type == TYPEMULTIPART &&
		   strucmp(body->subtype, "ALTERNATIVE") != 0){
		    PART *part;
		    int   atts = 0;

		    part = body->nested.part;  /* 1st part, don't count */
		    while(part && part->next && atts < 10){
			atts++;
			part = part->next;
		    }

		    if(atts > 9)
		      str[0] = '*';
		    else if(atts > 0)
		      str[0] = '0' + atts;
		}

		break;

	      case iSubject:
		p = str;
		if(ps_global->nr_mode){
		    str[0] = ' ';
		    str[1] = '\0';
		    p++;
		    width--;
		}

		if(s_tmp = fetch_subject(idata)){
		    unsigned char *tmp;
		    size_t len;
		    len = strlen(s_tmp)+1;
		    tmp = fs_get(len * sizeof(unsigned char));
		    istrncpy(p,
			     (char *) rfc1522_decode(tmp, len, s_tmp, NULL),
			     width);
		    fs_give((void **) &tmp);
		}

		break;

	      case iNews:
		if(newsgroups = fetch_newsgroups(idata))
		  strncpy(str, newsgroups, width);

		break;

	      case iNewsAndTo:
		if(newsgroups = fetch_newsgroups(idata))
		  strncpy(str, newsgroups, width);

		if((l = strlen(str)) < width){
		    if(width - l < 6)
		      strncpy(str+l, "...", width-l);
		    else{
			if(l > 0){
			    strcpy(str+l, " and ");
			    set_index_addr(idata, "To", fetch_to(idata),
					   NULL, width-l-5, str+l+5);
			    if(!str[l+5])
			      str[l] = '\0';
			}
			else
			  set_index_addr(idata, "To", fetch_to(idata),
				         NULL, width, str);
		    }
		}

		break;

	      case iToAndNews:
		set_index_addr(idata, "To", fetch_to(idata),
			       NULL, width, str);
		if((l = strlen(str)) < width &&
		   (newsgroups = fetch_newsgroups(idata))){
		    if(width - l < 6)
		      strncpy(str+l, "...", width-l);
		    else{
			if(l > 0)
			  strcpy(str+l, " and ");

			if(l > 0)
			  strncpy(str+l+5, newsgroups, width-l-5);
			else
			  strncpy(str, newsgroups, width);
		    }
		}

		break;

	      case iNewsAndRecips:
		if(newsgroups = fetch_newsgroups(idata))
		  strncpy(str, newsgroups, width);

		if((l = strlen(str)) < width){
		    if(width - l < 6)
		      strncpy(str+l, "...", width-l);
		    else{
			toaddr = fetch_to(idata);
			ccaddr = fetch_cc(idata);
			for(last_to = toaddr;
			    last_to && last_to->next;
			    last_to = last_to->next)
			  ;
			 
			/* point end of to list temporarily at cc list */
			if(last_to)
			  last_to->next = ccaddr;

			if(l > 0){
			    strcpy(str+l, " and ");
			    set_index_addr(idata, "To", toaddr,
					   NULL, width-l-5, str+l+5);
			    if(!str[l+5])
			      str[l] = '\0';
			}
			else
			  set_index_addr(idata, "To", toaddr, NULL, width, str);

			if(last_to)
			  last_to->next = NULL;
		    }
		}

		break;

	      case iRecipsAndNews:
		toaddr = fetch_to(idata);
		ccaddr = fetch_cc(idata);
		for(last_to = toaddr;
		    last_to && last_to->next;
		    last_to = last_to->next)
		  ;
		 
		/* point end of to list temporarily at cc list */
		if(last_to)
		  last_to->next = ccaddr;

		set_index_addr(idata, "To", toaddr, NULL, width, str);

		if(last_to)
		  last_to->next = NULL;

		if((l = strlen(str)) < width &&
		   (newsgroups = fetch_newsgroups(idata))){
		    if(width - l < 6)
		      strncpy(str+l, "...", width-l);
		    else{
			if(l > 0)
			  strcpy(str+l, " and ");

			if(l > 0)
			  strncpy(str+l+5, newsgroups, width-l-5);
			else
			  strncpy(str, newsgroups, width);
		    }
		}

		break;
	    }
      }

    *(p = buffer = hline->line) = '\0';

    /*--- Put them all together ---*/
    for(cdesc = ps_global->index_disp_format;
	cdesc->ctype != iNothing;
	cdesc++)
      if(width = cdesc->width){
	  /* space between columns */
	  if(p > buffer){
	      *p++ = ' ';
	      *p = '\0';
	  }

	  if(cdesc->adjustment == Left)
	    sprintf(p, "%-*.*s", width, width, cdesc->string);
	  else
	    sprintf(p, "%*.*s", width, width, cdesc->string);

	  p += width;
      }

    for(i = 0; i < OFFS; i++)
      hline->offs[i].offset = -1;
	
    /* sort the color offsets to make it easy to use */
    if(offsets_set){
	int i = 0;
	OFFCOLOR_S tmp;

	if(plus_off >= 0 && VAR_IND_PLUS_FORE_COLOR && VAR_IND_PLUS_BACK_COLOR){
	    hline->offs[i].offset = plus_off;
	    strncpy(hline->offs[i].color.fg, VAR_IND_PLUS_FORE_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i].color.fg[MAXCOLORLEN] = '\0';
	    strncpy(hline->offs[i].color.bg, VAR_IND_PLUS_BACK_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i++].color.bg[MAXCOLORLEN] = '\0';
	}

	if(imp_off >= 0 && VAR_IND_IMP_FORE_COLOR && VAR_IND_IMP_BACK_COLOR){
	    hline->offs[i].offset = imp_off;
	    strncpy(hline->offs[i].color.fg, VAR_IND_IMP_FORE_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i].color.fg[MAXCOLORLEN] = '\0';
	    strncpy(hline->offs[i].color.bg, VAR_IND_IMP_BACK_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i++].color.bg[MAXCOLORLEN] = '\0';
	}

	if(del_off >= 0 && VAR_IND_DEL_FORE_COLOR && VAR_IND_DEL_BACK_COLOR){
	    hline->offs[i].offset = del_off;
	    strncpy(hline->offs[i].color.fg, VAR_IND_DEL_FORE_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i].color.fg[MAXCOLORLEN] = '\0';
	    strncpy(hline->offs[i].color.bg, VAR_IND_DEL_BACK_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i++].color.bg[MAXCOLORLEN] = '\0';
	}

	if(ans_off >= 0 && VAR_IND_ANS_FORE_COLOR && VAR_IND_ANS_BACK_COLOR){
	    hline->offs[i].offset = ans_off;
	    strncpy(hline->offs[i].color.fg, VAR_IND_ANS_FORE_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i].color.fg[MAXCOLORLEN] = '\0';
	    strncpy(hline->offs[i].color.bg, VAR_IND_ANS_BACK_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i++].color.bg[MAXCOLORLEN] = '\0';
	}

	if(new_off >= 0 && VAR_IND_NEW_FORE_COLOR && VAR_IND_NEW_BACK_COLOR){
	    hline->offs[i].offset = new_off;
	    strncpy(hline->offs[i].color.fg, VAR_IND_NEW_FORE_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i].color.fg[MAXCOLORLEN] = '\0';
	    strncpy(hline->offs[i].color.bg, VAR_IND_NEW_BACK_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i++].color.bg[MAXCOLORLEN] = '\0';
	}

	if(uns_off >= 0 && VAR_IND_UNS_FORE_COLOR && VAR_IND_UNS_BACK_COLOR){
	    hline->offs[i].offset = uns_off;
	    strncpy(hline->offs[i].color.fg, VAR_IND_UNS_FORE_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i].color.fg[MAXCOLORLEN] = '\0';
	    strncpy(hline->offs[i].color.bg, VAR_IND_UNS_BACK_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i++].color.bg[MAXCOLORLEN] = '\0';
	}

	if(rec_off >= 0 && VAR_IND_REC_FORE_COLOR && VAR_IND_REC_BACK_COLOR){
	    hline->offs[i].offset = rec_off;
	    strncpy(hline->offs[i].color.fg, VAR_IND_REC_FORE_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i].color.fg[MAXCOLORLEN] = '\0';
	    strncpy(hline->offs[i].color.bg, VAR_IND_REC_BACK_COLOR,
		    MAXCOLORLEN);
	    hline->offs[i++].color.bg[MAXCOLORLEN] = '\0';
	}

	/* sort by offsets */
	for(j = 0; j < OFFS-1; j++){
	    /* find smallest offset which isn't negative */
	    smallest = j;
	    for(i = j+1; i < OFFS; i++){
		if(hline->offs[i].offset >= 0 &&
		   (hline->offs[i].offset < hline->offs[smallest].offset ||
		    hline->offs[smallest].offset < 0))
		  smallest = i;
	    }

	    if(smallest != j){
		/* swap */
		tmp = hline->offs[j];
		hline->offs[j] = hline->offs[smallest];
		    hline->offs[smallest] = tmp;
	    }
	}
    }

    /* Truncate it to be sure not too wide */
    buffer[min(ps_global->ttyo->screen_cols, i_cache_width())] = '\0';
    hline->id = line_hash(buffer);
    dprint(9, (debugfile, "INDEX(%p) -->%s<-- (%d), %ld>\n",
	       hline, hline->line, strlen(hline->line), hline->id));

    return(hline);
}



/*
 * Look in current mail_stream for matches for messages in the searchset
 * which match a color rule pattern. Return the color.
 * The searched bit will be set for all of the messages which match the
 * first pattern which has a match.
 *
 * Args     stream -- the mail stream
 *       searchset -- restrict attention to this set of messages
 *          pstate -- The pattern state. On the first call it will be Null.
 *                      Null means start over with a new first_pattern.
 *                      After that it will be pointing to our local PAT_STATE
 *                      so that next_pattern goes to the next one after the
 *                      ones we've already checked.
 *
 * Returns   The color that goes with the matched rule.
 */
COLOR_PAIR *
get_index_line_color(stream, searchset, pstate)
    MAILSTREAM *stream;
    SEARCHSET  *searchset;
    PAT_STATE **pstate;
{
    PAT_S           *pat = NULL;
    long             rflags = ROLE_INCOL;
    COLOR_PAIR      *color = NULL;
    static PAT_STATE localpstate;

    dprint(4, (debugfile, "get_index_line_color\n"));

    if(*pstate)
      pat = next_pattern(*pstate);
    else{
	*pstate = &localpstate;
	if(!nonempty_patterns(rflags, *pstate))
	  *pstate = NULL;

	if(*pstate)
	  pat = first_pattern(*pstate);
    }

    if(*pstate){
    
	/* Go through the possible roles one at a time until we get a match. */
	while(!color && pat){
	    if(match_pattern(pat->patgrp, stream, searchset, NULL,
			     get_msg_score))
	      color = new_color_pair(pat->action->incol->fg,
				     pat->action->incol->bg);
	    else
	      pat = next_pattern(*pstate);
	}
    }

    return(color);
}


/*
 * Calculates all of the scores for the searchset and stores them in the
 * mail elts. Careful, this function uses patterns so if the caller is using
 * patterns then the caller will probably have to reset the pattern functions.
 * That is, will have to call first_pattern again with the correct type.
 */
void
calculate_some_scores(stream, searchset)
    MAILSTREAM *stream;
    SEARCHSET  *searchset;
{
    PAT_S         *pat = NULL;
    PAT_STATE      pstate;
    char          *savebits;
    int            newscore, score;
    long           rflags = ROLE_SCORE;
    long           n, i;
    HLINE_S       *h;
    SEARCHSET     *s;

    dprint(4, (debugfile, "calculate_some_scores\n"));

    if(nonempty_patterns(rflags, &pstate)){

	/* calculate scores */
	if(searchset){
	    
	    /* this calls match_pattern which messes up searched bits */
	    savebits = (char *)fs_get((stream->nmsgs+1) * sizeof(char));
	    for(i = 1L; i <= stream->nmsgs; i++)
	      savebits[i] = mail_elt(stream, i)->searched;

	    /*
	     * First set all the scores in the searchset to zero so that they
	     * will no longer be undefined.
	     */
	    score = 0;
	    for(s = searchset; s; s = s->next)
	      for(n = s->first; n <= s->last; n++)
		set_msg_score(stream, n, score);

	    for(pat = first_pattern(&pstate);
		pat;
		pat = next_pattern(&pstate)){

		if(match_pattern(pat->patgrp, stream, searchset, NULL, NULL)){
		    newscore = pat->action->scoreval;
		    for(s = searchset; s; s = s->next)
		      for(n = s->first; n <= s->last; n++){
			if(mail_elt(stream, n)->searched){
			    if((score = get_msg_score(stream,n)) == SCORE_UNDEF)
			      score = 0;
			    
			    score += newscore;
			    set_msg_score(stream, n, score);
			}
		    }
		}
	    }

	    for(i = 1L; i <= stream->nmsgs; i++)
	      mail_elt(stream, i)->searched = savebits[i];

	    fs_give((void **)&savebits);
	}
    }
}


/*
 *
 */
int
index_in_overview(stream)
    MAILSTREAM *stream;
{
    INDEX_COL_S	 *cdesc = NULL;

    if(!(stream->mailbox && IS_REMOTE(stream->mailbox)))
      return(FALSE);			/* no point! */

    if(stream->dtb && !strcmp(stream->dtb->name, "nntp"))
      for(cdesc = ps_global->index_disp_format;
	  cdesc->ctype != iNothing;
	  cdesc++)
	switch(cdesc->ctype){
	  case iTo:			/* can't be satisfied by XOVER */
	  case iSender:			/* ... or specifically handled */
	  case iDescripSize:		/* ... in news case            */
	  case iAtt:
	    return(FALSE);

	  default :
	    break;
	}

    return(TRUE);
}



/*
 * fetch_from - called to get a the index entry's "From:" field
 */
int
resent_to_us(idata)
    INDEXDATA_S *idata;
{
    if(!idata->valid_resent_to){
	static char *fields[] = {"Resent-To", NULL};
	char *h;

	if(idata->no_fetch){
	    idata->bogus = 1;	/* don't do this */
	    return(FALSE);
	}

	if(h = pine_fetchheader_lines(idata->stream,idata->rawno,NULL,fields)){
	    idata->resent_to_us = parsed_resent_to_us(h);
	    fs_give((void **) &h);
	}

	idata->valid_resent_to = 1;
    }

    return(idata->resent_to_us);
}


int
parsed_resent_to_us(h)
    char *h;
{
    char    *p, *q;
    ADDRESS *addr = NULL;
    int	     rv = FALSE;

    if(p = strindex(h, ':')){
	for(q = ++p; q = strpbrk(q, "\015\012"); q++)
	  *q = ' ';		/* quash junk */

	rfc822_parse_adrlist(&addr, p, ps_global->maildomain);
	if(addr){
	    rv = address_is_us(addr, ps_global);
	    mail_free_address(&addr);
	}
    }

    return(rv);
}



/*
 * fetch_from - called to get a the index entry's "From:" field
 */
ADDRESS *
fetch_from(idata)
    INDEXDATA_S *idata;
{
    if(idata->no_fetch)			/* implies from is valid */
      return(idata->from);
    else if(idata->bogus)
      idata->bogus = 2;
    else{
	ENVELOPE *env;

	/* c-client call's just cache access at this point */
	if(env = mail_fetchenvelope(idata->stream, idata->rawno))
	  return(env->from);

	idata->bogus = 1;
    }

    return(NULL);
}


/*
 * fetch_to - called to get a the index entry's "To:" field
 */
ADDRESS *
fetch_to(idata)
    INDEXDATA_S *idata;
{
    if(idata->no_fetch){		/* check for specific validity */
	if(idata->valid_to)
	  return(idata->to);
	else
	  idata->bogus = 1;		/* can't give 'em what they want */
    }
    else if(idata->bogus){
	idata->bogus = 2;		/* elevate bogosity */
    }
    else{
	ENVELOPE *env;

	/* c-client call's just cache access at this point */
	if(env = mail_fetchenvelope(idata->stream, idata->rawno))
	  return(env->to);

	idata->bogus = 1;
    }

    return(NULL);
}


/*
 * fetch_cc - called to get a the index entry's "Cc:" field
 */
ADDRESS *
fetch_cc(idata)
    INDEXDATA_S *idata;
{
    if(idata->no_fetch){		/* check for specific validity */
	if(idata->valid_cc)
	  return(idata->cc);
	else
	  idata->bogus = 1;		/* can't give 'em what they want */
    }
    else if(idata->bogus){
	idata->bogus = 2;		/* elevate bogosity */
    }
    else{
	ENVELOPE *env;

	/* c-client call's just cache access at this point */
	if(env = mail_fetchenvelope(idata->stream, idata->rawno))
	  return(env->cc);

	idata->bogus = 1;
    }

    return(NULL);
}



/*
 * fetch_sender - called to get a the index entry's "Sender:" field
 */
ADDRESS *
fetch_sender(idata)
    INDEXDATA_S *idata;
{
    if(idata->no_fetch){		/* check for specific validity */
	if(idata->valid_sender)
	  return(idata->sender);
	else
	  idata->bogus = 1;		/* can't give 'em what they want */
    }
    else if(idata->bogus){
	idata->bogus = 2;		/* elevate bogosity */
    }
    else{
	ENVELOPE *env;

	/* c-client call's just cache access at this point */
	if(env = mail_fetchenvelope(idata->stream, idata->rawno))
	  return(env->sender);

	idata->bogus = 1;
    }

    return(NULL);
}


/*
 * fetch_newsgroups - called to get a the index entry's "Newsgroups:" field
 */
char *
fetch_newsgroups(idata)
    INDEXDATA_S *idata;
{
    if(idata->no_fetch){		/* check for specific validity */
	if(idata->valid_news)
	  return(idata->newsgroups);
	else
	  idata->bogus = 1;		/* can't give 'em what they want */
    }
    else if(idata->bogus){
	idata->bogus = 2;		/* elevate bogosity */
    }
    else{
	ENVELOPE *env;

	/* c-client call's just cache access at this point */
	if(env = mail_fetchenvelope(idata->stream, idata->rawno))
	  return(env->newsgroups);

	idata->bogus = 1;
    }

    return(NULL);
}


/*
 * fetch_subject - called to get at the index entry's "Subject:" field
 */
char *
fetch_subject(idata)
    INDEXDATA_S *idata;
{
    if(idata->no_fetch)			/* implies subject is valid */
      return(idata->subject);
    else if(idata->bogus)
      idata->bogus = 2;
    else{
	ENVELOPE *env;

	/* c-client call's just cache access at this point */
	if(env = mail_fetchenvelope(idata->stream, idata->rawno))
	  return(env->subject);

	idata->bogus = 1;
    }

    return(NULL);
}


/*
 * fetch_date - called to get at the index entry's "Date:" field
 */
char *
fetch_date(idata)
    INDEXDATA_S *idata;
{
    if(idata->no_fetch)			/* implies date is valid */
      return(idata->date);
    else if(idata->bogus)
      idata->bogus = 2;
    else{
	ENVELOPE *env;

	/* c-client call's just cache access at this point */
	if(env = mail_fetchenvelope(idata->stream, idata->rawno))
	  return(env->date);

	idata->bogus = 1;
    }

    return(NULL);
}


/*
 * fetch_size - called to get at the index entry's "size" field
 */
long
fetch_size(idata)
    INDEXDATA_S *idata;
{
    if(idata->no_fetch)			/* implies size is valid */
      return(idata->size);
    else if(idata->bogus)
      idata->bogus = 2;
    else{
	MESSAGECACHE *mc;

	if(mc = mail_elt(idata->stream, idata->rawno))
	  return(mc->rfc822_size);

	idata->bogus = 1;
    }

    return(0L);
}


/*
 * fetch_body - called to get a the index entry's body structure
 */
BODY *
fetch_body(idata)
    INDEXDATA_S *idata;
{
    BODY *body;
    
    if(idata->bogus || idata->no_fetch){
	idata->bogus = 2;
	return(NULL);
    }

    if(mail_fetchstructure(idata->stream, idata->rawno, &body))
      return(body);

    idata->bogus = 1;
    return(NULL);
}


/*
 * load_overview - c-client call back to gather overview data
 *
 * Note: if we never get called, UID represents a hole
 *       if we're passed a zero UID, totally bogus overview data
 *       if we're passed a zero obuf, mostly bogus overview data
 */
void
load_overview(stream, uid, obuf)
    MAILSTREAM	  *stream;
    unsigned long  uid;
    OVERVIEW	  *obuf;
{
    if(obuf && uid){
	INDEXDATA_S  idata;
	HLINE_S	    *hline;

	memset(&idata, 0, sizeof(INDEXDATA_S));
	idata.no_fetch = 1;

	/*
	 * Only really load the thing if we've got an NNTP stream
	 * otherwise we're just using mail_fetch_overview to load the
	 * IMAP envelope cache with the specific set of messages
	 * in a single RTT.
	 */
	idata.stream  = stream;
	idata.uid     = uid;
	idata.rawno   = mail_msgno(stream, uid);
	idata.msgno   = mn_raw2m(ps_global->msgmap, idata.rawno);
	idata.size    = obuf->optional.octets;
	idata.from    = obuf->from;
	idata.date    = obuf->date;
	idata.subject = obuf->subject;

	hline = format_index_line(&idata);
	if(idata.bogus)
	  hline->line[0] = '\0';
	else if(F_OFF(F_QUELL_NEWS_ENV_CB, ps_global))
	  paint_index_hline(stream, idata.msgno, hline);
    }
}


/*
 * s is at least size width+1
 */
int
set_index_addr(idata, field, addr, prefix, width, s)
    INDEXDATA_S *idata;
    char       *field;
    ADDRESS    *addr;
    char       *prefix;
    int		width;
    char       *s;
{
    ADDRESS *atmp;

    for(atmp = addr; idata->stream && atmp; atmp = atmp->next)
      if(atmp->host && atmp->host[0] == '.'){
	  char *p, *h, *fields[2];
	  
	  if(idata->no_fetch){
	      idata->bogus = 1;
	      break;
	  }

	  fields[0] = field;
	  fields[1] = NULL;
	  if(h = pine_fetchheader_lines(idata->stream, idata->rawno,
					NULL, fields)){
	      /* skip "field:" */
	      for(p = h + strlen(field) + 1;
		  *p && isspace((unsigned char)*p); p++)
		;

	      while(width--)
		if(*p == '\015' || *p == '\012')
		  p++;				/* skip CR LF */
		else if(!*p)
		  *s++ = ' ';
		else
		  *s++ = *p++;

	      *s = '\0';			/* tie off return string */
	      fs_give((void **) &h);
	      return(TRUE);
	  }
	  /* else fall thru and display what c-client gave us */
      }

    if(addr && !addr->next		/* only one address */
       && addr->host			/* not group syntax */
       && addr->personal){		/* there is a personal name */
	char *dummy = NULL;
	char  buftmp[MAILTMPLEN];
	int   l;

	if(l = prefix ? strlen(prefix) : 0)
	  strcpy(s, prefix);

	sprintf(buftmp, "%.75s", addr->personal);
	istrncpy(s + l,
		 (char *) rfc1522_decode((unsigned char *)tmp_20k_buf,
					 SIZEOF_20KBUF, buftmp, &dummy),
		 width - l);
	if(dummy)
	  fs_give((void **)&dummy);

	return(TRUE);
    }
    else if(addr){
	char *a_string;
	int   l;

	a_string = addr_list_string(addr, NULL, 0, 0);
	if(l = prefix ? strlen(prefix) : 0)
	  strcpy(s, prefix);

	istrncpy(s + l, a_string, width - l);

	fs_give((void **)&a_string);
	return(TRUE);
    }

    return(FALSE);
}


void
index_data_env(idata, env)
    INDEXDATA_S *idata;
    ENVELOPE	*env;
{
    if(!env){
	idata->bogus = 2;
	return;
    }

    idata->from	      = env->from;
    idata->to	      = env->to;
    idata->cc	      = env->cc;
    idata->sender     = env->sender;
    idata->subject    = env->subject;
    idata->date	      = env->date;
    idata->newsgroups = env->newsgroups;

    idata->valid_to = 1;	/* signal that everythings here */
    idata->valid_cc = 1;
    idata->valid_sender = 1;
    idata->valid_news = 1;
}


/*
 * Put a string representing the date into str. The source date is
 * in the string datesrc. The format to be used is in type.
 * Notice that type is an IndexColType, but really only a subset of
 * IndexColType types are allowed.
 *
 * Args  datesrc -- The source date string
 *          type -- What type of output we want
 *             v -- If set, variable width output is ok. (Oct 9 not Oct  9)
 *           str -- Put the answer here.
 */
void
date_str(datesrc, type, v, str)
    char        *datesrc;
    IndexColType type;
    int          v;
    char         str[];
{
    char	year4[5],	/* 4 digit year			*/
		yearzero[3],	/* zero padded, 2-digit year	*/
		monzero[3],	/* zero padded, 2-digit month	*/
		dayzero[3],	/* zero padded, 2-digit day	*/
		day[3],		/* 1 or 2-digit day, no pad	*/
		dayord[3],	/* 2-letter ordinal label	*/
		monabb[4],	/* 3-letter month abbrev	*/
		hour24[3],	/* 2-digit, 24 hour clock hour	*/
		hour12[3],	/* 12 hour clock hour, no pad	*/
		minzero[3],	/* zero padded, 2-digit minutes */
		timezone[6];	/* timezone, like -0800 or +... */
    int		hr12;
    int         curtype;
    struct	date d;

    curtype = (type == iCurDate ||
	       type == iCurDateIso ||
	       type == iCurDateIsoS ||
	       type == iCurTime24 ||
	       type == iCurTime12);
    str[0] = '\0';
    if(!(datesrc && datesrc[0]) && !curtype)
      return;

    if(curtype){
	char dbuf[200];

	rfc822_date(dbuf);
	parse_date(dbuf, &d);
    }
    else
      parse_date(datesrc, &d);

    switch(type){
      case iSDate:
      case iLDate:
      case iYear:
      case iRDate:
	strcpy(year4, (d.year >= 0 && d.year < 10000)
			? int2string(d.year) : "");
	/* fall through */
      case iDate:
      case iCurDate:
      case iMonAbb:
      case iDay:
      case iDayOrdinal:
	strcpy(monabb, (d.month > 0 && d.month < 13)
			? month_abbrev(d.month) : "");
	strcpy(day, (d.day > 0 && d.day < 32)
			? int2string(d.day) : "");
	strcpy(dayord,
	       (d.day <= 0 || d.day > 31) ? "" :
	        (d.day == 1 || d.day == 21 || d.day == 31) ? "st" :
		 (d.day == 2 || d.day == 22 ) ? "nd" :
		  (d.day == 3 || d.day == 23 ) ? "rd" : "th");
	break;

      case iDateIso:
      case iCurDateIso:
	strcpy(year4, (d.year >= 0 && d.year < 10000)
			? int2string(d.year) : "");
	/* fall through */

      case iS1Date:
      case iS2Date:
      case iS3Date:
      case iS4Date:
      case iDateIsoS:
      case iCurDateIsoS:
      case iDay2Digit:
      case iMon2Digit:
      case iYear2Digit:
	if(d.year >= 0){
	    if((d.year % 100) < 10){
		yearzero[0] = '0';
		strcpy(yearzero+1, int2string(d.year % 100));
	    }
	    else
	      strcpy(yearzero, int2string(d.year % 100));
	}
	else
	  strcpy(yearzero, "??");

	if(d.month > 0 && d.month < 10){
	    monzero[0] = '0';
	    strcpy(monzero+1, int2string(d.month));
	}
	else if(d.month >= 10 && d.month <= 12)
	  strcpy(monzero, int2string(d.month));
	else
	  strcpy(monzero, "??");

	if(d.day > 0 && d.day < 10){
	    dayzero[0] = '0';
	    strcpy(dayzero+1, int2string(d.day));
	}
	else if(d.day >= 10 && d.day <= 31)
	  strcpy(dayzero, int2string(d.day));
	else
	  strcpy(dayzero, "??");
	break;

      case iTime24:
      case iTime12:
      case iCurTime24:
      case iCurTime12:
	hr12 = (d.hour == 0) ? 12 :
		 (d.hour > 12) ? (d.hour - 12) : d.hour;
	hour12[0] = '\0';
	if(hr12 > 0 && hr12 <= 12)
	  strcpy(hour12, int2string(hr12));

	hour24[0] = '\0';
	if(d.hour >= 0 && d.hour < 10){
	    hour24[0] = '0';
	    strcpy(hour24+1, int2string(d.hour));
	}
	else if(d.hour >= 10 && d.hour < 24)
	  strcpy(hour24, int2string(d.hour));

	minzero[0] = '\0';
	if(d.minute >= 0 && d.minute < 10){
	    minzero[0] = '0';
	    strcpy(minzero+1, int2string(d.minute));
	}
	else if(d.minute >= 10 && d.minute <= 60)
	  strcpy(minzero, int2string(d.minute));
	
	break;

      case iTimezone:
	if(d.hours_off_gmt <= 0){
	    timezone[0] = '-';
	    d.hours_off_gmt *= -1;
	    d.min_off_gmt *= -1;
	}
	else
	  timezone[0] = '+';

	timezone[1] = '\0';
	if(d.hours_off_gmt >= 0 && d.hours_off_gmt < 10){
	    timezone[1] = '0';
	    strcpy(timezone+2, int2string(d.hours_off_gmt));
	}
	else if(d.hours_off_gmt >= 10 && d.hours_off_gmt < 24)
	  strcpy(timezone+1, int2string(d.hours_off_gmt));
	else{
	    timezone[1] = '0';
	    timezone[2] = '0';
	}

	timezone[3] = '\0';
	if(d.min_off_gmt >= 0 && d.min_off_gmt < 10){
	    timezone[3] = '0';
	    strcpy(timezone+4, int2string(d.min_off_gmt));
	}
	else if(d.min_off_gmt >= 10 && d.min_off_gmt <= 60)
	  strcpy(timezone+3, int2string(d.min_off_gmt));
	else{
	    timezone[3] = '0';
	    timezone[4] = '0';
	}

	timezone[5] = '\0';

	break;
    }

    switch(type){
      case iRDate:
	sprintf(str, "%s%s%s %s %s",
		(d.wkday != -1) ? week_abbrev(d.wkday) : "",
		(d.wkday != -1) ? ", " : "",
		day, monabb, year4);
	break;
      case iYear:
	strcpy(str, year4);
	break;
      case iDay2Digit:
	strcpy(str, dayzero);
	break;
      case iMon2Digit:
	strcpy(str, monzero);
	break;
      case iYear2Digit:
	strcpy(str, yearzero);
	break;
      case iTimezone:
	strcpy(str, timezone);
	break;
      case iDay:
	strcpy(str, day);
	break;
      case iDayOrdinal:
        sprintf(str, "%s%s", day, dayord);
	break;
      case iMon:
	if(d.month > 0 && d.month <= 12)
	  strcpy(str, int2string(d.month));

	break;
      case iMonAbb:
	strcpy(str, monabb);
	break;
      case iMonLong:
	strcpy(str, (d.month > 0 && d.month < 13)
			? month_name(d.month) : "");
	break;
      case iDate:
      case iCurDate:
	if(v)
	  sprintf(str, "%s%s%s", monabb, (monabb[0] && day[0]) ? " " : "", day);
	else
	  sprintf(str, "%3s %2s", monabb, day);

	break;
      case iLDate:
	if(v)
	  sprintf(str, "%s%s%s%s%s", monabb,
	          (monabb[0] && day[0]) ? " " : "", day,
	          ((monabb[0] || day[0]) && year4[0]) ? ", " : "",
		  year4);
	else
	  sprintf(str, "%3s %2s%c %4s", monabb, day,
		  (monabb[0] && day[0] && year4[0]) ? ',' : ' ',
		  year4);
	break;
      case iS1Date:
      case iS2Date:
      case iS3Date:
      case iS4Date:
      case iDateIso:
      case iDateIsoS:
      case iCurDateIso:
      case iCurDateIsoS:
	if(monzero[0] == '?' && dayzero[0] == '?' &&
	   yearzero[0] == '?')
	  sprintf(str, "%8s", "");
	else{
	    switch(type){
	      case iS1Date:
		sprintf(str, "%2s/%2s/%2s",
			monzero, dayzero, yearzero);
		break;
	      case iS2Date:
		sprintf(str, "%2s/%2s/%2s",
			dayzero, monzero, yearzero);
		break;
	      case iS3Date:
		sprintf(str, "%2s.%2s.%2s",
			dayzero, monzero, yearzero);
		break;
	      case iS4Date:
		sprintf(str, "%2s.%2s.%2s",
			yearzero, monzero, dayzero);
		break;
	      case iDateIsoS:
	      case iCurDateIsoS:
		sprintf(str, "%2s-%2s-%2s",
			yearzero, monzero, dayzero);
		break;
	      case iDateIso:
	      case iCurDateIso:
		sprintf(str, "%4s-%2s-%2s",
			year4, monzero, dayzero);
		break;
	    }
	}

	break;
      case iTime24:
      case iCurTime24:
	sprintf(str, "%2s%c%2s",
		(hour24[0] && minzero[0]) ? hour24 : "",
		(hour24[0] && minzero[0]) ? ':' : ' ',
		(hour24[0] && minzero[0]) ? minzero : "");
	break;
      case iTime12:
      case iCurTime12:
	sprintf(str, "%s%c%2s%s",
		(hour12[0] && minzero[0]) ? hour12 : "",
		(hour12[0] && minzero[0]) ? ':' : ' ',
		(hour12[0] && minzero[0]) ? minzero : "",
		(hour12[0] && minzero[0] && d.hour < 12) ? "am" :
		  (hour12[0] && minzero[0] && d.hour >= 12) ? "pm" :
		    "  ");
	break;
      case iSDate:
	{ struct date now, last_day;
	  char        dbuf[200];
	  int         msg_day_of_year, now_day_of_year, today;
	  int         diff, ydiff, last_day_of_year;

	  rfc822_date(dbuf);
	  parse_date(dbuf, &now);
	  today = day_of_week(&now) + 7;

	  if(today >= 0+7 && today <= 6+7){
	      now_day_of_year = day_of_year(&now);
	      msg_day_of_year = day_of_year(&d);
	      ydiff = now.year - d.year;

	      if(ydiff == 0)
		diff = now_day_of_year - msg_day_of_year;
	      else if(ydiff == 1){
		  last_day = d;
		  last_day.month = 12;
		  last_day.day = 31;
		  last_day_of_year = day_of_year(&last_day);

		  diff = now_day_of_year +
			  (last_day_of_year - msg_day_of_year);
	      }
	      else if(ydiff == -1){
		  last_day = now;
		  last_day.month = 12;
		  last_day.day = 31;
		  last_day_of_year = day_of_year(&last_day);

		  diff = -1 * (msg_day_of_year +
			  (last_day_of_year - now_day_of_year));
	      }
	      else
		diff = -100;

	      if(ydiff > 1){
		  if(v)
		    sprintf(str, "%s%s%s", monabb,
			    (monabb[0] && year4[0]) ? " " : "",
			    year4);
		  else
		    sprintf(str, "%3s %4s", monabb, year4);
	      }
	      else if(ydiff < -1){
		  if(v)
		    sprintf(str, "%s%s%s%s", monabb,
			    (monabb[0] && year4[0]) ? " " : "",
			    year4,
			    (monabb[0] && year4[0]) ? "!" : "");
		  else
		    sprintf(str, "%3s %4s!", monabb, year4);
	      }
	      else if(diff == 0)
		strcpy(str, "Today");
	      else if(diff == 1)
		strcpy(str, "Yesterday");
	      else if(diff > 1 && diff < 6)
		sprintf(str, "%s", day_name[(today - diff) % 7]);
	      else if(diff == -1)
		strcpy(str, "Tomorrow");
	      else if(diff < -1 && diff > -6)
		sprintf(str, "Next %.3s!",
			day_name[(today - diff) % 7]);
	      else if(diff >= 6 &&
		      (ydiff == 0 &&
		       (now.month - d.month > 6 ) ||
		      (ydiff == 1 &&
		       12 + now.month - d.month > 6))){
		  if(v)
		    sprintf(str, "%s%s%s", monabb,
			    (monabb[0] && year4[0]) ? " " : "",
			    year4);
		  else
		    sprintf(str, "%3s %4s", monabb, year4);
	      }
	      else if(diff <= -6 &&
		      (ydiff == 0 &&
		       (now.month - d.month < 0)) ||
		       ydiff < 0){
		  if(v)
		    sprintf(str, "%s%s%s%s", monabb,
			    (monabb[0] && year4[0]) ? " " : "",
			    year4,
			    (monabb[0] && year4[0]) ? "!" : "");
		  else
		    sprintf(str, "%3s %4s!", monabb, year4);
	      }
	      else if(diff <= -6){
		  if(v)
		    sprintf(str, "%s%s%s%s", monabb,
			    (monabb[0] && day[0]) ? " " : "",
			    day,
			    (monabb[0] && day[0]) ? "!" : "");
		  else
		    sprintf(str, "%3s %2s!", monabb, day);
	      }
	      else{
		  if(v)
		    sprintf(str, "%s%s%s", monabb,
			    (monabb[0] && day[0]) ? " " : "", day);
		  else
		    sprintf(str, "%3s %2s", monabb, day);
	      }
	  }
	  else{
	      if(v)
		sprintf(str, "%s%s%s", monabb,
			(monabb[0] && day[0]) ? " " : "", day);
	      else
		sprintf(str, "%3s %2s", monabb, day);
	  }
	}
    }
}


long
line_hash(s)
     char *s;
{
    register long xsum = 0L;

    if(s)
      while(*s)
	xsum = ((((xsum << 4) & 0xffffffff)+(xsum >> 24)) & 0x0fffffff) + *s++;

    return(xsum ? xsum : 1L);
}


/*----------------------------------------------------------------------
    Print current folder index

  ---*/
int
print_index(state, msgmap, agg)
    struct pine *state;
    MSGNO_S     *msgmap;
    int		 agg;
{
    long i;

    for(i = 1L; i <= mn_get_total(msgmap); i++){
	if(agg && !get_lflag(state->mail_stream, msgmap, i, MN_SLCT))
	  continue;
	
	if(!agg && get_lflag(state->mail_stream, msgmap, i, MN_HIDE))
	  continue;

	if(!print_char((mn_is_cur(msgmap, i)) ? '>' : ' ')
	   || !gf_puts(build_header_line(state, state->mail_stream,
					 msgmap, i)->line + 1, print_char)
	   || !gf_puts(NEWLINE, print_char))
	  return(0);
    }

    return(1);
}



/*----------------------------------------------------------------------
      Search the message headers as display in index
 
  Args: command_line -- The screen line to prompt on
        msg          -- The current message number to start searching at
        max_msg      -- The largest message number in the current folder

  The headers are searched exactly as they are displayed on the screen. The
search will wrap around to the beginning if not string is not found right 
away.
  ----*/
void
index_search(state, stream, command_line, msgmap)
    struct pine *state;
    MAILSTREAM  *stream;
    int          command_line;
    MSGNO_S     *msgmap;
{
    int         rc, select_all = 0, flags;
    long        i, sorted_msg, selected = 0L;
    char        prompt[MAX_SEARCH+50], new_string[MAX_SEARCH+1];
    HelpType	help;
    static char search_string[MAX_SEARCH+1] = { '\0' };
    static ESCKEY_S header_search_key[] = { {0, 0, NULL, NULL },
					    {ctrl('Y'), 10, "^Y", "First Msg"},
					    {ctrl('V'), 11, "^V", "Last Msg"},
					    {-1, 0, NULL, NULL} };

    dprint(4, (debugfile, "\n - search headers - \n"));

    if(!any_messages(msgmap, NULL, "to search")){
	return;
    }
    else if(mn_total_cur(msgmap) > 1L){
	q_status_message1(SM_ORDER, 0, 2, "%s msgs selected; Can't search",
			  comatose(mn_total_cur(msgmap)));
	return;
    }
    else
      sorted_msg = mn_get_cur(msgmap);

    help = NO_HELP;
    new_string[0] = '\0';

    while(1) {
	sprintf(prompt, "Word to search for [%.*s] : ", sizeof(prompt)-50,
		search_string);

	if(F_ON(F_ENABLE_AGG_OPS, ps_global)){
	    header_search_key[0].ch    = ctrl('X');
	    header_search_key[0].rval  = 12;
	    header_search_key[0].name  = "^X";
	    header_search_key[0].label = "Select Matches";
	}
	else{
	    header_search_key[0].ch   = header_search_key[0].rval  = 0;
	    header_search_key[0].name = header_search_key[0].label = NULL;
	}
	
	flags = OE_APPEND_CURRENT | OE_KEEP_TRAILING_SPACE;
	
        rc = optionally_enter(new_string, command_line, 0, sizeof(new_string),
			      prompt, header_search_key, help, &flags);

        if(rc == 3) {
	    help = (help != NO_HELP) ? NO_HELP :
		     F_ON(F_ENABLE_AGG_OPS, ps_global) ? h_os_index_whereis_agg
		       : h_os_index_whereis;
            continue;
        }
	else if(rc == 10){
	    q_status_message(SM_ORDER, 0, 3, "Searched to First Message.");
	    if(any_lflagged(msgmap, MN_HIDE)){
		do{
		    selected = sorted_msg;
		    mn_dec_cur(stream, msgmap);
		    sorted_msg = mn_get_cur(msgmap);
		}
		while(selected != sorted_msg);
	    }
	    else
	      sorted_msg = (mn_get_total(msgmap) > 0L) ? 1L : 0L;

	    mn_set_cur(msgmap, sorted_msg);
	    return;
	}
	else if(rc == 11){
	    q_status_message(SM_ORDER, 0, 3, "Searched to Last Message.");
	    if(any_lflagged(msgmap, MN_HIDE)){
		do{
		    selected = sorted_msg;
		    mn_inc_cur(stream, msgmap);
		    sorted_msg = mn_get_cur(msgmap);
		}
		while(selected != sorted_msg);
	    }
	    else
	      sorted_msg = mn_get_total(msgmap);

	    mn_set_cur(msgmap, sorted_msg);
	    return;
	}
	else if(rc == 12){
	    select_all = 1;
	    break;
	}

        if(rc != 4)			/* redraw */
          break; /* redraw */
    }

    if(rc == 1 || (new_string[0] == '\0' && search_string[0] == '\0')) {
	cmd_cancelled("Search");
        return;
    }

    if(new_string[0] == '\0'){
	strncpy(new_string, search_string, sizeof(new_string));
	new_string[sizeof(new_string)-1] = '\0';
    }

    strncpy(search_string, new_string, sizeof(search_string));
    search_string[sizeof(search_string)-1] = '\0';

#ifndef	DOS
    intr_handling_on();
#endif

    for(i = sorted_msg + ((select_all)?0:1);
	i <= mn_get_total(msgmap) && !ps_global->intr_pending;
	i++)
      if(!get_lflag(stream, msgmap, i, MN_HIDE) &&
	 srchstr(build_header_line(state, stream, msgmap, i)->line,
		 search_string)){
	  selected++;
	  if(select_all)
	    set_lflag(stream, msgmap, i, MN_SLCT, 1);
	  else
	    break;
      }

    if(i > mn_get_total(msgmap))
      for(i = 1; i < sorted_msg && !ps_global->intr_pending; i++)
	if(!get_lflag(stream, msgmap, i, MN_HIDE) &&
	   srchstr(build_header_line(state, stream, msgmap, i)->line,
		   search_string)){
	    selected++;
	    if(select_all)
	      set_lflag(stream, msgmap, i, MN_SLCT, 1);
	    else
	      break;
	}

    if(ps_global->intr_pending){
	q_status_message1(SM_ORDER, 0, 3, "Search cancelled.%s",
			  select_all ? " Selected set may be incomplete.":"");
    }
    else if(select_all){
	q_status_message1(SM_ORDER, 0, 3, "%s messages found matching word",
			  long2string(selected));
    }
    else if(selected){
	q_status_message1(SM_ORDER, 0, 3, "Word found%s",
			  (i <= sorted_msg)
			    ? ". Search wrapped to beginning" : "");
	mn_set_cur(msgmap, i);
    }
    else
      q_status_message(SM_ORDER, 0, 3, "Word not found");

#ifndef	DOS
    intr_handling_off();
#endif
}


/*
 * Return value for use by progress bar.
 */
int
percent_sorted()
{
    /*
     * C-client's sort routine exports two types of status
     * indicators.  One's the progress thru loading the cache (typically
     * the elephantine bulk of the delay, and the progress thru the
     * actual sort (typically qsort).  Our job is to balance the two
     */
    
    return(g_sort.prog && g_sort.prog->nmsgs
      ? (((((g_sort.prog->progress.cached * 100)
					 / g_sort.prog->nmsgs) * 9) / 10)
	 + (((g_sort.prog->progress.sorted) * 10)
					 / g_sort.prog->nmsgs))
      : 0);
}


/*----------------------------------------------------------------------
  Compare raw message numbers 
 ----*/
int
pine_compare_long(a, b)
    const QSType *a, *b;
{
    long *mess_a = (long *)a, *mess_b = (long *)b, mdiff;

    return((mdiff = *mess_a - *mess_b) ? ((mdiff > 0L) ? 1 : -1) : 0);
}

/*
 * reverse version of pine_compare_long
 */
int
pine_compare_long_rev(a, b)
    const QSType *a, *b;
{
    long *mess_a = (long *)a, *mess_b = (long *)b, mdiff;

    return((mdiff = *mess_a - *mess_b) ? ((mdiff < 0L) ? 1 : -1) : 0);
}


int *g_score_arr;

/*
 * This calculate all of the scores and also puts them into a temporary array
 * for speed when sorting.
 */
void
build_score_array(stream, msgmap)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
{
    SEARCHSET *searchset;
    long       msgno, cnt, nmsgs, rawmsgno;
    int        score;
    HLINE_S   *h;

    nmsgs = mn_get_total(msgmap);
    g_score_arr = (int *)fs_get((nmsgs+1) * sizeof(int));
    memset(g_score_arr, 0, (nmsgs+1) * sizeof(int));

    /*
     * Build a searchset that contains everything except those that have
     * already been looked up.
     */

    for(msgno=1L; msgno <= stream->nmsgs; msgno++)
      mail_elt(stream, msgno)->sequence = 0;

    for(cnt=0L, msgno=1L; msgno <= nmsgs; msgno++){
	if(get_msg_score(stream, mn_m2raw(msgmap, msgno)) == SCORE_UNDEF){
	    mail_elt(stream, mn_m2raw(msgmap, msgno))->sequence = 1;
	    cnt++;
	}
    }

    if(cnt){
	searchset = build_searchset(stream);
	calculate_some_scores(stream, searchset);
	mail_free_searchset(&searchset);
    }

    /*
     * Copy scores to g_score_arr. They should all be defined now but if
     * they aren't assign score zero.
     */
    for(rawmsgno = 1L; rawmsgno <= nmsgs; rawmsgno++){
	score = get_msg_score(stream, rawmsgno);
	g_score_arr[rawmsgno] = (score == SCORE_UNDEF) ? 0 : score;
    }
}


void
free_score_array()
{
    if(g_score_arr)
      fs_give((void *)&g_score_arr);
}


/*----------------------------------------------------------------------
  Compare scores
 ----*/
int
pine_compare_scores(a, b)
    const QSType *a, *b;
{
    long *mess_a = (long *)a, *mess_b = (long *)b, mdiff;
    int   sdiff;

    return((sdiff = g_score_arr[*mess_a] - g_score_arr[*mess_b])
	    ? ((sdiff > 0) ? 1 : -1)
	    : ((mdiff = *mess_a - *mess_b) ? ((mdiff > 0) ? 1 : -1) : 0));
}


/*----------------------------------------------------------------------
    Sort the current folder into the order set in the msgmap

Args: msgmap --
      new_sort --
      new_rev -- 

    The idea of the deferred sort is to let the user interrupt a long sort
    and have a chance to do a different command, such as a sort by arrival
    or a Goto.  The next newmail call will increment the deferred variable,
    then the user may do a command, then the newmail call after that
    causes the sort to happen if it is still needed.
  ----*/
void
sort_folder(msgmap, new_sort, new_rev, verbose)
    MSGNO_S   *msgmap;
    SortOrder  new_sort;
    int	       new_rev, verbose;
{
    long	   raw_current, i, j;
    unsigned long *sort = NULL;
    int		   we_cancel = 0;
    char	   sort_msg[MAX_SCREEN_COLS+1];

    dprint(2, (debugfile, "Sorting by %s%s\n",
	       sort_name(new_sort), new_rev ? "/reverse" : ""));

    if(mn_get_total(msgmap) <= 1L){
	mn_set_sort(msgmap, new_sort);
	mn_set_revsort(msgmap, new_rev);
	return;
    }

    raw_current = mn_m2raw(msgmap, mn_get_cur(msgmap));

    if(new_sort == SortArrival){
	/*
	 * NOTE: RE c-client sorting, our idea of arrival is really
	 *	 just the natural sequence order.  C-client, and probably
	 *	 rightly so, considers "arrival" the order based on the
	 *	 message's internal date.  This is more costly to compute
	 *	 since it means touching the message store (say "nntp"?),
	 *	 so we just preempt it here.
	 *
	 *	 Someday c-client will support "unsorted" and do what
	 *	 we're doing here.  That day this gets scrapped.
	 */

	if(mn_get_sort(msgmap) != new_sort ||
	   mn_get_revsort(msgmap) != new_rev ||
	   any_lflagged(ps_global->msgmap, MN_EXLD))
	  clear_index_cache();

	if(any_lflagged(ps_global->msgmap, MN_EXLD)){
	    /*
	     * BEWARE: "exclusion" may leave holes in the unsorted sort order
	     * so we have to do a real sort if that is the case.
	     */
	    qsort(msgmap->sort+1, (size_t) mn_get_total(msgmap),
		  sizeof(long),
		  new_rev ? pine_compare_long_rev : pine_compare_long);
	}
	else if(mn_get_total(msgmap) > 0L){
	    if(new_rev){
		clear_index_cache();
		for(i = 1L, j = mn_get_total(msgmap); j >= 1; i++, j--)
		  msgmap->sort[i] = j;
	    }
	    else
	      for(i = 1L; i <= mn_get_total(msgmap); i++)
		msgmap->sort[i] = i;
	}
    }
    else if(new_sort == SortScore){

	/*
	 * We have to build a temporary array which maps raw msgno to
	 * score. We use the index cache machinery to build the array.
	 */
	
#ifndef	DOS
	intr_handling_on();
#endif

	if(verbose){
	    sprintf(sort_msg, "Sorting \"%.*s\"",
		    sizeof(sort_msg)-20,
		    strsquish(tmp_20k_buf + 500, ps_global->cur_folder,
			      ps_global->ttyo->screen_cols - 20));
	    we_cancel = busy_alarm(1, sort_msg, NULL, 1);
	}

	/*
	 * We do this so that we don't have to lookup the scores with function
	 * calls for each qsort compare.
	 */
	build_score_array(ps_global->mail_stream, msgmap);

	qsort(msgmap->sort+1, (size_t) mn_get_total(msgmap),
	      sizeof(long), pine_compare_scores);
	free_score_array();
	clear_index_cache();

#ifndef	DOS
	intr_handling_off();
#endif

	/*
	 * Flip the sort if necessary (cheaper to do it once than for
	 * every comparison in pine_compare_scores.
	 */
	if(new_rev && mn_get_total(msgmap) > 1L){
	    long *ep = &msgmap->sort[mn_get_total(msgmap)],
		 *sp = &msgmap->sort[1], tmp;

	    do{
		tmp   = *sp;
		*sp++ = *ep;
		*ep-- = tmp;
	    }
	    while(ep > sp);
	}

	if(we_cancel)
	  cancel_busy_alarm(1);
    }
    else{

	clear_index_cache();

	if(verbose){
	    int (*sort_func)() = NULL;

	    /*
	     * IMAP sort doesn't give us any way to get progress,
	     * so just spin the bar rather than show zero percent
	     * forever while a slow sort's going on...
	     */
	    if(!(ps_global->mail_stream
		 && ps_global->mail_stream->dtb
		 && ps_global->mail_stream->dtb->name
		 && !strucmp(ps_global->mail_stream->dtb->name, "imap")
		 && LEVELSORT(ps_global->mail_stream)))
	      sort_func = percent_sorted;

	    sprintf(sort_msg, "Sorting \"%.*s\"",
		    sizeof(sort_msg)-20,
		    strsquish(tmp_20k_buf + 500, ps_global->cur_folder,
			      ps_global->ttyo->screen_cols - 20));
	    we_cancel = busy_alarm(1, sort_msg, sort_func, 1);
	}

	/*
	 * limit the sort/thread if messages are hidden from view 
	 * by lighting searched bit of every interesting msg in 
	 * the folder and call c-client thread/sort to do the dirty work...
	 */
	for(i = 1L; i <= ps_global->mail_stream->nmsgs; i++)
	  mail_elt(ps_global->mail_stream, i)->searched
			= !get_lflag(ps_global->mail_stream, NULL, i, MN_EXLD);
	
#ifndef	DOS
	intr_handling_on();
#endif
	g_sort.msgmap = msgmap;
	if(new_sort == SortThread || new_sort == SortSubject2){
	    THREADNODE *thread;

	    /*
	     * Install callback to collect thread results
	     * and update sort mapping.  Problem this solves
	     * is that of receiving exists/expunged events
	     * within sort/thread response.  Since we update
	     * the sorted table within those handlers, we
	     * can get out of sync when we replace possibly
	     * stale sort/thread results once the function
	     * call's returned.  Make sense?  Thought so.
	     */
	    mail_parameters(NULL, SET_THREADRESULTS,
			    (void *) sort_thread_callback);

	    thread = mail_thread(ps_global->mail_stream,
				 (new_sort == SortThread)
				   ? "REFERENCES" : "ORDEREDSUBJECT",
				 NULL, NULL, 0L);

	    mail_parameters(NULL, SET_THREADRESULTS, (void *) NULL);

	    if(!thread || ps_global->intr_pending){
		new_sort = mn_get_sort(msgmap);
		new_rev  = mn_get_revsort(msgmap);
		q_status_message2(SM_ORDER, 3, 3,
				  "Sort %s!  Restored %s sort.",
				  ps_global->intr_pending
				    ? "Canceled" : "Failed",
				  sort_name(new_sort));
	    }

	    if(thread)
	      mail_free_threadnode(&thread);
	}
	else{
	    /*
	     * Set up the sort program.
	     * NOTE: we deal with reverse bit below.
	     */
	    g_sort.prog = mail_newsortpgm();
	    g_sort.prog->function = (new_sort == SortSubject)
				     ? SORTSUBJECT
				     : (new_sort == SortFrom)
					? SORTFROM
					: (new_sort == SortTo)
					   ? SORTTO
					   : (new_sort == SortCc)
					      ? SORTCC
					      : (new_sort == SortDate)
						 ? SORTDATE
						 : (new_sort == SortSize)
						    ? SORTSIZE
						    : SORTARRIVAL;

	    mail_parameters(NULL, SET_SORTRESULTS,
			    (void *) sort_sort_callback);

	    /* Where the rubber meets the road. */
	    sort = mail_sort(ps_global->mail_stream, NULL,
			     NULL, g_sort.prog, 0L);

	    mail_parameters(NULL, SET_SORTRESULTS, (void *) NULL);

	    if(!sort || ps_global->intr_pending){
		new_sort = mn_get_sort(msgmap);
		new_rev  = mn_get_revsort(msgmap);
		q_status_message2(SM_ORDER, 3, 3,
				  "Sort %s!  Restored %s sort.",
				  ps_global->intr_pending
				    ? "Canceled" : "Failed",
				  sort_name(new_sort));
	    }

	    if(sort)
	      fs_give((void **) &sort);

	    mail_free_sortpgm(&g_sort.prog);
	}

#ifndef	DOS
	intr_handling_off();
#endif

	/*
	 * Flip the sort if necessary (cheaper to do it once than for
	 * every comparison underneath mail_sort()
	 */
	if(new_rev && mn_get_total(msgmap) > 1L){
	    long *ep = &msgmap->sort[mn_get_total(msgmap)],
		 *sp = &msgmap->sort[1], tmp;

	    do{
		tmp   = *sp;
		*sp++ = *ep;
		*ep-- = tmp;
	    }
	    while(ep > sp);
	}

	if(we_cancel)
	  cancel_busy_alarm(1);
    }

    /* Fix up sort structure */
    mn_set_sort(msgmap, new_sort);
    mn_set_revsort(msgmap, new_rev);
    mn_reset_cur(msgmap, mn_raw2m(msgmap, raw_current));
    if(!ps_global->mail_box_changed)
      ps_global->unsorted_newmail = 0;
}


void
sort_sort_callback(stream, list, nmsgs)
    MAILSTREAM	  *stream;
    unsigned long *list;
    unsigned long  nmsgs;
{
    long i;

    if(mn_get_total(g_sort.msgmap) < nmsgs)
      panic("Message count shrank after sort!");

    /* copy ulongs to array of longs */
    for(i = nmsgs; i > 0; i--)
      g_sort.msgmap->sort[i] = (long) list[i-1];
}


void
sort_thread_callback(stream, tree)
    MAILSTREAM *stream;
    THREADNODE *tree;
{
    memset(&g_sort.msgmap->sort[1], 0,
	   mn_get_total(g_sort.msgmap) * sizeof(long));

    /* For now, just flatten out the result */
    (void) sort_thread_flatten(tree, &g_sort.msgmap->sort[1]);
}


long *
sort_thread_flatten(node, entry)
    THREADNODE *node;
    long       *entry;
{
    if(node){
	if(node->num){		/* holes happen */
	    long n = (long) (entry - &g_sort.msgmap->sort[1]);

	    for(; n > 0; n--)
	      if(g_sort.msgmap->sort[n] == node->num)
		break;	/* duplicate */

	    if(!n)
	      *entry++ = node->num;
	}

	if(node->next)
	  entry = sort_thread_flatten(node->next, entry);

	if(node->branch)
	  entry = sort_thread_flatten(node->branch, entry);
    }

    return(entry);
}



/*----------------------------------------------------------------------
    Map sort types to names
  ----*/
char *    
sort_name(so)
  SortOrder so;
{
    /*
     * Make sure the first upper case letter of any new sort name is
     * unique.  The command char and label for sort selection is 
     * derived from this name and its first upper case character.
     * See mailcmd.c:select_sort().
     */
    return((so == SortArrival)  ? "Arrival" :
	    (so == SortDate)	 ? "Date" :
	     (so == SortSubject)  ? "Subject" :
	      (so == SortCc)	   ? "Cc" :
	       (so == SortFrom)	    ? "From" :
		(so == SortTo)	     ? "To" :
		 (so == SortSize)     ? "siZe" :
		  (so == SortSubject2) ? "OrderedSubj" :
		   (so == SortScore)    ? "scorE" :
		     (so == SortThread)  ? "tHread" :
					  "BOTCH");
}



/*
 *           * * *  Message number management functions  * * *
 */


/*----------------------------------------------------------------------
  Initialize a message manipulation structure for the given total

   Accepts: msgs - pointer to pointer to message manipulation struct
	    tot - number of messages to initialize with
  ----*/
void
msgno_init(msgs, tot)
     MSGNO_S **msgs;
     long      tot;
{
    long   slop = (tot + 1L) % 64;
    size_t len;

    if(!msgs)
      return;

    if(!(*msgs)){
	(*msgs) = (MSGNO_S *)fs_get(sizeof(MSGNO_S));
	memset((void *)(*msgs), 0, sizeof(MSGNO_S));
    }

    (*msgs)->sel_cur  = 0L;
    (*msgs)->sel_cnt  = 1L;
    (*msgs)->sel_size = 8L;
    len		      = (size_t)(*msgs)->sel_size * sizeof(long);
    if((*msgs)->select)
      fs_resize((void **)&((*msgs)->select), len);
    else
      (*msgs)->select = (long *)fs_get(len);

    (*msgs)->select[0] = (tot) ? 1L : 0L;

    (*msgs)->sort_size = (tot + 1L) + (64 - slop);
    len		       = (size_t)(*msgs)->sort_size * sizeof(long);
    if((*msgs)->sort)
      fs_resize((void **)&((*msgs)->sort), len);
    else
      (*msgs)->sort = (long *)fs_get(len);

    memset((void *)(*msgs)->sort, 0, len);
    for(slop = 1L ; slop <= tot; slop++)	/* reusing "slop" */
      (*msgs)->sort[slop] = slop;

    (*msgs)->max_msgno    = tot;
    (*msgs)->nmsgs	  = tot;
    (*msgs)->sort_order   = ps_global->def_sort;
    (*msgs)->reverse_sort = ps_global->def_sort_rev;
    (*msgs)->flagged_hid  = 0L;
    (*msgs)->flagged_exld = 0L;
    (*msgs)->flagged_tmp  = 0L;
}



/*----------------------------------------------------------------------
  Release resources of a message manipulation structure

   Accepts: msgs - pointer to message manipulation struct
	    n - number to test
   Returns: with specified structure and its members free'd
  ----*/
void
msgno_give(msgs)
     MSGNO_S **msgs;
{
    if(msgs && *msgs){
	if((*msgs)->sort)
	  fs_give((void **) &((*msgs)->sort));

	if((*msgs)->select)
	  fs_give((void **) &((*msgs)->select));

	fs_give((void **) msgs);
    }
}



/*----------------------------------------------------------------------
  Release resources of a message part exception list

   Accepts: parts -- list of parts to free
   Returns: with specified structure and its members free'd
  ----*/
void
msgno_free_exceptions(parts)
    PARTEX_S **parts;
{
    if(parts && *parts){
	if((*parts)->next)
	  msgno_free_exceptions(&(*parts)->next);

	fs_give((void **) &(*parts)->partno);
	fs_give((void **) parts);
    }
}



/*----------------------------------------------------------------------
 Increment the current message number

   Accepts: msgs - pointer to message manipulation struct
  ----*/
void
msgno_inc(stream, msgs)
     MAILSTREAM *stream;
     MSGNO_S    *msgs;
{
    long i;

    if(!msgs || mn_get_total(msgs) < 1L)
      return;

    for(i = msgs->select[msgs->sel_cur] + 1; i <= mn_get_total(msgs); i++){
	if(!get_lflag(stream, msgs, i, MN_HIDE)){
	    (msgs)->select[((msgs)->sel_cur)] = i;
	    break;
	}
    }
}
 


/*----------------------------------------------------------------------
  Decrement the current message number

   Accepts: msgs - pointer to message manipulation struct
  ----*/
void
msgno_dec(stream, msgs)
     MAILSTREAM *stream;
     MSGNO_S     *msgs;
{
    long i;

    if(!msgs || mn_get_total(msgs) < 1L)
      return;

    for(i = (msgs)->select[((msgs)->sel_cur)] - 1L; i >= 1L; i--){
	if(!get_lflag(stream, msgs, i, MN_HIDE)){
	    (msgs)->select[((msgs)->sel_cur)] = i;
	    break;
	}
    }
}



/*----------------------------------------------------------------------
  Got thru the message mapping table, and remove messages with DELETED flag

   Accepts: stream -- mail stream to removed message references from
	    msgs -- pointer to message manipulation struct
	    f -- flags to use a purge criteria
  ----*/
void
msgno_exclude_deleted(stream, msgs)
     MAILSTREAM *stream;
     MSGNO_S     *msgs;
{
    long	  i;
    MESSAGECACHE *mc;

    if(!msgs || msgs->max_msgno < 1L)
      return;

    /*
     * With 3.91 we're using a new strategy for finding and operating
     * on all the messages with deleted status.  The idea is to do a
     * mail_search for deleted messages so the elt's "searched" bit gets
     * set, and then to scan the elt's for them and set our local bit
     * to indicate they're excluded...
     */
    (void) count_flagged(stream, F_DEL);

    for(i = 1L; i <= msgs->max_msgno; )
      if(((mc = mail_elt(stream, mn_m2raw(msgs, i)))->valid && mc->deleted)
	 || (!mc->valid && mc->searched))
	msgno_exclude(stream, msgs, i);
      else
	i++;

    /*
     * If we excluded away a zoomed display, unhide everything...
     */
    if(msgs->max_msgno > 0L && any_lflagged(msgs, MN_HIDE) >= msgs->max_msgno)
      for(i = 1L; i <= msgs->max_msgno; i++)
	set_lflag(stream, msgs, i, MN_HIDE, 0);
}



void
msgno_exclude(stream, msgmap, msgno)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    long	msgno;
{
    long i;

    /*--- clear all flags to keep our counts consistent  ---*/
    set_lflag(stream, msgmap, msgno, (MN_HIDE|MN_SLCT), 0);
    set_lflag(stream, msgmap, msgno, MN_EXLD, 1); /* mark excluded */

    /* --- erase knowledge in sort array (shift array down) --- */
    for(i = msgno + 1; i <= msgmap->max_msgno; i++)
      msgmap->sort[i-1] = msgmap->sort[i];

    msgmap->max_msgno = max(0L, msgmap->max_msgno - 1L);
    msgno_flush_selected(msgmap, msgno);
}



/*----------------------------------------------------------------------
  Got thru the message mapping table, and remove messages with given flag

   Accepts: stream -- mail stream to removed message references from
	    msgs -- pointer to message manipulation struct
	    f -- flags to use a purge criteria
  ----*/
void
msgno_include(stream, msgs, filtered)
     MAILSTREAM	*stream;
     MSGNO_S	*msgs;
     int	 filtered;
{
    long   i, slop, old_total, old_size;
    int    exbits;
    size_t len;

    for(i = 1L; i <= stream->nmsgs; i++){
	if(!msgno_exceptions(stream, i, "0", &exbits, FALSE))
	  exbits = 0;

	if(((filtered && (exbits & MSG_EX_FILTERED) && 
	     !(exbits & MSG_EX_FILED))
	    || (!filtered && !(exbits & MSG_EX_FILTERED)))
	   && get_lflag(stream, NULL, i, MN_EXLD)){
	    old_total	     = msgs->max_msgno;
	    old_size	     = msgs->sort_size;
	    slop	     = (msgs->max_msgno + 1L) % 64;
	    msgs->sort_size  = (msgs->max_msgno + 1L) + (64 - slop);
	    len		     = (size_t) msgs->sort_size * sizeof(long);
	    if(msgs->sort){
		if(old_size != msgs->sort_size)
		  fs_resize((void **)&(msgs->sort), len);
	    }
	    else
	      msgs->sort = (long *)fs_get(len);

	    msgs->sort[++msgs->max_msgno] = i;
	    set_lflag(stream, msgs, msgs->max_msgno, MN_EXLD, 0);
	    if(filtered){
		exbits ^= MSG_EX_FILTERED;
		msgno_exceptions(stream, i, "0", &exbits, TRUE);
	    }

	    if(old_total <= 0L){	/* if no previous messages, */
		if(!msgs->select){	/* select the new message   */
		    msgs->sel_size = 8L;
		    len		   = (size_t)msgs->sel_size * sizeof(long);
		    msgs->select   = (long *)fs_get(len);
		}

		msgs->sel_cnt   = 1L;
		msgs->sel_cur   = 0L;
		msgs->select[0] = 1L;
	    }
	}
    }
}



/*----------------------------------------------------------------------
 Add the given number of raw message numbers to the end of the
 current list...

   Accepts: msgs - pointer to message manipulation struct
	    n - number to add
   Returns: with fixed up msgno struct

   Only have to adjust the sort array, as since new mail can't cause
   selection!
  ----*/
void
msgno_add_raw(msgs, n)
     MSGNO_S *msgs;
     long     n;
{
    long   slop, old_total, old_size;
    size_t len;

    if(!msgs || n <= 0L)
      return;

    old_total        = msgs->max_msgno;
    old_size         = msgs->sort_size;
    slop             = (old_total + n + 1L) % 64;
    msgs->sort_size  = (old_total + n + 1L) + (64 - slop);
    len		     = (size_t) msgs->sort_size * sizeof(long);
    if(msgs->sort){
	if(old_size != msgs->sort_size)
	  fs_resize((void **) &(msgs->sort), len);
    }
    else
      msgs->sort = (long *) fs_get(len);

    while(n-- > 0)
      msgs->sort[++msgs->max_msgno] = ++msgs->nmsgs;

    if(old_total <= 0L){			/* if no previous messages, */
	if(!msgs->select){			/* select the new message   */
	    msgs->sel_size = 8L;
	    len		   = (size_t) msgs->sel_size * sizeof(long);
	    msgs->select   = (long *) fs_get(len);
	}

	msgs->sel_cnt   = 1L;
	msgs->sel_cur   = 0L;
	msgs->select[0] = 1L;
    }
}



/*----------------------------------------------------------------------
  Remove all knowledge of the given raw message number

   Accepts: msgs - pointer to message manipulation struct
	    n - number to remove
   Returns: with fixed up msgno struct

   After removing *all* references, adjust the sort array and
   various pointers accordingly...
  ----*/
void
msgno_flush_raw(msgs, n)
     MSGNO_S *msgs;
     long     n;
{
    long i, old_sorted = 0L;
    int  shift = 0;

    if(!msgs)
      return;

    /*---- blast n from sort array ----*/
    for(i = 1L; i <= msgs->max_msgno; i++){
	if(msgs->sort[i] == n){
	    old_sorted = i;
	    shift++;
	}

	if(shift)
	  msgs->sort[i] = msgs->sort[i + 1L];

	if(msgs->sort[i] > n)
	  msgs->sort[i] -= 1L;
    }

    /*---- now, fixup counts and select array ----*/
    if(--msgs->nmsgs < 0)
      msgs->nmsgs = 0L;

    if(old_sorted){
	if(--msgs->max_msgno < 0)
	  msgs->max_msgno = 0L;

	msgno_flush_selected(msgs, old_sorted);
    }
}



/*----------------------------------------------------------------------
  Remove all knowledge of the given selected message number

   Accepts: msgs - pointer to message manipulation struct
	    n - number to remove
   Returns: with fixed up selec members in msgno struct

   Remove reference and fix up selected message numbers beyond
   the specified number
  ----*/
void
msgno_flush_selected(msgs, n)
     MSGNO_S *msgs;
     long     n;
{
    long i;
    int  shift = 0;

    for(i = 0L; i < msgs->sel_cnt; i++){
	if(!shift && (msgs->select[i] == n))
	  shift++;

	if(shift && i + 1L < msgs->sel_cnt)
	  msgs->select[i] = msgs->select[i + 1L];

	if(n < msgs->select[i] || msgs->select[i] > msgs->max_msgno)
	  msgs->select[i] -= 1L;
    }

    if(shift && msgs->sel_cnt > 1L)
      msgs->sel_cnt -= 1L;
}



/*----------------------------------------------------------------------
  Test to see if the given message number is in the selected message
  list...

   Accepts: msgs - pointer to message manipulation struct
	    n - number to test
   Returns: true if n is in selected array, false otherwise

  ----*/
int
msgno_in_select(msgs, n)
     MSGNO_S *msgs;
     long     n;
{
    long i;

    if(msgs)
      for(i = 0L; i < msgs->sel_cnt; i++)
	if(msgs->select[i] == n)
	  return(1);

    return(0);
}



/*----------------------------------------------------------------------
  return our index number for the given raw message number

   Accepts: msgs - pointer to message manipulation struct
	    n - number to locate
   Returns: our index number of given raw message

  ----*/
long
msgno_in_sort(msgs, n)
     MSGNO_S *msgs;
     long     n;
{
    static long start;
    long        i;

    if(mn_get_total(msgs) >= 1L){
      if(mn_get_sort(msgs) == SortArrival && !any_lflagged(msgs, MN_EXLD))
	return((mn_get_revsort(msgs)) ? 1 + mn_get_total(msgs) - n  : n);

      i = start = 1L;
      do {
	if(mn_m2raw(msgs, i) == n)
	  return(start = i);

	if(++i > mn_get_total(msgs))
	  i = 1L;
      }
      while(i != start);
    }

    return(0L);
}


/*----------------------------------------------------------------------
  return our index number for the given raw message number

   Accepts: msgs - pointer to message manipulation struct
	    msgno - number that's important
	    part
   Returns: our index number of given raw message

  ----*/
int
msgno_exceptions(stream, rawno, part, bits, set)
    MAILSTREAM *stream;
    long	rawno;
    char       *part;
    int	       *bits, set;
{
    PARTEX_S **partp, *pdelp;

    /*
     * Get pointer to exceptional part list, and scan down it
     * for the requested part...
     */
    for(partp = (PARTEX_S **) &mail_elt(stream, rawno)->sparep;
	*partp;
	partp = &(*partp)->next)
      if(part){
	  if(!strcmp(part, (*partp)->partno)){
	      if(bits){
		  if(set)
		    (*partp)->handling = *bits;
		  else
		    *bits = (*partp)->handling;
	      }

	      return(TRUE);		/* bingo! */
	  }
      }
      else if(bits){
	  /*
	   * The caller provided flags, but no part,
	   * so we're to test for the existance of
	   * any of the flags...
	   */
	  if((*bits & (*partp)->handling) == *bits)
	    return(TRUE);
      }
      else
	/*
	 * The caller didn't specify a part, so
	 * they must just be interested in whether
	 * the msg had any exceptions at all...
	 */
	return(TRUE);

    if(set && part){
	(*partp)	   = (PARTEX_S *) fs_get(sizeof(PARTEX_S));
	(*partp)->partno   = cpystr(part);
	(*partp)->next	   = NULL;
	(*partp)->handling = *bits;
	return(TRUE);
    }

    if(bits)			/* init bits */
      *bits = 0;

    return(FALSE);
}


/*
 * Checks whether any parts of any of the messages in msgmap are marked
 * for deletion.
 */
int
msgno_any_deletedparts(stream, msgmap)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
{
    long n;
    PARTEX_S **partp;

    for(n = mn_first_cur(msgmap); n > 0L; n = mn_next_cur(msgmap))
      for(partp = (PARTEX_S **) &mail_elt(stream, mn_m2raw(msgmap, n))->sparep;
	  *partp;
	  partp = &(*partp)->next)
	if(((*partp)->handling & MSG_EX_DELETE) &&
	   (*partp)->partno &&
	   *(*partp)->partno != '0' &&
	   isdigit((unsigned char)*(*partp)->partno))
	  return(1);

    return(0);
}


int
msgno_part_deleted(stream, rawno, part)
    MAILSTREAM *stream;
    long	rawno;
    char       *part;
{
    char *p;
    int   expbits;

    /*
     * Is this attachment or any of it's parents in the
     * MIME structure marked for deletion?
     */
    for(p = part; p && *p; p = strindex(++p, '.')){
	if(*p == '.')
	  *p = '\0';

	(void) msgno_exceptions(stream, rawno, part, &expbits, FALSE);
	if(!*p)
	  *p = '.';

	if(expbits & MSG_EX_DELETE)
	  return(TRUE);
    }

    /* Finally, check if the whole message body's deleted */
    return(msgno_exceptions(stream, rawno, "", &expbits, FALSE)
	   ? (expbits & MSG_EX_DELETE) : FALSE);
}



/*
 *           * * *  Index entry cache manager  * * *
 */

/*
 * at some point, this could be made part of the pine_state struct.
 * the only changes here would be to pass the ps pointer around
 */
static struct index_cache {
   void	  *cache;				/* pointer to cache         */
   char	  *name;				/* pointer to cache name    */
   long    num;					/* # of last index in cache */
   size_t  size;				/* size of each index line  */
   int     flags;
} icache = { (void *) NULL, (char *) NULL, (long) 0, (size_t) 0, (int) 0 };

#define IC_NEED_FORMAT_SETUP         0x01
#define IC_FORMAT_INCLUDES_MSGNO     0x02
#define IC_FORMAT_INCLUDES_SMARTDATE 0x04
  
/*
 * cache size growth increment
 */

#if defined(DOS) && !defined(_WINDOWS)
/*
 * the idea is to have the cache increment be a multiple of the block
 * size (4K), for efficient swapping of blocks.  we can pretty much
 * assume 81 character lines.
 *
 * REMEMBER: number of lines in the incore cache has to be a multiple 
 *           of the cache growth increment!
 */
#define	IC_SIZE		(50L)			/* cache growth increment  */
#define	ICC_SIZE	(50L)			/* enties in incore cache  */
#define FUDGE           (46L)			/* extra chars to make 4096*/

static char	*incore_cache = NULL;		/* pointer to incore cache */
static long      cache_block_s = 0L;		/* save recomputing time   */
static long      cache_base = 0L;		/* index of line 0 in block*/
#else
#define	IC_SIZE		100
#endif

/*
 * important values for cache building
 */
static MAILSTREAM *bc_this_stream = NULL;
static long  bc_start, bc_current;
static short bc_done = 0;


/*
 * way to return the current cache entry size
 */
int
i_cache_width()
{
    return(icache.size - sizeof(HLINE_S));
}


/* 
 * i_cache_size - make sure the cache is big enough to contain
 * requested entry
 */
int
i_cache_size(indx)
    long         indx;
{
    long j;
    size_t  newsize = sizeof(HLINE_S)
		     + (max(ps_global->ttyo->screen_cols, 80) * sizeof(char));

    if(j = (newsize % sizeof(long)))		/* alignment hack */
      newsize += (sizeof(long) - (size_t)j);

    if(icache.size != newsize){
	clear_index_cache();			/* clear cache, start over! */
	icache.size = newsize;
    }

    if(indx > (j = icache.num - 1L)){		/* make room for entry! */
	size_t  tmplen = icache.size;
	char   *tmpline;

	while(indx >= icache.num)
	  icache.num += IC_SIZE;

#if defined(DOS) && !defined(_WINDOWS)
	tmpline = fs_get(tmplen);
	memset(tmpline, 0, tmplen);
	if(icache.cache == NULL){
	    if(!icache.name)
	      icache.name = temp_nam(NULL, "pi");

	    if((icache.cache = (void *)fopen(icache.name,"w+b")) == NULL){
		sprintf(tmp_20k_buf, "Can't open index cache: %s",icache.name);
		fatal(tmp_20k_buf);
	    }

	    for(j = 0; j < icache.num; j++){
	        if(fwrite(tmpline,tmplen,(size_t)1,(FILE *)icache.cache) != 1)
		  fatal("Can't write index cache in resize");

		if(j%ICC_SIZE == 0){
		  if(fwrite(tmpline,(size_t)FUDGE,
				(size_t)1,(FILE *)icache.cache) != 1)
		    fatal("Can't write FUDGE factor in resize");
	        }
	    }
	}
	else{
	    /* init new entries */
	    fseek((FILE *)icache.cache, 0L, 2);		/* seek to end */

	    for(;j < icache.num; j++){
	        if(fwrite(tmpline,tmplen,(size_t)1,(FILE *)icache.cache) != 1)
		  fatal("Can't write index cache in resize");

		if(j%ICC_SIZE == 0){
		  if(fwrite(tmpline,(size_t)FUDGE,
				(size_t)1,(FILE *)icache.cache) != 1)
		    fatal("Can't write FUDGE factor in resize");
	        }
	    }
	}

	fs_give((void **)&tmpline);
#else
	if(icache.cache == NULL){
	    icache.cache = (void *)fs_get((icache.num+1)*tmplen);
	    memset(icache.cache, 0, (icache.num+1)*tmplen);
	}
	else{
            fs_resize((void **)&(icache.cache), (size_t)(icache.num+1)*tmplen);
	    tmpline = (char *)icache.cache + ((j+1) * tmplen);
	    memset(tmpline, 0, (icache.num - j) * tmplen);
	}
#endif
    }

    return(1);
}

#if defined(DOS) && !defined(_WINDOWS)
/*
 * read a block into the incore cache
 */
void
icread()
{
    size_t n;

    if(fseek((FILE *)icache.cache, (cache_base/ICC_SIZE) * cache_block_s, 0))
      fatal("ran off end of index cache file in icread");

    n = fread((void *)incore_cache, (size_t)cache_block_s, 
		(size_t)1, (FILE *)icache.cache);

    if(n != 1L)
      fatal("Can't read index cache block in from disk");
}


/*
 * write the incore cache out to disk
 */
void
icwrite()
{
    size_t n;

    if(fseek((FILE *)icache.cache, (cache_base/ICC_SIZE) * cache_block_s, 0))
      fatal("ran off end of index cache file in icwrite");

    n = fwrite((void *)incore_cache, (size_t)cache_block_s,
		(size_t)1, (FILE *)icache.cache);

    if(n != 1L)
      fatal("Can't write index cache block in from disk");
}


/*
 * make sure the necessary block of index lines is in core
 */
void
i_cache_hit(indx)
    long         indx;
{
    dprint(9, (debugfile, "i_cache_hit: %ld\n", indx));
    /* no incore cache, create it */
    if(!incore_cache){
	cache_block_s = (((long)icache.size * ICC_SIZE) + FUDGE)*sizeof(char);
	incore_cache  = (char *)fs_get((size_t)cache_block_s);
	cache_base = (indx/ICC_SIZE) * ICC_SIZE;
	icread();
	return;
    }

    if(indx >= cache_base && indx < (cache_base + ICC_SIZE))
	return;

    icwrite();

    cache_base = (indx/ICC_SIZE) * ICC_SIZE;
    icread();
}
#endif



/*
 * return the index line associated with the given message number
 */
HLINE_S *
get_index_cache(msgno)
    long         msgno;
{
    if(need_format_setup())
      setup_header_widths();

    if(!i_cache_size(--msgno)){
	q_status_message(SM_ORDER, 0, 3, "get_index_cache failed!");
	return(NULL);
    }

#if defined(DOS) && !defined(_WINDOWS)
    i_cache_hit(msgno);			/* get entry into core */
    return((HLINE_S *)(incore_cache 
	      + ((msgno%ICC_SIZE) * (long)max(icache.size,FUDGE))));
#else
    return((HLINE_S *) ((char *)(icache.cache) 
	   + (msgno * (long)icache.size * sizeof(char))));
#endif
}


/*
 * the idea is to pre-build and cache index lines while waiting
 * for command input.
 */
void
build_header_cache()
{
    long lines_per_page = max(0,ps_global->ttyo->screen_rows - 5);

    if(mn_get_total(ps_global->msgmap) == 0 || ps_global->mail_stream == NULL
       || (bc_this_stream == ps_global->mail_stream && bc_done >= 2)
       || any_lflagged(ps_global->msgmap, (MN_HIDE|MN_EXLD|MN_SLCT)))
      return;

    if(bc_this_stream != ps_global->mail_stream){ /* reset? */
	bc_this_stream = ps_global->mail_stream;
	bc_current = bc_start = top_ent_calc(ps_global->mail_stream,
					     ps_global->msgmap,
					     mn_get_cur(ps_global->msgmap),
					     lines_per_page);
	bc_done  = 0;
    }

    if(!bc_done && bc_current > mn_get_total(ps_global->msgmap)){ /* wrap? */
	bc_current = bc_start - lines_per_page;
	bc_done++;
    }
    else if(bc_done == 1 && (bc_current % lines_per_page) == 1)
      bc_current -= (2L * lines_per_page);

    if(bc_current < 1)
      bc_done = 2;			/* really done! */
    else
      (void)build_header_line(ps_global, ps_global->mail_stream,
			      ps_global->msgmap, bc_current++);
}


/*
 * erase a particular entry in the cache
 */
void
clear_index_cache_ent(indx)
    long indx;
{
    HLINE_S *tmp = get_index_cache(indx);

    if(tmp->id || tmp->color_lookup_done || *tmp->line)
      memset((void *)tmp, 0, sizeof(*tmp));
}


/*
 * clear the index cache associated with the current mailbox
 */
void
clear_index_cache()
{
#if defined(DOS) && !defined(_WINDOWS)
    cache_base = 0L;
    if(incore_cache)
      fs_give((void **)&incore_cache);

    if(icache.cache){
	fclose((FILE *)icache.cache);
	icache.cache = NULL;
    }

    if(icache.name){
	unlink(icache.name);
	fs_give((void **)&icache.name);
    }
#else
    if(icache.cache)
      fs_give((void **)&(icache.cache));
#endif
    icache.num  = 0L;
    icache.size = 0;
    bc_this_stream = NULL;
    set_need_format_setup();
}


void
clear_icache_flags()
{
    icache.flags = 0;
}

void
set_need_format_setup()
{
    icache.flags |= IC_NEED_FORMAT_SETUP;
}

int
need_format_setup()
{
    return(icache.flags & IC_NEED_FORMAT_SETUP);
}

void
set_format_includes_msgno()
{
    icache.flags |= IC_FORMAT_INCLUDES_MSGNO;
}

int
format_includes_msgno()
{
    return(icache.flags & IC_FORMAT_INCLUDES_MSGNO);
}

void
set_format_includes_smartdate()
{
    icache.flags |= IC_FORMAT_INCLUDES_SMARTDATE;
}

int
format_includes_smartdate()
{
    return(icache.flags & IC_FORMAT_INCLUDES_SMARTDATE);
}


#if defined(DOS) && !defined(_WINDOWS)
/*
 * flush the incore_cache, but not the whole enchilada
 */
void
flush_index_cache()
{
    if(incore_cache){
	if(mn_get_total(ps_global->msgmap) > 0L)
	  icwrite();			/* write this block out to disk */

	fs_give((void **)&incore_cache);
	cache_base = 0L;
    }
}
#endif


#ifdef _WINDOWS
/*----------------------------------------------------------------------
  Callback to get the text of the current message.  Used to display
  a message in an alternate window.	  

  Args: cmd - what type of scroll operation.
	text - filled with pointer to text.
	l - length of text.
	style - Returns style of text.  Can be:
		GETTEXT_TEXT - Is a pointer to text with CRLF deliminated
				lines
		GETTEXT_LINES - Is a pointer to NULL terminated array of
				char *.  Each entry points to a line of
				text.
					
		this implementation always returns GETTEXT_TEXT.

  Returns: TRUE - did the scroll operation.
	   FALSE - was not able to do the scroll operation.
 ----*/
int
index_scroll_callback (cmd, scroll_pos)
int	cmd;
long	scroll_pos;
{
    int paint = TRUE;

    switch (cmd) {
      case MSWIN_KEY_SCROLLUPLINE:
	paint = index_scroll_up (scroll_pos);
	break;

      case MSWIN_KEY_SCROLLDOWNLINE:
	paint = index_scroll_down (scroll_pos);
	break;

      case MSWIN_KEY_SCROLLUPPAGE:
	paint = index_scroll_up (current_index_state->lines_per_page);
	break;

      case MSWIN_KEY_SCROLLDOWNPAGE:
	paint = index_scroll_down (current_index_state->lines_per_page);
	break;

      case MSWIN_KEY_SCROLLTO:
	/* Normalize msgno in zoomed case */
	if(any_lflagged(ps_global->msgmap, MN_HIDE)){
	    long n, x;

	    for(n = 1L, x = 0;
		x < scroll_pos && n < mn_get_total(ps_global->msgmap);
		n++)
	      if(!get_lflag(ps_global->mail_stream,
			    ps_global->msgmap, n, MN_HIDE))
		x++;

	    scroll_pos = n - 1;	/* list-position --> message number  */
	}

	paint = index_scroll_to_pos (scroll_pos + 1);
	break;
    }

    if(paint){
	mswin_beginupdate();
	update_titlebar_message();
	update_titlebar_status();
	redraw_index_body();
	mswin_endupdate();
    }

    return(paint);
}


/*----------------------------------------------------------------------
     MSWin scroll callback to get the text of the current message

  Args: title - title for new window
	text - 
	l - 
	style - 

  Returns: TRUE - got the requested text
	   FALSE - was not able to get the requested text
 ----*/
int
index_gettext_callback(title, text, l, style)
    char  *title;
    void **text;
    long  *l;
    int   *style;
{
    int	      rv = 0;
    ENVELOPE *env;
    BODY     *body;
    STORE_S  *so;
    gf_io_t   pc;

    if(mn_get_total(ps_global->msgmap) > 0L
       && (so = so_get(CharStar, NULL, WRITE_ACCESS))){
	gf_set_so_writec(&pc, so);

	if((env = mail_fetchstructure(ps_global->mail_stream,
				      mn_m2raw(ps_global->msgmap,
					       mn_get_cur(ps_global->msgmap)),
				      &body))
	   && format_message(mn_m2raw(ps_global->msgmap,
				      mn_get_cur(ps_global->msgmap)),
			     env, body, FM_NEW_MESS, pc)){
	    sprintf(title, "Folder %s  --  Message %ld of %ld",
		    strsquish(tmp_20k_buf + 500, ps_global->cur_folder, 50),
		    mn_get_cur(ps_global->msgmap),
		    mn_get_total(ps_global->msgmap));
	    *text  = so_text(so);
	    *l     = strlen((char *)so_text(so));
	    *style = GETTEXT_TEXT;

	    /* free alloc'd so, but preserve the text passed back to caller */
	    so->txt = (void *) NULL;
	    rv = 1;
	}

	gf_clear_so_writec(so);
	so_give(&so);
    }

    return(rv);
}


/*
 *
 */
int
index_sort_callback(set, order)
    int  set;
    long order;
{
    int i = 0;

    if(set){
	sort_folder(ps_global->msgmap, order & 0x000000ff,
		    (order & 0x00000100) != 0, 1);
	mswin_beginupdate();
	update_titlebar_message();
	update_titlebar_status();
	redraw_index_body();
	mswin_endupdate();
	flush_status_messages(1);
    }
    else{
	i = (int) mn_get_sort(ps_global->msgmap);
	if(mn_get_revsort(ps_global->msgmap))
	  i |= 0x0100;
    }

    return(i);
}


/*
 *
 */
void
index_popup(stream,  msgmap, full)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    int		full;
{
    int		  n;
    MESSAGECACHE *mc;
    MPopup	  view_index_popup[32];

    if(full){
	view_index_popup[0].type	     = tQueue;
	view_index_popup[0].label.string = "&View";
	view_index_popup[0].label.style  = lNormal;
	view_index_popup[0].data.val     = 'V';

	view_index_popup[1].type	     = tIndex;
	view_index_popup[1].label.style  = lNormal;
	view_index_popup[1].label.string = "View in New Window";

	view_index_popup[2].type = tSeparator;

	/* Make "delete/undelete" item sensitive */
	mc = mail_elt(stream, mn_m2raw(msgmap, mn_get_cur(msgmap)));
	view_index_popup[3].type	  = tQueue;
	view_index_popup[3].label.style = lNormal;
	if(mc->deleted){
	    view_index_popup[3].label.string = "&Undelete";
	    view_index_popup[3].data.val     = 'U';
	}
	else{
	    view_index_popup[3].label.string = "&Delete";
	    view_index_popup[3].data.val     = 'D';
	}

	if(F_ON(F_ENABLE_FLAG, ps_global)){
	    view_index_popup[4].type		 = tSubMenu;
	    view_index_popup[4].label.string	 = "Flag";
	    view_index_popup[4].data.submenu = flag_submenu(mc);
	    n = 5;
	}
	else
	  n = 4;

	view_index_popup[n].type	   = tQueue;
	view_index_popup[n].label.style  = lNormal;
	view_index_popup[n].label.string = "&Save";
	view_index_popup[n++].data.val   = 'S';

	view_index_popup[n].type	   = tQueue;
	view_index_popup[n].label.style  = lNormal;
	view_index_popup[n].label.string = "Print";
	view_index_popup[n++].data.val   = '%';

	view_index_popup[n].type	   = tQueue;
	view_index_popup[n].label.style  = lNormal;
	view_index_popup[n].label.string = "&Reply";
	view_index_popup[n++].data.val   = 'R';

	view_index_popup[n].type	   = tQueue;
	view_index_popup[n].label.style  = lNormal;
	view_index_popup[n].label.string = "&Forward";
	view_index_popup[n++].data.val   = 'F';

	view_index_popup[n++].type = tSeparator;
    }
    else
      n = 0;

    view_index_popup[n].type	     = tQueue;
    view_index_popup[n].label.style  = lNormal;
    view_index_popup[n].label.string = "Folder &List";
    view_index_popup[n++].data.val   = '<';

    view_index_popup[n].type	     = tQueue;
    view_index_popup[n].label.style  = lNormal;
    view_index_popup[n].label.string = "&Main Menu";
    view_index_popup[n++].data.val   = 'M';

    view_index_popup[n].type = tTail;

    if((n = mswin_popup(view_index_popup)) == 1 && full){
	static int lastWind;
	char title[GETTEXT_TITLELEN+1];
	void	*text;
	long	len;
	int	format;

	/* Launch text in alt window. */
#if	0
	if (mp.flags & M_KEY_CONTROL)
	  lastWind = 0;
#endif
	if (index_gettext_callback (title, &text,
				    &len, &format)) {
	    if (format == GETTEXT_TEXT) 
	      lastWind = mswin_displaytext (title, text, (size_t)len, 
					    NULL, lastWind, 0);
	    else if (format == GETTEXT_LINES) 
	      lastWind = mswin_displaytext (title, NULL, 0,
					    text, lastWind, 0);
	}
    }
}

char *
pcpine_help_index(title)
    char *title;
{
    if(title)
      strcpy(title, "PC-Pine MESSAGE INDEX Help");

    return(pcpine_help(h_mail_index));
}

char *
pcpine_help_index_simple(title)
    char *title;
{
    if(title)
      strcpy(title, "PC-Pine SELECT MESSAGE Help");

    return(pcpine_help(h_simple_index));
}


int
pcpine_resize_index()
{
    int orig_col = ps_global->ttyo->screen_cols;

    reset_index_border();
    (void) get_windsize (ps_global->ttyo);

    if(orig_col != ps_global->ttyo->screen_cols)
      clear_index_cache();

    mswin_beginupdate();
    update_titlebar_message();
    update_titlebar_status();
    redraw_index_body();
    mswin_endupdate();
    return(0);
}
#endif	/* _WINDOWS */
