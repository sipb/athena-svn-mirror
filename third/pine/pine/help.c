#if !defined(lint) && !defined(DOS)
static char rcsid[] = "$Id: help.c,v 1.1.1.1 2001-02-19 07:05:18 ghudson Exp $";
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
   1989-2000 by the University of Washington.

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
     help.c
     Functions to support the pine help screens
 ====*/


#include "headers.h"


static struct key help_keys[] =
       {MAIN_MENU,
	{NULL,NULL,{MC_EXIT,1,{'e'}}, KS_EXITMODE},
	{NULL,NULL,{MC_EXIT,1,{'e'}}, KS_EXITMODE},
	{NULL,NULL,{MC_VIEW_HANDLE,3,{'v',ctrl('m'),ctrl('j')}},KS_NONE},
	{"^B","PrevLink",{MC_PREV_HANDLE,1,{ctrl('B')}},KS_NONE},
	{"^F","NextLink",{MC_NEXT_HANDLE,1,{ctrl('F')}},KS_NONE},
	PREVPAGE_MENU,
	NEXTPAGE_MENU,
	PRYNTMSG_MENU,
	{"Z","Print All",{MC_PRINTALL,1,{'z'}},KS_NONE},
	NULL_MENU,
	WHEREIS_MENU};
INST_KEY_MENU(help_keymenu, help_keys);
#define	HLP_MAIN_KEY	0
#define	HLP_SUBEXIT_KEY	1
#define	HLP_EXIT_KEY	2
#define	HLP_VIEW_HANDLE	3
#define	HLP_PREV_HANDLE	4
#define	HLP_NEXT_HANDLE	5
#define	HLP_ALL_KEY	9


/*
 * Keys for the Report Bug screen.
 */
static struct key gripe_modal_keys[] = 
       {NULL_MENU,
	NULL_MENU,
	{"Ret","Finished",{MC_EXIT,2,{ctrl('m'),ctrl('j')}},KS_NONE},
	NULL_MENU,
	NULL_MENU,
	NULL_MENU,
	PREVPAGE_MENU,
	NEXTPAGE_MENU,
	NULL_MENU,
	NULL_MENU,
	NULL_MENU,
	NULL_MENU
       };
INST_KEY_MENU(gripe_modal_km, gripe_modal_keys);


typedef struct _help_scroll {
    unsigned	keys_formatted:1;	/* Has full keymenu been formatted? */
    char      **help_source;		/* Source of displayed help text */
} HELP_SCROLL_S;


static struct {
    unsigned   crlf:1;
    char     **line,
	      *offset;
} g_h_text;


typedef struct _help_print_state {
    int	  page;
    char *title;
    int   title_len;
} HPRT_S;

static HPRT_S *g_hprt;


static char att_cur_msg[] = "\
         Reporting a bug...\n\
\n\
  If you think that the \"current\" message may be related to the bug you\n\
  are reporting you may include it as an attachment.  If you want to\n\
  include a message but you aren't sure if it is the current message,\n\
  cancel this bug report, go to the folder index, place the cursor on\n\
  the message you wish to include, then return to the main menu and run\n\
  the bug report command again.  Answer \"Y\" when asked the question\n\
  \"Attach current message to report?\"\n\
\n\
  This bug report will also automatically include your pine\n\
  configuration file, which is helpful when investigating the problem.";


/* Helpful def's */
#define	GRIPE_OPT_CONF	0x01
#define	GRIPE_OPT_MSG	0x02
#define	GRIPE_OPT_LOCAL	0x04
#define	GRIPE_OPT_KEYS	0x08



/*
 * Internal prototypes
 */
int	 helper_internal PROTO((HelpType, char *, char *, int));
HelpType help_name2section PROTO((char *, int));
int	 help_processor PROTO((int, MSGNO_S *, SCROLL_S *));
void	 help_keymenu_tweek PROTO((SCROLL_S *, int));
int	 help_bogus_input PROTO((int));
void     print_all_help PROTO((void));
void	 print_help_page_title PROTO((char *, HPRT_S *));
int	 print_help_page_break PROTO((long, char *, LT_INS_S **, void *));
int	 gripe_bogus_input PROTO((int));
ADDRESS *gripe_token_addr PROTO((char *));
int	 gripe_newbody PROTO((struct pine *, BODY **, long, int));
char	*gripe_id PROTO((char *));
void	 att_cur_drawer PROTO((void));
#ifdef	_WINDOWS
int	 help_popup PROTO((SCROLL_S *, int));
int	 help_subsection_popup PROTO((SCROLL_S *, int));
#endif



/*----------------------------------------------------------------------
     Get the help text in the proper format and call scroller

    Args: text   -- The help text to display (from pine.help --> helptext.c)
          title  -- The title of the help text 
  
  Result: format text and call scroller

  The pages array contains the line number of the start of the pages in
the text. Page 1 is in the 0th element of the array.
The list is ended with a page line number of -1. Line number 0 is also
the first line in the text.
  -----*/
