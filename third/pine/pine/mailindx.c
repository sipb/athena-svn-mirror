#if !defined(lint) && !defined(DOS)
static char rcsid[] = "$Id: mailindx.c,v 1.1.1.3 2003-05-01 01:13:26 ghudson Exp $";
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
	{"<", NULL, {MC_FOLDERS,2,{'<',','}}, KS_NONE},
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
	{"/","Collapse/Expand",{MC_COLLAPSE,1,{'/'}},KS_NONE},
	NULL_MENU,
	NULL_MENU};
INST_KEY_MENU(index_keymenu, index_keys);
#define BACK_KEY 2
#define PREVM_KEY 4
#define NEXTM_KEY 5
#define EXCLUDE_KEY 26		/* used for thread_keymenu, too */
#define UNEXCLUDE_KEY 27	/* used for thread_keymenu, too */
#define SELECT_KEY 28
#define APPLY_KEY 29
#define VIEW_FULL_HEADERS_KEY 32
#define BOUNCE_KEY 33
#define FLAG_KEY 34
#define VIEW_PIPE_KEY 35
#define SELCUR_KEY 38
#define ZOOM_KEY 39
#define COLLAPSE_KEY 45

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


static struct key thread_keys[] = 
       {HELP_MENU,
	OTHER_MENU,
	{"<", "FldrList", {MC_FOLDERS,2,{'<',','}}, KS_NONE},
	{">", "[ViewThd]", {MC_VIEW_ENTRY,5,{'v','.','>',ctrl('M'),ctrl('J')}},
								    KS_VIEW},
	{"P", "PrevThd", {MC_PREVITEM, 1, {'p'}}, KS_PREVMSG},
	{"N", "NextThd", {MC_NEXTITEM, 1, {'n'}}, KS_NEXTMSG},
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
	{"/","Collapse/Expand",{MC_COLLAPSE,1,{'/'}},KS_NONE},
	NULL_MENU,
	NULL_MENU};
INST_KEY_MENU(thread_keymenu, thread_keys);


static OtherMenu what_keymenu = FirstMenu;

/*
 * structures to maintain index line text
 */

typedef struct _offsets {
    int        offset;		/* the offset into the line */
    COLOR_PAIR color;		/* color for this offset    */
} OFFCOLOR_S;

#define OFFS 5

typedef struct _header_line {
    unsigned long id;				/* header line's hash value */
    unsigned      color_lookup_done:1;
    COLOR_PAIR	  linecolor;
    OFFCOLOR_S    offs[OFFS];
    int           plus;
    struct _header_line *tihl;			/* thread index header line */
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
    int      plus;
    unsigned long id;
};


#define MAXIFLDS 20  /* max number of fields in index format */
static struct index_state {
    long        msg_at_top,
	        lines_per_page;
    struct      entry_state *entry_state;
    MSGNO_S    *msgmap;
    MAILSTREAM *stream;
    int         status_col;		/* column for select X's */
    int         plus_col;		/* column for threading '+' or '>' */
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



struct save_thrdinfo {
    HLINE_S  *(*format_index_line) PROTO((INDEXDATA_S *));
    void      (*setup_header_widths) PROTO((void));
    unsigned  viewing_a_thread:1;
    unsigned  inbox_viewing_a_thread:1;
};


/*
 * Keep track of the sort array and some of the thread structure
 * temporarily as we're building the PINETHRD_S structures.
 */
struct pass_along {
    unsigned long rawno;
    PINETHRD_S   *thrd;
} *thrd_flatten_array;


/*
 * Binary tree to accumulate runs of subject groups (poor man's threads)
 */
typedef struct _subject_run_s {
    long	start;			/* raw msgno of run start */
    long	run;			/* run's length */
    struct _subject_run_s *left, *right;
} SUBR_S;


HLINE_S	       *(*format_index_line) PROTO((INDEXDATA_S *));
void		(*setup_header_widths) PROTO((void));



/*
 * Internal prototypes
 */
void            index_index_screen PROTO((struct pine *));
void            thread_index_screen PROTO((struct pine *));
void            setup_for_index_index_screen PROTO((void));
void            setup_for_thread_index_screen PROTO((void));
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
HLINE_S	       *get_tindex_cache PROTO((long));
int		fetch_sort_data PROTO((MAILSTREAM *, long, int, char *));
HLINE_S	       *build_header_line PROTO((struct pine *, MAILSTREAM *,
					  MSGNO_S *, long, int *));
HLINE_S	       *format_index_index_line PROTO((INDEXDATA_S *));
HLINE_S	       *format_thread_index_line PROTO((INDEXDATA_S *));
void		index_data_env PROTO((INDEXDATA_S *, ENVELOPE *));
int		set_index_addr PROTO((INDEXDATA_S *, char *, ADDRESS *,
				      char *, int, char *));
int		i_cache_size PROTO((long));
int		i_cache_width PROTO(());
int             ctype_is_fixed_length PROTO((IndexColType));
void		setup_index_header_widths PROTO((void));
void		setup_thread_header_widths PROTO((void));
int		parse_index_format PROTO((char *, INDEX_COL_S **));
void		clear_icache_flags PROTO(());
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
void		subj_str PROTO((INDEXDATA_S *, int, char *));
void		from_str PROTO((IndexColType, INDEXDATA_S *, int, char *));
void            set_msg_score PROTO((MAILSTREAM *, long, int));
void		load_overview PROTO((MAILSTREAM *, unsigned long, OVERVIEW *));
int		paint_index_line PROTO((long, HLINE_S *, int, int, int,
					struct entry_state *, int, int));
void		paint_index_hline PROTO((MAILSTREAM *, long, HLINE_S *));
void		index_search PROTO((struct pine *,MAILSTREAM *,int,MSGNO_S *));
void		msgno_flush_selected PROTO((MSGNO_S *, long));
void            msgno_reset_isort PROTO((MSGNO_S *));
void		sort_sort_callback PROTO((MAILSTREAM *, unsigned long *,
					  unsigned long));
void		sort_thread_callback PROTO((MAILSTREAM *, THREADNODE *));
struct pass_along
	       *sort_thread_flatten PROTO((THREADNODE *, MAILSTREAM *,
					   struct pass_along *,
					   PINETHRD_S *, unsigned));
void            make_thrdflags_consistent PROTO((MAILSTREAM *, MSGNO_S *,
						 PINETHRD_S *, int));
THREADNODE     *collapse_threadnode_tree PROTO((THREADNODE *));
PINETHRD_S     *msgno_thread_info PROTO((MAILSTREAM *, unsigned long,
					 PINETHRD_S *, unsigned));
long            calculate_visible_threads PROTO((MAILSTREAM *));
void		set_thread_subtree PROTO((MAILSTREAM *, PINETHRD_S *,
					  MSGNO_S *, int, int));
void		thread_command PROTO((struct pine *, MAILSTREAM *, MSGNO_S *,
				      int, int));
void		set_flags_for_thread PROTO((MAILSTREAM *, MSGNO_S *, int,
					    PINETHRD_S *, int));
unsigned long   count_flags_in_thread PROTO((MAILSTREAM *, PINETHRD_S *, long));
int             mark_msgs_in_thread PROTO((MAILSTREAM *, PINETHRD_S *,
					   MSGNO_S *));
char            to_us_symbol_for_thread PROTO((MAILSTREAM *, PINETHRD_S *,
					       int));
char            status_symbol_for_thread PROTO((MAILSTREAM *, PINETHRD_S *,
						IndexColType));
int             msgline_hidden PROTO((MAILSTREAM *, MSGNO_S *, long, int));
void		copy_lflags PROTO((MAILSTREAM *, MSGNO_S *, int, int));
void		set_lflags PROTO((MAILSTREAM *, MSGNO_S *, int, int));
int             day_of_week PROTO((struct date *));
int             day_of_year PROTO((struct date *));
#ifdef	_WINDOWS
int		index_scroll_callback PROTO((int,long));
int		index_gettext_callback PROTO((char *, void **, long *, int *));
void		index_popup PROTO((MAILSTREAM *, MSGNO_S *, int));
char	       *pcpine_help_index PROTO((char *));
char	       *pcpine_help_index_simple PROTO((char *));
int		pcpine_resize_index PROTO((void));
#endif

#if defined(ISORT_ASSERT)

char           *assert_isort_validity PROTO((MSGNO_S *));
void            dump_isort PROTO((MSGNO_S *));

#define ASSERT_ISORT(msgmap, place) {char *qqq; \
				     if(qqq=assert_isort_validity(msgmap)){ \
				       dprint(1, (debugfile, place));  \
				       panic(qqq);                          \
				     }}
#else
#define ASSERT_ISORT(msgmap, place)
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
    struct key_menu *km = (style == ThreadIndex)
			    ? &thread_keymenu
			    : (ps_global->mail_stream != stream)
			      ? &simple_index_keymenu
			      : &index_keymenu;

    if(flags & INDX_CLEAR)
      ClearScreen();

    if(flags & INDX_HEADER)
      set_titlebar((style == ThreadIndex)
		     ? "THREAD INDEX"
		     : (stream == ps_global->mail_stream)
		       ? (style == MsgIndex || style == MultiMsgIndex)
		         ? "MESSAGE INDEX"
			 : "ZOOMED MESSAGE INDEX"
		       : (!strcmp(folder, INTERRUPTED_MAIL))
			 ? "COMPOSE: SELECT INTERRUPTED"
			 : (ps_global->VAR_FORM_FOLDER
			    && !strcmp(ps_global->VAR_FORM_FOLDER, folder))
			     ? "COMPOSE: SELECT FORM LETTER"
			     : "COMPOSE: SELECT POSTPONED",
		   stream, cntxt, folder, msgmap, 1,
		   (style == ThreadIndex) ? ThrdIndex
					  : (THREADING()
					     && ps_global->viewing_a_thread)
					    ? ThrdMsgNum
					    : MessageNumber,
		   0, 0, NULL);

    if(flags & INDX_FOOTER) {
	bitmap_t bitmap;
	int	 cmd;

	setbitmap(bitmap);

	if(km == &index_keymenu){
	    if(THREADING() && ps_global->viewing_a_thread){
		menu_init_binding(km, '<', MC_THRDINDX, "<",
				  "ThrdIndex", BACK_KEY);
		menu_add_binding(km, ',', MC_THRDINDX);
	    }
	    else{
		menu_init_binding(km, '<', MC_FOLDERS, "<",
				  "FldrList", BACK_KEY);
		menu_add_binding(km, ',', MC_FOLDERS);
	    }
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

	    if(style == MultiMsgIndex){
		clrbitn(PREVM_KEY, bitmap);
		clrbitn(NEXTM_KEY, bitmap);
	    }
	}

	if(km == &index_keymenu || km == &thread_keymenu){
	    if(IS_NEWS(stream)){
		km->keys[EXCLUDE_KEY].label = "eXclude";
		KS_OSDATASET(&km->keys[EXCLUDE_KEY], KS_NONE);
	    }
	    else {
		clrbitn(UNEXCLUDE_KEY, bitmap);
		km->keys[EXCLUDE_KEY].label = "eXpunge";
		KS_OSDATASET(&km->keys[EXCLUDE_KEY], KS_EXPUNGE);
	    }
	}

	if(km != &simple_index_keymenu && !THRD_COLLAPSE_ENABLE())
	  clrbitn(COLLAPSE_KEY, bitmap);

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
    if(!state->mail_stream) {
	q_status_message(SM_ORDER, 0, 3, "No folder is currently open");
        state->prev_screen = mail_index_screen;
	state->next_screen = main_menu_screen;
	return;
    }

    state->prev_screen = mail_index_screen;
    state->next_screen = SCREEN_FUN_NULL;

    if(THRD_AUTO_VIEW() && state->viewing_a_thread
       && state->view_skipped_index)
      unview_thread(state, state->mail_stream, state->msgmap);

    adjust_cur_to_visible(state->mail_stream, state->msgmap);

    if(THRD_INDX())
      thread_index_screen(state);
    else
      index_index_screen(state);
}


void
index_index_screen(state)
     struct pine *state;
{
    dprint(1, (debugfile, "\n\n ---- MAIL INDEX ----\n"));

    setup_for_index_index_screen();

    index_lister(state, state->context_current, state->cur_folder,
		 state->mail_stream, state->msgmap);
}


void
thread_index_screen(state)
     struct pine *state;
{
    dprint(1, (debugfile, "\n\n ---- THREAD INDEX ----\n"));

    setup_for_thread_index_screen();

    index_lister(state, state->context_current, state->cur_folder,
		 state->mail_stream, state->msgmap);
}


void
setup_for_index_index_screen()
{
    format_index_line = format_index_index_line;
    setup_header_widths = setup_index_header_widths;
    if(ps_global->mail_stream == ps_global->inbox_stream && THREADING())
      ps_global->inbox_viewing_a_thread = ps_global->viewing_a_thread;
}


void
setup_for_thread_index_screen()
{
    format_index_line = format_thread_index_line;
    setup_header_widths = setup_thread_header_widths;
    if(ps_global->mail_stream == ps_global->inbox_stream)
      ps_global->inbox_viewing_a_thread = ps_global->viewing_a_thread;
}
    

void *
stop_threading_temporarily()
{
    struct save_thrdinfo *ti;

    ps_global->turn_off_threading_temporarily = 1;

    ti = (struct save_thrdinfo *) fs_get(sizeof(*ti));
    ti->format_index_line = format_index_line;
    ti->setup_header_widths = setup_header_widths;
    ti->viewing_a_thread = ps_global->viewing_a_thread;
    ti->inbox_viewing_a_thread = ps_global->inbox_viewing_a_thread;

    setup_for_index_index_screen();

    return((void *) ti);
}