int
helper_internal(text, frag, title, flags)
    HelpType  text;
    char     *frag;
    char     *title;
    int	      flags;
{
    char	  **shown_text;
    int		    cmd = MC_NONE;
    long	    offset = 0L;
    char	   *error = NULL, tmp_title[MAX_SCREEN_COLS + 1];
    STORE_S	   *store;
    HANDLE_S	   *handles = NULL, *htmp;
    HELP_SCROLL_S   hscroll;
    gf_io_t	    pc;
#ifdef	HELPFILE
    char	  **dynamic_text;
#endif /* HELPFILE */

    dprint(1, (debugfile, "\n\n    ---- HELPER ----\n"));

#ifdef	HELPFILE
    if((shown_text = dynamic_text = get_help_text(text)) == NULL)
      return(cmd);
#else
    shown_text = text;
#endif /* HELPFILE */

    if(F_ON(F_BLANK_KEYMENU,ps_global)){
	FOOTER_ROWS(ps_global) = 3;
	clearfooter(ps_global);
	ps_global->mangled_screen = 1;
    }

    if(flags & HLPD_NEWWIN){
 	fix_windsize(ps_global);
	init_sigwinch();
    }

    /*
     * At this point, shown_text is a charstarstar with html
     * Turn it into a charstar with digested html
     */
    do{
	init_helper_getc(shown_text);
	init_handles(&handles);

	memset(&hscroll, 0, sizeof(HELP_SCROLL_S));
	hscroll.help_source = shown_text;
	if(store = so_get(CharStar, NULL, EDIT_ACCESS)){
	    gf_set_so_writec(&pc, store);
	    gf_filter_init();

	    if(!struncmp(shown_text[0], "<html>", 6))
	      gf_link_filter(gf_html2plain,
			     gf_html2plain_opt("x-pine-help:",
					ps_global->ttyo->screen_cols,
					GFHP_HANDLES | GFHP_LOCAL_HANDLES));
	    else
	      gf_link_filter(gf_wrap, gf_wrap_filter_opt(
						  ps_global->ttyo->screen_cols,
						  ps_global->ttyo->screen_cols,
						  0, GFW_HANDLES));

	    error = gf_pipe(helper_getc, pc);

	    gf_clear_so_writec(store);

	    if(!error){
		SCROLL_S	sargs;
		struct key_menu km;
		struct key	keys[24];

		for(htmp = handles; htmp; htmp = htmp->next)
		  if(htmp->type == URL
		     && htmp->h.url.path
		     && (htmp->h.url.path[0] == 'x'
			 || htmp->h.url.path[0] == '#'))
		    htmp->force_display = 1;

		/* This is mostly here to get the curses variables
		 * for line and column in sync with where the
		 * cursor is on the screen. This gets warped when
		 * the composer is called because it does it's own
		 * stuff
		 */
		ClearScreen();

		memset(&sargs, 0, sizeof(SCROLL_S));
		sargs.text.text	   = so_text(store);
		sargs.text.src	   = CharStar;
		sargs.text.desc	   = "help text";
		if(sargs.text.handles = handles)
		  while(sargs.text.handles->type == URL
			&& !sargs.text.handles->h.url.path
			&& sargs.text.handles->next)
		    sargs.text.handles = sargs.text.handles->next;

		if(!(sargs.bar.title = title)){
		    if(!struncmp(shown_text[0], "<html>", 6)){
			char *p;
			int   i;

			/* if we're looking at html, look for a <title>
			 * in the <head>... */
			for(i = 1;
			    shown_text[i]
			      && struncmp(shown_text[i], "</head>", 7);
			    i++)
			  if(!struncmp(shown_text[i], "<title>", 7)){
			      strcpy(tmp_20k_buf, &shown_text[i][7]);
			      if(p = strchr(tmp_20k_buf, '<'))
				*p = '\0';

			      sprintf(sargs.bar.title = tmp_title,
				      "HELP -- %.*s",
				      ps_global->ttyo->screen_cols-10,
				      strsquish(tmp_20k_buf +  500,
					tmp_20k_buf,
					ps_global->ttyo->screen_cols / 3));
			      break;
			  }
		    }

		    if(!sargs.bar.title)
		      sargs.bar.title = "HELP TEXT";
		}

		sargs.bar.style	   = TextPercent;
		sargs.proc.tool	   = help_processor;
		sargs.proc.data.p  = (void *) &hscroll;
		sargs.resize_exit  = 1;
		sargs.help.text	   = h_special_help_nav;
		sargs.help.title   = "HELP FOR HELP TEXT";
		sargs.keys.menu	   = &km;
		km		   = help_keymenu;
		km.keys		   = keys;
		memcpy(&keys[0], help_keymenu.keys,
		       (help_keymenu.how_many * 12) * sizeof(struct key));
		setbitmap(sargs.keys.bitmap);
		if(flags & HLPD_FROMHELP){
		    km.keys[HLP_EXIT_KEY].name	     = "P";
		    km.keys[HLP_EXIT_KEY].label	     = "Prev Help";
		    km.keys[HLP_EXIT_KEY].bind.cmd   = MC_FINISH;
		    km.keys[HLP_EXIT_KEY].bind.ch[0] = 'p';

		    km.keys[HLP_SUBEXIT_KEY].name	= "E";
		    km.keys[HLP_SUBEXIT_KEY].label	= "Exit Help";
		    km.keys[HLP_SUBEXIT_KEY].bind.cmd   = MC_EXIT;
		    km.keys[HLP_SUBEXIT_KEY].bind.ch[0] = 'e';
		}
		else if(text == h_special_help_nav){
		    km.keys[HLP_EXIT_KEY].name	     = "P";
		    km.keys[HLP_EXIT_KEY].label	     = "Prev Help";
		    km.keys[HLP_EXIT_KEY].bind.cmd   = MC_FINISH;
		    km.keys[HLP_EXIT_KEY].bind.ch[0] = 'p';

		    clrbitn(HLP_MAIN_KEY, sargs.keys.bitmap);
		    clrbitn(HLP_SUBEXIT_KEY, sargs.keys.bitmap);
		}
		else{
		    km.keys[HLP_EXIT_KEY].name	     = "E";
		    km.keys[HLP_EXIT_KEY].label	     = "Exit Help";
		    km.keys[HLP_EXIT_KEY].bind.cmd   = MC_EXIT;
		    km.keys[HLP_EXIT_KEY].bind.ch[0] = 'e';

		    km.keys[HLP_SUBEXIT_KEY].name	= "?";
		    km.keys[HLP_SUBEXIT_KEY].label	= "Help Help";
		    km.keys[HLP_SUBEXIT_KEY].bind.cmd   = MC_HELP;
		    km.keys[HLP_SUBEXIT_KEY].bind.ch[0] = '?';
		}

		if(flags & HLPD_SIMPLE){
		    clrbitn(HLP_MAIN_KEY, sargs.keys.bitmap);
		}
		else
		  sargs.bogus_input = help_bogus_input;

		if(handles){
		    sargs.keys.each_cmd = help_keymenu_tweek;
		    hscroll.keys_formatted = 0;
		}
		else{
		    clrbitn(HLP_VIEW_HANDLE, sargs.keys.bitmap);
		    clrbitn(HLP_PREV_HANDLE, sargs.keys.bitmap);
		    clrbitn(HLP_NEXT_HANDLE, sargs.keys.bitmap);
		}

		if(text != main_menu_tx) /* only main can "print all" */
		  clrbitn(HLP_ALL_KEY, sargs.keys.bitmap);

		if(frag){
		    sargs.start.on	 = Fragment;
		    sargs.start.loc.frag = frag;
		    frag		 = NULL; /* ignore next time */
		}
		else if(offset){
		    sargs.start.on	   = Offset;
		    sargs.start.loc.offset = offset;
		}
		else
		  sargs.start.on = FirstPage;

#ifdef	_WINDOWS
		sargs.mouse.popup = (flags & HLPD_FROMHELP)
				      ? help_subsection_popup : help_popup;
#endif

		cmd = scrolltool(&sargs);

		offset = sargs.start.loc.offset;

		if(F_ON(F_BLANK_KEYMENU,ps_global))
		  FOOTER_ROWS(ps_global) = 1;

		ClearScreen();
	    }

	    so_give(&store);
	}

	free_handles(&handles);
    }
    while(cmd == MC_RESIZE);

#ifdef	HELPFILE
    free_list_array(&dynamic_text);
#endif

    return(cmd);
}


/*
 * helper -- compatibility function around newer helper_internal
 */
int
helper(text, title, flags)
    HelpType  text;
    char     *title;
    int	      flags;
{
    return(helper_internal(text, NULL, title, flags));
}



void
init_helper_getc(help_txt)
    char **help_txt;
{
    g_h_text.crlf   = 0;
    g_h_text.line   = help_txt;
    g_h_text.offset = *g_h_text.line;
}



int
helper_getc(c)
    char *c;
{
    if(g_h_text.crlf){
	*c = '\012';
	g_h_text.crlf = 0;
	return(1);
    }
    else if(g_h_text.offset && *g_h_text.line){
	if(!(*c = *g_h_text.offset++)){
	    g_h_text.offset = *++g_h_text.line;
	    *c = '\015';
	    g_h_text.crlf = 1;
	}

	return(1);
    }

    return(0);
}



int
help_processor(cmd, msgmap, sparms)
    int	      cmd;
    MSGNO_S  *msgmap;
    SCROLL_S *sparms;
{
    int rv = 0;
    char message[64];

    switch(cmd){
	/*----------- Print all the help ------------*/
      case MC_PRINTALL :
	print_all_help();
	break;

      case MC_PRINTMSG :
	sprintf(message, "%.60s ", STYLE_NAME(sparms));
	if(open_printer(message) == 0){
	    print_help(((HELP_SCROLL_S *)sparms->proc.data.p)->help_source);
	    close_printer();
	}

	break;

      case MC_FINISH :
	rv = 1;
	break;

      default :
	panic("Unhandled case");
    }

    return(rv);
}


void
help_keymenu_tweek(sparms, handle_hidden)
    SCROLL_S *sparms;
    int	      handle_hidden;
{
    if(handle_hidden){
	sparms->keys.menu->keys[HLP_VIEW_HANDLE].name  = "";
	sparms->keys.menu->keys[HLP_VIEW_HANDLE].label = "";
    }
    else{
	if(!((HELP_SCROLL_S *)sparms->proc.data.p)->keys_formatted){
	    /* If label's always been blank, force reformatting */
	    mark_keymenu_dirty();
	    sparms->keys.menu->width = 0;
	    ((HELP_SCROLL_S *)sparms->proc.data.p)->keys_formatted = 1;
	}

	sparms->keys.menu->keys[HLP_VIEW_HANDLE].name  = "V";
	sparms->keys.menu->keys[HLP_VIEW_HANDLE].label = "[View Link]";
    }
}


/*
 * print_help - send the raw array of lines to printer
 */
void
print_help(text)
    char **text;
{
    char   *error, buf[256];
    HPRT_S  help_data;

    init_helper_getc(text);

    memset(g_hprt = &help_data, 0, sizeof(HPRT_S));

    help_data.page = 1;
    
    gf_filter_init();

    if(!struncmp(text[0], "<html>", 6)){
	int   i;
	char *p;

	gf_link_filter(gf_html2plain,gf_html2plain_opt(NULL,80,GFHP_STRIPPED));
	for(i = 1; i <= 5 && text[i]; i++)
	  if(!struncmp(text[i], "<title>", 7)
	     && (p = srchstr(text[i] + 7, "</title>"))
	     && p - text[i] > 7){
	      help_data.title	  = text[i] + 7;
	      help_data.title_len = p - help_data.title;
	      break;
	  }
    }
    else
      gf_link_filter(gf_wrap, gf_wrap_filter_opt(80, 80, 0, 0));

    gf_link_filter(gf_line_test,
		   gf_line_test_opt(print_help_page_break, NULL));
    gf_link_filter(gf_nvtnl_local, NULL);

    print_help_page_title(buf, &help_data);
    print_text(buf);
    print_text(NEWLINE);		/* terminate it */
    print_text(NEWLINE);		/* and write two blank links */
    print_text(NEWLINE);

    if(error = gf_pipe(helper_getc, print_char))
      q_status_message1(SM_ORDER | SM_DING, 3, 3, "Printing Error: %s", error);

    print_char(ctrl('L'));		/* new page. */
}