void
restore_threading(p)
    void **p;
{
    struct save_thrdinfo *ti;

    ps_global->turn_off_threading_temporarily = 0;

    if(p && *p){
	ti = (struct save_thrdinfo *) (*p);
	format_index_line = ti->format_index_line;
	setup_header_widths = ti->setup_header_widths;
	ps_global->viewing_a_thread = ti->viewing_a_thread;
	ps_global->inbox_viewing_a_thread = ti->inbox_viewing_a_thread;

	fs_give(p);
    }
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
    old_max_msgno         = mn_get_total(msgmap);
    memset((void *)&id, 0, sizeof(struct index_state));
    current_index_state   = &id;
    id.msgmap		  = msgmap;
    if(msgmap->top != 0L)
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
		    id.entry_state[id.lines_per_page-2].id = 0;
		    id.entry_state[id.lines_per_page-1].id = 0;
		}
		else
		  state->mangled_body = 1;
	    }
	}

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
	    clear_iindex_cache();
	    state->mangled_body = 1;
        }

	old_max_msgno = mn_get_total(msgmap);

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
		clear_iindex_cache();
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
	style = THRD_INDX()
		  ? ThreadIndex
		  : (any_lflagged(msgmap, MN_HIDE))
		    ? ZoomIndex
		    : (mn_total_cur(msgmap) > 1L) ? MultiMsgIndex : MsgIndex;
	if(style != old_style){
            state->mangled_header = 1;
            state->mangled_footer = 1;
	    old_style = style;
	    if(!(style == ThreadIndex || old_style == ThreadIndex))
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
				|| style == ZoomIndex)
				   ? MsgIndx
				   : (style == ThreadIndex)
				     ? ThrdIndx
				     : View,
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
				|| style == ZoomIndex)
				   ? MsgIndx
				   : (style == ThreadIndex)
				     ? ThrdIndx
				     : View,
			       &force);
	    for(j = 0L, k = i = id.msg_at_top; ; i++){
		if(!msgline_hidden(stream, msgmap, i, 0)){
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
		if(!msgline_hidden(stream, msgmap, i, 0)){
		    k = i;
		    if(++j >= id.lines_per_page){
			if((id.msg_at_top = i) == 1L)
			  q_status_message(SM_ORDER, 0, 1, "First Index page");

			break;
		    }
	        }

		if(i <= 1L){
		    if((!THREADING() && mn_get_cur(msgmap) == 1L)
		       || (THREADING()
		           && mn_get_cur(msgmap) == first_sorted_flagged(F_NONE,
								         stream,
									 0L,
							        FSF_SKIP_CHID)))
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
		if(!msgline_hidden(stream, msgmap, i, 0)){
		    k = i;
		    if(++j >= id.lines_per_page){
			if(i+id.lines_per_page > mn_get_total(msgmap))
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
	    j = jump_to(msgmap, -FOOTER_ROWS(ps_global), ch, NULL,
			(style == ThreadIndex) ? ThrdIndx : MsgIndx);
	    if(j > 0L){
		if(style == ThreadIndex){
		    PINETHRD_S *thrd;

		    thrd = find_thread_by_number(stream, msgmap, j, NULL);

		    if(thrd && thrd->rawno)
		      mn_set_cur(msgmap, mn_raw2m(msgmap, thrd->rawno));
		}
		else{
		    /* jump to message */
		    if(mn_total_cur(msgmap) > 1L){
			mn_reset_cur(msgmap, j);
		    }
		    else{
			mn_set_cur(msgmap, j);
		    }
		}

		id.msg_at_top = 0L;
	    }

	    state->mangled_footer = 1;
	    break;


	  case MC_VIEW_ENTRY :		/* only happens in thread index */

	    /*
	     * If the feature F_THRD_AUTO_VIEW is turned on and there
	     * is only one message in the thread, then we skip the index
	     * view of the thread and go straight to the message view.
	     */
view_a_thread:
	    if(THRD_AUTO_VIEW() && style == ThreadIndex){
		PINETHRD_S *thrd;

		thrd = fetch_thread(stream,
				    mn_m2raw(msgmap, mn_get_cur(msgmap)));
		if(thrd
		   && (count_lflags_in_thread(stream, thrd,
		       msgmap, MN_NONE) == 1)){
		    if(view_thread(state, stream, msgmap, 1)){
			state->view_skipped_index = 1;
			cmd = MC_VIEW_TEXT;
			goto do_the_default;
		    }
		}
	    }

	    if(view_thread(state, stream, msgmap, 1)){
		ps_global->redrawer = NULL;
		current_index_state = NULL;
		if(id.entry_state)
		  fs_give((void **)&(id.entry_state));

		return(0);
	    }

	    break;


	  case MC_THRDINDX :
	    msgmap->top = msgmap->top_after_thrd;
	    if(unview_thread(state, stream, msgmap)){
		ps_global->redrawer = NULL;
		current_index_state = NULL;
		if(id.entry_state)
		  fs_give((void **)&(id.entry_state));

		return(0);
	    }

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
		if(!msgline_hidden(stream, msgmap, i, 0)){
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
						       MsgIndx);
			  }
		      }
		      else if(!(mp.flags & M_KEY_SHIFT)){
			  if (THREADING()
			      && mp.col >= 0
			      && mp.col == id.plus_col
			      && style != ThreadIndex){
			      collapse_or_expand(state, stream, msgmap,
						 mn_get_cur(msgmap));
			  }
			  else if (mp.doubleclick){
			      if(mp.button == M_BUTTON_LEFT){
				  if(stream == state->mail_stream){
				      if(THRD_INDX()){
					  cmd = MC_VIEW_ENTRY;
					  goto view_a_thread;
				      }
				      else{
					  cmd = MC_VIEW_TEXT;
					  goto do_the_default;
				      }
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


	  case MC_COLLAPSE :
	    thread_command(state, stream, msgmap, ch, -FOOTER_ROWS(state));
	    break;

          case MC_DELETE :
          case MC_UNDELETE :
          case MC_REPLY :
          case MC_FORWARD :
          case MC_TAKE :
          case MC_SAVE :
          case MC_EXPORT :
          case MC_BOUNCE :
          case MC_PIPE :
          case MC_FLAG :
          case MC_SELCUR :
	    { int collapsed = 0;
	      unsigned long rawno;
	      PINETHRD_S *thrd = NULL;

	      if(THREADING()){
		  rawno = mn_m2raw(msgmap, mn_get_cur(msgmap));
		  if(rawno)
		    thrd = fetch_thread(stream, rawno);

		  collapsed = thrd && thrd->next
			      && get_lflag(stream, NULL, rawno, MN_COLL);
	      }

	      if(collapsed){
		  thread_command(state, stream, msgmap,
				 ch, -FOOTER_ROWS(state));
		  /* increment current */
		  if(cmd == MC_DELETE){
		      advance_cur_after_delete(state, stream, msgmap,
					       (style == MsgIndex
						  || style == MultiMsgIndex
						  || style == ZoomIndex)
						    ? MsgIndx
						    : (style == ThreadIndex)
						      ? ThrdIndx
						      : View);
		  }
		  else if((cmd == MC_SELCUR
			   && (state->ugly_consider_advancing_bit
			       || F_OFF(F_UNSELECT_WONT_ADVANCE, state)))
			  || (state->ugly_consider_advancing_bit
			      && cmd == MC_SAVE
			      && F_ON(F_SAVE_ADVANCES, state))){
		      mn_inc_cur(stream, msgmap, MH_NONE);
		  }
	      }
	      else
		goto do_the_default;
	    }

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
   do_the_default:
	    if(stream == state->mail_stream){
		msgmap->top = id.msg_at_top;
		process_cmd(state, stream, msgmap, cmd,
			    (style == MsgIndex
			     || style == MultiMsgIndex
			     || style == ZoomIndex)
			       ? MsgIndx
			       : (style == ThreadIndex)
				 ? ThrdIndx
				 : View,
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

		    if(cmd == MC_ZOOM && THRD_INDX())
		      id.msg_at_top = 0L;
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
			long	      raw, t;
			int	      del = 0;

			raw = mn_m2raw(msgmap, mn_get_cur(msgmap));
			if(!mail_elt(stream, raw)->deleted){
			    if((t = mn_get_cur(msgmap)) > 0L)
			      clear_index_cache_ent(t);

			    mail_setflag(stream,long2string(raw),"\\DELETED");
			    update_titlebar_status();
			    del++;
			}

			q_status_message2(SM_ORDER, 0, 1,
					  "Message %.200s %.200sdeleted",
					  long2string(mn_get_cur(msgmap)),
					  (del) ? "" : "already ");
		    }

		    break;

		  case MC_UNDELETE :	/* UNdelete */
		    dprint(3,(debugfile, "Special UNdelete: msg %s\n",
			      long2string(mn_get_cur(msgmap))));
		    {
			long	      raw, t;
			int	      del = 0;

			raw = mn_m2raw(msgmap, mn_get_cur(msgmap));
			if(mail_elt(stream, raw)->deleted){
			    if((t = mn_get_cur(msgmap)) > 0L)
			      clear_index_cache_ent(t);

			    mail_clearflag(stream, long2string(raw),
					   "\\DELETED");
			    update_titlebar_status();
			    del++;
			}

			q_status_message2(SM_ORDER, 0, 1,
					  "Message %.200s %.200sdeleted",
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
		    mn_dec_cur(stream, msgmap, MH_NONE);
		    break;

		  case MC_NEXTITEM :		/* next */
		    mn_inc_cur(stream, msgmap, MH_NONE);
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
    int  i, retval = -1, row, already_fetched = 0;
    long n, visible;
    PINETHRD_S *thrd = NULL;

    dprint(7, (debugfile, "--update_index--\n"));

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
	  screen->entry_state[i].id = 0;
    }

    /*---- figure out the first message on the display ----*/
    if(screen->msg_at_top < 1L
       || msgline_hidden(screen->stream, screen->msgmap, screen->msg_at_top,0)){
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
	      if(!msgline_hidden(screen->stream, screen->msgmap, j, 0)){
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
	      if(!msgline_hidden(screen->stream, screen->msgmap, j, 0)){
		  k = j;
		  i--;
		  if(mn_get_cur(screen->msgmap) <= k)
		    break;
	      }

	    if(mn_get_cur(screen->msgmap) <= k)
	      break;
	    else{
		/* set msg_at_top to next displayed message */
		for(i = k + 1L; i <= mn_get_total(screen->msgmap); i++)
		  if(!msgline_hidden(screen->stream, screen->msgmap, i, 0)){
		      k = i;
		      break;
		  }

		screen->msg_at_top = k;
	    }
	}
    }

#ifdef	_WINDOWS
    /*
     * Set scroll range and position.  Note that message numbers start at 1
     * while scroll position starts at 0.
     */

    if(THREADING() && state->viewing_a_thread
       && mn_get_total(screen->msgmap) > 1L){
	long x = 0L, range = 0L, lowest_numbered_msg;

	/*
	 * We know that all visible messages in the thread are marked
	 * with MN_CHID2.
	 */
	thrd = fetch_thread(screen->stream,
			    mn_m2raw(screen->msgmap,
				     mn_get_cur(screen->msgmap)));
	if(thrd && thrd->top && thrd->top != thrd->rawno)
	  thrd = fetch_thread(screen->stream, thrd->top);

	if(thrd){
	    if(mn_get_revsort(screen->msgmap)){
		n = mn_raw2m(screen->msgmap, thrd->rawno);
		while(n > 1L && get_lflag(screen->stream, screen->msgmap,
					  n-1L, MN_CHID2))
		  n--;

		lowest_numbered_msg = n;
	    }
	    else
	      lowest_numbered_msg = mn_raw2m(screen->msgmap, thrd->rawno);
	}
	
	if(thrd){
	    n = lowest_numbered_msg;
	    for(; n <= mn_get_total(screen->msgmap); n++){

		if(!get_lflag(screen->stream, screen->msgmap, n, MN_CHID2))
		  break;
		
		if(!msgline_hidden(screen->stream, screen->msgmap, n, 0)){
		    range++;
		    if(n < screen->msg_at_top)
		      x++;
		}
	    }
	}

	scroll_setrange(screen->lines_per_page, range-1L);
	scroll_setpos(x);
    }
    else if(THRD_INDX()){
	if(any_lflagged(screen->msgmap, MN_HIDE)){
	    long x = 0L, range;

	    range = screen->msgmap->visible_threads - 1L;
	    scroll_setrange(screen->lines_per_page, range);
	    if(range >= screen->lines_per_page){	/* else not needed */
		PINETHRD_S *topthrd;
		int         thrddir;
		long        xdir;

		/* find top of currently displayed top line */
		topthrd = fetch_thread(screen->stream,
				       mn_m2raw(screen->msgmap,
					        screen->msg_at_top));
		if(topthrd && topthrd->top != topthrd->rawno)
		  topthrd = fetch_thread(screen->stream, topthrd->top);
		
		if(topthrd){
		    /*
		     * Split into two halves to speed up finding scroll pos.
		     * It's tricky because the thread list always goes from
		     * past to future but the thrdno's will be reversed if
		     * the sort is reversed and of course the order on the
		     * screen will be reversed.
		     */
		    if((!mn_get_revsort(screen->msgmap)
		        && topthrd->thrdno <= screen->msgmap->max_thrdno/2)
		       ||
		       (mn_get_revsort(screen->msgmap)
		        && topthrd->thrdno > screen->msgmap->max_thrdno/2)){

			/* start with head of thread list */
			if(topthrd && topthrd->head)
			  thrd = fetch_thread(screen->stream, topthrd->head);
			else
			  thrd = NULL;

			thrddir = 1;
		    }
		    else{
			long tailrawno;

			/*
			 * Start with tail thread and work back.
			 */
		        if(mn_get_revsort(screen->msgmap))
			  tailrawno = mn_m2raw(screen->msgmap, 1L);
			else
			  tailrawno = mn_m2raw(screen->msgmap,
					       mn_get_total(screen->msgmap));

			thrd = fetch_thread(screen->stream, tailrawno);
			if(thrd && thrd->top && thrd->top != thrd->rawno)
			  thrd = fetch_thread(screen->stream, thrd->top);

			thrddir = -1;
		    }

		    /*
		     * x is the scroll position. We try to use the fewest
		     * number of steps to find it, so we start with either
		     * the beginning or the end.
		     */
		    if(topthrd->thrdno <= screen->msgmap->max_thrdno/2){
			x = 0L;
			xdir = 1L;
		    }
		    else{
			x = range;
			xdir = -1L;
		    }

		    while(thrd && thrd != topthrd){
			if(!msgline_hidden(screen->stream, screen->msgmap,
					   mn_raw2m(screen->msgmap,thrd->rawno),
					   0))
			  x += xdir;
			
			if(thrddir > 0 && thrd->nextthd)
			  thrd = fetch_thread(screen->stream, thrd->nextthd);
			else if(thrddir < 0 && thrd->prevthd)
			  thrd = fetch_thread(screen->stream, thrd->prevthd);
			else
			  thrd = NULL;
		    }
		}

		scroll_setpos(x);
	    }
	}
	else{
	    long x;

	    /*
	     * This works for forward or reverse sort because the thrdno's
	     * will have been reversed.
	     */
	    thrd = fetch_thread(screen->stream,
				mn_m2raw(screen->msgmap, screen->msg_at_top));
	    if(thrd){
		scroll_setrange(screen->lines_per_page,
				screen->msgmap->max_thrdno - 1L);
		scroll_setpos(thrd->thrdno - 1L);
	    }
	}
    }
    else if(n = any_lflagged(screen->msgmap, MN_HIDE | MN_CHID)){
	long x, range;

	range = mn_get_total(screen->msgmap) - n - 1L;
	scroll_setrange(screen->lines_per_page, range);

	if(range >= screen->lines_per_page){	/* else not needed */
	    if(screen->msg_at_top < mn_get_total(screen->msgmap) / 2){
		for(n = 1, x = 0; n != screen->msg_at_top; n++)
		  if(!msgline_hidden(screen->stream, screen->msgmap, n, 0))
		    x++;
	    }
	    else{
		for(n = mn_get_total(screen->msgmap), x = range;
		    n != screen->msg_at_top; n--)
		  if(!msgline_hidden(screen->stream, screen->msgmap, n, 0))
		    x--;
	    }

	    scroll_setpos(x);
	}
    }
    else{
	scroll_setrange(screen->lines_per_page,
			mn_get_total(screen->msgmap) - 1L);
	scroll_setpos(screen->msg_at_top - 1L);
    }
#endif

    /*
     * Set up c-client call back to tell us about IMAP envelope arrivals
     * Can't do it (easily) if single lines on the screen need information
     * about more than a single message before they can be drawn.
     */
    if(F_OFF(F_QUELL_IMAP_ENV_CB, ps_global) && !THRD_INDX()
       && !(THREADING() && (state->viewing_a_thread
                            || any_lflagged(screen->msgmap, MN_COLL))))
      mail_parameters(NULL, SET_IMAPENVELOPE, (void *) pine_imap_envelope);

    if(THRD_INDX())
      visible = screen->msgmap->visible_threads;
    else if(THREADING() && state->viewing_a_thread){
	/*
	 * We know that all visible messages in the thread are marked
	 * with MN_CHID2.
	 */
	for(visible = 0L, n = screen->msg_at_top;
	    visible < (int) screen->lines_per_page
	    && n <= mn_get_total(screen->msgmap); n++){

	    if(!get_lflag(screen->stream, screen->msgmap, n, MN_CHID2))
	      break;
	    
	    if(!msgline_hidden(screen->stream, screen->msgmap, n, 0))
	      visible++;
	}
    }
    else
      visible = mn_get_total(screen->msgmap)
		  - any_lflagged(screen->msgmap, MN_HIDE|MN_CHID);

    /*---- march thru display lines, painting whatever is needed ----*/
    for(i = 0, n = screen->msg_at_top; i < (int) screen->lines_per_page; i++){
	if(visible == 0L || n < 1 || n > mn_get_total(screen->msgmap)){
	    if(screen->entry_state[i].id != LINE_HASH_N){
		/*
		 * Id is initialized to zero. We do the ClearLine in that
		 * case. If id is any legitimate line_hash value then we
		 * previously drew something here, so clear it. After we
		 * do the ClearLine we set id to LINE_HASH_N which is not
		 * zero and is not a legitimate line_hash value. That means
		 * we've already done the clear and don't have to do it
		 * again.
		 */
		screen->entry_state[i].hilite = 0;
		screen->entry_state[i].bolded = 0;
		screen->entry_state[i].id     = LINE_HASH_N;
		ClearLine(HEADER_ROWS(state) + i);
	    }
	}
	else{
	    HLINE_S *hline;

	    /*
	     * This changes status_col as a side effect so it has to be
	     * executed before next line.
	     */
	    hline = build_header_line(state, screen->stream, screen->msgmap,
				      n, &already_fetched);
	    if(visible > 0L)
	      visible--;

	    if(THRD_INDX()){
		unsigned long rawno;

		rawno = mn_m2raw(screen->msgmap, n);
		if(rawno)
		  thrd = fetch_thread(screen->stream, rawno);
	    }

	    row = paint_index_line(n, hline,
				   i, screen->status_col,
				   !THRD_INDX() ? screen->plus_col : -1,
				   &screen->entry_state[i],
				   mn_is_cur(screen->msgmap, n),
				   THRD_INDX()
				     ? (count_lflags_in_thread(screen->stream,
							       thrd,
							       screen->msgmap,
							       MN_SLCT) > 0)
				     : get_lflag(screen->stream, screen->msgmap,
					         n, MN_SLCT));
	    if(row && retval < 0)
	      retval = row;
	}

	/*--- increment n ---*/
	while((visible == -1L || visible > 0L)
	      && ++n <= mn_get_total(screen->msgmap)
	      && msgline_hidden(screen->stream, screen->msgmap, n, 0))
	  ;
    }

    mail_parameters(NULL, SET_IMAPENVELOPE, (void *) NULL);

#ifdef _WINDOWS
    mswin_endupdate();
#endif
    fflush(stdout);
    dprint(7, (debugfile, "--update_index done\n"));
    return(retval);
}



/*----------------------------------------------------------------------
     Paint the given index line


  Args: hline -- structure describing the header line to paint
	n -- message number to paint
	screen -- structure describing current screen state

  Returns: 0 or the row number if the message is "current"
 ----*/
int
paint_index_line(msg, hline, line, scol, pcol, entry, cur, sel)
    long		msg;
    HLINE_S	       *hline;
    int			line, scol, pcol;
    struct entry_state *entry;
    int			cur, sel;
{
    COLOR_PAIR *lastc = NULL, *base_color = NULL;
    int inverse_hack = 0;
    HLINE_S	      *h;

    h = (THRD_INDX() && hline) ? hline->tihl : hline;

    /* This better not happen! */
    if(!h){
	q_status_message1(SM_ORDER | SM_DING, 5, 5,
			  "NULL hline in paint_index_line: %.200s",
			  THRD_INDX() ? "THRD_INDX" : "reg index");
	dprint(1, (debugfile, "NULL hline in paint_index_line: %s\n",
			       THRD_INDX() ? "THRD_INDX" : "reg index"));
	return 0;
    }

    if(h->id != entry->id || (cur != entry->hilite)
       || (sel != entry->bolded) || (h->plus != entry->plus)){

	if(F_ON(F_FORCE_LOW_SPEED,ps_global) || ps_global->low_speed){
	    MoveCursor(HEADER_ROWS(ps_global) + line, scol);
	    Writechar((sel)
		      ? 'X'
		      : (cur && h->line[scol] == ' ')
			  ? '-'
			  : h->line[scol], 0);
	    Writechar((cur) ? '>' : h->line[scol+1], 0);

	    if(h->id != entry->id){
		if(scol == 0)
		  PutLine0(HEADER_ROWS(ps_global) + line, 2, &h->line[2]);
		else{ /* this will rarely be set up this way */
		    char save_char1, save_char2;

		    save_char1 = h->line[scol];
		    save_char2 = h->line[scol+1];
		    h->line[scol] = (sel) ? 'X' :
		      (cur && save_char1 == ' ') ?
		      '-' : save_char1;
		    h->line[scol+1] = (cur) ? '>' : save_char2;
		    PutLine0(HEADER_ROWS(ps_global) + line, 0, &h->line[0]);
		    h->line[scol]   = save_char1;
		    h->line[scol+1] = save_char2;
		}
	    }

	    if(pcol >= 0){
		MoveCursor(HEADER_ROWS(ps_global) + line, pcol);
		Writechar(h->plus, 0);
		Writechar(' ', 0);
	    }
	}
	else{
	    char *draw = h->line, save_schar, save_pchar, save;
	    int   uc, i, drew_X = 0, cols = ps_global->ttyo->screen_cols;

	    if(uc=pico_usingcolor())
	      lastc = pico_get_cur_color();

	    MoveCursor(HEADER_ROWS(ps_global) + line, 0);

	    if(cur){
		/*
		 * If the current line has a linecolor, apply the
		 * appropriate reverse transformation to show it is current.
		 */
		if(uc && h->linecolor.fg[0] && h->linecolor.bg[0] &&
		   pico_is_good_colorpair(&h->linecolor)){
		    base_color = pico_apply_rev_color(&h->linecolor,
						ps_global->index_color_style);

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

	    save_schar = draw[scol];

	    if(sel && (F_OFF(F_SELECTED_SHOWN_BOLD, ps_global)
		       || !StartBold())){
		draw[scol] = 'X';
		drew_X++;
	    }

	    if(pcol >= 0 && pcol < cols){
		save_pchar = draw[pcol];
		draw[pcol] = h->plus;
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
					      scol != h->offs[i].offset ||
					      save_schar != '+')){
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
					  scol != h->offs[4].offset ||
					  save_schar != '+')){
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
	      draw[scol] = save_schar;

	    if(sel && !drew_X)
	      EndBold();

	    if(cur)
	      EndInverse();

	    if(pcol >= 0 && pcol < cols)
	      draw[pcol] = save_pchar;
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
    entry->plus   = h->plus;

    if(!h->color_lookup_done && pico_usingcolor())
      entry->id = 0;

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

    dprint(7, (debugfile, "imap_env(%ld)\n", rawno));
    if(!ps_global->mail_box_changed
       && stream == ps_global->mail_stream
       && (mc = mail_elt(stream,rawno))->valid
       && mc->rfc822_size
       && !get_lflag(stream, NULL, rawno, MN_HIDE | MN_CHID | MN_EXLD)){
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

	hline = (*format_index_line)(&idata);
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
    PINETHRD_S *thrd;

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

	/*
	 * This test isn't right if there are hidden lines. The line will
	 * fail the test because it seems like it is past the end of the
	 * screen but since the hidden lines don't take up space the line
	 * might actually be on the screen. Don't know that it is worth
	 * it to fix this, though, since you may have to file through
	 * many hidden lines before finding the visible ones. I'm not sure
	 * if the logic inside the if is correct when we do pass the
	 * top-level test. Leave it for now.  Hubert - 2002-06-28
	 */
	if((line = (int)(msgno - current_index_state->msg_at_top)) >= 0
	   && line < current_index_state->lines_per_page){
	    if(any_lflagged(ps_global->msgmap, MN_HIDE | MN_CHID)){
		long n;
		long zoomhide, collapsehide;

		zoomhide = any_lflagged(ps_global->msgmap, MN_HIDE);
		collapsehide = any_lflagged(ps_global->msgmap, MN_CHID);

		/*
		 * Line is visible if it is selected and not hidden due to
		 * thread collapse, or if there is no zooming happening and
		 * it is not hidden due to thread collapse.
		 */
		for(line = 0, n = current_index_state->msg_at_top;
		    n != msgno;
		    n++)
		  if((zoomhide
		      && get_lflag(stream, current_index_state->msgmap,
				   n, MN_SLCT)
		      && (!collapsehide
		          || !get_lflag(stream, current_index_state->msgmap, n,
				        MN_CHID)))
		     ||
		     (!zoomhide
		      && !get_lflag(stream, current_index_state->msgmap,
				    n, MN_CHID)))
		    line++;
	    }

	    thrd = NULL;
	    if(THRD_INDX()){
		unsigned long rawno;

		rawno = mn_m2raw(current_index_state->msgmap, msgno);
		if(rawno)
		  thrd = fetch_thread(stream, rawno);
	    }

	    paint_index_line(msgno, hline, line,
			     current_index_state->status_col,
			     !THRD_INDX()
			       ? current_index_state->plus_col : -1,
			     &current_index_state->entry_state[line],
			     mn_is_cur(current_index_state->msgmap, msgno),
			     THRD_INDX()
			       ? (count_lflags_in_thread(stream, thrd,
						   current_index_state->msgmap,
						         MN_SLCT) > 0)
			       : get_lflag(stream, current_index_state->msgmap,
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

    /*
     * Starting at msg 'pos' find next visible message.
     */
    for(i=pos; i <= mn_get_total(current_index_state->msgmap); i++) {
      if(!msgline_hidden(current_index_state->stream,
			 current_index_state->msgmap, i, 0)){
	  current_index_state->msg_at_top = i;
	  break;
      }
    }

    /*
     * If single selection, move selected message to be on the screen.
     */
    if (mn_total_cur(current_index_state->msgmap) == 1L) {
      if (current_index_state->msg_at_top > 
			      mn_get_cur (current_index_state->msgmap)) {
	/* Selection was above screen, move to top of screen. */
	mn_set_cur(current_index_state->msgmap,current_index_state->msg_at_top);
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
	    if(!msgline_hidden(current_index_state->stream,
			       current_index_state->msgmap, i, 0)){
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
	if(!msgline_hidden(current_index_state->stream,
			   current_index_state->msgmap, i, 0)){
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
	    if(!msgline_hidden(current_index_state->stream,
			       current_index_state->msgmap, i, 0)){
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
	if(!msgline_hidden(current_index_state->stream,
			   current_index_state->msgmap, i, 0)){
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
	    if(!msgline_hidden(current_index_state->stream,
			       current_index_state->msgmap, i, 0)){
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
    long current, hidden, visible, lastn;
    long n, m = 0L, t = 1L;

    current = (mn_total_cur(msgs) <= 1L) ? mn_get_cur(msgs) : at_top;

    if(current < 0L)
      return(-1);

    if(lines_per_page == 0L)
      return(current);

    if(THRD_INDX_ENABLED()){
	long rawno;
	PINETHRD_S *thrd = NULL;

	rawno = mn_m2raw(msgs, mn_get_cur(msgs));
	if(rawno)
	  thrd = fetch_thread(stream, rawno);

	if(THRD_INDX()){

	    if(any_lflagged(msgs, MN_HIDE)){
		long vis = 0L;
		PINETHRD_S *is_current_thrd;

		is_current_thrd = thrd;
		if(is_current_thrd){
		    if(mn_get_revsort(msgs)){
			/* start with top of tail of thread list */
			thrd = fetch_thread(stream, mn_m2raw(msgs, 1L));
			if(thrd && thrd->top && thrd->top != thrd->rawno)
			  thrd = fetch_thread(stream, thrd->top);
		    }
		    else{
			/* start with head of thread list */
			thrd = fetch_head_thread(stream);
		    }

		    t = 1L;
		    m = 0L;
		    if(thrd)
		      n = mn_raw2m(msgs, thrd->rawno);

		    while(thrd){
			if(!msgline_hidden(stream, msgs, n, 0)
			   && (++m % lines_per_page) == 1L)
			  t = n;
			
			if(thrd == is_current_thrd)
			  break;

			if(mn_get_revsort(msgs) && thrd->prevthd)
			  thrd = fetch_thread(stream, thrd->prevthd);
			else if(!mn_get_revsort(msgs) && thrd->nextthd)
			  thrd = fetch_thread(stream, thrd->nextthd);
			else
			  thrd = NULL;

			if(thrd)
			  n = mn_raw2m(msgs, thrd->rawno);
		    }
		}
	    }
	    else{
		if(thrd){
		    n = thrd->thrdno;
		    m = lines_per_page * ((n - 1L)/ lines_per_page) + 1L;
		    n = thrd->rawno;
		    /*
		     * We want to find the m'th thread and the
		     * message number that goes with that. We just have
		     * to back up from where we are to get there.
		     * If we have a reverse sort backing up is going
		     * forward through the thread.
		     */
		    while(thrd && m < thrd->thrdno){
			n = thrd->rawno;
			if(mn_get_revsort(msgs) && thrd->nextthd)
			  thrd = fetch_thread(stream, thrd->nextthd);
			else if(!mn_get_revsort(msgs) && thrd->prevthd)
			  thrd = fetch_thread(stream, thrd->prevthd);
			else
			  thrd = NULL;
		    }

		    if(thrd)
		      n = thrd->rawno;

		    t = mn_raw2m(msgs, n);
		}
	    }
	}
	else{		/* viewing a thread */

	    lastn = mn_get_total(msgs);
	    t = 1L;

	    /* get top of thread */
	    if(thrd && thrd->top && thrd->top != thrd->rawno)
	      thrd = fetch_thread(stream, thrd->top);

	    if(thrd){
		if(mn_get_revsort(msgs))
		  lastn = mn_raw2m(msgs, thrd->rawno);
		else
		  t = mn_raw2m(msgs, thrd->rawno);
	    }

	    n = 0L;

	    /* n is the end of this thread */
	    while(thrd){
		n = mn_raw2m(msgs, thrd->rawno);
		if(thrd->branch)
		  thrd = fetch_thread(stream, thrd->branch);
		else if(thrd->next)
		  thrd = fetch_thread(stream, thrd->next);
		else
		  thrd = NULL;
	    }

	    if(n){
		if(mn_get_revsort(msgs))
		  t = n;
		else
		  lastn = n;
	    }

	    for(m = 0L, n = t; n <= min(current, lastn); n++)
	      if(!msgline_hidden(stream, msgs, n, 0)
		 && (++m % lines_per_page) == 1L)
		t = n;
	}

	return(t);
    }
    else if(hidden = any_lflagged(msgs, MN_HIDE | MN_CHID)){

	if(current < mn_get_total(msgs) / 2){
	    t = 1L;
	    m = 0L;
	    for(n = 1L; n <= min(current, mn_get_total(msgs)); n++)
	      if(!msgline_hidden(stream, msgs, n, 0)
		 && (++m % lines_per_page) == 1L)
	        t = n;
	}
	else{
	    t = current+1L;
	    m = mn_get_total(msgs)-hidden+1L;
	    for(n = mn_get_total(msgs); n >= 1L && t > current; n--)
	      if(!msgline_hidden(stream, msgs, n, 0)
		 && (--m % lines_per_page) == 1L)
	        t = n;
	    
	    if(t > current)
	      t = 1L;
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

    /*
     * Record the fact that SCORE appears in some index format. This
     * is a heavy-handed approach. It will stick at 1 if any format ever
     * contains score during this session. This is ok since it will just
     * cause recalculation if wrong and these things rarely change much.
     */
    if(!ps_global->a_format_contains_score && format
       && strstr(format, "SCORE")){
	ps_global->a_format_contains_score = 1;
	/* recalculate need for scores */
	scores_are_used(SCOREUSE_INVALID);
    }

    set_need_format_setup();
    /* if custom format is specified, try it, else go with default */
    if(!(format && *format && parse_index_format(format, answer))){
	static INDEX_COL_S answer_default[] = {
	    {iStatus, Fixed, 3},
	    {iMessNo, WeCalculate},
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
	      case iYear2Digit:
	      case iDay:
	      case iMon:
	      case iDay2Digit:
	      case iMon2Digit:
		(*answer)[column].req_width = 2;
		break;
	      case iStatus:
	      case iMessNo:
	      case iMonAbb:
	      case iInit:
	      case iDayOfWeekAbb:
		(*answer)[column].req_width = 3;
		break;
	      case iYear:
	      case iDayOrdinal:
		(*answer)[column].req_width = 4;
		break;
	      case iTime24:
	      case iTimezone:
	      case iSizeNarrow:
		(*answer)[column].req_width = 5;
		break;
	      case iFStatus:
	      case iIStatus:
	      case iDate:
	      case iScore:
		(*answer)[column].req_width = 6;
		break;
	      case iTime12:
	      case iSTime:
	      case iKSize:
	      case iSize:
		(*answer)[column].req_width = 7;
		break;
	      case iS1Date:
	      case iS2Date:
	      case iS3Date:
	      case iS4Date:
	      case iDateIsoS:
	      case iSizeComma:
		(*answer)[column].req_width = 8;
		break;
	      case iDescripSize:
	      case iSDate:
	      case iSDateTime:
	      case iMonLong:
	      case iDayOfWeek:
		(*answer)[column].req_width = 9;
		break;
	      case iDateIso:
		(*answer)[column].req_width = 10;
		break;
	      case iLDate:
		(*answer)[column].req_width = 12;
		break;
	      case iRDate:
		(*answer)[column].req_width = 16;
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
    {"SIZECOMMA",	iSizeComma,	FOR_INDEX},
    {"SIZENARROW",	iSizeNarrow,	FOR_INDEX},
    {"KSIZE",		iKSize,		FOR_INDEX},
    {"SUBJECT",		iSubject,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"FULLSTATUS",	iFStatus,	FOR_INDEX},
    {"IMAPSTATUS",	iIStatus,	FOR_INDEX},
    {"DESCRIPSIZE",	iDescripSize,	FOR_INDEX},
    {"ATT",		iAtt,		FOR_INDEX},
    {"SCORE",		iScore,		FOR_INDEX},
    {"LONGDATE",	iLDate,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SHORTDATE1",	iS1Date,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SHORTDATE2",	iS2Date,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SHORTDATE3",	iS3Date,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SHORTDATE4",	iS4Date,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"DATEISO",		iDateIso,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SHORTDATEISO",	iDateIsoS,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SMARTDATE",	iSDate,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SMARTTIME",	iSTime,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"SMARTDATETIME",	iSDateTime,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"TIME24",		iTime24,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"TIME12",		iTime12,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"TIMEZONE",	iTimezone,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"MONTHABBREV",	iMonAbb,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"DAYOFWEEKABBREV",	iDayOfWeekAbb,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"DAYOFWEEK",	iDayOfWeek,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
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
    {"DAYDATE",		iRDate,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"DAY",		iDay,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"DAYORDINAL",	iDayOrdinal,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"DAY2DIGIT",	iDay2Digit,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"MONTHLONG",	iMonLong,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"MONTH",		iMon,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"MONTH2DIGIT",	iMon2Digit,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"YEAR",		iYear,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"YEAR2DIGIT",	iYear2Digit,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"ADDRESS",		iAddress,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"MAILBOX",		iMailbox,	FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"ROLENICK",       	iRoleNick,	FOR_REPLY_INTRO|FOR_TEMPLATE},
    {"INIT",		iInit,		FOR_INDEX|FOR_REPLY_INTRO|FOR_TEMPLATE},
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
			      "Unrecognized string in index-format: %.200s", q);
	    continue;
	}

	cdesc[column].ctype = pt->ctype;

	/* skip over name and look for parens */
	p += strlen(pt->name);
	if(*p == '('){
	    p++;
	    q = p;
	    while(p && *p && isdigit((unsigned char) *p))
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


/*
 * These types are basically fixed in width.
 * The order is slightly significant. The ones towards the front of the
 * list get space allocated sooner than the ones at the end of the list.
 */
static IndexColType fixed_ctypes[] = {
    iMessNo, iStatus, iFStatus, iIStatus, iDate, iSDate, iSDateTime,
    iSTime, iLDate,
    iS1Date, iS2Date, iS3Date, iS4Date, iDateIso, iDateIsoS,
    iSize, iSizeComma, iSizeNarrow, iKSize, iDescripSize,
    iAtt, iTime24, iTime12, iTimezone, iMonAbb, iYear, iYear2Digit,
    iDay2Digit, iMon2Digit, iDayOfWeekAbb, iScore
};


int
ctype_is_fixed_length(ctype)
    IndexColType ctype;
{
    int j;

    for(j = 0; ; j++){
	if(j >= sizeof(fixed_ctypes)/sizeof(*fixed_ctypes))
	  break;
    
	if(ctype == fixed_ctypes[j])
	  return 1;
    }

    return 0;
}
    

/*----------------------------------------------------------------------
      Setup the widths of the various columns in the index display
 ----*/
void
setup_index_header_widths()
{
    int		 j, columns, some_to_calculate;
    int		 space_left, screen_width, width, fix, col, scol, altcol;
    int          pcol, pluscol;
    int		 keep_going, tot_pct, was_sl;
    long         max_msgno;
    WidthType	 wtype;
    INDEX_COL_S *cdesc;

    max_msgno = mn_get_total(ps_global->msgmap);

    dprint(8, (debugfile, "=== setup_index_header_widths() ===\n"));

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

	if(cdesc->wtype == Fixed){
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

    /*
     * Set the actual lengths for the fixed width fields and set up
     * the left or right adjustment for everything.
     * There should be a case setting actual_length for all of the types
     * in fixed_ctypes.
     */
    for(cdesc = ps_global->index_disp_format;
	cdesc->ctype != iNothing;
	cdesc++){

	wtype = cdesc->wtype;

	if(wtype == WeCalculate || wtype == Percent || cdesc->width != 0){
	    if(ctype_is_fixed_length(cdesc->ctype)){
		switch(cdesc->ctype){
		  case iAtt:
		    cdesc->actual_length = 1;
		    cdesc->adjustment = Left;
		    break;

		  case iYear2Digit:
		  case iDay2Digit:
		  case iMon2Digit:
		    cdesc->actual_length = 2;
		    cdesc->adjustment = Left;
		    break;

		  case iStatus:
		  case iMonAbb:
		  case iDayOfWeekAbb:
		    cdesc->actual_length = 3;
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

		  case iYear:
		    cdesc->actual_length = 4;
		    cdesc->adjustment = Left;
		    break;

		  case iTime24:
		  case iTimezone:
		    cdesc->actual_length = 5;
		    cdesc->adjustment = Left;
		    break;

		  case iSizeNarrow:
		    cdesc->actual_length = 5;
		    cdesc->adjustment = Right;
		    break;

		  case iFStatus:
		  case iIStatus:
		  case iDate:
		    cdesc->actual_length = 6;
		    cdesc->adjustment = Left;
		    break;

		  case iScore:
		    cdesc->actual_length = 6;
		    cdesc->adjustment = Right;
		    break;

		  case iTime12:
		  case iSize:
		  case iKSize:
		    cdesc->actual_length = 7;
		    cdesc->adjustment = Right;
		    break;

		  case iSTime:
		    set_format_includes_smartdate();
		    cdesc->actual_length = 7;
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

		  case iSizeComma:
		    cdesc->actual_length = 8;
		    cdesc->adjustment = Right;
		    break;

		  case iSDate:
		  case iSDateTime:
		    set_format_includes_smartdate();
		    cdesc->actual_length = 9;
		    cdesc->adjustment = Left;
		    break;

		  case iDescripSize:
		    cdesc->actual_length = 9;
		    cdesc->adjustment = Right;
		    break;

		  case iDateIso:
		    cdesc->actual_length = 10;
		    cdesc->adjustment = Left;
		    break;

		  case iLDate:
		    cdesc->actual_length = 12;
		    cdesc->adjustment = Left;
		    break;
		  
		  default:
		    panic("Unhandled fixed case in setup_index_header");
		    break;
		}
	    }
	    else
	      cdesc->adjustment = Left;
	}
    }

    /* if have reserved unneeded space for size, give it back */
    for(cdesc = ps_global->index_disp_format;
	cdesc->ctype != iNothing;
	cdesc++)
      if(cdesc->ctype == iSize || cdesc->ctype == iKSize ||
         cdesc->ctype == iSizeNarrow ||
	 cdesc->ctype == iSizeComma || cdesc->ctype == iDescripSize){
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
     * The set of fixed_ctypes here is the same as the set where we
     * set the actual_lengths above.
     */
    for(j = 0; space_left > 0 && some_to_calculate; j++){

      if(j >= sizeof(fixed_ctypes)/sizeof(*fixed_ctypes))
	break;

      for(cdesc = ps_global->index_disp_format;
	  cdesc->ctype != iNothing && space_left > 0 && some_to_calculate;
	  cdesc++)
	if(cdesc->ctype == fixed_ctypes[j] && cdesc->wtype == WeCalculate){
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
	int tot_requested = 0;

	/*
	 * Requests are treated as percent of screen width. See if they
	 * will all fit. If not, trim them back proportionately.
	 */
	for(cdesc = ps_global->index_disp_format;
	    cdesc->ctype != iNothing;
	    cdesc++){
	  if(cdesc->wtype == Percent){
	      /* The 2, 200, and +100 are because we're rounding */
	      fix = ((2*cdesc->req_width *
		      (screen_width-(columns-1)))+100) / 200;
	      tot_requested += fix;
	  }
	}

	if(tot_requested > space_left){
	  int multiplier = (100 * space_left) / tot_requested;

	  for(cdesc = ps_global->index_disp_format;
	      cdesc->ctype != iNothing && space_left > 0;
	      cdesc++){
	    if(cdesc->wtype == Percent){
	        /* The 2, 200, and +100 are because we're rounding */
	        fix = ((2*cdesc->req_width *
		        (screen_width-(columns-1)))+100) / 200;
		fix = (2 * fix * multiplier + 100) / 200;
	        fix = min(fix, space_left);
	        cdesc->width += fix;
	        space_left -= fix;
	    }
	  }
	}
	else{
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
	if(cdesc->wtype == WeCalculate && !ctype_is_fixed_length(cdesc->ctype)){
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
	if(cdesc->wtype == Percent && !ctype_is_fixed_length(cdesc->ctype)){
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
    /* figure out which column is start of status field */
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

    col = 0;
    pluscol = -1;
    /* figure out which column to use for threading '+' */
    if(THREADING()
       && ps_global->thread_disp_style != THREAD_NONE
       && ps_global->VAR_THREAD_MORE_CHAR[0]
       && ps_global->VAR_THREAD_EXP_CHAR[0])
      for(cdesc = ps_global->index_disp_format;
	cdesc->ctype != iNothing;
	cdesc++){
	width = cdesc->width;
	if(width == 0)
	  continue;

	/* space between columns */
	if(col > 0)
	  col++;

	if(cdesc->ctype == iSubject
	   && (ps_global->thread_disp_style == THREAD_STRUCT
	       || ps_global->thread_disp_style == THREAD_MUTTLIKE
	       || ps_global->thread_disp_style == THREAD_INDENT_SUBJ1
	       || ps_global->thread_disp_style == THREAD_INDENT_SUBJ2)){
	    pluscol = col;
	    break;
	}

	if((cdesc->ctype == iFrom
	    || cdesc->ctype == iFromToNotNews
	    || cdesc->ctype == iFromTo
	    || cdesc->ctype == iAddress
	    || cdesc->ctype == iMailbox)
	   && (ps_global->thread_disp_style == THREAD_INDENT_FROM1
	       || ps_global->thread_disp_style == THREAD_INDENT_FROM2
	       || ps_global->thread_disp_style == THREAD_STRUCT_FROM)){
	    pluscol = col;
	    break;
	}

	col += width;
      }

    if(current_index_state){
	current_index_state->status_col = scol;
	current_index_state->plus_col = pluscol;
    }
}


void
setup_thread_header_widths()
{
    clear_icache_flags();
    if(current_index_state){
	current_index_state->status_col = 0;
	current_index_state->plus_col = -1;
    }
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
build_header_line(state, stream, msgmap, msgno, already_fetched)
    struct pine *state;
    MAILSTREAM  *stream;
    MSGNO_S     *msgmap;
    long         msgno;
    int         *already_fetched;
{
    HLINE_S	 *hline;
    MESSAGECACHE *mc;
    long          n, i, cnt, rawno, visible, limit = -1L;

    /* cache hit? */
    if(THRD_INDX()){
	hline = get_index_cache(msgno);
	if(hline->tihl && *hline->tihl->line && hline->tihl->color_lookup_done){
	    dprint(9, (debugfile, "Hitt: Returning %p -> <%s (%d), 0x%lx>\n",
		       hline->tihl, hline->tihl->line,
		       strlen(hline->tihl->line), hline->tihl->id));
	    return(hline);
	}
    }
    else{
	if(*(hline = get_index_cache(msgno))->line && hline->color_lookup_done){
	    dprint(9, (debugfile, "Hit: Returning %p -> <%s (%d), 0x%lx>\n",
		       hline, hline->line, strlen(hline->line), hline->id));
	    return(hline);
	}
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
    if(!(already_fetched && *already_fetched) && index_in_overview(stream)
       && ((THRD_INDX() && !(hline->tihl && *hline->tihl->line))
           || (!THRD_INDX() && !*hline->line))){
	char	     *uidseq, *p;
	long	      uid, next;
	int	      count;
	MESSAGECACHE *mc;
	PINETHRD_S   *thrd;

	if(already_fetched)
	  (*already_fetched)++;

	/* clear sequence bits */
	for(n = 1L; n <= stream->nmsgs; n++)
	  mail_elt(stream, n)->sequence = 0;

	/*
	 * Light interesting bits
	 * NOTE: not set above because m2raw's cheaper
	 * than raw2m for every message
	 */

	/*
	 * Unfortunately, it is expensive to calculate visible pages
	 * in thread index if we are zoomed, so we don't try.
	 */
	if(THRD_INDX() && any_lflagged(msgmap, MN_HIDE))
	  visible = msgmap->visible_threads;
	else if(THREADING() && state->viewing_a_thread){
	    /*
	     * We know that all visible messages in the thread are marked
	     * with MN_CHID2.
	     */
	    for(visible = 0L, n = current_index_state->msg_at_top;
		visible < (int) current_index_state->lines_per_page
		&& n <= mn_get_total(msgmap); n++){

		if(!get_lflag(stream, msgmap, n, MN_CHID2))
		  break;
		
		if(!msgline_hidden(stream, msgmap, n, 0))
		  visible++;
	    }
	    
	}
	else
	  visible = mn_get_total(msgmap)
		      - any_lflagged(msgmap, MN_HIDE|MN_CHID);

	limit = min(visible, current_index_state->lines_per_page);

	if(THRD_INDX()){
	    HLINE_S    *h;

	    thrd = fetch_thread(stream,
				mn_m2raw(msgmap,
					 current_index_state->msg_at_top));
	    /*
	     * Loop through visible threads, marking them for fetching.
	     * Stop at end of screen or sooner if we run out of visible
	     * threads.
	     */
	    count = i = 0;
	    while(thrd){
		n = mn_raw2m(msgmap, thrd->rawno);
		if(n >= msgno
		   && n <= mn_get_total(msgmap)
		   && !((h=get_index_cache(n)->tihl) && *h->line)){
		    count += mark_msgs_in_thread(stream, thrd, msgmap);
		}

		if(++i >= limit)
		  break;

		/* find next thread which is visible */
		do{
		    if(mn_get_revsort(msgmap) && thrd->prevthd)
		      thrd = fetch_thread(stream, thrd->prevthd);
		    else if(!mn_get_revsort(msgmap) && thrd->nextthd)
		      thrd = fetch_thread(stream, thrd->nextthd);
		    else
		      thrd = NULL;
		} while(thrd
			&& msgline_hidden(stream, msgmap,
					  mn_raw2m(msgmap, thrd->rawno), 0));
	    }
	}
	else{
	    count = i = 0;
	    n = current_index_state->msg_at_top;
	    while(1){
		if(n >= msgno
		   && n <= mn_get_total(msgmap)
		   && !*get_index_cache(n)->line){
		    rawno = mn_m2raw(msgmap, n);
		    /*
		     * When a thread is expanded the msgline_hidden returns
		     * 0 below and we are counting many of the messages
		     * in a thread more than once. For example, if the
		     * thread has 3 members, the first call marks 3,
		     * then the next call marks the 2nd two, and the third
		     * call marks the 3rd for the 3rd time. So the count
		     * would be 6 instead of 3. The only implication is
		     * that the alloc below is too big, so we just
		     * don't care.
		     */
		    if(thrd = fetch_thread(stream, rawno))
		      count += mark_msgs_in_thread(stream, thrd, msgmap);
		    else if(!(mc = mail_elt(stream,rawno))->private.msg.env){
			mc->sequence = 1;
			count++;
		    }
		}

		if(++i >= limit)
		  break;

		/* find next n which is visible */
		while(++n <=  mn_get_total(msgmap)
		      && msgline_hidden(stream, msgmap, n, 0))
		  ;
	    }
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

    if((THRD_INDX() && !(hline->tihl && *hline->tihl->line))
       || (!THRD_INDX() && !*hline->line)){
	INDEXDATA_S idata;

	/*
	 * With pre-fetching/callback-formatting done and no success,
	 * fall into formatting the requested line...
	 */
	memset(&idata, 0, sizeof(INDEXDATA_S));
	idata.stream   = stream;
	idata.msgno    = msgno;
	idata.rawno    = mn_m2raw(msgmap, msgno);
	if(mc = mail_elt(stream, idata.rawno)){
	    idata.size = mc->rfc822_size;
	    index_data_env(&idata, mail_fetchenvelope(stream, idata.rawno));
	}
	else
	  idata.bogus = 2;

	hline = (*format_index_line)(&idata);
    }

    if(THRD_INDX() && hline->tihl)
      hline->tihl->color_lookup_done = 1;

    /*
     * Look for a color for this line (and other lines in the current
     * view). This does a SEARCH for each role which has a color until
     * it finds a match. This will be satisfied by the c-client
     * cache created by the mail_fetch_overview above if it is a header
     * search.
     */
    if(!THRD_INDX() && !hline->color_lookup_done){
	COLOR_PAIR *linecolor;
	SEARCHSET  *ss, *s;
	HLINE_S    *h;
	PAT_STATE  *pstate = NULL;

	if(pico_usingcolor()){
	    if(limit < 0L){
		if(THREADING() && state->viewing_a_thread){
		    for(visible = 0L, n = current_index_state->msg_at_top;
			visible < (int) current_index_state->lines_per_page
			&& n <= mn_get_total(msgmap); n++){

			if(!get_lflag(stream, msgmap, n, MN_CHID2))
			  break;
			
			if(!msgline_hidden(stream, msgmap, n, 0))
			  visible++;
		    }
		    
		}
		else
		  visible = mn_get_total(msgmap)
			      - any_lflagged(msgmap, MN_HIDE|MN_CHID);

		limit = min(visible, current_index_state->lines_per_page);
	    }
	    /* clear sequence bits */
	    for(n = 1L; n <= stream->nmsgs; n++)
	      mail_elt(stream, n)->sequence = 0;

	    cnt = i = 0;
	    n = current_index_state->msg_at_top;
	    while(1){
		if(n >= msgno
		   && n <= mn_get_total(msgmap)
		   && !(h=get_index_cache(n))->color_lookup_done){

		    mail_elt(stream,mn_m2raw(msgmap,n))->sequence = 1;
		    cnt++;
		}

		if(++i >= limit)
		  break;

		/* find next n which is visible */
		while(++n <=  mn_get_total(msgmap)
		      && msgline_hidden(stream, msgmap, n, 0))
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
		    int colormatch;

		    linecolor = NULL;
		    colormatch = get_index_line_color(stream, ss, &pstate,
						      &linecolor);

		    /*
		     * Assign this color to all matched msgno's and
		     * turn off the sequence bit so we won't check
		     * for them again.
		     */
		    if(colormatch){
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
				    if(linecolor){
					strcpy(h->linecolor.fg, linecolor->fg);
					strcpy(h->linecolor.bg, linecolor->bg);
				    }
				    else{
					h->linecolor.fg[0] = '\0';
					h->linecolor.bg[0] = '\0';
				    }
				}
			    }
			  }
			}

			if(linecolor)
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

    return(hline);		/* Return formatted index data */
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
clear_folder_scores(stream)
    MAILSTREAM *stream;
{
    long n;

    for(n = 1L; n <= stream->nmsgs; n++)
      clear_msg_score(stream, n);
}


void
clear_msg_score(stream, rawmsgno)
    MAILSTREAM *stream;
    long        rawmsgno;
{
    set_msg_score(stream, rawmsgno, SCORE_UNDEF);
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
format_index_index_line(idata)
    INDEXDATA_S	*idata;
{
    char          str_buf[MAXIFLDS][MAX_SCREEN_COLS+1], to_us, status, *field,
		 *buffer, *s_tmp, *p, *str, *newsgroups;
    int		  width, offsets_set = 0, i, j, smallest, which_array = 0;
    int           plus_off = -1, imp_off = -1, del_off = -1, ans_off = -1,
		  new_off = -1, rec_off = -1, uns_off = -1, status_offset = 0,
		  score, collapsed = 0;
    long	  l;
    HLINE_S	 *hline;
    BODY	 *body = NULL;
    MESSAGECACHE *mc;
    ADDRESS      *addr, *toaddr, *ccaddr, *last_to;
    PINETHRD_S   *thrd = NULL;
    INDEX_COL_S	 *cdesc = NULL;
    struct variable *vars = ps_global->vars;

    dprint(8, (debugfile, "=== format_index_line(%ld,%ld) ===\n",
	       idata ? idata->msgno : -1, idata ? idata->rawno : -1));

    memset(str_buf, 0, sizeof(str_buf));

    hline = get_index_cache(idata->msgno);

    /* is this a collapsed thread index line? */
    if(!idata->bogus && THREADING()){
	thrd = fetch_thread(idata->stream, idata->rawno);
	collapsed = thrd && thrd->next
		    && get_lflag(idata->stream, NULL,
				 idata->rawno, MN_COLL);
    }

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
		to_us = status = ' ';
		if(collapsed){
		    thrd = fetch_thread(idata->stream, idata->rawno);
		    to_us = to_us_symbol_for_thread(idata->stream, thrd, 1);
		    status = status_symbol_for_thread(idata->stream, thrd,
						      cdesc->ctype);
		}
		else{
		    if((mc=mail_elt(idata->stream, idata->rawno))->flagged)
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

		    status = (!idata->stream || !IS_NEWS(idata->stream)
			      || F_ON(F_FAKE_NEW_IN_NEWS, ps_global))
			       ? 'N' : ' ';

		     if(mc->seen)
		       status = ' ';

		     if(mc->answered)
		       status = 'A';

		     if(mc->deleted)
		       status = 'D';
		}

		sprintf(str, "%c %c", to_us, status);

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

		  if(collapsed){
		      thrd = fetch_thread(idata->stream, idata->rawno);
		      to_us = to_us_symbol_for_thread(idata->stream, thrd, 0);
		  }
		  else{
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
		  }

		  new = answered = deleted = flagged = ' ';

		  if(collapsed){
		      unsigned long save_branch, cnt, tot_in_thrd;

		      /*
		       * Branch is a sibling, not part of the thread, so
		       * don't consider it when displaying this line.
		       */
		      save_branch = thrd->branch;
		      thrd->branch = 0L;

		      tot_in_thrd = count_flags_in_thread(idata->stream, thrd,
							  F_NONE);

		      cnt = count_flags_in_thread(idata->stream, thrd, F_DEL);
		      if(cnt)
			deleted = (cnt == tot_in_thrd) ? 'D' : 'd';

		      cnt = count_flags_in_thread(idata->stream, thrd, F_ANS);
		      if(cnt)
			answered = (cnt == tot_in_thrd) ? 'A' : 'a';

		      /* no lower case *, same thing for some or all */
		      if(count_flags_in_thread(idata->stream, thrd, F_FLAG))
			flagged = '*';

		      new = status_symbol_for_thread(idata->stream, thrd,
						     cdesc->ctype);

		      thrd->branch = save_branch;
		  }
		  else{
		      mc = mail_elt(idata->stream, idata->rawno);
		      if(mc->valid){
			  if(cdesc->ctype == iIStatus){
			      if(mc->recent)
				new = mc->seen ? 'R' : 'N';
			      else if (!mc->seen)
				new = 'U';
			  }
			  else if(!mc->seen
				  && (!IS_NEWS(idata->stream)
				      || F_ON(F_FAKE_NEW_IN_NEWS, ps_global)))
			    new = 'N';

			  if(mc->answered)
			    answered = 'A';

			  if(mc->deleted)
			    deleted = 'D';

			  if(mc->flagged)
			    flagged = '*';
		      }
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

	      case iScore:
		score = get_msg_score(idata->stream, idata->rawno);
		if(score == SCORE_UNDEF){
		    SEARCHSET *ss = NULL;

		    ss = mail_newsearchset();
		    ss->first = ss->last = (unsigned long) idata->rawno;
		    if(ss){
			/*
			 * This looks like it might be expensive to get the
			 * score for each message when needed but it shouldn't
			 * be too bad because we know we have the envelope
			 * data cached. We can't calculate all of the scores
			 * we need for the visible messages right here in
			 * one fell swoop because we don't have the other
			 * envelopes yet. And we can't get the other
			 * envelopes at this point because we may be in
			 * the middle of a c-client callback (pine_imap_env).
			 * (Actually we could, because we know whether or
			 * not we're in the callback because of the no_fetch
			 * parameter.)
			 * We have another problem if the score rules depend
			 * on something other than envelope data. I guess they
			 * only do that if they have an alltext (search the
			 * text of the message) definition. So, we're going
			 * to pass no_fetch to calculate_scores so that it
			 * can return an error if we need the text data but
			 * can't get it because of no_fetch. Setting bogus
			 * will cause us to do the scores calculation later
			 * when we are no longer in the callback.
			 */
			idata->bogus =
			    (calculate_some_scores(current_index_state->stream,
						   ss, idata->no_fetch) == 0)
					? 1 : 0;
			score = get_msg_score(idata->stream, idata->rawno);
			mail_free_searchset(&ss);
		    }
		}

		sprintf(str, "%d", score != SCORE_UNDEF ? score : 0);
		break;

	      case iDate:
	      case iMonAbb:
	      case iLDate:
	      case iSDate:
	      case iSTime:
	      case iSDateTime:
	      case iS1Date:
	      case iS2Date:
	      case iS3Date:
	      case iS4Date:
	      case iDateIso:
	      case iDateIsoS:
	      case iTime24:
	      case iTime12:
	      case iTimezone:
	      case iYear:
	      case iYear2Digit:
	      case iRDate:
	      case iDay:
	      case iDay2Digit:
	      case iMon2Digit:
	      case iDayOrdinal:
	      case iMon:
	      case iMonLong:
	      case iDayOfWeekAbb:
	      case iDayOfWeek:
		date_str(fetch_date(idata), cdesc->ctype, 0, str);
		break;

	      case iFromTo:
	      case iFromToNotNews:
	      case iFrom:
	      case iAddress:
	      case iMailbox:
		from_str(cdesc->ctype, idata, width, str);
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

	      case iInit:
		{ADDRESS *addr;

		 if((addr = fetch_from(idata)) && addr->personal){
		    char *name, *initials = NULL, *dummy = NULL;

			
		    name = (char *) rfc1522_decode((unsigned char *)tmp_20k_buf,
						   SIZEOF_20KBUF,
						   addr->personal, &dummy);
		    if(dummy)
		      fs_give((void **)&dummy);

		    if(name == addr->personal){
			strncpy(tmp_20k_buf, name, SIZEOF_20KBUF-1);
			tmp_20k_buf[SIZEOF_20KBUF - 1] = '\0';
			name = (char *) tmp_20k_buf;
		    }

		    if(name && *name){
			initials = reply_quote_initials(name);
			sprintf(str, "%-*.*s", width, width, initials);
		    }
		 }
		}

	        break;

	      case iSize:
		/* 0 ... 9999 */
		if((l = fetch_size(idata)) < 10*1000L)
		  sprintf(str, "(%lu)", l);
		/* 10K ... 999K */
		else if(l < 1000L*1000L - 1000L/2){
		    l = l/1000L + (l%1000L >= 1000L/2 ? 1L : 0L);
		    sprintf(str, "(%luK)", l);
		}
		/* 1.0M ... 99.9M */
		else if(l < 1000L*100L*1000L - 100L*1000L/2){
		    l = l/(100L*1000L) + (l%(100L*1000L) >= (100*1000L/2)
								? 1L : 0L);
		    sprintf(str, "(%lu.%luM)", l/10L, l % 10L);
		}
		/* 100M ... 2000M */
		else if(l <= 2*1000L*1000L*1000L){
		    l = l/(1000L*1000L) + (l%(1000L*1000L) >= (1000L*1000L/2)
								? 1L : 0L);
		    sprintf(str, "(%luM)", l);
		}
		else
		  strcpy(str, "(HUGE!)");

		break;

	      case iSizeComma:
		/* 0 ... 99,999 */
		if((l = fetch_size(idata)) < 100*1000L)
		  sprintf(str, "(%s)", comatose(l));
		/* 100K ... 9,999K */
		else if(l < 10L*1000L*1000L - 1000L/2){
		    l = l/1000L + (l%1000L >= 1000L/2 ? 1L : 0L);
		    sprintf(str, "(%sK)", comatose(l));
		}
		/* 10.0M ... 999.9M */
		else if(l < 1000L*1000L*1000L - 100L*1000L/2){
		    l = l/(100L*1000L) + (l%(100L*1000L) >= (100*1000L/2)
								? 1L : 0L);
		    sprintf(str, "(%lu.%luM)", l/10L, l % 10L);
		}
		/* 1,000M ... 2,000M */
		else if(l <= 2*1000L*1000L*1000L){
		    l = l/(1000L*1000L) + (l%(1000L*1000L) >= (1000L*1000L/2)
								? 1L : 0L);
		    sprintf(str, "(%sM)", comatose(l));
		}
		else
		  strcpy(str, "(HUGE!)");

		break;

	      case iSizeNarrow:
		/* 0 ... 999 */
		if((l = fetch_size(idata)) < 1000L)
		  sprintf(str, "(%lu)", l);
		/* 1K ... 99K */
		else if(l < 100L*1000L - 1000L/2){
		    l = l/1000L + (l%1000L >= 1000L/2 ? 1L : 0L);
		    sprintf(str, "(%luK)", l);
		}
		/* .1M ... .9M */
		else if(l < 1000L*1000L - 100L*1000L/2){
		    l = l/(100L*1000L) + (l%(100L*1000L) >= 100L*1000L/2
								? 1L : 0L);
		    sprintf(str, "(.%luM)", l);
		}
		/* 1M ... 99M */
		else if(l < 1000L*100L*1000L - 1000L*1000L/2){
		    l = l/(1000L*1000L) + (l%(1000L*1000L) >= (1000L*1000L/2)
								? 1L : 0L);
		    sprintf(str, "(%luM)", l);
		}
		/* .1G ... .9G */
		else if(l < 1000L*1000L*1000L - 100L*1000L*1000L/2){
		    l = l/(100L*1000L*1000L) + (l%(100L*1000L*1000L) >=
					    (100L*1000L*1000L/2) ? 1L : 0L);
		    sprintf(str, "(.%luG)", l);
		}
		/* 1G ... 2G */
		else if(l <= 2*1000L*1000L*1000L){
		    l = l/(1000L*1000L*1000L) + (l%(1000L*1000L*1000L) >=
					    (1000L*1000L*1000L/2) ? 1L : 0L);
		    sprintf(str, "(%luG)", l);
		}
		else
		  strcpy(str, "(HUGE!)");

		break;

	      /* From Carl Jacobsen <carl@ucsd.edu> */
	      case iKSize:
		l = fetch_size(idata);
		l = (l / 1024L) + (l % 1024L != 0 ? 1 : 0);

		if(l < 1024L) {				/* 0k .. 1023k */
		  sprintf(str, "(%luk)", l);

		} else if (l < 100L * 1024L){		/* 1.0M .. 99.9M */
		  sprintf(str, "(%lu.M)", (l * 10L) / 1024L);
		  if ((p = strchr(str, '.')) != NULL) {
		    p--; p[1] = p[0]; p[0] = '.';  /* swap last digit & . */
		  }
		} else if (l <= 2L * 1024L * 1024L) {	/* 100M .. 2048 */
		  sprintf(str, "(%luM)", l / 1024L);
		} else {
		  strcpy(str, "(HUGE!)");
		}

		break;

	      case iDescripSize:
		if(body = fetch_body(idata))
		  switch(body->type){
		    case TYPETEXT:
		    {
			mc = mail_elt(idata->stream, idata->rawno);
			if(mc->rfc822_size < 6000)
			  strcpy(str, "(short  )");
			else if(mc->rfc822_size < 25000)
			  strcpy(str, "(medium )");
			else if(mc->rfc822_size < 100000)
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
		subj_str(idata, width, str);
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
	  char *q;

	  /* space between columns */
	  if(p > buffer){
	      *p++ = ' ';
	      *p = '\0';
	  }

	  if(cdesc->adjustment == Left)
	    sprintf(p, "%-*.*s", width, width, cdesc->string);
	  else
	    sprintf(p, "%*.*s", width, width, cdesc->string);

	  /*
	   * Make sure there are no nulls in the part we were supposed to
	   * have just written. This may happen if sprintf returns an
	   * error, but we don't want to check for that because some
	   * sprintfs don't return anything. If there are nulls, rewrite it.
	   */
	  for(q = p; q < p+width; q++)
	    if(*q == '\0')
	      break;
	    
	  if(q < p+width){
	      strncpy(p, repeat_char(width, ' '), width);
	      p[width] = '\0';
	      /* throw a ? in there too */
	      if(width > 4){
		  p[(width-1)/2 - 1] = '?';
		  p[(width-1)/2    ] = '?';
		  p[(width-1)/2 + 1] = '?';
	      }
	      else if(width > 2)
		p[(width-1)/2] = '?';
	  }

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
    dprint(9, (debugfile, "INDEX(%p) -->%s<-- (%d), 0x%lx>\n",
	       hline, hline->line, strlen(hline->line), hline->id));

    return(hline);
}


HLINE_S *
format_thread_index_line(idata)
    INDEXDATA_S	*idata;
{
    char         *p, *buffer;
    int           thdlen, space_left, i;
    HLINE_S	 *hline, *thline = NULL;
    PINETHRD_S   *thrd;

    dprint(8, (debugfile, "=== format_thread_index_line(%ld,%ld) ===\n",
	       idata ? idata->msgno : -1, idata ? idata->rawno : -1));

    space_left = ps_global->ttyo->screen_cols;

    if(ps_global->msgmap->max_thrdno < 1000)
      thdlen = 3;
    else if(ps_global->msgmap->max_thrdno < 10000)
      thdlen = 4;
    else if(ps_global->msgmap->max_thrdno < 100000)
      thdlen = 5;
    else
      thdlen = 6;

    hline = get_index_cache(idata->msgno);

    thrd = fetch_thread(idata->stream, idata->rawno);

    if(!thrd)			/* can't happen? */
      return(hline);
    
    hline->tihl = thline = get_tindex_cache(thrd->thrdno);

    if(!thline)
      return(hline);

    for(i = 0; i < OFFS; i++)
      thline->offs[i].offset = -1;

    *(p = buffer = thline->line) = '\0';

    if(space_left >= 3){
	char to_us, status;

	to_us = to_us_symbol_for_thread(idata->stream, thrd, 1);
	status = status_symbol_for_thread(idata->stream, thrd, iStatus);

	if(pico_usingcolor()){
	    struct variable *vars = ps_global->vars;

	    i = 0;
	    if(to_us == '*'
	       && VAR_IND_IMP_FORE_COLOR && VAR_IND_PLUS_BACK_COLOR){
		thline->offs[i].offset = p - buffer;
		strncpy(thline->offs[i].color.fg, VAR_IND_IMP_FORE_COLOR,
			MAXCOLORLEN);
		thline->offs[i].color.fg[MAXCOLORLEN] = '\0';
		strncpy(thline->offs[i].color.bg, VAR_IND_IMP_BACK_COLOR,
			MAXCOLORLEN);
		thline->offs[i++].color.bg[MAXCOLORLEN] = '\0';
		if(F_ON(F_COLOR_LINE_IMPORTANT, ps_global)){
		    strncpy(thline->linecolor.fg, VAR_IND_IMP_FORE_COLOR,
			    MAXCOLORLEN);
		    strncpy(thline->linecolor.bg, VAR_IND_IMP_BACK_COLOR,
			    MAXCOLORLEN);
		}
	    }
	    else if((to_us == '+' || to_us == '-')
		    && VAR_IND_PLUS_FORE_COLOR && VAR_IND_PLUS_BACK_COLOR){
		thline->offs[i].offset = p - buffer;
		strncpy(thline->offs[i].color.fg, VAR_IND_PLUS_FORE_COLOR,
			MAXCOLORLEN);
		thline->offs[i].color.fg[MAXCOLORLEN] = '\0';
		strncpy(thline->offs[i].color.bg, VAR_IND_PLUS_BACK_COLOR,
			MAXCOLORLEN);
		thline->offs[i++].color.bg[MAXCOLORLEN] = '\0';
	    }

	    if(status == 'D'
	       && VAR_IND_DEL_FORE_COLOR && VAR_IND_DEL_BACK_COLOR){
		thline->offs[i].offset = p + 2 - buffer;
		strncpy(thline->offs[i].color.fg, VAR_IND_DEL_FORE_COLOR,
			MAXCOLORLEN);
		thline->offs[i].color.fg[MAXCOLORLEN] = '\0';
		strncpy(thline->offs[i].color.bg, VAR_IND_DEL_BACK_COLOR,
			MAXCOLORLEN);
		thline->offs[i++].color.bg[MAXCOLORLEN] = '\0';
	    }
	    else if(status == 'N'
		    && VAR_IND_NEW_FORE_COLOR && VAR_IND_NEW_BACK_COLOR){
		thline->offs[i].offset = p + 2 - buffer;
		strncpy(thline->offs[i].color.fg, VAR_IND_NEW_FORE_COLOR,
			MAXCOLORLEN);
		thline->offs[i].color.fg[MAXCOLORLEN] = '\0';
		strncpy(thline->offs[i].color.bg, VAR_IND_NEW_BACK_COLOR,
			MAXCOLORLEN);
		thline->offs[i++].color.bg[MAXCOLORLEN] = '\0';
	    }
	}

	p[0] = to_us;
	p[1] = ' ';

	p[2] = status;
	p += 3;
	space_left -= 3;
    }

    if(space_left >= thdlen+1){
	char threadnum[50];

	*p++ = ' ';
	space_left--;

	threadnum[0] = '\0';
	if(thrd->thrdno)
	  sprintf(threadnum, "%ld", thrd->thrdno);

	sprintf(p, "%*.*s", thdlen, thdlen, threadnum);
	p += thdlen;
	space_left -= thdlen;
    }

    if(space_left >= 7){
	*p++ = ' ';
	space_left--;
	date_str(fetch_date(idata), iDate, 0, p);
	p[6] = '\0';
	if(strlen(p) < 6){
	    char *q;

	    for(q = p + strlen(p); q < p + 6; q++)
	      *q = ' ';
	}

	p += 6;
	space_left -= 6;
    }

    if(space_left > 3){
	int   from_width, subj_width, bigthread_adjust;
	long  in_thread;
	char *subj_start;
	char  from[MAX_SCREEN_COLS+1];
	char *from_start;
	char  tcnt[50];

	*p++ = ' ';
	space_left--;

	in_thread = count_lflags_in_thread(idata->stream, thrd,
					   ps_global->msgmap, MN_NONE);

	sprintf(tcnt, "(%ld)", in_thread);
	bigthread_adjust = max(0, strlen(tcnt) - 3);
	
	/* third of the rest */
	from_start = p;
	from_width = max((space_left-1)/3 - bigthread_adjust, 1);

	/* the rest */
	subj_start = p + from_width + 1;
	subj_width = space_left - from_width - 1;

	from[0] = '\0';
	from_str(iFromTo, idata, from_width, from);
	sprintf(from_start, "%-*.*s", from_width, from_width, from);

	subj_start[-1] = ' ';

	if(strlen(tcnt) > subj_width)
	  tcnt[subj_width] = '\0';

	strncpy(subj_start, tcnt, subj_width);
	subj_width -= strlen(tcnt);
	subj_start += strlen(tcnt);

	if(subj_width > 0){
	    *subj_start++ = ' ';
	    subj_width--;
	}

	if(subj_width > 0){
	    if(idata->bogus){
		if(idata->bogus < 2)
		  sprintf(subj_start, "%-*.*s", subj_width, subj_width,
			  "[ No Message Text Available ]");
	    }
	    else{
		char  subj[MAX_SCREEN_COLS+1];

		subj[0] = '\0';
		subj_str(idata, subj_width, subj);
		sprintf(subj_start, "%-*.*s", subj_width, subj_width, subj);
	    }
	}
    }
    else if(space_left > 0)
      sprintf(p, "%-*.*s", space_left, space_left, "");

    /* Truncate it to be sure not too wide */
    buffer[min(ps_global->ttyo->screen_cols, i_cache_width())] = '\0';
    thline->id = line_hash(buffer);
    dprint(9, (debugfile, "THDINDEX(%p) -->%s<-- (%d), 0x%lx>\n",
	       thline, thline->line, strlen(thline->line), thline->id));

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
 * Returns   0 if no match, 1 if a match.
 *           The color that goes with the matched rule in returned_color.
 *           It may be NULL, which indicates default.
 */
int
get_index_line_color(stream, searchset, pstate, returned_color)
    MAILSTREAM *stream;
    SEARCHSET  *searchset;
    PAT_STATE **pstate;
    COLOR_PAIR **returned_color;
{
    PAT_S           *pat = NULL;
    long             rflags = ROLE_INCOL;
    COLOR_PAIR      *color = NULL;
    int              match = 0;
    static PAT_STATE localpstate;

    dprint(7, (debugfile, "get_index_line_color\n"));

    if(returned_color)
      *returned_color = NULL;

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
	while(!match && pat){
	    if(match_pattern(pat->patgrp, stream, searchset, NULL,
			     get_msg_score, 0)){
		if(!pat->action || pat->action->bogus)
		  break;

		match++;
		if(pat->action && pat->action->incol)
		  color = new_color_pair(pat->action->incol->fg,
				         pat->action->incol->bg);
	    }
	    else
	      pat = next_pattern(*pstate);
	}
    }

    if(match && returned_color)
      *returned_color = color;

    return(match);
}


/*
 * Calculates all of the scores for the searchset and stores them in the
 * mail elts. Careful, this function uses patterns so if the caller is using
 * patterns then the caller will probably have to reset the pattern functions.
 * That is, will have to call first_pattern again with the correct type.
 *
 * Args:     stream
 *        searchset -- calculate scores for this set of messages
 *         no_fetch -- we're in a callback from c-client, don't call c-client
 *
 * Returns   1 -- ok
 *           0 -- error, because of no_fetch
 */
int
calculate_some_scores(stream, searchset, no_fetch)
    MAILSTREAM *stream;
    SEARCHSET  *searchset;
{
    PAT_S         *pat = NULL;
    PAT_STATE      pstate;
    char          *savebits;
    int            newscore, score, error = 0;
    long           rflags = ROLE_SCORE;
    long           n, i;
    SEARCHSET     *s;

    dprint(7, (debugfile, "calculate_some_scores\n"));

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
		!error && pat;
		pat = next_pattern(&pstate)){

		switch(match_pattern(pat->patgrp, stream, searchset, NULL, NULL,
				     no_fetch)){
		  case 1:
		    if(!pat->action || pat->action->bogus)
		      break;

		    newscore = pat->action->scoreval;

		    for(s = searchset; s; s = s->next)
		      for(n = s->first; n <= s->last; n++)
			if(mail_elt(stream, n)->searched){
			    if((score = get_msg_score(stream,n)) == SCORE_UNDEF)
			      score = 0;
			    
			    score += newscore;
			    set_msg_score(stream, n, score);
			}

		    break;
		
		  case 0:
		    break;

		  case -1:
		    error++;
		    break;
		}
	    }

	    for(i = 1L; i <= stream->nmsgs; i++)
	      mail_elt(stream, i)->searched = savebits[i];

	    fs_give((void **)&savebits);

	    if(error){
		/*
		 * Revert to undefined scores.
		 */
		score = SCORE_UNDEF;
		for(s = searchset; s; s = s->next)
		  for(n = s->first; n <= s->last; n++)
		    set_msg_score(stream, n, score);
	    }
	}
    }

    return(error ? 0 : 1);
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

    if(stream->dtb && !strcmp(stream->dtb->name, "nntp")){

      if(THRD_INDX())
        return(TRUE);

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
	idata.rawno   = mail_msgno(stream, uid);
	idata.msgno   = mn_raw2m(ps_global->msgmap, idata.rawno);
	idata.size    = obuf->optional.octets;
	idata.from    = obuf->from;
	idata.date    = obuf->date;
	idata.subject = obuf->subject;

	hline = (*format_index_line)(&idata);
	if(idata.bogus && hline){
	    if(THRD_INDX()){
		if(hline->tihl)
		  hline->tihl->line[0] = '\0';
	    }
	    else
	      hline->line[0] = '\0';
	}
	else if(F_OFF(F_QUELL_NEWS_ENV_CB, ps_global)){
	    if((!THRD_INDX() || (hline && hline->tihl))
	       && !msgline_hidden(stream, ps_global->msgmap, idata.msgno, 0))
	      paint_index_hline(stream, idata.msgno, hline);
	}
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
	  char *p, *pref, *h, *fields[2];
	  
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

	      /* add prefix */
	      for(pref = prefix; pref && *pref; pref++)
		if(width){
		    *s++ = *pref;
		    width--;
		}
		else
		  break;

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
#define TODAYSTR "Today"

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
      case iSDateTime:
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
      case iDayOfWeekAbb:
	strcpy(str, (d.wkday >= 0 && d.wkday <= 6) ? week_abbrev(d.wkday) : "");
	break;
      case iDayOfWeek:
	strcpy(str, (d.wkday >= 0 && d.wkday <= 6) ? day_name[d.wkday] : "");
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
      case iSDateTime:
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
		strcpy(str, TODAYSTR);
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

	break;
    }
    
    if(type == iSTime ||
       (type == iSDateTime && !strcmp(str, TODAYSTR))){
	struct date now, last_day;
	char        dbuf[200], *Ddd, *ampm;
	int         daydiff;

	str[0] = '\0';
	rfc822_date(dbuf);
	parse_date(dbuf, &now);

	/* Figure out if message date lands in the past week */

	/* (if message dated this month or last month...) */
	if((d.year == now.year && d.month >= now.month - 1) ||
	   (d.year == now.year - 1 && d.month == 12 && now.month == 1)){

	    daydiff = day_of_year(&now) - day_of_year(&d);

	    /*
	     * If msg in end of last year (and we're in first bit of "this"
	     * year), diff will be backwards; fix up by adding number of days
	     * in last year (usually 365, but occasionally 366)...
	     */
	    if(d.year == now.year - 1){
		last_day = d;
		last_day.month = 12;
		last_day.day   = 31;

		daydiff += day_of_year(&last_day);
	    }
	}
	else
	  daydiff = -100;	/* comfortably out of range (of past week) */

	/* Build 2-digit hour and am/pm indicator, used below */

	if(d.hour >= 0 && d.hour < 24){
	    sprintf(hour12, "%02d", (d.hour % 12 == 0) ? 12 : d.hour % 12);
	    ampm = (d.hour < 12) ? "am" : "pm";
	}
	else{
	    strcpy(hour12, "??");
	    ampm = "__";
	}

	/* Build date/time in str, in format similar to that used by w(1) */

	if(daydiff == 0){		    /* If date is today, "HH:MMap" */
	    if(d.minute >= 0 && d.minute < 60)
	      sprintf(minzero, "%02d", d.minute);
	    else
	      strcpy(minzero, "??");

	    sprintf(str, "%s:%s%s", hour12, minzero, ampm);
	}
	else if(daydiff >= 1 && daydiff < 6){ /* If <1wk ago, "DddHHap" */

	    if(d.month >= 1 && d.day >= 1 && d.year >= 0 &&
	       d.month <= 12 && d.day <= 31 && d.year <= 9999)
	      Ddd = week_abbrev(day_of_week(&d));
	    else
	      Ddd = "???";

	    sprintf(str, "%s%s%s", Ddd, hour12, ampm);
	}
	else{		       /* date is old or future, "ddMmmyy" */
	    strcpy(monabb, (d.month >= 1 && d.month <= 12)
			     ? month_abbrev(d.month) : "???");

	    if(d.day >= 1 && d.day <= 31)
	      sprintf(dayzero, "%02d", d.day);
	    else
	      strcpy(dayzero, "??");

	    if(d.year >= 0 && d.year <= 9999)
	      sprintf(yearzero, "%02d", d.year % 100);
	    else
	      strcpy(yearzero, "??");

	    sprintf(str, "%s%s%s", dayzero, monabb, yearzero);
	}

	if(str[0] == '0'){	/* leading 0 (date|hour) elided or blanked */
	    if(v)
	      memmove(str, str + 1, strlen(str));
	    else
	      str[0] = ' ';
	}
    }
}


/*
 * fills subject in for painting index lines
 */
void
subj_str(idata, width, str)
    INDEXDATA_S *idata;
    int          width;
    char        *str;
{
    char          *subject, *sptr = NULL;
    char          *p, *border, *q = NULL;
    unsigned char *tmp;
    size_t         len;
    int            depth = 0, mult = 2, collapsed;
    PINETHRD_S    *thd, *thdorig;
    HLINE_S       *hline;
    unsigned long  rawno;

    memset(str, 0, (width+1) * sizeof(*str));
    if(subject = fetch_subject(idata)){
	if(THREADING()
	   && (ps_global->thread_disp_style == THREAD_STRUCT
	       || ps_global->thread_disp_style == THREAD_MUTTLIKE
	       || ps_global->thread_disp_style == THREAD_INDENT_SUBJ1
	       || ps_global->thread_disp_style == THREAD_INDENT_SUBJ2)){
	    thdorig = thd = fetch_thread(idata->stream, idata->rawno);
	    border = str + width;
	    if(current_index_state->plus_col >= 0 && !THRD_INDX()){
		collapsed = thd && thd->next &&
			get_lflag(idata->stream, NULL, idata->rawno, MN_COLL);
		hline = get_index_cache(idata->msgno);
		hline->plus = collapsed ? ps_global->VAR_THREAD_MORE_CHAR[0]
			: (thd && thd->next)
			    ? ps_global->VAR_THREAD_EXP_CHAR[0] : ' ';
		if(width > 0){
		    *str++ = ' ';
		    width--;
		}

		if(width > 0){
		    *str++ = ' ';
		    width--;
		}
	    }

	    sptr = str;

	    if(thd)
	      while(thd->parent &&
		    (thd = fetch_thread(idata->stream, thd->parent)))
	        depth++;

	    if(depth > 0){
		if(ps_global->thread_disp_style == THREAD_INDENT_SUBJ1)
		  mult = 1;

		sptr += (mult*depth);
		for(thd = thdorig, p = str + mult*depth - mult;
		    thd && thd->parent && p >= str;
		    thd = fetch_thread(idata->stream, thd->parent), p -= mult){
		    if(p + 2 >= border && !q){
			if(width >= 4 && depth < 100){
			    sprintf(str, "%*s[%2d]", width-4, "", depth);
			    q = str + width-4;
			}
			else if(width >= 5 && depth < 1000){
			    sprintf(str, "%*s[%3d]", width-5, "", depth);
			    q = str + width-5;
			}
			else{
			    sprintf(str, "%s", repeat_char(width, '.'), width);
			    q = str;
			}

			border = q;
			sptr = NULL;
		    }

		    if(p + 1 < border){
			p[0] = p[1] = ' ';
			if(ps_global->thread_disp_style == THREAD_STRUCT
			   || ps_global->thread_disp_style == THREAD_MUTTLIKE){
			    if(thd == thdorig && !thd->branch)
			      p[0] = ps_global->VAR_THREAD_LASTREPLY_CHAR[0];
			    else if(thd == thdorig || thd->branch)
			      p[0] = '|';

			    if(thd == thdorig)
			      p[1] = '-';
			}
		    }
		    else if(p < border){
			p[0] = ' ';
			if(ps_global->thread_disp_style == THREAD_STRUCT
			   || ps_global->thread_disp_style == THREAD_MUTTLIKE){
			    if(thd == thdorig && !thd->branch)
			      p[0] = ps_global->VAR_THREAD_LASTREPLY_CHAR[0];
			    else if(thd == thdorig || thd->branch)
			      p[0] = '|';
			}
		    }
		}
	    }

	    if(sptr){
		int do_subj = 0;

		/*
		 * Look to see if the subject is the same as the previous
		 * message in the thread, if any. If it is the same, don't
		 * reprint the subject.
		 */
		if(ps_global->thread_disp_style == THREAD_MUTTLIKE){
		    if(depth == 0)
		      do_subj++;
		    else{
			if(thdorig->parent &&
			   (thd = fetch_thread(idata->stream, thdorig->parent))
			   && thd->rawno){
			    char       *s1 = NULL, *s2 = NULL, *free_s2 = NULL;
			    ENVELOPE   *env;
			    char       *prevsubj = NULL;
			    mailcache_t mc;
			    SORTCACHE  *sc = NULL;

			    /* get the stripped subject of previous message */
			    mc = (mailcache_t) mail_parameters(NIL, GET_CACHE,
							       NIL);
			    if(mc)
			      sc = (*mc)(idata->stream, thd->rawno,
					 CH_SORTCACHE);
			    
			    if(sc && sc->subject)
			      s2 = sc->subject;
			    else{
				env = mail_fetchenvelope(idata->stream,
						         thd->rawno);
				if(env && env->subject)
				  mail_strip_subject(env->subject, &s2);
				
				free_s2 = s2;
			    }

			    mail_strip_subject(subject, &s1);
			    if(s1 && !s2 || s2 && !s1 || strucmp(s1, s2))
			      do_subj++;
			    
			    if(s1)
			      fs_give((void **) &s1);
			    if(free_s2)
			      fs_give((void **) &free_s2);
			}
			else
			  do_subj++;
		    }
		}
		else
		  do_subj++;

		if(do_subj){
		    width = (str + width) - sptr;
		    len = strlen(subject)+1;
		    tmp = fs_get(len * sizeof(unsigned char));
		    istrncpy(sptr, (char *) rfc1522_decode(tmp, len,
							   subject, NULL),
			     width);
		    fs_give((void **) &tmp);
		}
		else if(ps_global->thread_disp_style == THREAD_MUTTLIKE)
		  sptr[0] = '>';
	    }
	}
	else{
	    len = strlen(subject)+1;
	    tmp = fs_get(len * sizeof(unsigned char));
	    istrncpy(str,
		     (char *) rfc1522_decode(tmp, len, subject, NULL),
		     width);
	    fs_give((void **) &tmp);
	}
    }
    else{
	hline = get_index_cache(idata->msgno);
	hline->plus = ' ';
    }
}


PINETHRD_S *
fetch_thread(stream, rawno)
    MAILSTREAM   *stream;
    unsigned long rawno;
{
    MESSAGECACHE *mc;
    PINELT_S     *pelt;
    PINETHRD_S   *thrd = NULL;

    if(stream && rawno > 0L && rawno <= stream->nmsgs
       && !ps_global->need_to_rethread){
	mc = mail_elt(stream, rawno);
	if(pelt = (PINELT_S *)mc->sparep)
	  thrd = pelt->pthrd;
    }

    return(thrd);
}


PINETHRD_S *
fetch_head_thread(stream)
    MAILSTREAM *stream;
{
    unsigned long rawno;
    PINETHRD_S   *thrd = NULL;

    if(stream){
	/* first find any thread */
	for(rawno = 1L; !thrd && rawno <= stream->nmsgs; rawno++)
	  thrd = fetch_thread(stream, rawno);

	if(thrd && thrd->head)
	  thrd = fetch_thread(stream, thrd->head);
    }

    return(thrd);
}


void
from_str(ctype, idata, width, str)
    IndexColType ctype;
    INDEXDATA_S *idata;
    int          width;
    char        *str;
{
    char       *field, *newsgroups, *border, *p, *fptr = NULL, *q = NULL;
    ADDRESS    *addr;
    int         depth = 0, mult = 2, collapsed;
    PINETHRD_S *thd, *thdorig;
    HLINE_S *   hline;

    if(THREADING()
       && (ps_global->thread_disp_style == THREAD_INDENT_FROM1
           || ps_global->thread_disp_style == THREAD_INDENT_FROM2
           || ps_global->thread_disp_style == THREAD_STRUCT_FROM)){
	thdorig = thd = fetch_thread(idata->stream, idata->rawno);
	border = str + width;
	if(current_index_state->plus_col >= 0 && !THRD_INDX()){
	    collapsed = thd && thd->next &&
			get_lflag(idata->stream, NULL, idata->rawno, MN_COLL);
	    hline = get_index_cache(idata->msgno);
	    hline->plus = collapsed ? ps_global->VAR_THREAD_MORE_CHAR[0]
			: (thd && thd->next)
			    ? ps_global->VAR_THREAD_EXP_CHAR[0] : ' ';
	    if(width > 0){
		*str++ = ' ';
		width--;
	    }

	    if(width > 0){
		*str++ = ' ';
		width--;
	    }
	}

	fptr = str;

	if(thd)
	  while(thd->parent && (thd = fetch_thread(idata->stream, thd->parent)))
	    depth++;

	if(depth > 0){
	    if(ps_global->thread_disp_style == THREAD_INDENT_FROM1)
	      mult = 1;

	    fptr += (mult*depth);
	    for(thd = thdorig, p = str + mult*depth - mult;
		thd && thd->parent && p >= str;
		thd = fetch_thread(idata->stream, thd->parent), p -= mult){
		if(p + 2 >= border && !q){
		    if(width >= 4 && depth < 100){
			sprintf(str, "%*s[%2d]", width-4, "", depth);
			q = str + width-4;
		    }
		    else if(width >= 5 && depth < 1000){
			sprintf(str, "%*s[%3d]", width-5, "", depth);
			q = str + width-5;
		    }
		    else{
			sprintf(str, "%s", repeat_char(width, '.'), width);
			q = str;
		    }

		    border = q;
		    fptr = NULL;
		}

		if(p + 1 < border){
		    p[0] = p[1] = ' ';
		    if(ps_global->thread_disp_style == THREAD_STRUCT_FROM){
			if(thd == thdorig && !thd->branch)
			  p[0] = ps_global->VAR_THREAD_LASTREPLY_CHAR[0];
			else if(thd == thdorig || thd->branch)
			  p[0] = '|';

			if(thd == thdorig)
			  p[1] = '-';
		    }
		}
		else if(p < border){
		    p[0] = ' ';
		    if(ps_global->thread_disp_style == THREAD_STRUCT_FROM){
			if(thd == thdorig && !thd->branch)
			  p[0] = ps_global->VAR_THREAD_LASTREPLY_CHAR[0];
			else if(thd == thdorig || thd->branch)
			  p[0] = '|';
		    }
		}
	    }
	}
    }
    else
      fptr = str;

    if(fptr){
	width = (str + width) - fptr;
	switch(ctype){
	  case iFromTo:
	  case iFromToNotNews:
	    if(!(addr = fetch_from(idata)) || address_is_us(addr, ps_global)){
		if(width <= 4){
		    strcpy(fptr, "To: ");
		    fptr[width] = '\0';
		    break;
		}
		else{
		    if((field = ((addr = fetch_to(idata))
				 ? "To"
				 : (addr = fetch_cc(idata))
				 ? "Cc"
				 : NULL))
		       && set_index_addr(idata, field, addr, "To: ",
					 width, fptr))
		      break;

		    if(ctype == iFromTo &&
		       (newsgroups = fetch_newsgroups(idata)) &&
		       *newsgroups){
			sprintf(fptr, "To: %-*.*s", width-4, width-4,
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
			   NULL, width, fptr);
	    break;

	  case iAddress:
	  case iMailbox:
	    if((addr = fetch_from(idata)) && addr->mailbox && addr->mailbox[0]){
		char *mb = NULL, *hst = NULL, *at = NULL;
		size_t len;

		mb = addr->mailbox;
		if(ctype == iAddress && addr->host && addr->host[0]
		   && addr->host[0] != '.'){
		    at = "@";
		    hst = addr->host;
		}

		len = strlen(mb);
		if(!at || width <= len)
		  sprintf(fptr, "%-*.*s", width, width, mb);
		else
		  sprintf(fptr, "%s@%-*.*s", mb, width-len-1, width-len-1, hst);
	    }

	    break;
	}
    }
}


/*
 * Simple hash function from K&R 2nd edition, p. 144.
 *
 * This one is modified to never return 0 so we can use that as a special
 * value. Also, LINE_HASH_N fits in an unsigned long, so it too can be used
 * as a special value that can't be returned by line_hash.
 */
unsigned long
line_hash(s)
    char *s;
{
    unsigned long hashval;

    for(hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;

    hashval = hashval % LINE_HASH_N;

    if(!hashval)
      hashval++;

    return(hashval);
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
	
	if(!agg && msgline_hidden(state->mail_stream, msgmap, i, 0))
	  continue;

	if(!print_char((mn_is_cur(msgmap, i)) ? '>' : ' ')
	   || !gf_puts(build_header_line(state, state->mail_stream,
					 msgmap, i, NULL)->line + 1, print_char)
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
    HLINE_S    *hl;
    static ESCKEY_S header_search_key[] = { {0, 0, NULL, NULL },
					    {ctrl('Y'), 10, "^Y", "First Msg"},
					    {ctrl('V'), 11, "^V", "Last Msg"},
					    {-1, 0, NULL, NULL} };

    dprint(4, (debugfile, "\n - search headers - \n"));

    if(!any_messages(msgmap, NULL, "to search")){
	return;
    }
    else if(mn_total_cur(msgmap) > 1L){
	q_status_message1(SM_ORDER, 0, 2, "%.200s msgs selected; Can't search",
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
	    if(any_lflagged(msgmap, MN_HIDE | MN_CHID)){
		do{
		    selected = sorted_msg;
		    mn_dec_cur(stream, msgmap, MH_NONE);
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
	    if(any_lflagged(msgmap, MN_HIDE | MN_CHID)){
		do{
		    selected = sorted_msg;
		    mn_inc_cur(stream, msgmap, MH_NONE);
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
      if(!msgline_hidden(stream, msgmap, i, 0) &&
	 srchstr(((hl = build_header_line(state, stream, msgmap, i, NULL))
		  && THRD_INDX() && hl->tihl) ? hl->tihl->line : hl->line,
		 search_string)){
	  selected++;
	  if(select_all)
	    set_lflag(stream, msgmap, i, MN_SLCT, 1);
	  else
	    break;
      }

    if(i > mn_get_total(msgmap))
      for(i = 1; i < sorted_msg && !ps_global->intr_pending; i++)
        if(!msgline_hidden(stream, msgmap, i, 0) &&
	   srchstr(((hl = build_header_line(state, stream, msgmap, i, NULL))
		    && THRD_INDX() && hl->tihl) ? hl->tihl->line : hl->line,
		   search_string)){
	    selected++;
	    if(select_all)
	      set_lflag(stream, msgmap, i, MN_SLCT, 1);
	    else
	      break;
	}

    if(ps_global->intr_pending){
	q_status_message1(SM_ORDER, 0, 3, "Search cancelled.%.200s",
			  select_all ? " Selected set may be incomplete.":"");
    }
    else if(select_all){
	if(selected
	   && any_lflagged(msgmap, MN_SLCT) > 0L
	   && !any_lflagged(msgmap, MN_HIDE)
	   && F_ON(F_AUTO_ZOOM, state))
	  (void) zoom_index(state, stream, msgmap);
	    
	q_status_message1(SM_ORDER, 0, 3, "%.200s messages found matching word",
			  long2string(selected));
    }
    else if(selected){
	q_status_message1(SM_ORDER, 0, 3, "Word found%.200s",
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
	(void)calculate_some_scores(stream, searchset, 0);
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
sort_folder(msgmap, new_sort, new_rev, flags)
    MSGNO_S   *msgmap;
    SortOrder  new_sort;
    int	       new_rev;
    unsigned   flags;
{
    long	   raw_current, i, j;
    unsigned long *sort = NULL;
    int		   we_cancel = 0;
    char	   sort_msg[MAX_SCREEN_COLS+1];
    SortOrder      current_sort;
    int	           current_rev;

    dprint(2, (debugfile, "Sorting by %s%s\n",
	       sort_name(new_sort), new_rev ? "/reverse" : ""));

    current_sort = mn_get_sort(msgmap);
    current_rev = mn_get_revsort(msgmap);
    /*
     * If we were previously threading (spare == 1) and now we're switching
     * sorts (other than just a rev switch) then erase the information
     * about the threaded state (collapsed and so forth).
     */
    if(ps_global->mail_stream && ps_global->mail_stream->spare
       && (current_sort != new_sort))
      erase_threading_info(ps_global->mail_stream, msgmap);

    if(mn_get_total(msgmap) <= 1L
       && !(mn_get_total(msgmap) == 1L
            && (new_sort == SortThread || new_sort == SortSubject2))){
	mn_set_sort(msgmap, new_sort);
	mn_set_revsort(msgmap, new_rev);
	if(!mn_get_mansort(msgmap))
	  mn_set_mansort(msgmap, (flags & SRT_MAN) ? 1 : 0);
	  
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

	mn_set_sort(msgmap, new_sort);
	mn_set_revsort(msgmap, new_rev);

	if(current_sort != new_sort || current_rev != new_rev ||
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

	/* reset the inverse array */
	msgno_reset_isort(msgmap);

	ASSERT_ISORT(msgmap, "validity fail in sort_folder Arrival sort\n");
    }
    else if(new_sort == SortScore){

	/*
	 * We have to build a temporary array which maps raw msgno to
	 * score. We use the index cache machinery to build the array.
	 */

	mn_set_sort(msgmap, new_sort);
	mn_set_revsort(msgmap, new_rev);
	
#ifndef	DOS
	intr_handling_on();
#endif

	if(flags & SRT_VRB){
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

	if(we_cancel)
	  cancel_busy_alarm(1);

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

	/* reset the inverse array */
	msgno_reset_isort(msgmap);

	ASSERT_ISORT(msgmap, "validity fail in sort_folder Score sort\n");
    }
    else{

	mn_set_sort(msgmap, new_sort);
	mn_set_revsort(msgmap, new_rev);
	clear_index_cache();

#ifndef	DOS
	/*
	 * Because this draws a keymenu, which in turn may call malloc
	 * and free in pico/osdep functions having to do with colors,
	 * we don't want to call it when alarm handling is turned on.
	 * The reason is because the alarm could interrupt us in a precarious
	 * state. For example, we are drawing this keymenu and have just
	 * freed _last_bg_color but haven't set it to NULL yet, then the
	 * alarm happens, does some status line drawing which resets the
	 * background color, bango, it tries to free the thing we just
	 * freed. So move this before the busy_alarm call below.
	 */
	intr_handling_on();
#endif

	if(flags & SRT_VRB){
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
	 * Limit the sort/thread if messages are hidden from view 
	 * by lighting searched bit of every interesting msg in 
	 * the folder and call c-client thread/sort to do the dirty work.
	 *
	 * Unfortunately it isn't that easy. IMAP servers are not able to
	 * handle medium to large sized sequence sets (more than 1000
	 * characters in the command line breaks some) so we have to try
	 * to handle it locally. By lighting the searched bits and
	 * providing a NULL search program we get a special c-client
	 * interface. This means that c-client will attempt to send the
	 * sequence set with the SORT or THREAD but it may get back
	 * a BAD response because of long command lines. In that case,
	 * if it is a SORT call, c-client will issue the full SORT
	 * without the sequence sets and will then filter the results
	 * locally. So sort_sort_callback will see the correctly
	 * filtered results. If it is a mail_thread call, a similar thing
	 * will be done. If a BAD is received, then there is no way to
	 * easily filter the results. C-client (in this special case where
	 * we provide a NULL search program) will set tree->num to zero
	 * for nodes of the thread tree which were supposed to be
	 * filtered out of the thread. Then pine, in sort_thread_callback,
	 * will treat those as dummy nodes (nodes which are part of the
	 * tree logically but where we don't have access to the messages).
	 * This will give us a different answer than we would have gotten
	 * if the restricted thread would have worked, but it's something.
	 *
	 * It isn't in general possible to give some shorter search set
	 * in place of the long sequence set because the long sequence set
	 * may be the result of several filter rules or of excluded
	 * messages in news (in this 2nd case maybe we could give
	 * a shorter search set).
	 *
	 * We note also that the too-long commands in imap is a general
	 * imap deficiency. It comes up in particular also in SEARCH
	 * commands. Pine likes to exclude the hidden messages from the
	 * SEARCH. SEARCH will be handled transparently by the local
	 * c-client by first issuing the full SEARCH command, if that
	 * comes back with a BAD and there is a pgm->msgno at the top
	 * level of pgm, then c-client will re-issue the SEARCH command
	 * but without the msgno sequence set in hopes that the resulting
	 * command will now be short enough, and then it will filter out
	 * the sequence set locally. If that doesn't work, it will
	 * download the messages and do the SEARCH locally. That is
	 * controllable by a flag bit.
	 */
	for(i = 1L; i <= ps_global->mail_stream->nmsgs; i++)
	  mail_elt(ps_global->mail_stream, i)->searched
			= !get_lflag(ps_global->mail_stream, NULL, i, MN_EXLD);
	
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
		new_sort = current_sort;
		new_rev  = current_rev;
		q_status_message2(SM_ORDER, 3, 3,
				  "Sort %.200s!  Restored %.200s sort.",
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
		new_sort = current_sort;
		new_rev  = current_rev;
		q_status_message2(SM_ORDER, 3, 3,
				  "Sort %.200s!  Restored %.200s sort.",
				  ps_global->intr_pending
				    ? "Canceled" : "Failed",
				  sort_name(new_sort));
	    }

	    if(sort)
	      fs_give((void **) &sort);

	    mail_free_sortpgm(&g_sort.prog);
	}

	if(we_cancel)
	  cancel_busy_alarm(1);

#ifndef	DOS
	/*
	 * Same here as intr_handling_on. We don't want the busy alarm
	 * to be active when we call this.
	 */
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

	    /* reset the inverse array */
	    msgno_reset_isort(msgmap);

	    ASSERT_ISORT(msgmap, "validity fail in sort_folder Flip sort\n");

	    /*
	     * Flip the thread numbers around.
	     * This puts us in a weird state that requires keeping track
	     * of. The direction of the thread list hasn't changed, but the
	     * thrdnos have and the display direction has.
	     *
	     * For Sort   thrdno 1       thread list head
	     *            thrdno 2              |
	     *            thrdno .              v  nextthd this direction
	     *            thrdno .
	     *            thrdno n       thread list tail
	     *
	     * Rev Sort   thrdno 1       thread list tail
	     *            thrdno 2
	     *            thrdno .              ^  nextthd this direction
	     *            thrdno .              |
	     *            thrdno n       thread list head
	     */
	    if(new_sort == SortThread || new_sort == SortSubject2){
		PINETHRD_S *thrd;

		thrd = fetch_head_thread(ps_global->mail_stream);
		for(j = msgmap->max_thrdno; thrd && j >= 1L; j--){
		    thrd->thrdno = j;

		    if(thrd->nextthd)
		      thrd = fetch_thread(ps_global->mail_stream,
					  thrd->nextthd);
		    else
		      thrd = NULL;
		}
	    }
	}
    }

    /* Fix up sort structure */
    mn_set_sort(msgmap, new_sort);
    mn_set_revsort(msgmap, new_rev);
    /*
     * Once a folder has been sorted manually, we continue treating it
     * as manually sorted until it is closed.
     */
    if(!mn_get_mansort(msgmap))
      mn_set_mansort(msgmap, (flags & SRT_MAN) ? 1 : 0);
    
    /*
     * If current is hidden, change current to visible parent.
     * It can only be hidden if we are threading.
     */
    if(THREADING())
      mn_reset_cur(msgmap, first_sorted_flagged(new_rev ? F_NONE : F_SRCHBACK,
					        ps_global->mail_stream,
					        mn_raw2m(msgmap, raw_current),
					        FSF_SKIP_CHID));
    else
      mn_reset_cur(msgmap, mn_raw2m(msgmap, raw_current));
    
    msgmap->top = -1L;

    if(!ps_global->mail_box_changed)
      ps_global->unsorted_newmail = 0;

    /*
     * Turn off the MN_USOR flag. Don't bother going through the
     * function call and the message number mappings.
     */
    if(THREADING())
      for(i = 1L; i <= ps_global->mail_stream->nmsgs; i++)
        mail_elt(ps_global->mail_stream, i)->spare7 = 0;
}


void
erase_threading_info(stream, msgmap)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
{
    unsigned long n;
    MESSAGECACHE *mc;
    PINELT_S     *peltp;

    if(!(stream && stream->spare))
      return;
    
    ps_global->view_skipped_index = 0;
    ps_global->viewing_a_thread = 0;
    if(ps_global->inbox_stream == stream)
      ps_global->inbox_viewing_a_thread = 0;
    
    if(THRD_INDX())
      setup_for_thread_index_screen();
    else
      setup_for_index_index_screen();

    stream->spare = 0;

    for(n = 1L; n <= stream->nmsgs; n++){
	set_lflag(stream, msgmap, mn_raw2m(msgmap, n),
		  MN_COLL | MN_CHID | MN_CHID2, 0);
	mc = mail_elt(stream, n);
	if(mc && mc->sparep){
	    peltp = (PINELT_S *) mc->sparep;
	    if(peltp->pthrd)
	      fs_give((void **) &peltp->pthrd);
	}
    }
}


void
sort_sort_callback(stream, list, nmsgs)
    MAILSTREAM	  *stream;
    unsigned long *list;
    unsigned long  nmsgs;
{
    long i;

    dprint(2, (debugfile, "sort_sort_callback\n"));

    if(mn_get_total(g_sort.msgmap) < nmsgs)
      panic("Message count shrank after sort!");

    /* copy ulongs to array of longs */
    for(i = nmsgs; i > 0; i--)
      g_sort.msgmap->sort[i] = (long) list[i-1];

    /* reset the inverse array */
    msgno_reset_isort(g_sort.msgmap);

    ASSERT_ISORT(g_sort.msgmap, "validity fail in sort_sort_callback\n");
    dprint(2, (debugfile, "sort_sort_callback done\n"));
}


void
sort_thread_callback(stream, tree)
    MAILSTREAM *stream;
    THREADNODE *tree;
{
    THREADNODE *collapsed_tree = NULL;
    long          i;
    PINETHRD_S   *thrd = NULL, *nthrd, *top;
    unsigned long msgno, rawno, set_in_thread, in_thread;
    int           bail, this_is_vis;
    int           un_view_thread = 0;
    long          raw_current;


    dprint(2, (debugfile, "sort_thread_callback\n"));

    g_sort.msgmap->max_thrdno = 0L;

    thrd_flatten_array =
	(struct pass_along *) fs_get(mn_get_total(g_sort.msgmap) *
						 sizeof(*thrd_flatten_array));

    memset(thrd_flatten_array, 0,
	   mn_get_total(g_sort.msgmap) * sizeof(*thrd_flatten_array));

    /*
     * Eliminate dummy nodes from tree and collapse the tree in a logical
     * way. If the dummy node is at the top-level, then its children are
     * promoted to the top-level as separate threads.
     */
    collapsed_tree = collapse_threadnode_tree(tree);
    (void) sort_thread_flatten(collapsed_tree, stream, thrd_flatten_array,
			       NULL, THD_TOP);
    mail_free_threadnode(&collapsed_tree);

    if(any_lflagged(g_sort.msgmap, MN_HIDE))
      g_sort.msgmap->visible_threads = calculate_visible_threads(stream);
    else
      g_sort.msgmap->visible_threads = g_sort.msgmap->max_thrdno;

    raw_current = mn_m2raw(g_sort.msgmap, mn_get_cur(g_sort.msgmap));

    memset(&g_sort.msgmap->sort[1], 0,
	   mn_get_total(g_sort.msgmap) * sizeof(long));

    /*
     * Copy the results of the sort into the real sort array and
     * create the inverse array.
     */
    for(i = 1L; i <= mn_get_total(g_sort.msgmap); i++)
      g_sort.msgmap->sort[i] = thrd_flatten_array[i-1].rawno;

    /* reset the inverse array */
    msgno_reset_isort(g_sort.msgmap);

    ASSERT_ISORT(g_sort.msgmap, "validity fail in sort_thread_callback\n");

    ps_global->need_to_rethread = 0;

    /*
     * Set appropriate bits to start out collapsed if desired. We use the
     * stream spare bit to tell us if we've done this before for this
     * stream.
     */
    if(!stream->spare
       && (COLL_THRDS() || SEP_THRDINDX())
       && mn_get_total(g_sort.msgmap) > 1L){

	collapse_threads(stream, g_sort.msgmap, NULL);
    }
    else if(stream->spare){
	
	/*
	 * If we're doing auto collapse then new threads need to have
	 * their collapse bit set. This happens below if we're in the
	 * thread index, but if we're in the regular index with auto
	 * collapse we have to look for these.
	 */
	if(any_lflagged(g_sort.msgmap, MN_USOR)){
	    if(COLL_THRDS()){
		for(msgno = 1L; msgno <= mn_get_total(g_sort.msgmap); msgno++){
		    rawno = mn_m2raw(g_sort.msgmap, msgno);
		    if(get_lflag(stream, NULL, rawno, MN_USOR)){
			thrd = fetch_thread(stream, rawno);

			/*
			 * Node is new, unsorted, top-level thread,
			 * and we're using auto collapse.
			 */
			if(thrd && !thrd->parent)
			  set_lflag(stream, g_sort.msgmap, msgno, MN_COLL, 1);
			
			/*
			 * If a parent is collapsed, clear that parent's
			 * index cache entry. This is only necessary if
			 * the parent's index display can depend on its
			 * children, of course.
			 */
			if(thrd && thrd->parent){
			    thrd = fetch_thread(stream, thrd->parent);
			    while(thrd){
				long t;

				if(get_lflag(stream, NULL, thrd->rawno, MN_COLL)
				   && (t = mn_raw2m(g_sort.msgmap,
						    (long) thrd->rawno)))
				  clear_index_cache_ent(t);

				if(thrd->parent)
				  thrd = fetch_thread(stream, thrd->parent);
				else
				  thrd = NULL;
			    }
			}

		    }
		}
	    }

	    set_lflags(stream, g_sort.msgmap, MN_USOR, 0);
	}

	if(ps_global->viewing_a_thread){
	    if(any_lflagged(g_sort.msgmap, MN_CHID2)){
		/* current should be part of viewed thread */
		if(get_lflag(stream, NULL, raw_current, MN_CHID2)){
		    thrd = fetch_thread(stream, raw_current);
		    if(thrd && thrd->top && thrd->top != thrd->rawno)
		      thrd = fetch_thread(stream, thrd->top);
		    
		    if(thrd){
			/*
			 * For messages that are part of thread set MN_CHID2
			 * and for messages that aren't part of the thread
			 * clear MN_CHID2. Easiest is to just do it instead
			 * of checking if it is true first.
			 */
			set_lflags(stream, g_sort.msgmap, MN_CHID2, 0);
			set_thread_lflags(stream, thrd, g_sort.msgmap,
					  MN_CHID2, 1);
			
			/*
			 * Outside of the viewed thread everything else
			 * should be collapsed at the top-levels.
			 */
			collapse_threads(stream, g_sort.msgmap, thrd);

			/*
			 * Inside of the thread, the top of the thread
			 * can't be hidden, the rest are hidden if a
			 * parent somewhere above them is collapsed.
			 * There can be collapse points that are hidden
			 * inside of the tree. They remain collapsed even
			 * if the parent above them uncollapses.
			 */
			msgno = mn_raw2m(g_sort.msgmap, (long) thrd->rawno);
			if(msgno)
			  set_lflag(stream, g_sort.msgmap, msgno, MN_CHID, 0);

			if(thrd->next){
			    PINETHRD_S *nthrd;

			    nthrd = fetch_thread(stream, thrd->next);
			    if(nthrd)
			      make_thrdflags_consistent(stream, g_sort.msgmap,
							nthrd,
							get_lflag(stream, NULL,
								  thrd->rawno,
								  MN_COLL));
			}
		    }
		    else
		      un_view_thread++;
		}
		else
		  un_view_thread++;
	    }
	    else
	      un_view_thread++;

	    if(un_view_thread){
		set_lflags(stream, g_sort.msgmap, MN_CHID2, 0);
		unview_thread(ps_global, stream, g_sort.msgmap);
	    }
	    else{
		mn_reset_cur(g_sort.msgmap,
			     mn_raw2m(g_sort.msgmap, raw_current));
		view_thread(ps_global, stream, g_sort.msgmap, 0);
	    }
	}
	else if(SEP_THRDINDX()){
	    set_lflags(stream, g_sort.msgmap, MN_CHID2, 0);
	    collapse_threads(stream, g_sort.msgmap, NULL);
	}
	else{
	    thrd = fetch_head_thread(stream);
	    while(thrd){
		/*
		 * The top-level threads aren't hidden by collapse.
		 */
		msgno = mn_raw2m(g_sort.msgmap, thrd->rawno);
		if(msgno)
		  set_lflag(stream, g_sort.msgmap, msgno, MN_CHID, 0);

		if(thrd->next){
		    PINETHRD_S *nthrd;

		    nthrd = fetch_thread(stream, thrd->next);
		    if(nthrd)
		      make_thrdflags_consistent(stream, g_sort.msgmap,
						nthrd,
						get_lflag(stream, NULL,
							  thrd->rawno,
							  MN_COLL));
		}

		if(thrd->nextthd)
		  thrd = fetch_thread(stream, thrd->nextthd);
		else
		  thrd = NULL;
	    }
	}
    }

    stream->spare = 1;

    fs_give((void **) &thrd_flatten_array);

    dprint(2, (debugfile, "sort_thread_callback done\n"));
}


void
collapse_threads(stream, msgmap, not_this_thread)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    PINETHRD_S *not_this_thread;
{
    PINETHRD_S   *thrd = NULL, *nthrd;
    unsigned long msgno;

    dprint(9, (debugfile, "collapse_threads\n"));

    thrd = fetch_head_thread(stream);
    while(thrd){
	if(thrd != not_this_thread){
	    msgno = mn_raw2m(g_sort.msgmap, thrd->rawno);

	    /* set collapsed bit */
	    if(msgno){
		set_lflag(stream, g_sort.msgmap, msgno, MN_COLL, 1);
		set_lflag(stream, g_sort.msgmap, msgno, MN_CHID, 0);
	    }

	    /* hide its children */
	    if(thrd->next && (nthrd = fetch_thread(stream, thrd->next)))
	      set_thread_subtree(stream, nthrd, msgmap, 1, MN_CHID);
	}

	if(thrd->nextthd)
	  thrd = fetch_thread(stream, thrd->nextthd);
	else
	  thrd = NULL;
    }

    dprint(9, (debugfile, "collapse_threads done\n"));
}


void
make_thrdflags_consistent(stream, msgmap, thrd, a_parent_is_collapsed)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    PINETHRD_S *thrd;
    int         a_parent_is_collapsed;
{
    PINETHRD_S *nthrd, *bthrd;
    unsigned long msgno;

    if(!thrd)
      return;

    msgno = mn_raw2m(msgmap, thrd->rawno);

    if(a_parent_is_collapsed){
	/* if some parent is collapsed, we should be hidden */
	if(msgno)
	  set_lflag(stream, msgmap, msgno, MN_CHID, 1);
    }
    else{
	/* no parent is collapsed so we are not hidden */
	if(msgno)
	  set_lflag(stream, msgmap, msgno, MN_CHID, 0);
    }

    if(thrd->next){
	nthrd = fetch_thread(stream, thrd->next);
	if(nthrd)
	  make_thrdflags_consistent(stream, msgmap, nthrd,
				    a_parent_is_collapsed
				      ? a_parent_is_collapsed
				      : get_lflag(stream, NULL, thrd->rawno,
						  MN_COLL));
    }

    if(thrd->branch){
	bthrd = fetch_thread(stream, thrd->branch);
	if(bthrd)
	  make_thrdflags_consistent(stream, msgmap, bthrd,
				    a_parent_is_collapsed);
    }
}


long
calculate_visible_threads(stream)
    MAILSTREAM *stream;
{
    PINETHRD_S   *thrd = NULL;
    long          vis = 0L;

    thrd = fetch_head_thread(stream);
    while(thrd){
	vis += (thread_has_some_visible(stream, thrd) ? 1 : 0);

	if(thrd->nextthd)
	  thrd = fetch_thread(stream, thrd->nextthd);
	else
	  thrd = NULL;
    }

    return(vis);
}


struct pass_along *
sort_thread_flatten(node, stream, entry, thrd, flags)
    THREADNODE *node;
    MAILSTREAM *stream;
    struct pass_along *entry;
    PINETHRD_S *thrd;
    unsigned    flags;
{
    long n = 0L;
    PINETHRD_S *newthrd = NULL;

    if(node){
	if(node->num){		/* holes happen */
	    n = (long) (entry - thrd_flatten_array);

	    for(; n > 0; n--)
	      if(thrd_flatten_array[n].rawno == node->num)
		break;	/* duplicate */

	    if(!n)
	      entry->rawno = node->num;
	}

	/*
	 * Build a richer threading structure that will help us paint
	 * and operate on threads and subthreads.
	 */
	if(!n && node->num){
	    newthrd = msgno_thread_info(stream, node->num, thrd, flags);
	    if(newthrd){
		entry->thrd = newthrd;
		entry++;

		if(node->next)
		  entry = sort_thread_flatten(node->next, stream, entry,
					      newthrd, THD_NEXT);

		if(node->branch)
		  entry = sort_thread_flatten(node->branch, stream, entry,
					      newthrd,
					      (flags == THD_TOP) ? THD_TOP
								 : THD_BRANCH);
	    }
	}
    }

    return(entry);
}


/*
 * Make a copy of c-client's THREAD tree while eliminating dummy nodes.
 */
THREADNODE *
collapse_threadnode_tree(tree)
    THREADNODE *tree;
{
    THREADNODE *newtree = NULL;

    if(tree){
	if(tree->num){
	    newtree = mail_newthreadnode(NULL);
	    newtree->num  = tree->num;
	    if(tree->next)
	      newtree->next = collapse_threadnode_tree(tree->next);

	    if(tree->branch)
	      newtree->branch = collapse_threadnode_tree(tree->branch);
	}
	else{
	    if(tree->next)
	      newtree = collapse_threadnode_tree(tree->next);
	    
	    if(tree->branch){
		if(newtree){
		    THREADNODE *last_branch = NULL;

		    /*
		     * Next moved up to replace "tree" in the tree.
		     * If next has no branches, then we want to branch off
		     * of next. If next has branches, we want to branch off
		     * of the last of those branches instead.
		     */
		    last_branch = newtree;
		    while(last_branch->branch)
		      last_branch = last_branch->branch;
		    
		    last_branch->branch = collapse_threadnode_tree(tree->branch);
		}
		else
		  newtree = collapse_threadnode_tree(tree->branch);
	    }
	}
    }

    return(newtree);
}


/*
 * Args      stream -- the usual
 *            rawno -- the raw msg num associated with this new node
 * attached_to_thrd -- the PINETHRD_S node that this is either a next or branch
 *                       off of
 *            flags --
 */
PINETHRD_S *
msgno_thread_info(stream, rawno, attached_to_thrd, flags)
    MAILSTREAM   *stream;
    unsigned long rawno;
    PINETHRD_S   *attached_to_thrd;
    unsigned      flags;
{
    PINELT_S   **peltp;

    if(!stream || rawno < 1L || rawno > stream->nmsgs)
      return NULL;

    /*
     * any private elt data yet?
     */
    if(*(peltp = (PINELT_S **) &mail_elt(stream, rawno)->sparep) == NULL){
	*peltp = (PINELT_S *) fs_get(sizeof(PINELT_S));
	memset(*peltp, 0, sizeof(PINELT_S));
    }

    if((*peltp)->pthrd == NULL)
      (*peltp)->pthrd = (PINETHRD_S *) fs_get(sizeof(PINETHRD_S));

    memset((*peltp)->pthrd, 0, sizeof(PINETHRD_S));

    (*peltp)->pthrd->rawno = rawno;

    if(attached_to_thrd)
      (*peltp)->pthrd->head = attached_to_thrd->head;
    else
      (*peltp)->pthrd->head = (*peltp)->pthrd->rawno;	/* it's me */

    if(flags == THD_TOP){
	/*
	 * We can tell this thread is a top-level thread because it doesn't
	 * have a parent.
	 */
	(*peltp)->pthrd->top = (*peltp)->pthrd->rawno;	/* I am a top */
	if(attached_to_thrd){
	    attached_to_thrd->nextthd = (*peltp)->pthrd->rawno;
	    (*peltp)->pthrd->prevthd  = attached_to_thrd->rawno;
	    (*peltp)->pthrd->thrdno   = attached_to_thrd->thrdno + 1L;
	}
	else
	    (*peltp)->pthrd->thrdno   = 1L;		/* 1st thread */

	g_sort.msgmap->max_thrdno = (*peltp)->pthrd->thrdno;
    }
    else if(flags == THD_NEXT){
	if(attached_to_thrd){
	    attached_to_thrd->next  = (*peltp)->pthrd->rawno;
	    (*peltp)->pthrd->parent = attached_to_thrd->rawno;
	    (*peltp)->pthrd->top    = attached_to_thrd->top;
	}
    }
    else if(flags == THD_BRANCH){
	if(attached_to_thrd){
	    attached_to_thrd->branch = (*peltp)->pthrd->rawno;
	    (*peltp)->pthrd->parent  = attached_to_thrd->parent;
	    (*peltp)->pthrd->top     = attached_to_thrd->top;
	}
    }

    return((*peltp)->pthrd);
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

    /*
     * If there is filtering happening, isort will become larger than sort.
     * Sort is a list of raw message numbers in their sorted order. There
     * are missing raw numbers because some of the messages are excluded
     * (MN_EXLD) from the view. Isort has one entry for every raw message
     * number, which maps to the corresponding msgno (the row in the sort
     * array). Some of the entries in isort are not used because those
     * messages are excluded, but the entry is still there because we want
     * to map from rawno to message number and the row number is the rawno.
     */
    (*msgs)->isort_size = (*msgs)->sort_size;
    if((*msgs)->isort)
      fs_resize((void **)&((*msgs)->isort), len);
    else
      (*msgs)->isort = (long *)fs_get(len);

    (*msgs)->max_msgno    = tot;
    (*msgs)->nmsgs	  = tot;

    /* set the inverse array */
    msgno_reset_isort(*msgs);

    (*msgs)->sort_order   = ps_global->def_sort;
    (*msgs)->reverse_sort = ps_global->def_sort_rev;
    (*msgs)->flagged_hid  = 0L;
    (*msgs)->flagged_exld = 0L;
    (*msgs)->flagged_chid = 0L;
    (*msgs)->flagged_chid2= 0L;
    (*msgs)->flagged_coll = 0L;
    (*msgs)->flagged_usor = 0L;
    (*msgs)->flagged_tmp  = 0L;
    (*msgs)->flagged_stmp = 0L;

    /*
     * This one is the total number of messages which are flagged
     * hid OR chid. It isn't the sum of those two because a
     * message may be flagged both at the same time.
     */
    (*msgs)->flagged_invisible = 0L;

    /*
     * And this keeps track of visible threads in the THRD_INDX. This is
     * weird because a thread is visible if any of its messages are
     * not hidden, including those that are CHID hidden. You can't just
     * count up all the messages that are hid or chid because you would
     * miss a thread that has its top-level message hidden but some chid
     * message not hidden.
     */
    (*msgs)->visible_threads = -1L;
    ASSERT_ISORT(*msgs, "validity fail in init\n");
}


#if defined(ISORT_ASSERT)

char *
assert_isort_validity(msgs)
    MSGNO_S *msgs;
{
    long *sort, *isort, i, nmsgs, max_msgno;
    static char buf[500];
    int   count_zeroes = 0;

    if(!msgs)
      return("isort: msgs = NULL");
    
    sort      = msgs->sort;
    isort     = msgs->isort;
    nmsgs     = msgs->nmsgs;
    max_msgno = msgs->max_msgno;

    if(!isort)
      return("isort: isort = NULL");

    if(max_msgno + 1 > msgs->sort_size){
	dump_isort(msgs);
	return("isort: sort_size too small");
    }

    if(nmsgs + 1 > msgs->isort_size){
	dump_isort(msgs);
	return("isort: isort_size too small");
    }

    for(i = 1L; i <= nmsgs; i++){
	if(isort[i] < 0L || isort[i] > max_msgno){
	    dump_isort(msgs);
	    sprintf(buf, "isort: out of range: isort[%ld]=%ld",
		    i, isort[i]);
	    return(buf);
	}

	if(isort[i] == 0L)
	  count_zeroes++;

	if(i <= max_msgno){
	    if(sort[i] < 1){
		dump_isort(msgs);
		sprintf(buf,
		 "sort array out of range: sort[%ld]=%ld, which is less than 1",
			i, sort[i]);
		return(buf);
	    }

	    if(sort[i] > nmsgs){
		dump_isort(msgs);
		sprintf(buf,
    "sort array out of range: sort[%ld]=%ld which is greater than nmsgs=%ld",
			i, sort[i], nmsgs);
		return(buf);
	    }

	    if(isort[sort[i]] != i){
		dump_isort(msgs);
		sprintf(buf,
	   "isort inconsistent: sort[%ld]=%ld isort[%ld]=%ld which is not %ld",
			i, sort[i], sort[i], isort[sort[i]], i);
		return(buf);
	    }
	}
    }

    if(count_zeroes != any_lflagged(msgs, MN_EXLD)){
	dump_isort(msgs);
	sprintf(buf, "isort: undefineds off: isort zeroes=%d excluded=%ld",
		count_zeroes, any_lflagged(msgs, MN_EXLD));
	return(buf);
    }

    return(NULL);
}

void
dump_isort(msgs)
    MSGNO_S *msgs;
{
    long *sort, *isort, i, nmsgs, max_msgno;

    sort      = msgs->sort;
    isort     = msgs->isort;
    nmsgs     = msgs->nmsgs;
    max_msgno = msgs->max_msgno;

    dprint(1, (debugfile, "\n\n ---- Isort Debug Info ----\n\n"));

    dprint(1, (debugfile, "sort: %s%s\n",
	    msgs->reverse_sort ? "Reverse " : "",
	    sort_name(ps_global->sort_types[msgs->sort_order])));
    dprint(1, (debugfile, "sort_order=%d\t\treverse_sort=%d\n",
	    msgs->sort_order, (int) msgs->reverse_sort));
    dprint(1, (debugfile, "manual_sort=%d\t\tflagged_hid=%ld\n",
	    (int) msgs->manual_sort, msgs->flagged_hid));
    dprint(1, (debugfile, "flagged_exld=%ld\t\tflagged_coll=%ld\n",
	    msgs->flagged_exld, msgs->flagged_coll));
    dprint(1, (debugfile, "flagged_chid=%ld\t\tflagged_chid2=%ld\n",
	    msgs->flagged_chid, msgs->flagged_chid2));
    dprint(1, (debugfile, "flagged_usor=%ld\t\tflagged_usor=%ld\n",
	    msgs->flagged_exld, msgs->flagged_coll));
    dprint(1, (debugfile, "flagged_tmp=%ld\t\tflagged_stmp=%ld\n",
	    msgs->flagged_tmp, msgs->flagged_stmp));
    dprint(1, (debugfile, "flagged_invisible=%ld\tvisible_threads=%ld\n",
	    msgs->flagged_invisible, msgs->visible_threads));

    dprint(1, (debugfile, "top=%ld\t\tmax_thrdno=%ld\n",
	    msgs->top, msgs->max_thrdno));
    dprint(1, (debugfile, "sort_size=%ld\tisort_size=%ld\tsel_size=%ld\n",
	    msgs->sort_size, msgs->isort_size, msgs->sel_size));
    dprint(1, (debugfile, "max_msgno=%ld\tnmsgs=%ld\n",
	    msgs->max_msgno, msgs->nmsgs));

    if(!msgs->sort)
      dprint(1, (debugfile, "sort is NULL\n"));
    if(!msgs->isort)
      dprint(1, (debugfile, "isort is NULL\n"));
    if(!msgs->select)
      dprint(1, (debugfile, "select is NULL\n"));

    if(msgs->select)
      for(i = 0L; i < msgs->sel_cnt; i++){
	  dprint(1, (debugfile, "select[%ld]=%ld\n", i, msgs->select[i]));
      }

    dprint(1, (debugfile, "row\tsort\tisort\n"));
    for(i = 1L; i <= nmsgs; i++){
	char buf1[100], buf2[100];

	if(sort && i <= max_msgno)
	  sprintf(buf1, "%ld", sort[i]);
	else
	  buf1[0] = '\0';

	if(isort)
	  sprintf(buf2, "%ld", isort[i]);
	else
	  buf2[0] = '\0';

	dprint(1, (debugfile, "%ld\t%s\t%s\n", i, buf1, buf2));
    }
}

#endif /* ISORT_ASSERT */


/*
 * Isort makes mn_raw2m fast. Alternatively, we could look through
 * the sort array to do mn_raw2m.
 */
void
msgno_reset_isort(msgs)
    MSGNO_S *msgs;
{
    long i;

    if(msgs){
	/*
	 * Zero isort so raw messages numbers which don't appear in the
	 * sort array show up as undefined.
	 */
	memset((void *) msgs->isort, 0,
	       (size_t) msgs->isort_size * sizeof(long));

	/* fill in all the defined entries */
	for(i = 1L; i <= mn_get_total(msgs); i++)
	  msgs->isort[msgs->sort[i]] = i;
    }
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
	ASSERT_ISORT(*msgs, "validity fail in msgno_give\n");
	if((*msgs)->sort)
	  fs_give((void **) &((*msgs)->sort));

	if((*msgs)->isort)
	  fs_give((void **) &((*msgs)->isort));

	if((*msgs)->select)
	  fs_give((void **) &((*msgs)->select));

	fs_give((void **) msgs);
    }
}


void
free_pine_elt(peltp)
    PINELT_S **peltp;
{
    if(peltp && *peltp){
	msgno_free_exceptions(&(*peltp)->exceptions);
	if((*peltp)->pthrd)
	  fs_give((void **) &(*peltp)->pthrd);

	fs_give((void **) peltp);
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
msgno_inc(stream, msgs, flags)
    MAILSTREAM *stream;
    MSGNO_S    *msgs;
    int         flags;
{
    long i;

    if(!msgs || mn_get_total(msgs) < 1L)
      return;
    
    for(i = msgs->select[msgs->sel_cur] + 1; i <= mn_get_total(msgs); i++){
        if(!msgline_hidden(stream, msgs, i, flags)){
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
msgno_dec(stream, msgs, flags)
    MAILSTREAM *stream;
    MSGNO_S     *msgs;
    int          flags;
{
    long i;

    if(!msgs || mn_get_total(msgs) < 1L)
      return;

    for(i = (msgs)->select[((msgs)->sel_cur)] - 1L; i >= 1L; i--){
        if(!msgline_hidden(stream, msgs, i, flags)){
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
    char b[100];

    sprintf(b, "Isort validity: start of msgno_exclude: msgno=%ld\n", msgno);
    ASSERT_ISORT(msgmap, b);
    /*--- clear all flags to keep our counts consistent  ---*/
    set_lflag(stream, msgmap, msgno, MN_HIDE | MN_CHID | MN_CHID2 | MN_SLCT, 0);
    set_lflag(stream, msgmap, msgno, MN_EXLD, 1); /* mark excluded */

    /* erase knowledge in sort array (shift array down) */
    for(i = msgno + 1L; i <= msgmap->max_msgno; i++)
      msgmap->sort[i-1L] = msgmap->sort[i];

    msgmap->max_msgno = max(0L, msgmap->max_msgno - 1L);
    msgno_reset_isort(msgmap);
    msgno_flush_selected(msgmap, msgno);
    sprintf(b, "Isort validity: end of msgno_exclude: msgno=%ld\n", msgno);
    ASSERT_ISORT(msgmap, b);
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
	    msgs->isort[i] = msgs->max_msgno;
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
	else if(filtered && (exbits & MSG_EX_FILTERED)
		&& !(exbits & MSG_EX_FILED)
		&& !(exbits & MSG_EX_MANFLAGGED)){
	    /*
	     * We get here if the message was filtered by a filter that
	     * just changes status bits (it wasn't excluded). It has also
	     * not been manually flagged. If it was manually flagged, we
	     * don't want to reprocess the filter, undoing the user's
	     * manual flagging. Of course, a new pine will re check this
	     * message anyway, so the user had better be using this
	     * manual flagging only to temporarily save him or herself
	     * from an expunge before Saving or printing or something.
	     */
	    exbits ^= MSG_EX_FILTERED;
	    msgno_exceptions(stream, i, "0", &exbits, TRUE);
	}
    }

    ASSERT_ISORT(msgs, "isort validity: end of msgno_include\n");
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
    long   slop, islop, old_total, old_size, old_isize;
    size_t len, ilen;

    if(!msgs || n <= 0L)
      return;

    old_total        = msgs->max_msgno;
    old_size         = msgs->sort_size;
    old_isize        = msgs->isort_size;
    slop             = (msgs->max_msgno + n + 1L) % 64;
    islop            = (msgs->nmsgs + n + 1L) % 64;
    msgs->sort_size  = (msgs->max_msgno + n + 1L) + (64 - slop);
    msgs->isort_size = (msgs->nmsgs + n + 1L) + (64 - islop);
    len		     = (size_t) msgs->sort_size * sizeof(long);
    ilen	     = (size_t) msgs->isort_size * sizeof(long);
    if(msgs->sort){
	if(old_size != msgs->sort_size)
	  fs_resize((void **) &(msgs->sort), len);
    }
    else
      msgs->sort = (long *) fs_get(len);

    if(msgs->isort){
	if(old_isize != msgs->isort_size)
	  fs_resize((void **) &(msgs->isort), ilen);
    }
    else
      msgs->isort = (long *) fs_get(ilen);

    while(n-- > 0){
	msgs->sort[++msgs->max_msgno] = ++msgs->nmsgs;
	msgs->isort[msgs->nmsgs] = msgs->max_msgno;
    }

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

    {	char b[100];
	sprintf(b, "isort validity: msgno_add_raw: n=%ld\n", n);
	ASSERT_ISORT(msgs, b);
    }
}


/*----------------------------------------------------------------------
  Remove all knowledge of the given raw message number

   Accepts: msgs - pointer to message manipulation struct
	    rawno - number to remove
   Returns: with fixed up msgno struct

   After removing *all* references, adjust the sort array and
   various pointers accordingly...
  ----*/
void
msgno_flush_raw(msgs, rawno)
     MSGNO_S *msgs;
     long     rawno;
{
    long i, old_sorted = 0L;
    int  shift = 0;

    if(!msgs)
      return;

    /* blast rawno from sort array */
    for(i = 1L; i <= msgs->max_msgno; i++){
	if(msgs->sort[i] == rawno){
	    old_sorted = i;
	    shift++;
	}

	if(shift && i < msgs->max_msgno)
	  msgs->sort[i] = msgs->sort[i + 1L];

	if(msgs->sort[i] > rawno)
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

    msgno_reset_isort(msgs);

    {	char b[100];
	sprintf(b,
		"isort validity: end of msgno_flush_raw: rawno=%ld\n", rawno);
	ASSERT_ISORT(msgs, b);
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
    PINELT_S **peltp;
    PARTEX_S **partp;

    if(!stream || rawno < 1L || rawno > stream->nmsgs)
      return FALSE;

    /*
     * Get pointer to exceptional part list, and scan down it
     * for the requested part...
     */
    if(*(peltp = (PINELT_S **) &mail_elt(stream, rawno)->sparep))
      for(partp = &(*peltp)->exceptions; *partp; partp = &(*partp)->next){
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
	       * The caller provided flags, but no part.
	       * We are looking to see if the bits are set in any of the
	       * parts. This doesn't count parts with non-digit partno's (like
	       * scores) because those are used differently.
	       * any of the flags...
	       */
	      if((*partp)->partno && *(*partp)->partno &&
		 isdigit((unsigned char) *(*partp)->partno) &&
		 (*bits & (*partp)->handling) == *bits)
		return(TRUE);
	  }
	  else
	    /*
	     * The caller didn't specify a part, so
	     * they must just be interested in whether
	     * the msg had any exceptions at all...
	     */
	    return(TRUE);
      }

    if(set && part){
	if(!*peltp){
	    *peltp = (PINELT_S *) fs_get(sizeof(PINELT_S));
	    memset(*peltp, 0, sizeof(PINELT_S));
	    partp = &(*peltp)->exceptions;
	}

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
    PINELT_S  *pelt;
    PARTEX_S **partp;

    for(n = mn_first_cur(msgmap); n > 0L; n = mn_next_cur(msgmap))
      if(pelt = (PINELT_S *) mail_elt(stream, mn_m2raw(msgmap, n))->sparep)
        for(partp = &pelt->exceptions; *partp; partp = &(*partp)->next)
	  if(((*partp)->handling & MSG_EX_DELETE)
	     && (*partp)->partno
	     && *(*partp)->partno != '0'
	     && isdigit((unsigned char) *(*partp)->partno))
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
struct index_cache {
   void	  *cache;				/* pointer to cache         */
   long    num;					/* # of last index in cache */
   size_t  size;				/* size of each index line  */
   int     flags;
};

static struct index_cache
    icache =  { (void *) NULL, (long) 0, (size_t) 0, (int) 0 },
    ticache = { (void *) NULL, (long) 0, (size_t) 0, (int) 0 };

#define IC_NEED_FORMAT_SETUP         0x01
#define IC_FORMAT_INCLUDES_MSGNO     0x02
#define IC_FORMAT_INCLUDES_SMARTDATE 0x04
  
/*
 * cache size growth increment
 */

#define	IC_SIZE		100

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
		 + ((max(ps_global->ttyo->screen_cols, 80)+1) * sizeof(char));

    if(j = (newsize % sizeof(long)))		/* alignment hack */
      newsize += (sizeof(long) - (size_t)j);

    if(icache.size != newsize){
	clear_iindex_cache();			/* clear cache, start over! */
	icache.size = newsize;
    }

    if(indx > (j = icache.num - 1L)){		/* make room for entry! */
	size_t  tmplen = icache.size;
	char   *tmpline;

	while(indx >= icache.num)
	  icache.num += IC_SIZE;

	if(icache.cache == NULL){
	    icache.cache = (void *)fs_get((icache.num+1)*tmplen);
	    memset(icache.cache, 0, (icache.num+1)*tmplen);
	}
	else{
            fs_resize((void **)&(icache.cache), (size_t)(icache.num+1)*tmplen);
	    tmpline = (char *)icache.cache + ((j+1) * tmplen);
	    memset(tmpline, 0, (icache.num - j) * tmplen);
	}
    }

    if(SEP_THRDINDX()){
	if(ps_global->msgmap->max_thrdno > 0L){
	    if(ticache.size != newsize){
		clear_tindex_cache();
		ticache.size = newsize;
	    }

	    if(ps_global->msgmap->max_thrdno > ticache.num){
		size_t  tmplen = ticache.size;
		char   *tmpline;

		ticache.num = ps_global->msgmap->max_thrdno;

		if(ticache.cache == NULL){
		    /* The +1 seems superflous, but I'm leaving it */
		    ticache.cache = (void *)fs_get((ticache.num+1)*tmplen);
		    memset(ticache.cache, 0, (ticache.num+1)*tmplen);
		}
		else{
		    j = ticache.num - 1L;
		    fs_resize((void **)&(ticache.cache),
			      (size_t)(ticache.num+1)*tmplen);
		    tmpline = (char *)ticache.cache + ((j+1) * tmplen);
		    memset(tmpline, 0, (ticache.num - j) * tmplen);
		}
	    }
	}
    }

    return(1);
}


/*
 * return the index line associated with the given message number
 */
HLINE_S *
get_index_cache(msgno)
    long         msgno;
{
    static HLINE_S *dummy_to_protect_ourselves = NULL;

    /*
     * This is not necessary if we did everything right elsewhere.
     */
    if(msgno <= 0L){
	size_t big_enough;

	dprint(2, (debugfile, "Called get_index_cache with msgno=%ld\n",
		msgno));
	big_enough = sizeof(HLINE_S) + (MAX_SCREEN_COLS * sizeof(char))
		     + sizeof(long);
	if(!dummy_to_protect_ourselves)
	  dummy_to_protect_ourselves = (HLINE_S *) fs_get(big_enough);
	
	memset(dummy_to_protect_ourselves, 0, big_enough);

	return(dummy_to_protect_ourselves);
    }

    if(need_format_setup() && setup_header_widths)
      (*setup_header_widths)();

    if(!i_cache_size(--msgno)){
	q_status_message(SM_ORDER, 0, 3, "get_index_cache failed!");
	return(NULL);
    }

    return((HLINE_S *) ((char *)(icache.cache) 
	   + (msgno * (long)icache.size * sizeof(char))));
}


HLINE_S *
get_tindex_cache(thrdno)
    long         thrdno;
{
    if(thrdno > 0)		/* this should always be true */
      --thrdno;
    else
      return NULL;

    return((HLINE_S *) ((char *)(ticache.cache) 
	   + (thrdno * (long)ticache.size * sizeof(char))));
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
       || any_lflagged(ps_global->msgmap, (MN_HIDE|MN_CHID|MN_EXLD|MN_SLCT)))
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
			      ps_global->msgmap, bc_current++, NULL);
}


/*
 * erase a particular entry in the cache
 */
void
clear_index_cache_ent(indx)
    long indx;
{
    HLINE_S *hline = get_index_cache(indx);

    if(indx <= 0L)
      return;

    if(SEP_THRDINDX()){
	if(!hline->tihl){
	    PINETHRD_S   *thrd;

	    thrd = fetch_thread(ps_global->mail_stream,
				mn_m2raw(ps_global->msgmap, indx));
	    if(thrd && thrd->top && thrd->top != thrd->rawno
	       && (thrd=fetch_thread(ps_global->mail_stream, thrd->top))){
		hline->tihl = get_tindex_cache(thrd->thrdno);
	    }
	}

	if(hline->tihl
	   && (hline->tihl->id || hline->tihl->color_lookup_done
	       || *hline->tihl->line))
	  memset((void *)hline->tihl, 0, sizeof(*hline->tihl));
    }

    if(hline->id || hline->color_lookup_done || *hline->line)
      memset((void *)hline, 0, sizeof(*hline));
}


/*
 * clear the index caches associated with the current mailbox
 */
void
clear_index_cache()
{
    clear_iindex_cache();
    clear_tindex_cache();
}


/*
 * clear the index cache associated with the current mailbox
 */
void
clear_iindex_cache()
{
    if(icache.cache)
      fs_give((void **)&(icache.cache));

    icache.num  = 0L;
    icache.size = 0;
    bc_this_stream = NULL;
    set_need_format_setup();
}


void
clear_tindex_cache()
{
    int i;
    HLINE_S *hline;

    if(ticache.cache)
      fs_give((void **)&(ticache.cache));

    ticache.num  = 0L;
    ticache.size = 0;
    
    /*
     * Erase references to ticache.cache in icache.cache. Do it without
     * calling get_index_cache which calls i_cache_size and does stuff
     * we don't want, like re-calling clear_tindex_cache.
     */
    for(i = 0; i < icache.num; i++){
	hline = (HLINE_S *) ((char *)(icache.cache)
				+ (i * (long)icache.size * sizeof(char)));
	hline->tihl = NULL;
    }
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
	if(any_lflagged(ps_global->msgmap, MN_HIDE | MN_CHID)){
	    long n, x;

	    for(n = 1L, x = 0;
		x < scroll_pos && n < mn_get_total(ps_global->msgmap);
		n++)
	      if(!msgline_hidden(ps_global->mail_stream, ps_global->msgmap,
			         n, 0))
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
			     env, body, NULL, FM_NEW_MESS, pc)){
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
		    (order & 0x00000100) != 0, SRT_VRB);
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


void
thread_command(state, stream, msgmap, preloadkeystroke, q_line)
    struct pine *state;
    MAILSTREAM	*stream;
    MSGNO_S     *msgmap;
    int	         preloadkeystroke;
    int	         q_line;
{
    PINETHRD_S   *thrd = NULL;
    unsigned long rawno, save_branch;
    int           we_cancel = 0;
    int           flags = AC_FROM_THREAD;

    if(!(stream && msgmap))
      return;

    rawno = mn_m2raw(msgmap, mn_get_cur(msgmap));
    if(rawno)
      thrd = fetch_thread(stream, rawno);

    if(!thrd)
      return;

    save_branch = thrd->branch;
    thrd->branch = 0L;		/* branch is a sibling, not part of thread */

    if(!preloadkeystroke){
	if(!THRD_INDX()){
	    if(get_lflag(stream, NULL, rawno, MN_COLL) && thrd->next)
	      flags |= AC_EXPN;
	    else
	      flags |= AC_COLL;
	}

	if(count_lflags_in_thread(stream, thrd, msgmap, MN_SLCT)
	       == count_lflags_in_thread(stream, thrd, msgmap, MN_NONE))
	  flags |= AC_UNSEL;
    }

    we_cancel = busy_alarm(1, NULL, NULL, 0);

    /* save the SLCT flags in STMP for restoring at the bottom */
    copy_lflags(stream, msgmap, MN_SLCT, MN_STMP);

    /* clear the values from the SLCT flags */
    set_lflags(stream, msgmap, MN_SLCT, 0);

    /* set SLCT for thrd on down */
    set_flags_for_thread(stream, msgmap, MN_SLCT, thrd, 1);
    thrd->branch = save_branch;

    if(we_cancel)
      cancel_busy_alarm(0);

    (void ) apply_command(state, stream, msgmap, preloadkeystroke, flags,
			  q_line);

    /* restore the original flags */
    copy_lflags(stream, msgmap, MN_STMP, MN_SLCT);

    if(any_lflagged(msgmap, MN_HIDE) > 0L){
	long cur;

	/* if nothing left selected, unhide all */
	if(any_lflagged(msgmap, MN_SLCT) == 0L){
	    (void) unzoom_index(ps_global, stream, msgmap);
	    dprint(4, (debugfile, "\n\n ---- Exiting ZOOM mode ----\n"));
	    q_status_message(SM_ORDER,0,2, "Index Zoom Mode is now off");
	}

	/* if current is hidden, adjust */
	adjust_cur_to_visible(stream, msgmap);
    }
}


/*
 * Set flag f to v for all messages in thrd.
 *
 * Watch out when calling this. The thrd->branch is not part of thrd.
 * Branch is a sibling to thrd, not a child. Zero out branch before calling
 * or call on thrd->next and worry about thrd separately.
 * Ok to call it on top-level thread which has no branch already.
 */
void
set_flags_for_thread(stream, msgmap, f, thrd, v)
    MAILSTREAM  *stream;
    MSGNO_S     *msgmap;
    int          f;
    PINETHRD_S  *thrd;
    int          v;
{
    PINETHRD_S *nthrd, *bthrd;

    if(!(stream && thrd && msgmap))
      return;

    set_lflag(stream, msgmap, mn_raw2m(msgmap, thrd->rawno), f, v);

    if(thrd->next){
	nthrd = fetch_thread(stream, thrd->next);
	if(nthrd)
	  set_flags_for_thread(stream, msgmap, f, nthrd, v);
    }

    if(thrd->branch){
	bthrd = fetch_thread(stream, thrd->branch);
	if(bthrd)
	  set_flags_for_thread(stream, msgmap, f, bthrd, v);
    }
}


/*
 * Set search bit for every message in a thread.
 *
 * Watch out when calling this. The thrd->branch is not part of thrd.
 * Branch is a sibling to thrd, not a child. Zero out branch before calling
 * or call on thrd->next and worry about thrd separately. Top-level threads
 * already have a branch equal to zero.
 */
void
set_search_bit_for_thread(stream, thrd)
    MAILSTREAM  *stream;
    PINETHRD_S  *thrd;
{
    PINETHRD_S *nthrd, *bthrd;

    if(!(stream && thrd))
      return;

    if(thrd->rawno > 0L && thrd->rawno <= stream->nmsgs)
      mm_searched(stream, thrd->rawno);

    if(thrd->next){
	nthrd = fetch_thread(stream, thrd->next);
	if(nthrd)
	  set_search_bit_for_thread(stream, nthrd);
    }

    if(thrd->branch){
	bthrd = fetch_thread(stream, thrd->branch);
	if(bthrd)
	  set_search_bit_for_thread(stream, bthrd);
    }
}


/*
 * Copy value of flag from to flag to.
 */
void
copy_lflags(stream, msgmap, from, to)
    MAILSTREAM  *stream;
    MSGNO_S     *msgmap;
    int          from, to;
{
    unsigned long i;
    int           hide;

    hide = ((to == MN_SLCT) && (any_lflagged(msgmap, MN_HIDE) > 0L));

    set_lflags(stream, msgmap, to, 0);

    if(any_lflagged(msgmap, from))
      for(i = 1L; i <= mn_get_total(msgmap); i++)
	if(get_lflag(stream, msgmap, i, from))
	  set_lflag(stream, msgmap, i, to, 1);
	else if(hide)
	  set_lflag(stream, msgmap, i, MN_HIDE, 1);
}


/*
 * Set flag f to value v in all message.
 */
void
set_lflags(stream, msgmap, f, v)
    MAILSTREAM  *stream;
    MSGNO_S     *msgmap;
    int          f;
    int          v;
{
    unsigned long i;

    if((v == 0 && any_lflagged(msgmap, f)) || v )
      for(i = 1L; i <= mn_get_total(msgmap); i++)
	set_lflag(stream, msgmap, i, f, v);
}


/*
 * Collapse or expand a threading subtree. Not called from separate thread
 * index.
 */
void
collapse_or_expand(state, stream, msgmap, msgno)
    struct pine *state;
    MAILSTREAM  *stream;
    MSGNO_S     *msgmap;
    unsigned long msgno;
{
    int           collapsed, adjust_current = 0;
    PINETHRD_S   *thrd = NULL, *nthrd;
    HLINE_S      *hline;
    unsigned long rawno;

    if(!stream)
      return;

    /*
     * If msgno is a good msgno, then we collapse or expand the subthread
     * which begins at msgno. If msgno is 0, we collapse or expand the
     * entire current thread.
     */

    if(msgno > 0L && msgno <= mn_get_total(msgmap)){
	rawno = mn_m2raw(msgmap, msgno);
	if(rawno)
	  thrd = fetch_thread(stream, rawno);
    }
    else if(msgno == 0L){
	rawno = mn_m2raw(msgmap, mn_get_cur(msgmap));
	if(rawno)
	  thrd = fetch_thread(stream, rawno);
	
	if(thrd && thrd->top != thrd->rawno){
	    adjust_current++;
	    thrd = fetch_thread(stream, thrd->top);
	}
    }


    if(!thrd)
      return;

    collapsed = get_lflag(stream, NULL, thrd->rawno, MN_COLL) && thrd->next;

    if(collapsed){
	msgno = mn_raw2m(msgmap, thrd->rawno);
	if(msgno > 0L && msgno <= mn_get_total(msgmap)){
	    set_lflag(stream, msgmap, msgno, MN_COLL, 0);
	    if(thrd->next){
		if(nthrd = fetch_thread(stream, thrd->next))
		  set_thread_subtree(stream, nthrd, msgmap, 0, MN_CHID);

		clear_index_cache_ent(msgno);
	    }
	}
    }
    else if(thrd && thrd->next){
	msgno = mn_raw2m(msgmap, thrd->rawno);
	if(msgno > 0L && msgno <= mn_get_total(msgmap)){
	    set_lflag(stream, msgmap, msgno, MN_COLL, 1);
	    if(nthrd = fetch_thread(stream, thrd->next))
	      set_thread_subtree(stream, nthrd, msgmap, 1, MN_CHID);

	    clear_index_cache_ent(msgno);
	}
    }
    else
      q_status_message(SM_ORDER, 0, 1,
		       "No thread to collapse or expand on this line");
    
    /* if current is hidden, adjust */
    if(adjust_current)
      adjust_cur_to_visible(stream, msgmap);
}


/*
 * Select the messages in a subthread. If all of the messages are already
 * selected, unselect them. This routine is a bit strange because it
 * doesn't set the MN_SLCT bit. Instead, it sets MN_STMP in apply_command
 * and then thread_command copies the MN_STMP messages back to MN_SLCT.
 */
void
select_thread_stmp(state, stream, msgmap)
    struct pine *state;
    MAILSTREAM  *stream;
    MSGNO_S     *msgmap;
{
    PINETHRD_S   *thrd;
    unsigned long rawno, in_thread, set_in_thread, save_branch;

    /* ugly bit means the same thing as return of 1 from individual_select */
    state->ugly_consider_advancing_bit = 0;

    if(!(stream && msgmap))
      return;

    rawno = mn_m2raw(msgmap, mn_get_cur(msgmap));
    if(rawno)
      thrd = fetch_thread(stream, rawno);
    
    if(!thrd)
      return;
    
    /* run through thrd to see if it is all selected */
    save_branch = thrd->branch;
    thrd->branch = 0L;
    if((set_in_thread = count_lflags_in_thread(stream, thrd, msgmap, MN_STMP))
       == (in_thread = count_lflags_in_thread(stream, thrd, msgmap, MN_NONE)))
      set_thread_lflags(stream, thrd, msgmap, MN_STMP, 0);
    else{
	set_thread_lflags(stream, thrd, msgmap, MN_STMP, 1);
	state->ugly_consider_advancing_bit = 1;
    }

    thrd->branch = save_branch;
    
    if(set_in_thread == in_thread)
      q_status_message1(SM_ORDER, 0, 3, "Unselected %.200s messages in thread",
			comatose((long) in_thread));
    else if(set_in_thread == 0)
      q_status_message1(SM_ORDER, 0, 3, "Selected %.200s messages in thread",
			comatose((long) in_thread));
    else
      q_status_message1(SM_ORDER, 0, 3,
			"Selected %.200s more messages in thread",
			comatose((long) (in_thread-set_in_thread)));
}


/*
 * Count how many of this system flag in this thread subtree.
 * If flags == 0 count the messages in the thread.
 *
 * Watch out when calling this. The thrd->branch is not part of thrd.
 * Branch is a sibling to thrd, not a child. Zero out branch before calling
 * or call on thrd->next and worry about thrd separately.
 * Ok to call it on top-level thread which has no branch already.
 */
unsigned long
count_flags_in_thread(stream, thrd, flags)
    MAILSTREAM *stream;
    PINETHRD_S *thrd;
    long        flags;		/* flag to count */
{
    unsigned long rawno, count = 0;
    PINETHRD_S *nthrd, *bthrd;
    MESSAGECACHE *mc;

    if(!thrd || !stream || thrd->rawno < 1L || thrd->rawno > stream->nmsgs)
      return count;
    
    if(thrd->next){
	nthrd = fetch_thread(stream, thrd->next);
	if(nthrd)
	  count += count_flags_in_thread(stream, nthrd, flags);
    }

    if(thrd->branch){
	bthrd = fetch_thread(stream, thrd->branch);
	if(bthrd)
	  count += count_flags_in_thread(stream, bthrd, flags);
    }

    mc = mail_elt(stream, thrd->rawno);
    if(mc && mc->valid && FLAG_MATCH(flags, mc))
      count++;

    return count;
}


/*
 * Count how many of this local flag in this thread subtree.
 * If flags == MN_NONE then we just count the messages instead of whether
 * the messages have a flag set.
 *
 * Watch out when calling this. The thrd->branch is not part of thrd.
 * Branch is a sibling to thrd, not a child. Zero out branch before calling
 * or call on thrd->next and worry about thrd separately.
 * Ok to call it on top-level thread which has no branch already.
 */
unsigned long
count_lflags_in_thread(stream, thrd, msgmap, flags)
    MAILSTREAM *stream;
    PINETHRD_S *thrd;
    MSGNO_S    *msgmap;
    int         flags;		/* flag to count */
{
    unsigned long rawno, count = 0;
    PINETHRD_S *nthrd, *bthrd;

    if(!thrd || !stream || thrd->rawno < 1L || thrd->rawno > stream->nmsgs)
      return count;

    if(thrd->next){
	nthrd = fetch_thread(stream, thrd->next);
	if(nthrd)
	  count += count_lflags_in_thread(stream, nthrd, msgmap, flags);
    }

    if(thrd->branch){
	bthrd = fetch_thread(stream, thrd->branch);
	if(bthrd)
	  count += count_lflags_in_thread(stream, bthrd, msgmap,flags);
    }

    if(flags == MN_NONE)
      count++;
    else
      count += get_lflag(stream, msgmap, mn_raw2m(msgmap, thrd->rawno), flags);

    return count;
}


/*
 * Special-purpose for performance improvement.
 */
int
thread_has_some_visible(stream, thrd)
    MAILSTREAM *stream;
    PINETHRD_S *thrd;
{
    unsigned long rawno, count = 0;
    PINETHRD_S *nthrd, *bthrd;

    if(!thrd || !stream || thrd->rawno < 1L || thrd->rawno > stream->nmsgs)
      return 0;

    if(get_lflag(stream, NULL, thrd->rawno, MN_HIDE) == 0)
      return 1;

    if(thrd->next){
	nthrd = fetch_thread(stream, thrd->next);
	if(nthrd && thread_has_some_visible(stream, nthrd))
	  return 1;
    }

    if(thrd->branch){
	bthrd = fetch_thread(stream, thrd->branch);
	if(bthrd && thread_has_some_visible(stream, bthrd))
	  return 1;
    }

    return 0;
}


/*
 * Returns nonzero if considered hidden, 0 if not considered hidden.
 */
int
msgline_hidden(stream, msgmap, msgno, flags)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    long        msgno;
    int         flags;
{
    int ret;

    if(flags & MH_ANYTHD){
	ret = ((any_lflagged(msgmap, MN_HIDE) > 0)
	       && get_lflag(stream, msgmap, msgno, MN_HIDE));
    }
    else if(flags & MH_THISTHD && THREADING() && ps_global->viewing_a_thread){
	ret = (get_lflag(stream, msgmap, msgno, MN_HIDE)
	       || !get_lflag(stream, msgmap, msgno, MN_CHID2));
    }
    else{
	if(THREADING() && ps_global->viewing_a_thread){
	    ret = (get_lflag(stream, msgmap, msgno, MN_HIDE)
		   || !get_lflag(stream, msgmap, msgno, MN_CHID2)
		   || get_lflag(stream, msgmap, msgno, MN_CHID));
	}
	else if(THRD_INDX()){
	    /*
	     * If this message is in the collapsed part of a thread,
	     * it's hidden. It must be a top-level of a thread to be
	     * considered visible. Even if it is top-level, it is only
	     * visible if some message in the thread is not hidden.
	     */
	    if(get_lflag(stream, msgmap, msgno, MN_CHID))	/* not top */
	      ret = 1;
	    else{
		unsigned long rawno;
		PINETHRD_S   *thrd = NULL;

		rawno = mn_m2raw(msgmap, msgno);
		if(rawno)
		  thrd = fetch_thread(stream, rawno);

		ret = !thread_has_some_visible(stream, thrd);
	    }
	}
	else{
	    ret = ((any_lflagged(msgmap, MN_HIDE | MN_CHID) > 0)
		   && get_lflag(stream, msgmap, msgno, MN_HIDE | MN_CHID));
	}
    }
    
    dprint(10,(debugfile,
	       "msgline_hidden(%ld): %s\n", msgno, ret ? "HID" : "VIS"));

    return(ret);
}


int
mark_msgs_in_thread(stream, thrd, msgmap)
    MAILSTREAM *stream;
    PINETHRD_S *thrd;
    MSGNO_S    *msgmap;
{
    int           count = 0;
    long          n;
    PINETHRD_S   *nthrd, *bthrd;
    MESSAGECACHE *mc;

    if(!thrd || !stream || thrd->rawno < 1L || thrd->rawno > stream->nmsgs)
      return count;

    if(thrd->next){
	nthrd = fetch_thread(stream, thrd->next);
	if(nthrd)
	  count += mark_msgs_in_thread(stream, nthrd, msgmap);
    }

    if(thrd->branch){
	bthrd = fetch_thread(stream, thrd->branch);
	if(bthrd)
	  count += mark_msgs_in_thread(stream, bthrd, msgmap);
    }

    n = mn_raw2m(msgmap, thrd->rawno);

    if(thrd->rawno >= 1L && thrd->rawno <= stream->nmsgs &&
       !(mc = mail_elt(stream,thrd->rawno))->private.msg.env){
	mc->sequence = 1;
	count++;
    }

    return count;
}


/*
 * This sets or clears flags for the messages at this node and below in
 * a tree.
 *
 * Watch out when calling this. The thrd->branch is not part of thrd.
 * Branch is a sibling to thrd, not a child. Zero out branch before calling
 * or call on thrd->next and worry about thrd separately.
 * Ok to call it on top-level thread which has no branch already.
 */
void
set_thread_lflags(stream, thrd, msgmap, flags, v)
    MAILSTREAM *stream;
    PINETHRD_S *thrd;
    MSGNO_S    *msgmap;
    int         flags;		/* flags to set or clear */
    int         v;		/* set or clear? */
{
    unsigned long rawno, msgno;
    PINETHRD_S *nthrd, *bthrd;

    if(!thrd || !stream || thrd->rawno < 1L || thrd->rawno > stream->nmsgs)
      return;

    msgno = mn_raw2m(msgmap, thrd->rawno);

    set_lflag(stream, msgmap, msgno, flags, v);

    /*
     * Careful, performance hack. This should logically be a separate
     * operation on the thread but it is convenient to stick it in here.
     * Line[0] won't be set if we haven't been in here before.
     *
     * When we back out of viewing a thread to the separate-thread-index
     * we may leave behind some cached hlines that aren't quite right
     * because they were collapsed. In particular, the plus_col character
     * may be wrong. Instead of trying to figure out what it should be just
     * clear the cache entries for the this thread when we come back in
     * to view it again.
     */
    if(msgno > 0L && flags == MN_CHID2
       && v == 1 && get_index_cache(msgno)->line[0])
      clear_index_cache_ent(msgno);

    if(thrd->next){
	nthrd = fetch_thread(stream, thrd->next);
	if(nthrd)
	  set_thread_lflags(stream, nthrd, msgmap, flags, v);
    }

    if(thrd->branch){
	bthrd = fetch_thread(stream, thrd->branch);
	if(bthrd)
	  set_thread_lflags(stream, bthrd, msgmap, flags, v);
    }
}


/*
 * This is D if all of thread is deleted,
 * else N if any unseen and not deleted,
 * else blank.
 */
char
status_symbol_for_thread(stream, thrd, type)
    MAILSTREAM *stream;
    PINETHRD_S *thrd;
    IndexColType type;
{
    char        status = ' ';
    PINETHRD_S *nthrd, *bthrd;
    unsigned long save_branch, cnt, tot_in_thrd;

    if(!thrd || !stream || thrd->rawno < 1L || thrd->rawno > stream->nmsgs)
      return status;

    save_branch = thrd->branch;
    thrd->branch = 0L;		/* branch is a sibling, not part of thread */
    
    if(type == iStatus){
	/* all deleted */
	if(count_flags_in_thread(stream, thrd, F_DEL) ==
	   count_flags_in_thread(stream, thrd, F_NONE))
	  status = 'D';
	/* or any new and not deleted */
	else if((!IS_NEWS(stream)
		 || F_ON(F_FAKE_NEW_IN_NEWS, ps_global))
		&& count_flags_in_thread(stream, thrd, F_UNDEL|F_UNSEEN))
	  status = 'N';
    }
    else if(type == iFStatus){
	if(!IS_NEWS(stream) || F_ON(F_FAKE_NEW_IN_NEWS, ps_global)){
	    tot_in_thrd = count_flags_in_thread(stream, thrd, F_NONE);
	    cnt = count_flags_in_thread(stream, thrd, F_UNSEEN);
	    if(cnt)
	      status = (cnt == tot_in_thrd) ? 'N' : 'n';
	}
    }
    else if(type == iIStatus){
	tot_in_thrd = count_flags_in_thread(stream, thrd, F_NONE);

	/* unseen and recent */
	cnt = count_flags_in_thread(stream, thrd, F_RECENT|F_UNSEEN);
	if(cnt)
	  status = (cnt == tot_in_thrd) ? 'N' : 'n';
	else{
	    /* unseen and !recent */
	    cnt = count_flags_in_thread(stream, thrd, F_UNSEEN);
	    if(cnt)
	      status = (cnt == tot_in_thrd) ? 'U' : 'u';
	    else{
		/* seen and recent */
		cnt = count_flags_in_thread(stream, thrd, F_RECENT|F_SEEN);
		if(cnt)
		  status = (cnt == tot_in_thrd) ? 'R' : 'r';
	    }
	}
    }

    thrd->branch = save_branch;

    return status;
}


/*
 * Symbol is * if some message in thread is important,
 * + if some message is to us,
 * - if mark-for-cc and some message is cc to us, else blank.
 */
char
to_us_symbol_for_thread(stream, thrd, consider_flagged)
    MAILSTREAM *stream;
    PINETHRD_S *thrd;
    int         consider_flagged;
{
    char        to_us = ' ';
    PINETHRD_S *nthrd, *bthrd;

    if(!thrd || !stream || thrd->rawno < 1L || thrd->rawno > stream->nmsgs)
      return to_us;

    if(thrd->next){
	nthrd = fetch_thread(stream, thrd->next);
	if(nthrd)
	  to_us = to_us_symbol_for_thread(stream, nthrd, consider_flagged);
    }

    if(to_us == ' ' && thrd->branch){
	bthrd = fetch_thread(stream, thrd->branch);
	if(bthrd)
	  to_us = to_us_symbol_for_thread(stream, bthrd, consider_flagged);
    }

    if(to_us != '*'){
	if(consider_flagged && FLAG_MATCH(F_FLAG, mail_elt(stream,thrd->rawno)))
	  to_us = '*';
	else if(to_us != '+' && !IS_NEWS(stream)){
	    INDEXDATA_S   idata;
	    MESSAGECACHE *mc;
	    ADDRESS      *addr;

	    memset(&idata, 0, sizeof(INDEXDATA_S));
	    idata.stream   = stream;
	    idata.rawno    = thrd->rawno;
	    idata.msgno    = mn_raw2m(current_index_state->msgmap, idata.rawno);
	    if(mc = mail_elt(stream, idata.rawno)){
		idata.size = mc->rfc822_size;
		index_data_env(&idata, mail_fetchenvelope(stream, idata.rawno));
	    }
	    else
	      idata.bogus = 2;

	    for(addr = fetch_to(&idata); addr; addr = addr->next)
	      if(address_is_us(addr, ps_global)){
		  to_us = '+';
		  break;
	      }
	    
	    if(to_us != '+' && resent_to_us(&idata))
	      to_us = '+';

	    if(to_us == ' ' && F_ON(F_MARK_FOR_CC,ps_global))
	      for(addr = fetch_cc(&idata); addr; addr = addr->next)
		if(address_is_us(addr, ps_global)){
		    to_us = '-';
		    break;
		}
	}
    }

    return to_us;
}


/*
 * This sets or clears flags for the messages at this node and below in
 * a tree. It doesn't just blindly do it, perhaps it should. Instead,
 * when un-hiding a subtree it leaves the sub-subtree hidden if a node
 * is collapsed.
 *
 * Watch out when calling this. The thrd->branch is not part of thrd.
 * Branch is a sibling to thrd, not a child. Zero out branch before calling
 * or call on thrd->next and worry about thrd separately.
 * Ok to call it on top-level thread which has no branch already.
 */
void
set_thread_subtree(stream, thrd, msgmap, v, flags)
    MAILSTREAM *stream;
    PINETHRD_S *thrd;
    MSGNO_S    *msgmap;
    int         v;		/* set or clear? */
    int         flags;		/* flags to set or clear */
{
    int hiding;
    unsigned long rawno, msgno;
    PINETHRD_S *nthrd, *bthrd;

    hiding = (flags == MN_CHID) && v;

    if(!thrd || !stream || thrd->rawno < 1L || thrd->rawno > stream->nmsgs)
      return;

    msgno = mn_raw2m(msgmap, thrd->rawno);

    set_lflag(stream, msgmap, msgno, flags, v);

    if(thrd->next && (hiding || !get_lflag(stream,NULL,thrd->rawno,MN_COLL))){
	nthrd = fetch_thread(stream, thrd->next);
	if(nthrd)
	  set_thread_subtree(stream, nthrd, msgmap, v, flags);
    }

    if(thrd->branch){
	bthrd = fetch_thread(stream, thrd->branch);
	if(bthrd)
	  set_thread_subtree(stream, bthrd, msgmap, v, flags);
    }
}


/*
 * View a thread. Move from the thread index screen to a message index
 * screen for the current thread.
 *
 *      set_lflags - Set the local flags appropriately to start viewing
 *                   the thread. We would not want to set this if we are
 *                   already viewing the thread (and expunge or new mail
 *                   happened) and we want to preserve the collapsed state
 *                   of the subthreads.
 */
int
view_thread(state, stream, msgmap, set_lflags)
    struct pine *state;
    MAILSTREAM	*stream;
    MSGNO_S     *msgmap;
    int          set_lflags;
{
    PINETHRD_S   *thrd = NULL;
    unsigned long rawno, cur;

    if(!any_messages(msgmap, NULL, "to View"))
      return 0;

    if(!(stream && msgmap))
      return 0;

    rawno = mn_m2raw(msgmap, mn_get_cur(msgmap));
    if(rawno)
      thrd = fetch_thread(stream, rawno);

    if(thrd && thrd->top && thrd->top != thrd->rawno)
      thrd = fetch_thread(stream, thrd->top);
    
    if(!thrd)
      return 0;
    
    /*
     * Clear hidden and collapsed flag for this thread.
     * And set CHID2.
     * Don't have to worry about there being a branch because
     * this is a toplevel thread.
     */
    if(set_lflags){
	set_thread_lflags(stream, thrd, msgmap, MN_COLL | MN_CHID, 0);
	set_thread_lflags(stream, thrd, msgmap, MN_CHID2, 1);
    }

    if(current_index_state)
      msgmap->top_after_thrd = current_index_state->msg_at_top;

    /*
     * If this is one of those wacky users who like to sort backwards
     * they would probably prefer that the current message be the last
     * one in the thread (the one highest up the screen).
     */
    if(mn_get_revsort(msgmap)){
	cur = mn_get_cur(msgmap);
	while(cur > 1L && get_lflag(stream, msgmap, cur-1L, MN_CHID2))
	  cur--;

	if(cur != mn_get_cur(msgmap))
	  mn_set_cur(msgmap, cur);
    }

    /* first message in thread might be hidden if zoomed */
    if(any_lflagged(msgmap, MN_HIDE)){
	cur = mn_get_cur(msgmap);
	while(get_lflag(stream, msgmap, cur, MN_HIDE))
          cur++;
	
	if(cur != mn_get_cur(msgmap))
	  mn_set_cur(msgmap, cur);
    }

    msgmap->top = mn_get_cur(msgmap);

    state->next_screen = mail_index_screen;
    state->viewing_a_thread = 1;

    state->mangled_screen = 1;
    setup_for_index_index_screen();

    return 1;
}


int
unview_thread(state, stream, msgmap)
    struct pine *state;
    MAILSTREAM	*stream;
    MSGNO_S     *msgmap;
{
    PINETHRD_S   *thrd = NULL, *topthrd = NULL;
    unsigned long rawno, i;

    if(!(stream && msgmap))
      return 0;

    rawno = mn_m2raw(msgmap, mn_get_cur(msgmap));
    if(rawno)
      thrd = fetch_thread(stream, rawno);
    
    if(thrd && thrd->top)
      topthrd = fetch_thread(stream, thrd->top);
    
    if(!topthrd)
      return 0;

    /* hide this thread */
    set_thread_lflags(stream, topthrd, msgmap, MN_CHID, 1);

    /* clear special CHID2 flags for this thread */
    set_thread_lflags(stream, topthrd, msgmap, MN_CHID2, 0);

    /* clear CHID for top-level message and set COLL */
    set_lflag(stream, msgmap, mn_raw2m(msgmap, topthrd->rawno), MN_CHID, 0);
    set_lflag(stream, msgmap, mn_raw2m(msgmap, topthrd->rawno), MN_COLL, 1);

    mn_set_cur(msgmap, mn_raw2m(msgmap, topthrd->rawno));
    state->next_screen = mail_index_screen;
    state->viewing_a_thread = 0;
    state->view_skipped_index = 0;
    state->mangled_screen = 1;
    setup_for_thread_index_screen();

    return 1;
}


PINETHRD_S *
find_thread_by_number(stream, msgmap, target, startthrd)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    long        target;
    PINETHRD_S *startthrd;
{
    PINETHRD_S *thrd = NULL;

    if(!(stream && msgmap))
      return(thrd);

    thrd = startthrd;
    
    if(!thrd || !(thrd->prevthd || thrd->nextthd))
      thrd = fetch_thread(stream, mn_m2raw(msgmap, mn_get_cur(msgmap)));

    if(thrd && !(thrd->prevthd || thrd->nextthd) && thrd->head)
      thrd = fetch_thread(stream, thrd->head);

    if(thrd){
	/* go forward from here */
	if(thrd->thrdno < target){
	    while(thrd){
		if(thrd->thrdno == target)
		  break;

		if(mn_get_revsort(msgmap) && thrd->prevthd)
		  thrd = fetch_thread(stream, thrd->prevthd);
		else if(!mn_get_revsort(msgmap) && thrd->nextthd)
		  thrd = fetch_thread(stream, thrd->nextthd);
		else
		  thrd = NULL;
	    }
	}
	/* back up from here */
	else if(thrd->thrdno > target
		&& (mn_get_revsort(msgmap)
		    || (thrd->thrdno - target) < (target - 1L))){
	    while(thrd){
		if(thrd->thrdno == target)
		  break;

		if(mn_get_revsort(msgmap) && thrd->nextthd)
		  thrd = fetch_thread(stream, thrd->nextthd);
		else if(!mn_get_revsort(msgmap) && thrd->prevthd)
		  thrd = fetch_thread(stream, thrd->prevthd);
		else
		  thrd = NULL;
	    }
	}
	/* go forward from head */
	else if(thrd->thrdno > target){
	    if(thrd->head){
		thrd = fetch_thread(stream, thrd->head);
		while(thrd){
		    if(thrd->thrdno == target)
		      break;

		    if(thrd->nextthd)
		      thrd = fetch_thread(stream, thrd->nextthd);
		    else
		      thrd = NULL;
		}
	    }
	}
    }

    return(thrd);
}


void
adjust_cur_to_visible(stream, msgmap)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
{
    long n, cur;
    int  dir;

    cur = mn_get_cur(msgmap);

    /* if current is hidden, adjust */
    if(msgline_hidden(stream, msgmap, cur, 0)){

	dir = mn_get_revsort(msgmap) ? -1 : 1;

	for(n = cur;
	    ((dir == 1 && n >= 1L) || (dir == -1 && n <= mn_get_total(msgmap)))
	    && msgline_hidden(stream, msgmap, n, 0);
	    n -= dir)
	  ;
	
	if((dir == 1 && n >= 1L) || (dir == -1 && n <= mn_get_total(msgmap)))
	  mn_reset_cur(msgmap, n);
	else{				/* no visible in that direction */
	    for(n = cur;
		((dir == 1 && n >= 1L)
		 || (dir == -1 && n <= mn_get_total(msgmap)))
		&& msgline_hidden(stream, msgmap, n, 0);
		n += dir)
	      ;

	    if((dir == -1 && n >= 1L)
	       || (dir == 1 && n <= mn_get_total(msgmap)))
	      mn_reset_cur(msgmap, n);
	    /* else trouble! */
	}
    }
}