void
print_all_help()
{
#ifdef	HELPFILE
    short t;
#else
    struct _help_texts *t;
#endif
    char **h;

    if(open_printer("all 150+ pages of help text ") == 0) {
#ifndef	DOS
	intr_handling_on();
#endif
#ifdef	HELPFILE
        for(t = 0; t < LASTHELP; t++) {
	    if(!(h = get_help_text(t)))
	      return;
#else
	for(t = h_texts; (h = t->help_text) != NO_HELP; t++) {
#endif
	    if(ps_global->intr_pending){
		q_status_message(SM_ORDER, 3, 3,
				 "Print of all help cancelled");
		break;
	    }

	    print_help(h);

#ifdef	HELPFILE
	    free_list_array(&h);
#endif
        }

#ifndef	DOS
	intr_handling_off();
#endif
        close_printer();
    }
}


/*
 * print_help_page_title -- 
 */
void
print_help_page_title(buf, hprt)
    char    *buf;
    HPRT_S  *hprt;
{
    sprintf(buf, "  Pine Help%s%.*s%*s%d",
	    hprt->title_len ? ": " : " Text",
	    min(55, hprt->title_len), hprt->title_len ? hprt->title : "",
	    59 - (hprt->title_len ? min(55, hprt->title_len) : 5),
	    "Page ", hprt->page);
}


/*
 * print_help_page_break -- insert page breaks and such for printed
 *			    help text
 */
int
print_help_page_break(linenum, line, ins, local)
    long       linenum;
    char      *line;
    LT_INS_S **ins;
    void      *local;
{
    char buf[256];

    if(((linenum + (g_hprt->page * 3)) % 62) == 0){
	g_hprt->page++;			/* start on new page */
	buf[0] = ctrl('L');
	print_help_page_title(buf + 1, g_hprt);
	strncat(buf, "\015\012\015\012\015\012", sizeof(buf)-strlen(buf));
	ins = gf_line_test_new_ins(ins, line, buf, strlen(buf));
    }

    return(0);
}


/*
 * help_bogus_input - used by scrolltool to complain about
 *		      invalid user input.
 */
int
help_bogus_input(ch)
    int ch;		
{
    bogus_command(ch, NULL);
    return(0);
}


int
url_local_helper(url)
    char *url;
{
    if(!struncmp(url, "x-pine-help:", 12) && *(url += 12)){
	char		   *frag;
	HelpType	    newhelp;

	/* internal fragment reference? */
	if(frag = strchr(url, '#')){
	    size_t len;

	    if(len = frag - url){
		newhelp = help_name2section(url, len);
	    }
	    else{
		url_local_fragment(url);
		return(1);
	    }
	}
	else
	  newhelp = help_name2section(url, strlen(url));


	if(newhelp != NO_HELP){
	    int rv;

	    rv = helper_internal(newhelp, frag, "HELP SUB-SECTION",
				 HLPD_NEWWIN | HLPD_SIMPLE | HLPD_FROMHELP);
	    ps_global->mangled_screen = 1;
	    return((rv == MC_EXIT) ? 2 : 1);
	}
    }

    q_status_message1(SM_ORDER | SM_DING, 0, 3,
		      "Unrecognized Internal help: \"%s\"", url);
    return(0);
}


HelpType
help_name2section(url, url_len)
    char *url;
    int   url_len;
{
    char		name[256];
    HelpType		newhelp = NO_HELP;
#ifdef	HELPFILE
    HelpType		t;
#else
    struct _help_texts *t;
#endif

    sprintf(name, "%.*s", min(url_len,sizeof(name)), url);

#ifdef	HELPFILE
    {
	int		  i;
	char	  buf[MAILTMPLEN];
	FILE	 *helpfile;
	struct hindx  hindex_record;

	/*
	 * Make sure index file is available,
	 * and read appropriate record
	 */
	build_path(buf, ps_global->pine_dir, HELPINDEX, sizeof(buf));
	if(helpfile = fopen(buf, "rb")){
	    for(t = 0; t < LASTHELP; t++){
		i = fseek(helpfile, t * sizeof(struct hindx), SEEK_SET) == 0
		     && fread((void *) &hindex_record,
			      sizeof(struct hindx), 1 ,helpfile) == 1;

		if(!i){	/* problem in fseek or read */
		    q_status_message(SM_ORDER | SM_DING, 3, 5,
		   "No Help!  Can't locate proper offset for help text file.");
		    break;
		}

		if(!strucmp(hindex_record.key, name)){
		    newhelp = t;
		    break;
		}
	    }

	    fclose(helpfile);
	}
	else
	  q_status_message1(SM_ORDER | SM_DING, 3, 5,
			    "No Help!  Index \"%s\" not found.", buf);

    }
#else
    for(t = h_texts; t->help_text != NO_HELP; t++)
      if(!strucmp(t->tag, name)){
	  newhelp = t->help_text;
	  break;
      }
#endif

    return(newhelp);
}



#ifdef HELPFILE
/*
 * get_help_text - return the help text associated with index
 *                 in an array of pointers to each line of text.
 */
char **
get_help_text(index)
    HelpType index;
{
    int  i, len;
    char buf[1024], **htext, *tmp;
    struct hindx hindex_record;
    FILE *helpfile;

    if(index < 0 || index >= LASTHELP)
	return(NULL);

    /* make sure index file is available, and read appropriate record */
    build_path(buf, ps_global->pine_dir, HELPINDEX, sizeof(buf));
    if(!(helpfile = fopen(buf, "rb"))){
	q_status_message1(SM_ORDER,3,5,
	    "No Help!  Index \"%s\" not found.", buf);
	return(NULL);
    }

    i = fseek(helpfile, index * sizeof(struct hindx), SEEK_SET) == 0
	 && fread((void *)&hindex_record,sizeof(struct hindx),1,helpfile) == 1;

    fclose(helpfile);
    if(!i){	/* problem in fseek or read */
        q_status_message(SM_ORDER, 3, 5,
		  "No Help!  Can't locate proper offset for help text file.");
	return(NULL);
    }

    /* make sure help file is open */
    build_path(buf, ps_global->pine_dir, HELPFILE, sizeof(buf));
    if((helpfile = fopen(buf, "rb")) == NULL){
	q_status_message2(SM_ORDER,3,5,"No Help!  \"%s\" : %s", buf,
			  error_description(errno));
	return(NULL);
    }

    if(fseek(helpfile, hindex_record.offset, SEEK_SET) != 0
       || fgets(buf, sizeof(buf) - 1, helpfile) == NULL
       || strncmp(hindex_record.key, buf, strlen(hindex_record.key))){
	/* problem in fseek, or don't see key */
        q_status_message(SM_ORDER, 3, 5,
		     "No Help!  Can't locate proper entry in help text file.");
	fclose(helpfile);
	return(NULL);
    }

    htext = (char **)fs_get(sizeof(char *) * (hindex_record.lines + 1));

    for(i = 0; i < hindex_record.lines; i++){
	if(fgets(buf, sizeof(buf) - 1, helpfile) == NULL){
	    htext[i] = NULL;
	    free_list_array(&htext);
	    fclose(helpfile);
            q_status_message(SM_ORDER,3,5,"No Help!  Entry not in help text.");
	    return(NULL);
	}

	if(*buf){
	    if((len = strlen(buf)) > 1
	       && (buf[len-2] == '\n' || buf[len-2] == '\r'))
	      buf[len-2] = '\0';
	    else if(buf[len-1] == '\n' || buf[len-1] == '\r')
	      buf[len-1] = '\0';

	    htext[i] = cpystr(buf);
	}
	else
	  htext[i] = cpystr("");
    }

    htext[i] = NULL;

    fclose(helpfile);
    return(htext);
}
#endif	/* HELPFILE */




#if defined(DOS) && !defined(_WINDOWS)
#define NSTATUS 25  /* how many status messages to save for review */
#else
#define NSTATUS 100
#endif

static char *stat_msgs[NSTATUS];
static int   latest;

/*----------------------------------------------------------------------
     Review last N status messages

    Args: title  -- The title of the screen
  -----*/
void
review_messages(title)
    char  *title;
{
#define INDENT 2
    int             how_many_lines, how_many_bytes = 0, width, resetwidth;
    char           *tmp_text, *cur;
    char           *p, *q, *last_space, *cp;
    char            save, buf[MAX_SCREEN_COLS + 1];
    register char **e;
    register int    i, j, k, m, line_len;
    SCROLL_S	    sargs;

    e = stat_msgs;
    width = resetwidth = max(20, ps_global->ttyo->screen_cols);

    /* conservative estimate of space we'll use */
    for(i = 0; i < NSTATUS; i++)
      if(e[i] && *e[i]){
	  line_len = strlen(e[i]);
	  how_many_lines = line_len/(width - INDENT) + 1;
	  how_many_bytes += (line_len + how_many_lines +
					      (how_many_lines-1) * INDENT);
      }

    cur = tmp_text = (char *)fs_get((how_many_bytes + 1) * sizeof(char *));
    *cur = '\0';

    /* allocate strings */
    for(k = 0, j = 0, i = latest; k < NSTATUS; k++){
	i = (i + 1) % NSTATUS;
	if(e[i] && *e[i]){
	    p = q = e[i];
	    last_space = NULL;
	    line_len   = 0;
	    width      = resetwidth;
	    while(*p){
		if(*p == TAB){
		    last_space = p++;
		    while(line_len < width
			  && ((++line_len)&0x07) != 0)
		      ;
		}
		else if(*p == SPACE){
		    last_space = p++;
		    line_len++;
		}
		else{
		    p++;
		    line_len++;
		}

		if(line_len > width){
		    if(width != resetwidth){
			for(m = 0; m < INDENT; m++)
			  buf[m] = SPACE;
			
			cp = buf + INDENT;
		    }
		    else
		      cp = buf;

		    if(last_space){
			save = *last_space;
			*last_space = '\0';
			strncpy(cp, q,
				(width != resetwidth) ? sizeof(buf)-INDENT
						      : sizeof(buf));
			buf[sizeof(buf)-1] = '\0';
			sstrcpy(&cur, buf);
			sstrcpy(&cur, "\n");
			*last_space = save;
			q = last_space + 1;
		    }
		    else{
			save = q[width];
			q[width] = '\0';
			strncpy(cp, q,
				(width != resetwidth) ? sizeof(buf)-INDENT
						      : sizeof(buf));
			buf[sizeof(buf)-1] = '\0';
			sstrcpy(&cur, buf);
			sstrcpy(&cur, "\n");
			q[width] = save;
			q = q + width;
		    }

		    p = q;
		    line_len = 0;
		    last_space = NULL;
		    width = resetwidth - 2;
		}
	    }

	    if(*q){
		if(width != resetwidth){
		    for(m = 0; m < INDENT; m++)
		      buf[m] = SPACE;
		    
		    cp = buf + INDENT;
		}
		else
		  cp = buf;

		strncpy(cp, q,
			(width != resetwidth) ? sizeof(buf)-INDENT
					      : sizeof(buf));
		buf[sizeof(buf)-1] = '\0';
		sstrcpy(&cur, buf);
		sstrcpy(&cur, "\n");
	    }
	}
    }

    *cur = '\0';

    memset(&sargs, 0, sizeof(SCROLL_S));
    sargs.text.text = tmp_text;
    sargs.text.src  = CharStar;
    sargs.text.desc = "journal";
    sargs.bar.title = title;
    sargs.start.on  = LastPage;

    scrolltool(&sargs);

    fs_give((void **)&tmp_text);
}


/*----------------------------------------------------------------------
     Add a message to the circular status message review buffer

    Args: message  -- The message to add
  -----*/
void
add_review_message(message)
    char *message;
{
    if(!(message && *message))
      return;

    latest = (latest + 1) % NSTATUS;
    if(stat_msgs[latest] && strlen(stat_msgs[latest]) >= strlen(message))
      strcpy(stat_msgs[latest], message);  /* already enough space */
    else{
	if(stat_msgs[latest])
	  fs_give((void **)&stat_msgs[latest]);

	stat_msgs[latest] = cpystr(message);
    }
}



/*----------------------------------------------------------------------
    Free resources associated with the status message review list

    Args: 
  -----*/
void
end_status_review()
{
    for(latest = NSTATUS - 1; latest >= 0; latest--)
      if(stat_msgs[latest])
	fs_give((void **)&stat_msgs[latest]);
}



/*
 *  * * * * * * * *    Bug Report support routines    * * * * * * * *
 */



/*
 * standard type of storage object used for body parts...
 */
#ifdef	DOS
#define		  PART_SO_TYPE	TmpFileStar
#else
#define		  PART_SO_TYPE	CharStar
#endif


int
gripe_gripe_to(url)
    char *url;
{
    char      *composer_title, *url_copy, *optstr, *p;
    int	       opts = 0;
    BODY      *body = NULL;
    ENVELOPE  *outgoing = NULL;
    REPLY_S    fake_reply;
    PINEFIELD *pf = NULL;
    long       msgno = mn_m2raw(ps_global->msgmap, 
				mn_get_cur(ps_global->msgmap));

    url_copy = cpystr(url + 13);
    if(optstr = strchr(url_copy, '?'))
      *optstr++ = '\0';

    outgoing		 = mail_newenvelope();
    outgoing->message_id = generate_message_id();

    if(outgoing->to = gripe_token_addr(url_copy)){
	composer_title = "COMPOSE TO LOCAL SUPPORT";
	dprint(1, (debugfile, 
		   "\n\n   -- Send to local support(%s@%s) --\n",
		   outgoing->to->mailbox ? outgoing->to->mailbox : "NULL",
		   outgoing->to->host ? outgoing->to->host : "NULL"));
    }
    else{			/* must be global */
	composer_title = "REQUEST FOR ASSISTANCE";
	rfc822_parse_adrlist(&outgoing->to, url_copy, ps_global->maildomain);
    }

    /*
     * Sniff thru options
     */
    while(optstr){
	if(p = strchr(optstr, '?'))	/* tie off list item */
	  *p++ = '\0';

	if(!strucmp(optstr, "config"))
	  opts |= GRIPE_OPT_CONF;
	else if(!strucmp(optstr, "curmsg"))
	  opts |= GRIPE_OPT_MSG;
	else if(!strucmp(optstr, "local"))
	  opts |= GRIPE_OPT_LOCAL;
	else if(!strucmp(optstr, "keys"))
	  opts |= GRIPE_OPT_KEYS;

	optstr = p;
    }

    /* build body and hand off to composer... */
    if(gripe_newbody(ps_global, &body, msgno, opts) == 0){
	pf = (PINEFIELD *) fs_get(sizeof(PINEFIELD));
	memset(pf, 0, sizeof(PINEFIELD));
	pf->name		   = cpystr("X-Generated-Via");
	pf->type		   = FreeText;
	pf->textbuf		   = gripe_id("Pine Bug Report screen");
	memset((void *)&fake_reply, 0, sizeof(fake_reply));
	fake_reply.flags	   = REPLY_PSEUDO;
	fake_reply.data.pico_flags = P_HEADEND;
	pine_send(outgoing, &body, composer_title, NULL, NULL,
		  &fake_reply, NULL, NULL, pf, 0);
    }
    
    ps_global->mangled_screen = 1;
    mail_free_envelope(&outgoing);

    if(body)
      pine_free_body(&body);

    fs_give((void **) &url_copy);
    
    return(10);
}


int
gripe_newbody(ps, body, msgno, flags)
    struct pine *ps;
    BODY       **body;
    long         msgno;
    int          flags;
{
    BODY        *pb;
    PART       **pp;
    STORE_S	*store;
    gf_io_t      pc;
    static char *err = "Problem creating space for message text.";
    int          i;
    char         tmp[MAILTMPLEN], *p;

    if(store = so_get(PicoText, NULL, EDIT_ACCESS)){
	*body = mail_newbody();

	if((p = detoken(NULL, NULL, 2, 0, 1, NULL, NULL)) != NULL){
	    if(*p)
	      so_puts(store, p);

	    fs_give((void **) &p);
	}
    }
    else{
	q_status_message(SM_ORDER | SM_DING, 3, 4, err);
	return(-1);
    }

    if(flags){
	/*---- Might have multiple parts ----*/
	(*body)->type			= TYPEMULTIPART;
	/*---- The TEXT part/body ----*/
	(*body)->nested.part            = mail_newbody_part();
	(*body)->nested.part->body.type = TYPETEXT;
	(*body)->nested.part->body.contents.text.data = (void *) store;

	/*---- create object, and write current config into it ----*/
	pp = &((*body)->nested.part->next);

	if(flags & GRIPE_OPT_CONF){
	    *pp			     = mail_newbody_part();
	    pb			     = &((*pp)->body);
	    pp			     = &((*pp)->next);
	    pb->type		     = TYPETEXT;
	    pb->id		     = generate_message_id();
	    pb->description	     = cpystr("Pine Configuration Data");
	    pb->parameter	     = mail_newbody_parameter();
	    pb->parameter->attribute = cpystr("name");
	    pb->parameter->value     = cpystr("config.txt");

	    if(store = so_get(CharStar, NULL, EDIT_ACCESS)){
		extern char datestamp[], hoststamp[];

		pb->contents.text.data = (void *) store;
		gf_set_so_writec(&pc, store);
		gf_puts("Pine built ", pc);
		gf_puts(datestamp, pc);
		gf_puts(" on host: ", pc);
		gf_puts(hoststamp, pc);
		gf_puts("\n", pc);

		dump_pine_struct(ps, pc);
		dump_config(ps, pc, 0);

		pb->size.bytes = strlen((char *) so_text(store));
		gf_clear_so_writec(store);
	    }
	    else{
		q_status_message(SM_ORDER | SM_DING, 3, 4, err);
		return(-1);
	    }
	}

	if(flags & GRIPE_OPT_KEYS){
	    *pp			     = mail_newbody_part();
	    pb			     = &((*pp)->body);
	    pp			     = &((*pp)->next);
	    pb->type		     = TYPETEXT;
	    pb->id		     = generate_message_id();
	    pb->description	     = cpystr("Recent User Input");
	    pb->parameter	      = mail_newbody_parameter();
	    pb->parameter->attribute  = cpystr("name");
	    pb->parameter->value      = cpystr("uinput.txt");

	    if(store = so_get(CharStar, NULL, EDIT_ACCESS)){
		pb->contents.text.data = (void *) store;

		so_puts(store, "User's most recent input:\n");

		/* dump last n keystrokes */
		so_puts(store, "========== Latest keystrokes ==========\n");
		while((i = key_playback(0)) != -1){
		    sprintf(tmp, "\t%.20s\t(0x%04.4x)\n", pretty_command(i), i);
		    so_puts(store, tmp);
		}

		pb->size.bytes = strlen((char *) so_text(store));
	    }
	    else{
		q_status_message(SM_ORDER | SM_DING, 3, 4, err);
		return(-1);
	    }
	}

	/* check for local debugging info? */
	if((flags & GRIPE_OPT_LOCAL)
	   && ps_global->VAR_BUGS_EXTRAS 
	   && can_access(ps_global->VAR_BUGS_EXTRAS, EXECUTE_ACCESS) == 0){
	    char *error		      = NULL;

	    *pp			      = mail_newbody_part();
	    pb			      = &((*pp)->body);
	    pp			      = &((*pp)->next);
	    pb->type		      = TYPETEXT;
	    pb->id		      = generate_message_id();
	    pb->description	      = cpystr("Local Configuration Data");
	    pb->parameter	      = mail_newbody_parameter();
	    pb->parameter->attribute  = cpystr("name");
	    pb->parameter->value      = cpystr("lconfig.txt");

	    if(store = so_get(CharStar, NULL, EDIT_ACCESS)){
		PIPE_S  *syspipe;		
		gf_io_t  gc;
		
		pb->contents.text.data = (void *) store;
		gf_set_so_writec(&pc, store);
		if(syspipe = open_system_pipe(ps_global->VAR_BUGS_EXTRAS,
					 NULL, NULL,
					 PIPE_READ | PIPE_STDERR | PIPE_USER,
					 0)){
		    gf_set_readc(&gc, (void *)syspipe->in.f, 0, FileStar);
		    gf_filter_init();
		    error = gf_pipe(gc, pc);
		    (void) close_system_pipe(&syspipe);
		}
		else
		  error = "executing config collector";

		gf_clear_so_writec(store);
	    }
	    
	    if(error){
		q_status_message1(SM_ORDER | SM_DING, 3, 4, 
				  "Problem %s", error);
		return(-1);
	    }
	    else			/* fixup attachment's size */
	      pb->size.bytes = strlen((char *) so_text(store));
	}

	if((flags & GRIPE_OPT_MSG) && mn_get_total(ps->msgmap) > 0L){
	    int ch = 0;
	
	    ps->redrawer = att_cur_drawer;
	    att_cur_drawer();

	    if((ch = one_try_want_to("Attach current message to report",
				     'y','x',NO_HELP,
				     WT_FLUSH_IN|WT_SEQ_SENSITIVE)) == 'y'){
		*pp		      = mail_newbody_part();
		pb		      = &((*pp)->body);
		pp		      = &((*pp)->next);
		pb->type	      = TYPEMESSAGE;
		pb->id		      = generate_message_id();
		sprintf(tmp, "Problem Message (%ld of %ld)",
			mn_get_cur(ps->msgmap), mn_get_total(ps->msgmap));
		pb->description	      = cpystr(tmp);

		/*---- Package each message in a storage object ----*/
		if(store = so_get(PART_SO_TYPE, NULL, EDIT_ACCESS)){
		    pb->contents.text.data = (void *) store;
		}
		else{
		    q_status_message(SM_ORDER | SM_DING, 3, 4, err);
		    return(-1);
		}

		/* write the header */
		if((p = mail_fetchheader(ps->mail_stream, msgno)) && *p)
		  so_puts(store, p);
		else
		  return(-1);

#if	defined(DOS) && !defined(WIN32)
		/* write fetched text to disk */
		mail_parameters(ps->mail_stream, SET_GETS, (void *)dos_gets);
		append_file = (FILE *) so_text(store);

		/* HACK!  See mailview.c:format_message for details... */
		ps->mail_stream->text = NULL;
		/* write the body */
		if(!mail_fetchtext(ps->mail_stream, msgno))
		  return(-1);

		pb->size.bytes = ftell(append_file);
		/* next time body may stay in core */
		mail_parameters(ps->mail_stream, SET_GETS, (void *)NULL);
		append_file   = NULL;
		mail_gc(ps->mail_stream, GC_TEXTS);
		so_release(store);
#else
		pb->size.bytes = strlen(p);
		so_puts(store, "\015\012");

		if((p = mail_fetchtext(ps->mail_stream, msgno)) &&  *p)
		  so_puts(store, p);
		else
		  return(-1);

		pb->size.bytes += strlen(p);
#endif
	    }
	    else if(ch == 'x'){
		q_status_message(SM_ORDER, 0, 3, "Bug report cancelled.");
		return(-1);
	    }
	}
    }
    else{
	/*---- Only one part! ----*/
	(*body)->type = TYPETEXT;
	(*body)->contents.text.data = (void *) store;
    }

    return(0);
}


ADDRESS *
gripe_token_addr(token)
    char *token;
{
    int	     rv = 0;
    char    *p;
    ADDRESS *a = NULL;

    if(token && *token++ == '_'){
	if(!strcmp(token, "LOCAL_ADDRESS_")){
	    p = (ps_global->VAR_LOCAL_ADDRESS
		 && ps_global->VAR_LOCAL_ADDRESS[0])
		    ? ps_global->VAR_LOCAL_ADDRESS
		    : "postmaster";
	    a = rfc822_parse_mailbox(&p, ps_global->maildomain);
	    a->personal = cpystr((ps_global->VAR_LOCAL_FULLNAME 
				  && ps_global->VAR_LOCAL_FULLNAME[0])
				    ? ps_global->VAR_LOCAL_FULLNAME 
				    : "Place to report Pine Bugs");
	    rv = 1;
	}
	else if(!strcmp(token, "BUGS_ADDRESS_")){
	    p = (ps_global->VAR_BUGS_ADDRESS
		 && ps_global->VAR_BUGS_ADDRESS[0])
		    ? ps_global->VAR_BUGS_ADDRESS : "postmaster";
	    a = rfc822_parse_mailbox(&p, ps_global->maildomain);
	    a->personal = cpystr((ps_global->VAR_BUGS_FULLNAME 
				  && ps_global->VAR_BUGS_FULLNAME[0])
				    ? ps_global->VAR_BUGS_FULLNAME 
				    : "Place to report Pine Bugs");
	    rv = 1;
	}
    }

    return(a);
}


char *
gripe_id(key)
    char *key;
{
    int i;
    
    /*
     * Build our contribution to the subject; part constant string
     * and random 4 character alpha numeric string.
     */
    tmp_20k_buf[0] = '\0';
    sprintf(tmp_20k_buf, "%s (ID %c%c%d%c%c)", key,
	    ((i = (int)(random() % 36L)) < 10) ? '0' + i : 'A' + (i - 10),
	    ((i = (int)(random() % 36L)) < 10) ? '0' + i : 'A' + (i - 10),
	    (int)(random() % 10L),
	    ((i = (int)(random() % 36L)) < 10) ? '0' + i : 'A' + (i - 10),
	    ((i = (int)(random() % 36L)) < 10) ? '0' + i : 'A' + (i - 10));
    return(cpystr(tmp_20k_buf));
}


int
gripe_bogus_input(ch)
    int ch;
{
    return(1);			/* done! */
}



/*
 * Used by gripe_tool.
 */
void
att_cur_drawer()
{
    int	       i, dline, j;
    char       buf[256+1];

    /* blat helpful message to screen */
    ClearBody();
    j = 0;
    for(dline = 2;
	dline < ps_global->ttyo->screen_rows - FOOTER_ROWS(ps_global);
	dline++){
	for(i = 0; i < 256 && att_cur_msg[j] && att_cur_msg[j] != '\n'; i++)
	  buf[i] = att_cur_msg[j++];

	buf[i] = '\0';
	if(att_cur_msg[j])
	  j++;
	else if(!i)
	  break;

        PutLine0(dline, 1, buf);
    }
}


#ifdef	_WINDOWS
/*
 * 
 */
int
help_popup(sparms, in_handle)
    SCROLL_S *sparms;
    int	      in_handle;
{
    MPopup hp_menu[10];
    int	   i = -1;

    if(in_handle){
	hp_menu[++i].type	= tQueue;
	hp_menu[i].label.style	= lNormal;
	hp_menu[i].label.string = "View Help Section";
	hp_menu[i].data.val	= 'V';
    }

    hp_menu[++i].type	    = tQueue;
    hp_menu[i].label.style  = lNormal;
    hp_menu[i].label.string = "Exit Help";
    hp_menu[i].data.val	    = 'E';

    hp_menu[++i].type = tTail;

    mswin_popup(hp_menu);

    return(0);
}


/*
 * 
 */
int
help_subsection_popup(sparms, in_handle)
    SCROLL_S *sparms;
    int	      in_handle;
{
    MPopup hp_menu[10];
    int	   i = -1;

    if(in_handle){
	hp_menu[++i].type	= tQueue;
	hp_menu[i].label.style  = lNormal;
	hp_menu[i].label.string = "View Help Section";
	hp_menu[i].data.val	= 'V';
    }

    hp_menu[++i].type	    = tQueue;
    hp_menu[i].label.style  = lNormal;
    hp_menu[i].label.string = "Previous Help Section";
    hp_menu[i].data.val	    = 'P';

    hp_menu[++i].type	    = tQueue;
    hp_menu[i].label.style  = lNormal;
    hp_menu[i].label.string = "Exit Help";
    hp_menu[i].data.val	    = 'E';

    hp_menu[++i].type = tTail;

    if(mswin_popup(hp_menu) == (in_handle ? 1 : 0))
      /*(void) helper_internal()*/;

    return(0);
}
#endif
