#if !defined(lint) && !defined(DOS)
static char rcsid[] = "$Id: pine.c,v 1.1.1.3 2003-05-01 01:13:34 ghudson Exp $";
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

#include "headers.h"
#include "../c-client/imap4r1.h"


/*
 * Handy local definitions...
 */
#define	LEGAL_NOTICE \
   "Copyright 1989-2003.  PINE is a trademark of the University of Washington."

#define	PIPED_FD	5			/* Some innocuous desc	    */


/*
 * Globals referenced throughout pine...
 */
struct pine *ps_global;				/* THE global variable! */
char	    *pine_version = PINE_VERSION;	/* version string */


/*----------------------------------------------------------------------
  General use big buffer. It is used in the following places:
    compose_mail:    while parsing header of postponed message
    append_message2: while writing header into folder
    q_status_messageX: while doing printf formatting
    addr_book: Used to return expanded address in. (Can only use here 
               because mm_log doesn't q_status on PARSE errors !)
    pine.c: When address specified on command line
    init.c: When expanding variable values
    and many many more...

 ----*/
char         tmp_20k_buf[SIZEOF_20KBUF];


/*
 * byte count used by our gets routine to keep track 
 */
unsigned long gets_bytes;


/*
 * Internal prototypes
 */
int	setup_menu PROTO((struct pine *));
void	do_menu PROTO((int, Pos *, struct key_menu *));
void	main_redrawer PROTO(());
void	new_user_or_version PROTO((struct pine *));
void	do_setup_task PROTO((int));
int     choose_setup_cmd PROTO((int, MSGNO_S *, SCROLL_S *));
void	queue_init_errors PROTO((struct pine *));
void	upgrade_old_postponed PROTO(());
void	goodnight_gracey PROTO((struct pine *, int));
int	read_stdin_char PROTO(());
void	pine_read_progress PROTO((GETS_DATA *, unsigned long));
void	flag_search PROTO((MAILSTREAM *, int, MsgNo, MSGNO_S *,
			   long (*) PROTO((MAILSTREAM *))));
long    flag_search_sequence PROTO((MAILSTREAM *, MSGNO_S *, long, int));
int	nuov_processor PROTO((int, MSGNO_S *, SCROLL_S *));
MAILSTREAM *stream_cache PROTO((char *));
void        cache_stream PROTO((MAILSTREAM *));
void        end_stream_cache PROTO((void));
#ifdef	WIN32
char   *pine_user_callback PROTO((void));
#endif
#ifdef	_WINDOWS
int	fkey_mode_callback PROTO((int, long));
void	imap_telemetry_on PROTO((void));
void	imap_telemetry_off PROTO((void));
char   *pcpine_help_main PROTO((char *));
int	pcpine_resize_main PROTO((void));
int	pcpine_main_cursor PROTO((int, long));
#endif


typedef struct setup_return_val {
    int cmd;
    int exc;
}SRV_S;

static struct key choose_setup_keys[] =
       {HELP_MENU,
	OTHER_MENU,
	{"E","Exit Setup",{MC_EXIT,3,{'e','m',ctrl('C')}},KS_EXITMODE},
	{"P","Printer",{MC_PRINTER,1,{'p'}},KS_NONE},
	{"N","Newpassword",{MC_PASSWD,1,{'n'}},KS_NONE},
	{"C","Config",{MC_CONFIG,1,{'c'}},KS_NONE},
	{"S","Signature",{MC_SIG,1,{'s'}},KS_NONE},
	{"A","AddressBooks",{MC_ABOOKS,1,{'a'}},KS_NONE},
	{"L","collectionList",{MC_CLISTS,1,{'l'}},KS_NONE},
	{"R","Rules",{MC_RULES,1,{'r'}},KS_NONE},
	{"D","Directory",{MC_DIRECTORY,1,{'d'}},KS_NONE},
	{"K","Kolor",{MC_KOLOR,1,{'k'}},KS_NONE},


	HELP_MENU,
	OTHER_MENU,
	NULL_MENU,
	{"Z","RemoteConfigSetup",{MC_REMOTE,1,{'z'}},KS_NONE},
	NULL_MENU,
	NULL_MENU,
	PREVPAGE_MENU,
	NEXTPAGE_MENU,
	NULL_MENU,
	NULL_MENU,
	NULL_MENU,
	NULL_MENU};
INST_KEY_MENU(choose_setup_keymenu, choose_setup_keys);

#define SETUP_PRINTER	3
#define SETUP_PASSWD	4
#define SETUP_CONFIG	5
#define SETUP_SIG	6
#define SETUP_DIRECTORY	10
#define SETUP_EXCEPT	14

static struct key main_keys[] =
       {HELP_MENU,
	OTHER_MENU,
	NULL_MENU,
	NULL_MENU,
	{"P","PrevCmd",{MC_PREVITEM,3,{'p',ctrl('P'),KEY_UP}},KS_NONE},
	{"N","NextCmd",{MC_NEXTITEM,3,{'n',ctrl('N'),KEY_DOWN}},KS_NONE},
	NULL_MENU,
	NULL_MENU,
	{"R","RelNotes",{MC_RELNOTES,1,{'r'}},KS_NONE},
	{"K","KBLock",{MC_KBLOCK,1,{'k'}},KS_NONE},
	NULL_MENU,
	NULL_MENU,

	HELP_MENU,
	OTHER_MENU,
	QUIT_MENU,
	COMPOSE_MENU,
	LISTFLD_MENU,
	GOTO_MENU,
	{"I","Index",{MC_INDEX,1,{'i'}},KS_FLDRINDEX},
	{"J","Journal",{MC_JOURNAL,1,{'j'}},KS_REVIEW},
	{"S","Setup",{MC_SETUP,1,{'s'}},KS_NONE},
	{"A","AddrBook",{MC_ADDRBOOK,1,{'a'}},KS_ADDRBOOK},
	RCOMPOSE_MENU,
	NULL_MENU};
INST_KEY_MENU(main_keymenu, main_keys);
#define MAIN_HELP_KEY		0
#define MAIN_DEFAULT_KEY	3
#define MAIN_KBLOCK_KEY		9
#define MAIN_QUIT_KEY		14
#define MAIN_COMPOSE_KEY	15
#define MAIN_FOLDER_KEY		16
#define MAIN_INDEX_KEY		18
#define MAIN_SETUP_KEY		20
#define MAIN_ADDRESS_KEY	21

/*
 * length of longest label from keymenu, of labels corresponding to
 * commands in the middle of the screen.  9 is length of ListFldrs
 */
#define LONGEST_LABEL 9  /* length of longest label from keymenu */
#define LONGEST_NAME 1   /* length of longest name from keymenu */

#define EDIT_EXCEPTION (0x100)



/*----------------------------------------------------------------------
     main routine -- entry point

  Args: argv, argc -- The command line arguments


 Initialize pine, parse arguments and so on

 If there is a user address on the command line go into send mode and exit,
 otherwise loop executing the various screens in Pine.

 NOTE: The Windows port def's this to "app_main"
  ----*/

main(argc, argv)
    int   argc;
    char *argv[];
{
    ARGDATA_S	 args;
    int		 rv;
    long	 rvl;
    struct pine *pine_state;
    gf_io_t	 stdin_getc = NULL;
    char        *args_for_debug = NULL, *init_pinerc_debugging = NULL;
#ifdef DYN
    char	 stdiobuf[64];
#endif

    /*----------------------------------------------------------------------
          Set up buffering and some data structures
      ----------------------------------------------------------------------*/

    pine_state                 = (struct pine *)fs_get(sizeof (struct pine));
    memset((void *)pine_state, 0, sizeof(struct pine));
    ps_global                  = pine_state;
    ps_global->def_sort        = SortArrival;
    ps_global->sort_types[0]   = SortSubject;
    ps_global->sort_types[1]   = SortArrival;
    ps_global->sort_types[2]   = SortFrom;
    ps_global->sort_types[3]   = SortTo;
    ps_global->sort_types[4]   = SortCc;
    ps_global->sort_types[5]   = SortDate;
    ps_global->sort_types[6]   = SortSize;
    ps_global->sort_types[7]   = SortSubject2;
    ps_global->sort_types[8]   = SortScore;
    ps_global->sort_types[9]   = SortThread;
    ps_global->sort_types[10]   = EndofList;
    ps_global->atmts           = (ATTACH_S *) fs_get(sizeof(ATTACH_S));
    ps_global->atmts_allocated = 1;
    ps_global->atmts->description = NULL;
    ps_global->low_speed       = 1;
    ps_global->init_context    = -1;
    mn_init(&ps_global->msgmap, 0L);
    init_init_vars(ps_global);
    ps_global->time_of_last_input = time((time_t *)0);

#if !defined(DOS) && !defined(OS2)
    /*
     * Seed the random number generator with the date & pid.  Random 
     * numbers are used for new mail notification and bug report id's
     */
    srandom(getpid() + time(0));
#endif

#ifdef DYN
    /*-------------------------------------------------------------------
      There's a bug in DYNIX that causes the terminal driver to lose
      characters when large I/O writes are done on slow lines. Like
      a 1Kb write(2) on a 1200 baud line. Usually CR is output which
      causes a flush before the buffer is too full, some the pine composer
      doesn't output newlines a lot. Either stdio should be fixed to
      continue with more writes when the write request is partial, or
      fix the tty driver to always complete the write.
     */
    setbuffer(stdout, stdiobuf, 64);
#endif

    /* need home directory early */
    get_user_info(&ps_global->ui);

    pine_state->home_dir = cpystr((getenv("HOME") != NULL)
				    ? getenv("HOME")
				    : ps_global->ui.homedir);

#if	defined(DOS) || defined(OS2)
    {
	char *p;

	/* normalize path delimiters */
	for(p = pine_state->home_dir; p = strchr(p, '/'); p++)
	  *p='\\';
    }
#endif

#ifdef DEBUG
    {   size_t len = 0;
	int   i;
	char *p;
	char *no_args = " <no args>";

	for(i = 0; i < argc; i++)
	  len += (strlen(argv[i] ? argv[i] : "")+3);
	
	if(argc == 1)
	  len += strlen(no_args);
	
	p = args_for_debug = (char *)fs_get((len+2) * sizeof(char));
	*p++ = '\n';
	*p = '\0';

	for(i = 0; i < argc; i++){
	    sprintf(p, "%s\"%s\"", i ? " " : "", argv[i] ? argv[i] : "");
	    p += strlen(p);
	}
	
	if(argc == 1)
	  strncat(args_for_debug, no_args, len-strlen(args_for_debug));
    }
#endif

    /*----------------------------------------------------------------------
           Parse arguments and initialize debugging
      ----------------------------------------------------------------------*/
    pine_args(pine_state, argc, argv, &args);

#ifndef	_WINDOWS
    if(!isatty(0)){
	/*
	 * monkey with descriptors so our normal tty i/o routines don't
	 * choke...
	 */
	dup2(STDIN_FD, PIPED_FD);	/* redirected stdin to new desc */
	dup2(STDERR_FD, STDIN_FD);	/* rebind stdin to the tty	*/
	stdin_getc = read_stdin_char;
	if(stdin_getc && args.action == aaURL){
	  display_args_err(
  "Cannot read stdin when using -url\nFor mailto URLs, use \'body=\' instead", 
	   NULL, 1);
	  args_help();
	  exit(-1);
	}
    }
#endif

    if(ps_global->convert_sigs &&
       (!ps_global->pinerc || !ps_global->pinerc[0])){
	fprintf(stderr, "Use -p <pinerc> with -convert_sigs\n");
	exit(-1);
    }

    /* set some default timeouts in case pinerc is remote */
    mail_parameters(NULL, SET_OPENTIMEOUT, (void *)(long)30);
    mail_parameters(NULL, SET_READTIMEOUT, (void *)(long)15);
    mail_parameters(NULL, SET_TIMEOUT, (void *) pine_tcptimeout);
    /* could be TO_BAIL_THRESHOLD, 15 seems more appropriate for now */
    pine_state->tcp_query_timeout = 15;

    init_pinerc(pine_state, &init_pinerc_debugging);

#ifdef DEBUG
    /* Since this is specific debugging we don't mind if the
       ifdef is the type of system.
     */
#if defined(HAVE_SMALLOC) || defined(NXT)
    if(ps_global->debug_malloc)
      malloc_debug(ps_global->debug_malloc);
#endif
#ifdef	CSRIMALLOC
    if(ps_global->debug_malloc)
      mal_debug(ps_global->debug_malloc);
#endif

    if(!ps_global->convert_sigs
#ifdef _WINDOWS
       && !ps_global->install_flag
#endif /* _WINDOWS */
	)
      init_debug();

    if(args_for_debug){
	dprint(0, (debugfile, " %s\n\n", args_for_debug));
	fs_give((void **)&args_for_debug);
    }

    if(getenv("HOME") != NULL){
	dprint(2, (debugfile, "Setting home dir from $HOME: \"%s\"\n",
	       getenv("HOME")));
    }
    else{
	dprint(2, (debugfile, "Setting home dir: \"%s\"\n",
	       pine_state->home_dir ? pine_state->home_dir : "<?>"));
    }

    /* Watch out. Sensitive information in debug file. */
    if(ps_global->debug_imap > 4)
      mail_parameters(NULL, SET_DEBUGSENSITIVE, (void *) TRUE);

#ifndef DEBUGJOURNAL
    if(ps_global->debug_tcp)
#endif
      mail_parameters(NULL, SET_TCPDEBUG, (void *) TRUE);

#ifdef	_WINDOWS
    mswin_setdebug(debug, debugfile);
    mswin_setdebugoncallback (imap_telemetry_on);
    mswin_setdebugoffcallback (imap_telemetry_off);
    mswin_enableimaptelemetry(ps_global->debug_imap != 0);
#endif
#endif  /* DEBUG */

#ifdef	_WINDOWS
    mswin_setsortcallback(index_sort_callback);
    mswin_setflagcallback(flag_callback);
    mswin_sethdrmodecallback(header_mode_callback);
    mswin_setselectedcallback(any_selected_callback);
    mswin_setzoomodecallback(zoom_mode_callback);
    mswin_setfkeymodecallback(fkey_mode_callback);
#endif

    /*------- Set up c-client drivers -------*/ 
#include "../c-client/linkage.c"
    /*
     * Lookups of long login names which don't exist are very slow in aix.
     * This would normally get set in system-wide config if not needed.
     */
    if(F_ON(F_DISABLE_SHARED_NAMESPACES, ps_global))
      mail_parameters(NULL, SET_DISABLEAUTOSHAREDNS, (void *) TRUE);

    /*------- ... then tune the drivers just installed -------*/ 
#ifdef	DOS
    if(getenv("HOME"))
      mail_parameters(NULL, SET_HOMEDIR, (void *) pine_state->home_dir);

#ifdef	WIN32
    mail_parameters(NULL, SET_USERPROMPT, (void *) pine_user_callback);
#else
    /*
     * install c-client callback to manage cache data outside
     * free memory
     */
    mail_parameters(NULL, SET_CACHE, (void *)dos_cache);
#endif

    /*
     * Sniff the environment for timezone offset.  We need to do this
     * here since Windows needs help figuring out UTC, and will adjust
     * what time() returns based on TZ.  THIS WILL SCREW US because
     * we use time() differences to manage status messages.  So, if 
     * rfc822_date, which calls localtime() and thus needs tzset(),
     * is called while a status message is displayed, it's possible
     * for time() to return a time *before* what we remember as the
     * time we put the status message on the display.  Sheesh.
     */
    tzset();
#else
    /*
     * We used to let c-client do this for us automatically, but it declines
     * to do so for root. This forces c-client to establish an environment,
     * even if the uid is 0.
     */
    env_init(ps_global->ui.login, ps_global->ui.homedir);

    /*
     * Install callback to let us know the progress of network reads...
     */
    (void) mail_parameters(NULL, SET_READPROGRESS, (void *)pine_read_progress);
#endif

    /*
     * Install callback to handle certificate validation failures,
     * allowing the user to continue if they wish.
     */
    mail_parameters(NULL, SET_SSLCERTIFICATEQUERY, (void *) pine_sslcertquery);
    mail_parameters(NULL, SET_SSLFAILURE, (void *) pine_sslfailure);

    if(init_pinerc_debugging){
	dprint(2, (debugfile, init_pinerc_debugging));
	fs_give((void **)&init_pinerc_debugging);
    }

    init_vars(pine_state);

    set_collation(F_OFF(F_DISABLE_SETLOCALE_COLLATE, ps_global),
		  F_ON(F_ENABLE_SETLOCALE_CTYPE, ps_global));

#ifdef _WINDOWS
    if(ps_global->install_flag){
	init_install_get_vars();

	if(ps_global->prc)
	  free_pinerc_s(&ps_global->prc);

	exit(0);
    }
#endif
    if(ps_global->convert_sigs){
	if(convert_sigs_to_literal(ps_global, 0) == -1){
	    fprintf(stderr, "trouble converting sigs\n");
	    exit(-1);
	}

	if(ps_global->prc){
	    if(ps_global->prc->outstanding_pinerc_changes)
	      write_pinerc(ps_global, Main);

	    free_pinerc_s(&pine_state->prc);
	}

	exit(0);
    }

    /*
     * Set up a c-client read timeout and timeout handler.  In general,
     * it shouldn't happen, but a server crash or dead link can cause
     * pine to appear wedged if we don't set this up...
     */
    rv = 30;
    if(pine_state->VAR_TCPOPENTIMEO)
      (void)SVAR_TCP_OPEN(pine_state, rv, tmp_20k_buf);
    mail_parameters(NULL, SET_OPENTIMEOUT, (void *)(long)rv);

    rv = 15;
    if(pine_state->VAR_TCPREADWARNTIMEO)
      (void)SVAR_TCP_READWARN(pine_state, rv, tmp_20k_buf);
    mail_parameters(NULL, SET_READTIMEOUT, (void *)(long)rv);

    rv = 0;
    if(pine_state->VAR_TCPWRITEWARNTIMEO){
	if(!SVAR_TCP_WRITEWARN(pine_state, rv, tmp_20k_buf))
	  if(rv == 0 || rv > 4)				/* making sure */
	    mail_parameters(NULL, SET_WRITETIMEOUT, (void *)(long)rv);
    }

    mail_parameters(NULL, SET_TIMEOUT, (void *) pine_tcptimeout);

    rv = 15;
    if(pine_state->VAR_RSHOPENTIMEO){
	if(!SVAR_RSH_OPEN(pine_state, rv, tmp_20k_buf))
	  if(rv == 0 || rv > 4)				/* making sure */
	    mail_parameters(NULL, SET_RSHTIMEOUT, (void *)(long)rv);
    }

    rv = 15;
    if(pine_state->VAR_SSHOPENTIMEO){
	if(!SVAR_SSH_OPEN(pine_state, rv, tmp_20k_buf))
	  if(rv == 0 || rv > 4)				/* making sure */
	    mail_parameters(NULL, SET_SSHTIMEOUT, (void *)(long)rv);
    }

    rvl = 60L;
    if(pine_state->VAR_MAILDROPCHECK){
	if(!SVAR_MAILDCHK(pine_state, rvl, tmp_20k_buf)){
	    if(rvl == 0L)
	      rvl = (60L * 60L * 24L * 100L);	/* 100 days */

	    if(rvl >= 60L)			/* making sure */
	      mail_parameters(NULL, SET_SNARFINTERVAL, (void *) rvl);
	}
    }

    rvl = 0L;
    if(pine_state->VAR_NNTPRANGE){
	if(!SVAR_NNTPRANGE(pine_state, rvl, tmp_20k_buf))
	  if(rvl > 0L)
	    mail_parameters(NULL, SET_NNTPRANGE, (void *) rvl);
    }

    /*
     * Tell c-client not to be so aggressive about uid mappings
     */
    mail_parameters(NULL, SET_UIDLOOKAHEAD, (void *) 20);

    /*
     * Setup referral handling
     */
    mail_parameters(NULL, SET_IMAPREFERRAL, (void *) imap_referral);
    mail_parameters(NULL, SET_MAILPROXYCOPY, (void *) imap_proxycopy);

    /*
     * Disable some drivers if requested.
     */
    if(ps_global->VAR_DISABLE_DRIVERS &&
       ps_global->VAR_DISABLE_DRIVERS[0] &&
       ps_global->VAR_DISABLE_DRIVERS[0][0]){
	char **t;

	for(t = ps_global->VAR_DISABLE_DRIVERS; t[0] && t[0][0]; t++)
	  if(mail_parameters(NULL, DISABLE_DRIVER, (void *)(*t))){
	      dprint(2, (debugfile, "Disabled mail driver \"%s\"\n", *t));
	  }
	  else{
	      sprintf(tmp_20k_buf,
		     "Failed to disable mail driver \"%.500s\": name not found",
		      *t);
	      init_error(ps_global, SM_ORDER | SM_DING, 3, 5, tmp_20k_buf);
	  }
    }

    /*
     * Disable some authenticators if requested.
     */
    if(ps_global->VAR_DISABLE_AUTHS &&
       ps_global->VAR_DISABLE_AUTHS[0] &&
       ps_global->VAR_DISABLE_AUTHS[0][0]){
	char **t;

	for(t = ps_global->VAR_DISABLE_AUTHS; t[0] && t[0][0]; t++)
	  if(mail_parameters(NULL, DISABLE_AUTHENTICATOR, (void *)(*t))){
	      dprint(2, (debugfile,"Disabled SASL authenticator \"%s\"\n", *t));
	  }
	  else{
	      sprintf(tmp_20k_buf,
	      "Failed to disable SASL authenticator \"%.500s\": name not found",
		      *t);
	      init_error(ps_global, SM_ORDER | SM_DING, 3, 5, tmp_20k_buf);
	  }
    }

    /*
     * setup alternative authentication driver preference for IMAP opens
     */
    if(F_ON(F_PREFER_ALT_AUTH, ps_global))
      mail_parameters(NULL, SET_IMAPTRYALT, (void *) TRUE);

    /*
     * Install handler to let us know about potential delays
     */
    (void) mail_parameters(NULL, SET_BLOCKNOTIFY, (void *) pine_block_notify);

    if(ps_global->dump_supported_options){
	dump_supported_options();
	exit(0);
    }

    /*
     * Install extra headers to fetch along with all the other stuff
     * mail_fetch_structure and mail_fetch_overview requests.
     */
    calc_extra_hdrs();
    if(get_extra_hdrs())
      (void) mail_parameters(NULL, SET_IMAPEXTRAHEADERS,
			     (void *) get_extra_hdrs());

    if(init_username(pine_state) < 0)
      exit(-1);

    if(init_hostname(pine_state) < 0)
      exit(-1);
    
    /*
     * Verify mail dir if we're not in send only mode...
     */
    if(args.action == aaFolder && init_mail_dir(pine_state) < 0)
      exit(-1);

    init_signals();

    /*--- input side ---*/
    if(init_tty_driver(pine_state)){
#if !defined(DOS) && !defined(OS2)	/* always succeeds under DOS! */
        fprintf(stderr, "Can't access terminal or input is not a terminal. ");
        fprintf(stderr, "Redirection of\nstandard input is not allowed. For ");
        fprintf(stderr, "example \"pine < file\" doesn't work.\n%c", BELL);
        exit(-1);
#endif
    }
        

    /*--- output side ---*/
    rv = config_screen(&(pine_state->ttyo));
#if !defined(DOS) && !defined(OS2)	/* always succeeds under DOS! */
    if(rv){
        switch(rv){
          case -1:
	    printf("Terminal type (environment variable TERM) not set.\n");
            break;
          case -2:
	    printf("Terminal type \"%s\", is unknown.\n", getenv("TERM"));
            break;
          case -3:
            printf("Can't open terminal capabilities database.\n");
            break;
          case -4:
            printf("Your terminal, of type \"%s\",", getenv("TERM"));
	    printf(" is lacking functions needed to run pine.\n");
            break;
        }

        printf("\r");
        end_tty_driver(pine_state);
        exit(-1);
    }
#endif

    if(F_ON(F_BLANK_KEYMENU,ps_global))
      FOOTER_ROWS(ps_global) = 1;

    init_screen();
    init_keyboard(pine_state->orig_use_fkeys);
    strcpy(pine_state->inbox_name, INBOX_NAME);
    init_folders(pine_state);		/* digest folder spec's */

    pine_state->in_init_seq = 0;	/* so output (& ClearScreen) show up */
    pine_state->dont_use_init_cmds = 1;	/* don't use up initial_commands yet */
    ClearScreen();

#ifdef	DEBUG
    if(ps_global->debug_imap > 4 || debug > 9){
	q_status_message(SM_ORDER | SM_DING, 5, 9,
	      "Warning: sensitive authentication data included in debug file");
	flush_status_messages(0);
    }
#endif

    if(args.action == aaPrcCopy || args.action == aaAbookCopy){
	int   exit_val = -1;
	char *err_msg = NULL;

	if(args.data.copy.local && args.data.copy.remote){
	    switch(args.action){
	      case aaPrcCopy:
		exit_val = copy_pinerc(args.data.copy.local,
				       args.data.copy.remote, &err_msg);
		break;

	      case aaAbookCopy:
		exit_val = copy_abook(args.data.copy.local,
				      args.data.copy.remote, &err_msg);
		break;
	    }
	}
	if(err_msg){
	  q_status_message(SM_ORDER | SM_DING, 3, 4, err_msg);
	  fs_give((void **)&err_msg);
	}
	goodnight_gracey(pine_state, exit_val);
    }

    if(args.action == aaFolder
       && (pine_state->first_time_user || pine_state->show_new_version)){
	pine_state->mangled_header = 1;
	show_main_screen(pine_state, 0, FirstMenu, &main_keymenu, 0, (Pos *)NULL);
	new_user_or_version(pine_state);
	ClearScreen();
    }
    
    /* put back in case we need to suppress output */
    pine_state->in_init_seq = pine_state->save_in_init_seq;

    /* queue any init errors so they get displayed in a screen below */
    queue_init_errors(ps_global);

    /* "Page" the given file? */
    if(args.action == aaMore){
	int dice = 1, redir = 0;

	if(pine_state->in_init_seq){
	    pine_state->in_init_seq = pine_state->save_in_init_seq = 0;
	    clear_cursor_pos();
	    if(pine_state->free_initial_cmds)
	      fs_give((void **)&(pine_state->free_initial_cmds));

	    pine_state->initial_cmds = 0;
	}

	/*======= Requested that we simply page the given file =======*/
	if(args.data.file){		/* Open the requested file... */
	    SourceType  src;
	    STORE_S    *store = NULL;
	    char       *decode_error = NULL;

	    if(stdin_getc){
		redir++;
		src = CharStar;
		if(isatty(0) && (store = so_get(src, NULL, EDIT_ACCESS))){
		    gf_io_t pc;

		    gf_set_so_writec(&pc, store);
		    gf_filter_init();
		    if(decode_error = gf_pipe(stdin_getc, pc)){
			dice = 0;
			q_status_message1(SM_ORDER, 3, 4,
					  "Problem reading stdin: %.200s",
					  decode_error);
		    }

		    gf_clear_so_writec(store);
		}
		else
		  dice = 0;
	    }
	    else{
		src = FileStar;
		strncpy(ps_global->cur_folder, args.data.file,
			sizeof(ps_global->cur_folder)-1);
		ps_global->cur_folder[sizeof(ps_global->cur_folder)-1] = '\0';
		if(!(store = so_get(src, args.data.file, READ_ACCESS)))
		  dice = 0;
	    }

	    if(dice){
		SCROLL_S sargs;
		static struct key simple_file_keys[] =
		{HELP_MENU,
		 NULL_MENU,
		 {"Q","Quit Viewer",{MC_EXIT,1,{'q'}},KS_NONE},
		 NULL_MENU,
		 NULL_MENU,
		 NULL_MENU,
		 PREVPAGE_MENU,
		 NEXTPAGE_MENU,
		 PRYNTTXT_MENU,
		 WHEREIS_MENU,
		 FWDEMAIL_MENU,
		 {"S", "Save", {MC_SAVETEXT,1,{'s'}}, KS_SAVE}};
		INST_KEY_MENU(simple_file_keymenu, simple_file_keys);
#define SAVE_KEY 9

		memset(&sargs, 0, sizeof(SCROLL_S));
		sargs.text.text = so_text(store);
		sargs.text.src  = src;
		sargs.text.desc = "file";
		sargs.bar.title = "FILE VIEW";
		sargs.bar.style = FileTextPercent;
		sargs.keys.menu = &simple_file_keymenu;
		setbitmap(sargs.keys.bitmap);
		scrolltool(&sargs);

		printf("\n\n");
		so_give(&store);
	    }
	}

	if(!dice){
	    q_status_message2(SM_ORDER, 3, 4,
		"Can't display \"%.200s\": %.200s",
		 (redir) ? "Standard Input" 
			 : args.data.file ? args.data.file : "NULL",
		 error_description(errno));
	}

	goodnight_gracey(pine_state, 0);
    }
    else if(args.action == aaMail || (stdin_getc && (args.action != aaURL))){
        /*======= address on command line/send one message mode ============*/
        char	   *to = NULL, *error = NULL, *addr = NULL;
        int	    len, good_addr = 1;
	int	    exit_val = 0;
	BUILDER_ARG fcc;

	if(pine_state->in_init_seq){
	    pine_state->in_init_seq = pine_state->save_in_init_seq = 0;
	    clear_cursor_pos();
	    if(pine_state->free_initial_cmds)
	      fs_give((void **) &(pine_state->free_initial_cmds));

	    pine_state->initial_cmds = 0;
	}

        /*----- Format the To: line with commas for the composer ---*/
	if(args.data.mail.addrlist){
	    STRLIST_S *p;

	    for(p = args.data.mail.addrlist, len = 0; p; p = p->next)
	      len += strlen(p->name) + 2;

	    to = (char *) fs_get((len + 5) * sizeof(char));
	    for(p = args.data.mail.addrlist, *to = '\0'; p; p = p->next){
		if(*to)
		  strcat(to, ", ");

		strcat(to, p->name);
	    }

	    memset((void *)&fcc, 0, sizeof(BUILDER_ARG));
	    dprint(2, (debugfile, "building addr: -->%s<--\n", to));
	    good_addr = (build_address(to, &addr, &error, &fcc, NULL) >= 0);
	    dprint(2, (debugfile, "mailing to: -->%s<--\n", addr));
	    free_strlist(&args.data.mail.addrlist);
	}
	else
	  memset(&fcc, 0, sizeof(fcc));

	if(good_addr){
	    compose_mail(addr, fcc.tptr, NULL,
			 args.data.mail.attachlist, stdin_getc);
	}
	else{
	    q_status_message1(SM_ORDER, 3, 4, "Bad address: %.200s", error);
	    exit_val = -1;
	}

	if(addr)
	  fs_give((void **) &addr);

	if(fcc.tptr)
	  fs_give((void **) &fcc.tptr);

	if(args.data.mail.attachlist)
	  free_attachment_list(&args.data.mail.attachlist);

	if(to)
	  fs_give((void **) &to);

	if(error)
	  fs_give((void **) &error);

	goodnight_gracey(pine_state, exit_val);
    }
    else{
	char             int_mail[MAXPATH+1];
        struct key_menu *km = &main_keymenu;

        /*========== Normal pine mail reading mode ==========*/
            
        pine_state->mail_stream    = NULL;
        pine_state->inbox_stream   = NULL;
        pine_state->mangled_screen = 1;

	if(args.action == aaURL){
	    url_tool_t f;

	    if(pine_state->in_init_seq){
		pine_state->in_init_seq = pine_state->save_in_init_seq = 0;
		clear_cursor_pos();
		if(pine_state->free_initial_cmds)
		  fs_give((void **) &(pine_state->free_initial_cmds));
		pine_state->initial_cmds = 0;
	    }
	    if(f = url_local_handler(args.data.url)){
		if(!((*f)(args.data.url) && pine_state->next_screen))
		  goodnight_gracey(pine_state, 0);	/* no return */
	    }
	    else{
		q_status_message1(SM_ORDER | SM_DING, 3, 4,
				  "Unrecognized URL \"%.200s\"", args.data.url);
		goodnight_gracey(pine_state, -1);	/* no return */
	    }
	}
	else if(!pine_state->start_in_index){
	    /* flash message about executing initial commands */
	    if(pine_state->in_init_seq){
	        pine_state->in_init_seq    = 0;
		clear_cursor_pos();
		pine_state->mangled_header = 1;
		pine_state->mangled_footer = 1;
		pine_state->mangled_screen = 0;
		/* show that this is Pine */
		show_main_screen(pine_state, 0, FirstMenu, km, 0, (Pos *)NULL);
		pine_state->mangled_screen = 1;
		pine_state->painted_footer_on_startup = 1;
		if(min(4, pine_state->ttyo->screen_rows - 4) > 1)
	          PutLine0(min(4, pine_state->ttyo->screen_rows - 4),
		    max(min(11, pine_state->ttyo->screen_cols -38), 0),
		    "Executing initial-keystroke-list......");

	        pine_state->in_init_seq = 1;
	    }
	    else{
                show_main_screen(pine_state, 0, FirstMenu, km, 0, (Pos *)NULL);
		pine_state->painted_body_on_startup   = 1;
		pine_state->painted_footer_on_startup = 1;
	    }
        }
	else{
	    /* cancel any initial commands, overridden by cmd line */
	    if(pine_state->in_init_seq){
		pine_state->in_init_seq      = 0;
		pine_state->save_in_init_seq = 0;
		clear_cursor_pos();
		if(pine_state->initial_cmds){
		    if(pine_state->free_initial_cmds)
		      fs_give((void **)&(pine_state->free_initial_cmds));

		    pine_state->initial_cmds = 0;
		}

		F_SET(F_USE_FK,pine_state, pine_state->orig_use_fkeys);
	    }

            (void) do_index_border(pine_state->context_current,
				   pine_state->cur_folder,
				   pine_state->mail_stream,
				   pine_state->msgmap, MsgIndex, NULL,
				   INDX_CLEAR|INDX_HEADER|INDX_FOOTER);
	    pine_state->painted_footer_on_startup = 1;
	    if(min(4, pine_state->ttyo->screen_rows - 4) > 1)
	      PutLine1(min(4, pine_state->ttyo->screen_rows - 4),
		max(min(11, pine_state->ttyo->screen_cols -40), 0),
		"Please wait, opening %s......",
		"mail folder");
        }

        fflush(stdout);

	if(pine_state->in_init_seq){
	    pine_state->in_init_seq = 0;
	    clear_cursor_pos();
	}

        if(args.action == aaFolder && args.data.folder){
	    CONTEXT_S *cntxt;

	    if((rv = pine_state->init_context) < 0)
	      /*
	       * As with almost all the folder vars in the pinerc,
	       * we subvert the collection "breakout" here if the
	       * folder name given looks like an asolute path on
	       * this system...
	       */
	      cntxt = (is_absolute_path(args.data.folder))
			? NULL : pine_state->context_current;
	    else if(rv == 0)
	      cntxt = NULL;
	    else
	      for(cntxt = pine_state->context_list;
		  rv > 1 && cntxt->next;
		  rv--, cntxt = cntxt->next)
		;

            if(do_broach_folder(args.data.folder, cntxt, NULL) <= 0){
		q_status_message1(SM_ORDER, 3, 4,
		    "Unable to open folder \"%.200s\"", args.data.folder);

		goodnight_gracey(pine_state, -1);
	    }
	}
	else if(args.action == aaFolder){
#if defined(DOS) || defined(OS2)
            /*
	     * need to ask for the inbox name if no default under DOS
	     * since there is no "inbox"
	     */

	    if(!pine_state->VAR_INBOX_PATH || !pine_state->VAR_INBOX_PATH[0]
	       || strucmp(pine_state->VAR_INBOX_PATH, "inbox") == 0){
		HelpType help = NO_HELP;
		static   ESCKEY_S ekey[] = {{ctrl(T), 2, "^T", "To Fldrs"},
					  {-1, 0, NULL, NULL}};

		pine_state->mangled_footer = 1;
		int_mail[0] = '\0';
    		while(1){
		    int flags = OE_APPEND_CURRENT;

        	    rv = optionally_enter(int_mail, -FOOTER_ROWS(pine_state),
				      0, sizeof(int_mail),
				      "No inbox!  Folder to open as inbox : ",
				      /* ekey */ NULL, help, &flags);
        	    if(rv == 3){
			help = (help == NO_HELP) ? h_sticky_inbox : NO_HELP;
			continue;
		    }

        	    if(rv != 4)
		      break;
    		}

    		if(rv == 1){
		    q_status_message(SM_ORDER, 0, 2 ,"Folder open cancelled");
		    rv = 0;		/* reset rv */
		} 
		else if(rv == 2){
#if	0
        	    if(!folder_lister(pine_state, OpenFolder, NULL,
			       &(pine_state->context_current),
			       int_mail, NULL,
			       pine_state->context_current, NULL))
		      *int_mail = '\0';	/* user cancelled! */
#endif
                    show_main_screen(pine_state,0,FirstMenu,km,0,(Pos *)NULL);
		}

		if(*int_mail){
		    removing_trailing_white_space(int_mail);
		    removing_leading_white_space(int_mail);
		    if((!pine_state->VAR_INBOX_PATH 
			|| strucmp(pine_state->VAR_INBOX_PATH, "inbox") == 0)
		     && want_to("Preserve folder as \"inbox-path\" in PINERC", 
				'y', 'n', NO_HELP, WT_NORM) == 'y'){
			set_variable(V_INBOX_PATH, int_mail, 1, 1, Main);
		    }
		    else{
			if(pine_state->VAR_INBOX_PATH)
			  fs_give((void **)&pine_state->VAR_INBOX_PATH);

			pine_state->VAR_INBOX_PATH = cpystr(int_mail);
		    }

		    do_broach_folder(pine_state->inbox_name, 
				     pine_state->context_list, NULL);
    		}
		else
		  q_status_message(SM_ORDER, 0, 2 ,"No folder opened");

	    }
	    else

#endif
            do_broach_folder(pine_state->inbox_name,
			     pine_state->context_list, NULL);
        }

        if(pine_state->mangled_footer)
	  pine_state->painted_footer_on_startup = 0;

        if(args.action == aaFolder
	   && pine_state->mail_stream
	   && expire_sent_mail())
	  pine_state->painted_footer_on_startup = 0;

	/*
	 * Initialize the defaults.  Initializing here means that
	 * if they're remote, the user isn't prompted for an imap login
	 * before the display's drawn, AND there's the chance that
	 * we can climb onto the already opened folder's stream...
	 */
	if(ps_global->first_time_user)
	  init_save_defaults();	/* initialize default save folders */

	build_path(int_mail,
		   ps_global->VAR_OPER_DIR ? ps_global->VAR_OPER_DIR
					   : pine_state->home_dir,
		   INTERRUPTED_MAIL, sizeof(int_mail));
	if(args.action == aaFolder
	   && (folder_exists(NULL, int_mail) & FEX_ISFILE))
	  q_status_message(SM_ORDER | SM_DING, 4, 5, 
		       "Use compose command to continue interrupted message.");

#if defined(USE_QUOTAS)
	{
	    long q;
	    int  over;
	    q = disk_quota(pine_state->home_dir, &over);
	    if(q > 0 && over){
		q_status_message2(SM_ASYNC | SM_DING, 4, 5,
		      "WARNING! Over your disk quota by %.200s bytes (%.200s)",
			      comatose(q),byte_string(q));
	    }
	}
#endif

	pine_state->in_init_seq = pine_state->save_in_init_seq;
	pine_state->dont_use_init_cmds = 0;
	clear_cursor_pos();

	if(pine_state->give_fixed_warning)
	  q_status_message(SM_ASYNC, 0, 10,
"Note: some of your config options conflict with site policy and are ignored");

	if(!prune_folders_ok())
	  q_status_message(SM_ASYNC, 0, 10, 
			   "Note: ignoring pruned-folders outside of default collection for saves");
	
	if(timeo == 0 &&
	   ps_global->VAR_INBOX_PATH &&
	   ps_global->VAR_INBOX_PATH[0] == '{')
	  q_status_message(SM_ASYNC, 0, 10,
"Note: mail-check-interval=0 may cause IMAP server connection to time out");

        /*-------------------------------------------------------------------
                         Loop executing the commands
    
            This is done like this so that one command screen can cause
            another one to execute it with out going through the main menu. 
          ------------------------------------------------------------------*/
	if(!pine_state->next_screen)
	  pine_state->next_screen = pine_state->start_in_index
				      ? mail_index_screen : main_menu_screen;
        while(1){
            if(pine_state->next_screen == SCREEN_FUN_NULL) 
              pine_state->next_screen = main_menu_screen;

            (*(pine_state->next_screen))(pine_state);
        }
    }
}



/*
 * read_stdin_char - simple function to return a character from
 *		     redirected stdin
 */
int
read_stdin_char(c)
    char *c;
{
    int rv;
    
    /* it'd probably be a good idea to fix this to pre-read blocks */
    while(1){
	rv = read(PIPED_FD, c, 1);
	if(rv < 0){
	    if(errno == EINTR){
		dprint(2, (debugfile, "read_stdin_char: read interrupted, restarting\n"));
		continue;
	    }
	    else
	      dprint(1, (debugfile, "read_stdin_char: read FAILED: %s\n",
			 error_description(errno)));
	}
	break;
    }
    return(rv);
}


/* this default is from the array of structs below */
#define DEFAULT_MENU_ITEM 6		/* LIST FOLDERS */
#define ABOOK_MENU_ITEM 8		/* ADDRESS BOOK */
#define MAX_DEFAULT_MENU_ITEM 12
#define UNUSED 0
static unsigned char menu_index = DEFAULT_MENU_ITEM;

/*
 * One of these for each line that gets printed in the middle of the
 * screen in the main menu.
 */
static struct menu_key {
    char         *key_and_name,
		 *news_addition;
    int		  key_index;	  /* index into keymenu array for this cmd */
} mkeys[] = {
    {" %s     HELP               -  Get help using Pine",
     NULL, MAIN_HELP_KEY},
    {"", NULL, UNUSED},
    {" %s     COMPOSE MESSAGE    -  Compose and send%s a message",
     "/post", MAIN_COMPOSE_KEY},
    {"", NULL, UNUSED},
    {" %s     MESSAGE INDEX      -  View messages in current folder",
     NULL, MAIN_INDEX_KEY},
    {"", NULL, UNUSED},
    {" %s     FOLDER LIST        -  Select a folder%s to view",
     " OR news group", MAIN_FOLDER_KEY},
    {"", NULL, UNUSED},
    {" %s     ADDRESS BOOK       -  Update address book",
     NULL, MAIN_ADDRESS_KEY},
    {"", NULL, UNUSED},
    {" %s     SETUP              -  Configure Pine Options",
     NULL, MAIN_SETUP_KEY},
    {"", NULL, UNUSED},
    {" %s     QUIT               -  Leave the Pine program",
     NULL, MAIN_QUIT_KEY}
};



/*----------------------------------------------------------------------
      display main menu and execute main menu commands

    Args: The usual pine structure

  Result: main menu commands are executed


              M A I N   M E N U    S C R E E N

   Paint the main menu on the screen, get the commands and either execute
the function or pass back the name of the function to execute for the menu
selection. Only simple functions that always return here can be executed
here.

This functions handling of new mail, redrawing, errors and such can 
serve as a template for the other screen that do much the same thing.

There is a loop that fetchs and executes commands until a command to leave
this screen is given. Then the name of the next screen to display is
stored in next_screen member of the structure and this function is exited
with a return.

First a check for new mail is performed. This might involve reading the new
mail into the inbox which might then cause the screen to be repainted.

Then the general screen painting is done. This is usually controlled
by a few flags and some other position variables. If they change they
tell this part of the code what to repaint. This will include cursor
motion and so on.
  ----*/
void
main_menu_screen(pine_state)
    struct pine *pine_state;
{
    int		    ch, cmd, just_a_navigate_cmd, setup_command, km_popped;
    char            *new_folder;
    CONTEXT_S       *tc;
    struct key_menu *km;
    OtherMenu        what;
    Pos              curs_pos;
#if defined(DOS) || defined(OS2)
    extern void (*while_waiting)();
#endif

    ps_global                 = pine_state;
    just_a_navigate_cmd       = 0;
    km_popped		      = 0;
    menu_index = DEFAULT_MENU_ITEM;
    what                      = FirstMenu;  /* which keymenu to display */
    ch                        = 'x'; /* For display_message 1st time through */
    pine_state->next_screen   = SCREEN_FUN_NULL;
    pine_state->prev_screen   = main_menu_screen;
    curs_pos.row = pine_state->ttyo->screen_rows-FOOTER_ROWS(pine_state);
    curs_pos.col = 0;
    km		 = &main_keymenu;

    mailcap_free(); /* free resources we won't be using for a while */

    if(!pine_state->painted_body_on_startup 
       && !pine_state->painted_footer_on_startup){
	pine_state->mangled_screen = 1;
    }
    else{
	int len = strlen(mkeys[menu_index].key_and_name);
	if(F_OFF(F_USE_FK,ps_global))
	  len += strlen(km->keys[mkeys[menu_index].key_index].name);

	if(ps_global->VAR_NEWS_SPEC && mkeys[menu_index].news_addition)
	  len += strlen(mkeys[menu_index].news_addition);

	curs_pos.col = max(((ps_global->ttyo->screen_cols - len) / 2) - 1, 0);
	curs_pos.col += 6;
	if(F_OFF(F_USE_FK,ps_global))
	  curs_pos.col++;
	
	curs_pos.col = min(ps_global->ttyo->screen_cols-1, curs_pos.col);
	curs_pos.row = menu_index + 3;
    }

    dprint(1, (debugfile, "\n\n    ---- MAIN_MENU_SCREEN ----\n"));

    while(1){
	if(km_popped){
	    km_popped--;
	    if(km_popped == 0){
		clearfooter(pine_state);
		pine_state->mangled_body = 1;
	    }
	}

	/*
	 * fix up redrawer just in case some submenu caused it to get
	 * reassigned...
	 */
	pine_state->redrawer = main_redrawer;

	/*----------- Check for new mail -----------*/
        if(new_mail(0, NM_TIMING(ch), NM_STATUS_MSG | NM_DEFER_SORT) >= 0)
          pine_state->mangled_header = 1;

        if(streams_died())
          pine_state->mangled_header = 1;

        show_main_screen(pine_state, just_a_navigate_cmd, what, km,
			 km_popped, &curs_pos);
        just_a_navigate_cmd = 0;
	what = SameMenu;

	/*---- This displays new mail notification, or errors ---*/
	if(km_popped){
	    FOOTER_ROWS(pine_state) = 3;
	    mark_status_dirty();
	}

        display_message(ch);
	if(km_popped){
	    FOOTER_ROWS(pine_state) = 1;
	    mark_status_dirty();
	}

	if(F_OFF(F_SHOW_CURSOR, ps_global)){
	    curs_pos.row =pine_state->ttyo->screen_rows-FOOTER_ROWS(pine_state);
	    curs_pos.col =0;
	}

        MoveCursor(curs_pos.row, curs_pos.col);

        /*------ Read the command from the keyboard ----*/      
#ifdef	MOUSE
	mouse_in_content(KEY_MOUSE, -1, -1, 0, 0);
	register_mfunc(mouse_in_content, HEADER_ROWS(pine_state), 0,
		    pine_state->ttyo->screen_rows-(FOOTER_ROWS(pine_state)+1),
		       pine_state->ttyo->screen_cols);
#endif
#if defined(DOS) || defined(OS2)
	/*
	 * AND pre-build header lines.  This works just fine under
	 * DOS since we wait for characters in a loop. Something will
         * will have to change under UNIX if we want to do the same.
	 */
	while_waiting = build_header_cache;
#ifdef	_WINDOWS
	mswin_sethelptextcallback(pcpine_help_main);
	mswin_setresizecallback(pcpine_resize_main);
	mswin_mousetrackcallback(pcpine_main_cursor);
#endif
#endif
        ch = read_command();
#ifdef	MOUSE
	clear_mfunc(mouse_in_content);
#endif
#if defined(DOS) || defined(OS2)
	while_waiting = NULL;
#ifdef	_WINDOWS
	mswin_sethelptextcallback(NULL);
	mswin_clearresizecallback(pcpine_resize_main);
	mswin_mousetrackcallback(NULL);
#endif
#endif

	/* No matter what, Quit here always works */
	if(ch == 'q' || ch == 'Q'){
	    cmd = MC_QUIT;
	}
#ifdef	DEBUG
	else if(debug && ch && strchr("123456789", ch)){
	    int olddebug;

	    olddebug = debug;
	    debug = ch - '0';
	    if(debug > 7)
	      ps_global->debug_timestamp = 1;
	    else
	      ps_global->debug_timestamp = 0;

	    if(debug > 7)
	      ps_global->debug_imap = 4;
	    else if(debug > 6)
	      ps_global->debug_imap = 3;
	    else if(debug > 4)
	      ps_global->debug_imap = 2;
	    else if(debug > 2)
	      ps_global->debug_imap = 1;
	    else
	      ps_global->debug_imap = 0;

	    if(ps_global->mail_stream){
		if(ps_global->debug_imap > 0){
		    mail_debug(ps_global->mail_stream);
#ifdef	_WINDOWS
		    mswin_enableimaptelemetry(TRUE);
#endif
		}
		else{
		    mail_nodebug(ps_global->mail_stream);
#ifdef	_WINDOWS
		    mswin_enableimaptelemetry(FALSE);
#endif
		}
	    }

	    if(debug > 7 && olddebug <= 7)
	      mail_parameters(NULL, SET_TCPDEBUG, (void *) TRUE);
	    else if(debug <= 7 && olddebug > 7 && !ps_global->debugmem)
	      mail_parameters(NULL, SET_TCPDEBUG, (void *) FALSE);

	    dprint(1, (debugfile, "*** Debug level set to %d ***\n", debug));
	    if(debugfile)
	      fflush(debugfile);

	    q_status_message1(SM_ORDER, 0, 1, "Debug level set to %.200s",
			      int2string(debug));
	    continue;
	}
#endif	/* DEBUG */
#if	defined(DOS) && !defined(_WINDOWS)
	else if(ch == 'h'){
/* while we're testing DOS */
#include <malloc.h>
	    int    heapresult;
	    int    totalused = 0;
	    int    totalfree = 0;
	    long   totalusedbytes = 0L;
	    long   totalfreebytes = 0L;
	    long   largestinuse = 0L;
	    long   largestfree = 0L, freeaccum = 0L;

	    _HEAPINFO hinfo;
	    extern long coreleft();
	    extern void dumpmetacache();

	    hinfo._pentry = NULL;
	    while((heapresult = _heapwalk(&hinfo)) == _HEAPOK){
		if(hinfo._useflag == _USEDENTRY){
		    totalused++;
		    totalusedbytes += (long)hinfo._size; 
		    if(largestinuse < (long)hinfo._size)
		      largestinuse = (long)hinfo._size;
		}
		else{
		    totalfree++;
		    totalfreebytes += (long)hinfo._size;
		}

		if(hinfo._useflag == _USEDENTRY){
		    if(freeaccum > largestfree) /* remember largest run */
		      largestfree = freeaccum;

		    freeaccum = 0L;
		}
		else
		  freeaccum += (long)hinfo._size;
	    }

	    sprintf(tmp_20k_buf,
		  "use: %d (%ld, %ld lrg), free: %d (%ld, %ld lrg), DOS: %ld", 
		    totalused, totalusedbytes, largestinuse,
		    totalfree, totalfreebytes, largestfree, coreleft());
	    q_status_message(SM_ORDER, 5, 7, tmp_20k_buf);

	    switch(heapresult/* = _heapchk()*/){
	      case _HEAPBADPTR:
		q_status_message(SM_ORDER | SM_DING, 1, 2,
				 "ERROR - Bad ptr in heap");
		break;
	      case _HEAPBADBEGIN:
		q_status_message(SM_ORDER | SM_DING, 1, 2,
				 "ERROR - Bad start of heap");
		break;
	      case _HEAPBADNODE:
		q_status_message(SM_ORDER | SM_DING, 1, 2,
				 "ERROR - Bad node in heap");
		break;
	      case _HEAPEMPTY:
		q_status_message(SM_ORDER, 1, 2, "Heap OK - empty");
		break;
	      case _HEAPEND:
		q_status_message(SM_ORDER, 1, 2, "Heap checks out!");
		break;
	      case _HEAPOK:
		q_status_message(SM_ORDER, 1, 2, "Heap checks out!");
		break;
	      default:
		q_status_message1(SM_ORDER | SM_DING, 1, 2,
				  "BS from heapchk: %d",
				  (void *)heapresult);
		break;
	    }

	    /*       dumpmetacache(ps_global->mail_stream);*/
	    /* DEBUG: heapcheck() */
	    /*       q_status_message1(SM_ORDER, 1, 3,
		     " * * There's %ld bytes of core left for Pine * * ", 
		     (void *)coreleft());*/
	}
#endif	/* DOS for testing */
	else{
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
		  clearfooter(pine_state);
		  break;
	      }
	}

	/*------ Execute the command ------*/
	switch (cmd){
help_case :
	    /*------ HELP ------*/
	  case MC_HELP :

	    if(FOOTER_ROWS(pine_state) == 1 && km_popped == 0){
		km_popped = 2;
		pine_state->mangled_footer = 1;
	    }
	    else{
		helper(main_menu_tx, "HELP FOR MAIN MENU", 0);
		pine_state->mangled_screen = 1;
	    }

	    break;


	    /*---------- display other key bindings ------*/
	  case MC_OTHER :
	    if(ch == 'o')
	      warn_other_cmds();

	    what = NextMenu;
	    pine_state->mangled_footer = 1;
	    break;


	    /*---------- Previous item in menu ----------*/
	  case MC_PREVITEM :
	    if(menu_index > 1) {
		menu_index -= 2;  /* 2 to skip the blank lines */
		pine_state->mangled_body = 1;
		if(km->which == 0)
		  pine_state->mangled_footer = 1;

		just_a_navigate_cmd++;
	    }
	    else
	      q_status_message(SM_ORDER, 0, 2, "Already at top of list");

	    break;


	    /*---------- Next item in menu ----------*/
	  case MC_NEXTITEM :
	    if(menu_index < (unsigned)(MAX_DEFAULT_MENU_ITEM-1)){
		menu_index += 2;
		pine_state->mangled_body = 1;
		if(km->which == 0)
		  pine_state->mangled_footer = 1;

		just_a_navigate_cmd++;
	    }
	    else
	      q_status_message(SM_ORDER, 0, 2, "Already at bottom of list");

	    break;


	    /*---------- Release Notes ----------*/
	  case MC_RELNOTES :
	    helper(h_news, "PINE RELEASE NOTES", 0);
	    pine_state->mangled_screen = 1;
	    break;


#ifndef NO_KEYBOARD_LOCK
	    /*---------- Keyboard lock ----------*/
	  case MC_KBLOCK :
	    (void) lock_keyboard();
	    pine_state->mangled_screen = 1;
	    break;
#endif /* !NO_KEYBOARD_LOCK */


	    /*---------- Quit pine ----------*/
	  case MC_QUIT :
	    pine_state->next_screen = quit_screen;
	    return;

  
	    /*---------- Go to composer ----------*/
	  case MC_COMPOSE :
	    pine_state->next_screen = compose_screen;
	    return;

  
	    /*---- Go to alternate composer ------*/
	  case MC_ROLE :
	    pine_state->next_screen = alt_compose_screen;
	    return;

  
	    /*---------- Top of Folder list ----------*/
	  case MC_COLLECTIONS : 
	    pine_state->next_screen = folder_screen;
	    return;


	    /*---------- Goto new folder ----------*/
	  case MC_GOTO :
	    tc = ps_global->context_current;
	    new_folder = broach_folder(-FOOTER_ROWS(pine_state), 1, &tc);
#if	defined(DOS) && !defined(_WINDOWS)
	    if(new_folder && *new_folder == '{' && coreleft() < 20000){
		q_status_message(SM_ORDER | SM_DING, 3, 3,
				 "Not enough memory to open IMAP folder");
		new_folder = NULL;
	    }
#endif
	    if(new_folder)
	      visit_folder(ps_global, new_folder, tc, NULL);

	    return;


	    /*---------- Go to index ----------*/
	  case MC_INDEX :
	    if(THREADING() && pine_state->viewing_a_thread)
	      unview_thread(pine_state, pine_state->mail_stream,
			    pine_state->msgmap);

	    pine_state->next_screen = mail_index_screen;
	    return;


	    /*---------- Review Status Messages ----------*/
	  case MC_JOURNAL :
	    review_messages();
	    pine_state->mangled_screen = 1;
	    break;


	    /*---------- Setup mini menu ----------*/
	  case MC_SETUP :
setup_case :
	    setup_command = setup_menu(pine_state);
	    pine_state->mangled_footer = 1;
	    do_setup_task(setup_command);
	    if(ps_global->next_screen != main_menu_screen)
	      return;

	    break;


	    /*---------- Go to address book ----------*/
	  case MC_ADDRBOOK :
	    pine_state->next_screen = addr_book_screen;
	    return;


	    /*------ Repaint the works -------*/
	  case MC_RESIZE :
          case MC_REPAINT :
	    ClearScreen();
	    pine_state->mangled_screen = 1;
	    break;

  
#ifdef	MOUSE
	    /*------- Mouse event ------*/
	  case MC_MOUSE :
	    {   
		MOUSEPRESS mp;
		unsigned char ndmi;

		mouse_get_last (NULL, &mp);

#ifdef	_WINDOWS
		if(mp.button == M_BUTTON_RIGHT){
		    if(!mp.doubleclick){
			static MPopup main_popup[] = {
			    {tQueue, {"Folder List", lNormal}, {'L'}},
			    {tQueue, {"Message Index", lNormal}, {'I'}},
			    {tSeparator},
			    {tQueue, {"Address Book", lNormal}, {'A'}},
			    {tQueue, {"Setup Options", lNormal}, {'S'}},
			    {tTail}
			};

			mswin_popup(main_popup);
		    }
		}
		else {
#endif
		    ndmi = mp.row - 3;
		    if (mp.row >= 3 && !(ndmi & 0x01)
			&& ndmi <= (unsigned)MAX_DEFAULT_MENU_ITEM
			&& ndmi < pine_state->ttyo->screen_rows
					       - 4 - FOOTER_ROWS(ps_global)) {
			if(mp.doubleclick){
			    switch(ndmi){	/* fake main_screen request */
			      case 0 :
				goto help_case;

			      case 2 :
				pine_state->next_screen = compose_screen;
				return;

			      case 4 :
				pine_state->next_screen = mail_index_screen;
				return;

			      case 6 :
				pine_state->next_screen = folder_screen;
				return;

			      case 8 :
				pine_state->next_screen = addr_book_screen;
				return;

			      case 10 :
				goto setup_case;

			      case 12 :
				pine_state->next_screen = quit_screen;
				return;

			      default:			/* no op */
				break;
			    }
			}
			else{
			    menu_index = ndmi;
			    pine_state->mangled_body = 1;
			    if(km->which == 0)
			      pine_state->mangled_footer = 1;

			    just_a_navigate_cmd++;
			}
		    }
#ifdef	_WINDOWS
		}
#endif
	    }

	    break;
#endif


	    /*------ Input timeout ------*/
	  case MC_NONE :
            break;	/* noop for timeout loop mail check */


	    /*------ Bogus Input ------*/
          case MC_UNKNOWN :
	  default:
	    bogus_command(ch, F_ON(F_USE_FK,pine_state) ? "F1" : "?");
	    break;
	 } /* the switch */
    } /* the BIG while loop! */
}



/*----------------------------------------------------------------------
    Re-Draw the main menu

    Args: none.

  Result: main menu is re-displayed
  ----*/
void
main_redrawer()
{
    struct key_menu *km = &main_keymenu;

    ps_global->mangled_screen = 1;
    show_main_screen(ps_global, 0, FirstMenu, km, 0, (Pos *)NULL);
}


	
/*----------------------------------------------------------------------
         Draw the main menu

    Args: pine_state - the usual struct
	  quick_draw - tells do_menu() it can skip some drawing
	  what       - tells which section of keymenu to draw
	  km         - the keymenu
	  cursor_pos - returns a good position for the cursor to be located

  Result: main menu is displayed
  ----*/
void
show_main_screen(ps, quick_draw, what, km, km_popped, cursor_pos)
    struct pine     *ps;
    int		     quick_draw;
    OtherMenu	     what;
    struct key_menu *km;
    int		     km_popped;
    Pos             *cursor_pos;
{
    if(ps->painted_body_on_startup || ps->painted_footer_on_startup){
	ps->mangled_screen = 0;		/* only worry about it here */
	ps->mangled_header = 1;		/* we have to redo header */
	if(!ps->painted_body_on_startup)
	  ps->mangled_body = 1;		/* make sure to paint body*/

	if(!ps->painted_footer_on_startup)
	  ps->mangled_footer = 1;	/* make sure to paint footer*/

	ps->painted_body_on_startup   = 0;
        ps->painted_footer_on_startup = 0;
    }

    if(ps->mangled_screen){
	ps->mangled_header = 1;
	ps->mangled_body   = 1;
	ps->mangled_footer = 1;
	ps->mangled_screen = 0;
    }

#ifdef _WINDOWS
    /* Reset the scroll range.  Main screen never scrolls. */
    scroll_setrange (0L, 0L);
    mswin_beginupdate();
#endif

    /* paint the titlebar if needed */
    if(ps->mangled_header){
	set_titlebar("MAIN MENU", ps->mail_stream, ps->context_current,
		     ps->cur_folder, ps->msgmap, 1, FolderName, 0, 0, NULL);
	ps->mangled_header = 0;
    }

    /* paint the body if needed */
    if(ps->mangled_body){
	if(!quick_draw)
	  ClearBody();

	do_menu(quick_draw, cursor_pos, km);
	ps->mangled_body = 0;
    }

    /* paint the keymenu if needed */
    if(km && ps->mangled_footer){
	static char label[LONGEST_LABEL + 2 + 1], /* label + brackets + \0 */
		    name[8];
	bitmap_t    bitmap;

	setbitmap(bitmap);

#ifndef NO_KEYBOARD_LOCK
	if(ps_global->restricted || F_ON(F_DISABLE_KBLOCK_CMD,ps_global))
#endif
	  clrbitn(MAIN_KBLOCK_KEY, bitmap);

	menu_clear_binding(km, '>');
	menu_clear_binding(km, '.');
	menu_clear_binding(km, KEY_RIGHT);
	menu_clear_binding(km, ctrl('M'));
	menu_clear_binding(km, ctrl('J'));
	km->keys[MAIN_DEFAULT_KEY].bind
				 = km->keys[mkeys[menu_index].key_index].bind;
	km->keys[MAIN_DEFAULT_KEY].label
				 = km->keys[mkeys[menu_index].key_index].label;

	/* put brackets around the default action */
	sprintf(label, "[%s]", km->keys[mkeys[menu_index].key_index].label);
#ifdef	notdef
	sprintf(name, "%s>", km->keys[mkeys[menu_index].key_index].name);
#else
	strcpy(name, ">");
#endif
	km->keys[MAIN_DEFAULT_KEY].label = label;
	km->keys[MAIN_DEFAULT_KEY].name = name;
	menu_add_binding(km, '>', km->keys[MAIN_DEFAULT_KEY].bind.cmd);
	menu_add_binding(km, '.', km->keys[MAIN_DEFAULT_KEY].bind.cmd);
	menu_add_binding(km, ctrl('M'), km->keys[MAIN_DEFAULT_KEY].bind.cmd);
	menu_add_binding(km, ctrl('J'), km->keys[MAIN_DEFAULT_KEY].bind.cmd);

	if(F_ON(F_ARROW_NAV,ps_global))
	  menu_add_binding(km, KEY_RIGHT, km->keys[MAIN_DEFAULT_KEY].bind.cmd);

	if(km_popped){
	    FOOTER_ROWS(ps) = 3;
	    clearfooter(ps);
	}

	draw_keymenu(km, bitmap, ps_global->ttyo->screen_cols,
		     1-FOOTER_ROWS(ps_global), 0, what);
	ps->mangled_footer = 0;
	if(km_popped){
	    FOOTER_ROWS(ps) = 1;
	    mark_keymenu_dirty();
	}
    }

#ifdef _WINDOWS
    mswin_endupdate();
#endif
}


/*----------------------------------------------------------------------
         Actually display the main menu

    Args: quick_draw - just a next or prev command was typed so we only have
		       to redraw the highlighting
          cursor_pos - a place to return a good value for cursor location

  Result: Main menu is displayed
  ---*/
void
do_menu(quick_draw, cursor_pos, km)
    int		     quick_draw;
    Pos		    *cursor_pos;
    struct key_menu *km;
{
    int  dline, indent, longest = 0;
    char buf[MAX_DEFAULT_MENU_ITEM+1][MAX_SCREEN_COLS+1];
    static int last_inverse = -1;
    Pos pos;


    /*
     * Build all the menu lines...
     */
    for(dline = 0; dline < sizeof(mkeys)/(sizeof(mkeys[1])); dline++){
	memset((void *)buf[dline], ' ', MAX_SCREEN_COLS * sizeof(char));
        sprintf(buf[dline], mkeys[dline].key_and_name,
		(F_OFF(F_USE_FK,ps_global)
		 && km->keys[mkeys[dline].key_index].name)
		   ? km->keys[mkeys[dline].key_index].name : "",
		(ps_global->VAR_NEWS_SPEC && mkeys[dline].news_addition)
		  ? mkeys[dline].news_addition : "");

	if(longest < (indent = strlen(buf[dline])))
	  longest = indent;

	buf[dline][indent] = ' ';	/* buf's really tied off below */
    }

    indent = max(((ps_global->ttyo->screen_cols - longest)/2) - 1, 0);

    /* leave room for keymenu, status line, and trademark message */
    for(dline = 3;
	dline - 3 < sizeof(mkeys)/sizeof(mkeys[1])
	  && dline < ps_global->ttyo->screen_rows-(FOOTER_ROWS(ps_global)+1);
	dline++){
	if(quick_draw && !(dline-3 == last_inverse
			   || dline-3 == menu_index))
	  continue;

	if(dline-3 == menu_index)
	  StartInverse();

	buf[dline-3][min(ps_global->ttyo->screen_cols-indent,longest+1)]= '\0';
	pos.row = dline;
	pos.col = indent;
        PutLine0(pos.row, pos.col, buf[dline-3]);

	if(dline-3 == menu_index){
	    if(cursor_pos){
		cursor_pos->row = pos.row;
		cursor_pos->col = pos.col + 6;
		if(F_OFF(F_USE_FK,ps_global))
		  cursor_pos->col++;
	    }

	    EndInverse();
	}
    }

    last_inverse = menu_index;

    if(!quick_draw){	/* the devi.. uh, I mean, lawyer made me do it. */
	PutLine0(ps_global->ttyo->screen_rows - (FOOTER_ROWS(ps_global)+1),
		 3, LEGAL_NOTICE);
	if(strlen(LEGAL_NOTICE) + 3 > ps_global->ttyo->screen_cols)
	  mark_status_dirty();
    }

    fflush(stdout);
}


int
choose_setup_cmd(cmd, msgmap, sparms)
    int	       cmd;
    MSGNO_S   *msgmap;
    SCROLL_S  *sparms;
{
    int rv = 1;
    SRV_S *srv;

    if(!(srv = (SRV_S *)sparms->proc.data.p)){
	sparms->proc.data.p = (SRV_S *)fs_get(sizeof(*srv));
	srv = (SRV_S *)sparms->proc.data.p;
	memset(srv, 0, sizeof(*srv));
    }

    ps_global->next_screen = SCREEN_FUN_NULL;

    switch(cmd){
      case MC_PRINTER :
	srv->cmd = 'p';
	break;

      case MC_PASSWD :
	srv->cmd = 'n';
	break;

      case MC_CONFIG :
	srv->cmd = 'c';
	break;

      case MC_SIG :
	srv->cmd = 's';
	break;

      case MC_ABOOKS :
	srv->cmd = 'a';
	break;

      case MC_CLISTS :
	srv->cmd = 'l';
	break;

      case MC_RULES :
	srv->cmd = 'r';
	break;

      case MC_DIRECTORY :
	srv->cmd = 'd';
	break;

      case MC_KOLOR :
	srv->cmd = 'k';
	break;

      case MC_REMOTE :
	srv->cmd = 'z';
	break;

      case MC_EXCEPT :
	srv->exc = !srv->exc;
	menu_clear_binding(sparms->keys.menu, 'x');
	if(srv->exc){
	  if(sparms->bar.title) fs_give((void **)&sparms->bar.title);
	  sparms->bar.title = cpystr("SETUP EXCEPTIONS");
	  ps_global->mangled_header = 1;
	  menu_init_binding(sparms->keys.menu, 'x', MC_EXCEPT, "X",
			    "not eXceptions", SETUP_EXCEPT);
	}
	else{
	  if(sparms->bar.title) fs_give((void **)&sparms->bar.title);
	  sparms->bar.title = cpystr("SETUP");
	  ps_global->mangled_header = 1;
	  menu_init_binding(sparms->keys.menu, 'x', MC_EXCEPT, "X",
			    "eXceptions", SETUP_EXCEPT);
	}

	if(sparms->keys.menu->which == 1)
	  ps_global->mangled_footer = 1;

	rv = 0;
	break;

      case MC_NO_EXCEPT :
#if defined(DOS) || defined(OS2)
        q_status_message(SM_ORDER, 0, 2, "Need argument \"-x <except_config>\" or \"PINERCEX\" file to use eXceptions");
#else
        q_status_message(SM_ORDER, 0, 2, "Need argument \"-x <except_config>\" or \".pinercex\" file to use eXceptions");
#endif
	rv = 0;
	break;

      default:
	panic("Unexpected command in choose_setup_cmd");
	break;
    }

    return(rv);
}


int
setup_menu(ps)
    struct pine *ps;
{
    int         ret = 0, exceptions = 0;
    int         printer = 0, passwd = 0, config = 0, sig = 0, dir = 0, exc = 0;
    SCROLL_S	sargs;
    SRV_S      *srv;
    STORE_S    *store;

    if(!(store = so_get(CharStar, NULL, EDIT_ACCESS))){
	q_status_message(SM_ORDER | SM_DING, 3, 3, "Error allocating space.");
	return(ret);
    }

#if	!defined(DOS)
    if(!ps_global->vars[V_PRINTER].is_fixed)	/* printer can be changed */
      printer++;
#endif

#ifdef	PASSWD_PROG
    if(F_OFF(F_DISABLE_PASSWORD_CMD,ps_global))	/* password is allowed */
      passwd++;
#endif

    if(F_OFF(F_DISABLE_CONFIG_SCREEN,ps_global))	/* config allowed */
      config++;

    if(F_OFF(F_DISABLE_SIGEDIT_CMD,ps_global))	/* .sig editing is allowed */
      sig++;

#ifdef	ENABLE_LDAP
    dir++;
#endif

    if(ps_global->post_prc)
      exc++;

    so_puts(store, "This is the Setup screen for Pine. Choose from the following commands:\n");

    so_puts(store, "\n");
    so_puts(store, "(E) Exit Setup:\n");
    so_puts(store, "    This puts you back at the Main Menu.\n");

    if(exc){
	so_puts(store, "\n");
	so_puts(store, "(X) eXceptions:\n");
	so_puts(store, "    This command is different from the rest. It is not actually a command\n");
	so_puts(store, "    itself. Instead, it is a toggle which modifies the behavior of the\n");
	so_puts(store, "    other commands. You toggle Exceptions editing on and off with this\n");
	so_puts(store, "    command. When it is off you will be editing (changing) your regular\n");
	so_puts(store, "    configuration file. When it is on you will be editing your exceptions\n");
	so_puts(store, "    configuration file. For example, you might want to type the command \n");
	so_puts(store, "    \"eXceptions\" followed by \"Kolor\" to setup different screen colors\n");
	so_puts(store, "    on a particular platform.\n");
	so_puts(store, "    (Note: this command does not show up on the keymenu at the bottom of\n");
	so_puts(store, "    the screen unless you press \"O\" for \"Other Commands\" --but you don't\n");
	so_puts(store, "    need to press the \"O\" in order to invoke the command.)\n");
    }

    if(printer){
	so_puts(store, "\n");
	so_puts(store, "(P) Printer:\n");
	so_puts(store, "    Allows you to set a default printer and to define custom\n");
	so_puts(store, "    print commands.\n");
    }

    if(passwd){
	so_puts(store, "\n");
	so_puts(store, "(N) Newpassword:\n");
	so_puts(store, "    Change your password.\n");
    }

    if(config){
	so_puts(store, "\n");
	so_puts(store, "(C) Config:\n");
	so_puts(store, "    Allows you to set many features which are not turned on by default.\n");
	so_puts(store, "    You may also set the values of many options with that command.\n");
    }

    if(sig){
	so_puts(store, "\n");
	so_puts(store, "(S) Signature:\n");
	so_puts(store, "    Enter or edit a custom signature which will\n");
	so_puts(store, "    be included with each new message you send.\n");
    }

    so_puts(store, "\n");
    so_puts(store, "(A) AddressBooks:\n");
    so_puts(store, "    Define a non-default address book.\n");

    so_puts(store, "\n");
    so_puts(store, "(L) collectionLists:\n");
    so_puts(store, "    You may define groups of folders to help you better organize your mail.\n");

    so_puts(store, "\n");
    so_puts(store, "(R) Rules:\n");
    so_puts(store, "    This has up to five sub-categories: Roles, Index Colors, Filters,\n");
    so_puts(store, "    SetScores, and Other. If the Index Colors option is missing\n");
    so_puts(store, "    you may turn it on (if possible) with Setup/Kolor.\n");
    so_puts(store, "    If Roles is missing it has probably been administratively disabled.\n");

    if(dir){
	so_puts(store, "\n");
	so_puts(store, "(D) Directory:\n");
	so_puts(store, "    Define an LDAP Directory server for Pine's use. A directory server is\n");
	so_puts(store, "    similar to an address book, but it is usually maintained by an\n");
	so_puts(store, "    organization. It is similar to a telephone directory.\n");
    }

    so_puts(store, "\n");
    so_puts(store, "(K) Kolor:\n");
    so_puts(store, "    Set custom colors for various parts of the Pine screens. For example, the\n");
    so_puts(store, "    command key labels, the titlebar at the top of each page, and quoted\n");
    so_puts(store, "    sections of messages you are viewing.\n");

    so_puts(store, "\n");
    so_puts(store, "(Z) RemoteConfigSetup:\n");
    so_puts(store, "    This is a command you will probably only want to use once, if at all.\n");
    so_puts(store, "    It helps you transfer your Pine configuration data to an IMAP server,\n");
    so_puts(store, "    where it will be accessible from any of the computers you read mail\n");
    so_puts(store, "    from (using Pine). The idea behind a remote configuration is that you\n");
    so_puts(store, "    can change your configuration in one place and have that change show\n");
    so_puts(store, "    up on all of the computers you use.\n");
    so_puts(store, "    (Note: this command does not show up on the keymenu at the bottom of\n");
    so_puts(store, "    the screen unless you press \"O\" for \"Other Commands\" --but you don't\n");
    so_puts(store, "    need to press the \"O\" in order to invoke the command.)\n");

    /* put this down here for people who don't have exceptions */
    if(!exc){
	so_puts(store, "\n");
	so_puts(store, "(X) eXceptions:\n");
	so_puts(store, "    This command is different from the rest. It is not actually a command\n");
	so_puts(store, "    itself. Instead, it is a toggle which modifies the behavior of the\n");
	so_puts(store, "    other commands. You toggle Exceptions editing on and off with this\n");
	so_puts(store, "    command. When it is off you will be editing (changing) your regular\n");
	so_puts(store, "    configuration file. When it is on you will be editing your exceptions\n");
	so_puts(store, "    configuration file. For example, you might want to type the command \n");
	so_puts(store, "    \"eXceptions\" followed by \"Kolor\" to setup different screen colors\n");
	so_puts(store, "    on a particular platform.\n");
	so_puts(store, "    (Note: this command does not do anything unless you have a configuration\n");
	so_puts(store, "    with exceptions enabled (you don't have that). Common ways to enable an\n");
	so_puts(store, "    exceptions config are the command line argument \"-x <exception_config>\";\n");
	so_puts(store, "    or the existence of the file \".pinercex\" for Unix Pine, or \"PINERCEX\")\n");
	so_puts(store, "    for PC-Pine.)\n");
	so_puts(store, "    (Another note: this command does not show up on the keymenu at the bottom\n");
	so_puts(store, "    of the screen unless you press \"O\" for \"Other Commands\" --but you\n");
	so_puts(store, "    don't need to press the \"O\" in order to invoke the command.)\n");
    }

    memset(&sargs, 0, sizeof(SCROLL_S));
    sargs.text.text   = so_text(store);
    sargs.text.src    = CharStar;
    sargs.text.desc   = "Information About Setup Command";
    sargs.bar.title   = cpystr("SETUP");
    sargs.proc.tool   = choose_setup_cmd;
    sargs.help.text   = NO_HELP;
    sargs.help.title  = NULL;
    sargs.keys.menu   = &choose_setup_keymenu;
    sargs.keys.menu->how_many = 2;

    setbitmap(sargs.keys.bitmap);
    if(!printer)
      clrbitn(SETUP_PRINTER, sargs.keys.bitmap);

    if(!passwd)
      clrbitn(SETUP_PASSWD, sargs.keys.bitmap);

    if(!config)
      clrbitn(SETUP_CONFIG, sargs.keys.bitmap);

    if(!sig)
      clrbitn(SETUP_SIG, sargs.keys.bitmap);

    if(!dir)
      clrbitn(SETUP_DIRECTORY, sargs.keys.bitmap);

    if(exc)
      menu_init_binding(sargs.keys.menu, 'x', MC_EXCEPT, "X",
			"eXceptions", SETUP_EXCEPT);
    else
      menu_init_binding(sargs.keys.menu, 'x', MC_NO_EXCEPT, "X",
			"eXceptions", SETUP_EXCEPT);


    scrolltool(&sargs);

    ps->mangled_screen = 1;

    srv = (SRV_S *)sargs.proc.data.p;

    exceptions = srv ? srv->exc : 0;

    so_give(&store);

    if(sargs.bar.title) fs_give((void**)&sargs.bar.title);
    if(srv){
	ret = srv->cmd;
	fs_give((void **)&sargs.proc.data.p);
    }
    else
      ret = 'e';

    return(ret | (exceptions ? EDIT_EXCEPTION : 0));
}


/*----------------------------------------------------------------------

Args: command -- command char to perform

  ----*/
void
do_setup_task(command)
    int command;
{
    char *err = NULL;
    int   rtype;
    int   edit_exceptions = 0;
    int   do_lit_sig = 0;

    if(command & EDIT_EXCEPTION){
	edit_exceptions = 1;
	command &= ~EDIT_EXCEPTION;
    }

    switch(command) {
        /*----- EDIT SIGNATURE -----*/
      case 's':
	if(ps_global->VAR_LITERAL_SIG)
	  do_lit_sig = 1;
	else {
	    char sig_path[MAXPATH+1];

	    if(!signature_path(ps_global->VAR_SIGNATURE_FILE, sig_path, MAXPATH))
	      do_lit_sig = 1;
	    else if((!IS_REMOTE(ps_global->VAR_SIGNATURE_FILE)
		     && can_access(sig_path, READ_ACCESS) == 0)
		    ||(IS_REMOTE(ps_global->VAR_SIGNATURE_FILE)
		       && (folder_exists(NULL, sig_path) & FEX_ISFILE)))
	      do_lit_sig = 0;
	    else if(!ps_global->vars[V_SIGNATURE_FILE].main_user_val.p
		    && !ps_global->vars[V_SIGNATURE_FILE].cmdline_val.p
		    && !ps_global->vars[V_SIGNATURE_FILE].fixed_val.p)
	      do_lit_sig = 1;
	    else
	      do_lit_sig = 0;
	}

	if(do_lit_sig){
	    char     *result = NULL;
	    char    **apval;
	    EditWhich ew;
	    int       readonly = 0;

	    ew = edit_exceptions ? ps_global->ew_for_except_vars : Main;

	    if(ps_global->restricted)
	      readonly = 1;
	    else switch(ew){
		   case Main:
		     readonly = ps_global->prc->readonly;
		     break;
		   case Post:
		     readonly = ps_global->post_prc->readonly;
		     break;
	    }

	    if(readonly)
	      err = cpystr(ps_global->restricted
				     ? "Pine demo can't change config file"
				     : "Config file not changeable");

	    if(!err){
		apval = APVAL(&ps_global->vars[V_LITERAL_SIG], ew);
		if(!apval)
		  err = cpystr("Problem accessing configuration");
		else{
		    char *input;

		    input = (char *)fs_get((strlen(*apval ? *apval : "")+1) *
								sizeof(char));
		    input[0] = '\0';
		    cstring_to_string(*apval, input);
		    err = signature_edit_lit(input, &result,
					     "SIGNATURE EDITOR");
		    fs_give((void **)&input);
		}
	    }

	    if(!err){
		char *cstring_version;

		cstring_version = string_to_cstring(result);

		set_variable(V_LITERAL_SIG, cstring_version, 0, 0, ew);

		if(cstring_version)
		  fs_give((void **)&cstring_version);
	    }

	    if(result)
	      fs_give((void **)&result);
	}
	else
	    err = signature_edit(ps_global->VAR_SIGNATURE_FILE,
				 "SIGNATURE EDITOR");

	if(err){
	    q_status_message(SM_ORDER, 3, 4, err);
	    fs_give((void **)&err);
	}

	ps_global->mangled_screen = 1;
	break;

        /*----- ADD ADDRESSBOOK ----*/
      case 'a':
	addr_book_config(ps_global, edit_exceptions);
	menu_index = ABOOK_MENU_ITEM;
	ps_global->mangled_screen = 1;
	break;

#ifdef	ENABLE_LDAP
        /*--- ADD DIRECTORY SERVER --*/
      case 'd':
	directory_config(ps_global, edit_exceptions);
	ps_global->mangled_screen = 1;
	break;
#endif

        /*----- CONFIGURE OPTIONS -----*/
      case 'c':
	option_screen(ps_global, edit_exceptions);
	ps_global->mangled_screen = 1;
	break;

        /*----- COLLECTION LIST -----*/
      case 'l':
	folder_config_screen(ps_global, edit_exceptions);
	ps_global->mangled_screen = 1;
	break;

        /*----- RULES -----*/
      case 'r':
	rtype = rule_setup_type(ps_global, RS_RULES, "Type of rule setup : ");
	switch(rtype){
	  case 'r':
	  case 's':
	  case 'i':
	  case 'f':
	  case 'o':
	    role_config_screen(ps_global, (rtype == 'r') ? ROLE_DO_ROLES :
					   (rtype == 's') ? ROLE_DO_SCORES :
					    (rtype == 'o') ? ROLE_DO_OTHER :
					     (rtype == 'f') ? ROLE_DO_FILTER :
							       ROLE_DO_INCOLS,
			       edit_exceptions);
	    break;

	  case 'Z':
	    q_status_message(SM_ORDER | SM_DING, 3, 5,
			"Try turning on color with the Setup/Kolor command.");
	    break;

	  default:
	    cmd_cancelled(NULL);
	    break;
	}

	ps_global->mangled_screen = 1;
	break;

        /*----- COLOR -----*/
      case 'k':
	color_config_screen(ps_global, edit_exceptions);
	ps_global->mangled_screen = 1;
	break;

      case 'z':
	convert_to_remote_config(ps_global, edit_exceptions);
	ps_global->mangled_screen = 1;
	break;

        /*----- EXIT -----*/
      case 'e':
        break;

        /*----- NEW PASSWORD -----*/
      case 'n':
#ifdef	PASSWD_PROG
        if(ps_global->restricted){
	    q_status_message(SM_ORDER, 3, 5,
	    "Password change unavailable in restricted demo version of Pine.");
        }else {
	    change_passwd();
	    ClearScreen();
	    ps_global->mangled_screen = 1;
	}
#else
        q_status_message(SM_ORDER, 0, 5,
		 "Password changing not configured for this version of Pine.");
	display_message('x');
#endif	/* DOS */
        break;

#if	!defined(DOS)
        /*----- CHOOSE PRINTER ------*/
      case 'p':
        select_printer(ps_global, edit_exceptions); 
	ps_global->mangled_screen = 1;
        break;
#endif
    }
}


int
rule_setup_type(ps, flags, prompt)
    struct pine *ps;
    int          flags;
    char        *prompt;
{
    ESCKEY_S opts[8];
    int ekey_num = 0, deefault = 0;

    if(flags & RS_INCADDR){
	deefault = 'a';
	opts[ekey_num].ch      = 'a';
	opts[ekey_num].rval    = 'a';
	opts[ekey_num].name    = "A";
	opts[ekey_num++].label = "Addrbook";
    }

  if(flags & RS_RULES){

    if(F_OFF(F_DISABLE_ROLES_SETUP,ps)){ /* roles are allowed */
	if(deefault != 'a')
	  deefault = 'r';

	opts[ekey_num].ch      = 'r';
	opts[ekey_num].rval    = 'r';
	opts[ekey_num].name    = "R";
	opts[ekey_num++].label = "Roles";
    }
    else if(deefault != 'a')
      deefault = 's';

    opts[ekey_num].ch      = 's';
    opts[ekey_num].rval    = 's';
    opts[ekey_num].name    = "S";
    opts[ekey_num++].label = "SetScores";

#ifndef	_WINDOWS
    if(ps->color_style != COL_NONE && pico_hascolor()){
#endif
	if(deefault != 'a')
	  deefault = 'i';

	opts[ekey_num].ch      = 'i';
	opts[ekey_num].rval    = 'i';
	opts[ekey_num].name    = "I";
	opts[ekey_num++].label = "Indexcolor";
#ifndef	_WINDOWS
    }
    else{
	opts[ekey_num].ch      = 'i';
	opts[ekey_num].rval    = 'Z';		/* notice this rval! */
	opts[ekey_num].name    = "I";
	opts[ekey_num++].label = "Indexcolor";
    }
#endif

    opts[ekey_num].ch      = 'f';
    opts[ekey_num].rval    = 'f';
    opts[ekey_num].name    = "F";
    opts[ekey_num++].label = "Filters";

    opts[ekey_num].ch      = 'o';
    opts[ekey_num].rval    = 'o';
    opts[ekey_num].name    = "O";
    opts[ekey_num++].label = "Other";

  }

    if(flags & RS_INCEXP){
	opts[ekey_num].ch      = 'e';
	opts[ekey_num].rval    = 'e';
	opts[ekey_num].name    = "E";
	opts[ekey_num++].label = "Export";
    }

    opts[ekey_num].ch    = -1;

    return(radio_buttons(prompt, -FOOTER_ROWS(ps), opts,
			 deefault, 'x', NO_HELP, RB_NORM));
}


/*
 * Make sure any errors during initialization get queued for display
 */
void
queue_init_errors(ps)
    struct pine *ps;
{
    int i;

    if(ps->init_errs){
	for(i = 0; (ps->init_errs)[i].message; i++){
	    q_status_message((ps->init_errs)[i].flags,
			     (ps->init_errs)[i].min_time,
			     (ps->init_errs)[i].max_time,
			     (ps->init_errs)[i].message);
	    fs_give((void **)&(ps->init_errs)[i].message);
	}

	fs_give((void **)&ps->init_errs);
    }
}



/*
 *  * * * * * * * *      New User or Version routines      * * * * * * * *
 *
 */
static struct key nuov_keys[] = {
    HELP_MENU,
    NULL_MENU,
    {"E",NULL,{MC_EXIT,1,{'e',ctrl('M'),ctrl('J')}},KS_NONE},
    {"Ret","[Be Counted!]",{MC_VIEW_HANDLE,2,{ctrl('M'),ctrl('J')}},KS_NONE},
    NULL_MENU,
    NULL_MENU,
    PREVPAGE_MENU,
    NEXTPAGE_MENU,
    PRYNTMSG_MENU,
    NULL_MENU,
    {"R","RelNotes",{MC_RELNOTES,1,{'r'}},KS_NONE},
    NULL_MENU};
INST_KEY_MENU(nuov_keymenu, nuov_keys);
#define	NUOV_EXIT	2
#define	NUOV_VIEW	3
#define	NUOV_NEXT_PG	6
#define	NUOV_PREV_PG	7
#define	NUOV_RELNOTES	10



/*
 * Display a new user or new version message.
 */
void
new_user_or_version(ps)
    struct pine *ps;
{
    char	  **shown_text;
    int		    cmd = MC_NONE;
    char	   *error = NULL;
    HelpType	    text;
    SCROLL_S	    sargs;
    STORE_S	   *store;
    HANDLE_S	   *handles = NULL, *htmp;
    gf_io_t	    pc;
#ifdef	HELPFILE
    char	  **dynamic_text;
#endif /* HELPFILE */

    text = ps->first_time_user ? new_user_greeting : new_version_greeting;
#ifdef	HELPFILE
    if((shown_text = dynamic_text = get_help_text(text)) == NULL)
      return;
#else
    shown_text = text;
#endif /* HELPFILE */

    /*
     * You may think this is weird.  We're trying to offer sending
     * the "phone home" message iff there's no evidence we've ever
     * run pine *or* this is the first time we've run a version
     * a version of pine >= 3.90.  The check for the existence of a var
     * new in 3.90, is to compensate for pre 3.90 pine's rewriting their
     * lower version number.
     */
    ps->phone_home = (ps->first_time_user
		      || (ps->pre390
			  && !var_in_pinerc(ps->vars[V_NNTP_SERVER].name))
		      || !var_in_pinerc(ps->vars[V_MAILCHECK].name)  /* 3.92 */
		      || !var_in_pinerc(ps->vars[V_BROWSER].name)    /* 4.00 */
		      || !var_in_pinerc(ps->vars[V_PATTERNS].name)   /* 4.10 */
								     /*4.20*/
		      || !var_in_pinerc(ps->vars[V_TITLE_FORE_COLOR].name)
		      || !var_in_pinerc(ps->vars[V_PAT_ROLES].name));/* 4.30 */


    /*
     * At this point, shown_text is a charstarstar with html
     * Turn it into a charstar with digested html
     */
    do{
	init_helper_getc(shown_text);
	init_handles(&handles);

	if(store = so_get(CharStar, NULL, EDIT_ACCESS)){
	    gf_set_so_writec(&pc, store);
	    gf_filter_init();

	    gf_link_filter(gf_html2plain,
			   gf_html2plain_opt("x-pine-help:",
					   ps->ttyo->screen_cols,
					   &handles, GFHP_LOCAL_HANDLES));

	    error = gf_pipe(helper_getc, pc);

	    gf_clear_so_writec(store);

	    if(!error){
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
		sargs.text.desc	   = "greeting text";
		sargs.text.handles = handles;
		sargs.bar.title	   = "GREETING TEXT";
		sargs.bar.style	   = TextPercent;
		sargs.proc.tool	   = nuov_processor;
		sargs.help.text	   = main_menu_tx;
		sargs.help.title   = "MAIN PINE HELP";
		sargs.resize_exit  = 1;
		sargs.keys.menu	   = &km;
		km		   = nuov_keymenu;
		km.keys		   = keys;
		memcpy(&keys[0], nuov_keymenu.keys,
		       (nuov_keymenu.how_many * 12) * sizeof(struct key));
		setbitmap(sargs.keys.bitmap);

		if(ps->phone_home){
		    km.keys[NUOV_EXIT].label = "Exit this greeting";
		    km.keys[NUOV_EXIT].bind.nch = 1;
		}
		else{
		    km.keys[NUOV_EXIT].label	= "[Exit this greeting]";
		    km.keys[NUOV_EXIT].bind.nch = 3;
		    clrbitn(NUOV_VIEW, sargs.keys.bitmap);
		}

		if(ps->first_time_user)
		  clrbitn(NUOV_RELNOTES, sargs.keys.bitmap);

		cmd = scrolltool(&sargs);

		flush_input();

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

    /*
     * Check to see if we have an old style postponed mail folder and
     * that there isn't a new style postponed folder.  If true,
     * fix up the old one, and move it into postition for pine >= 3.90...
     */
    if(!ps_global->first_time_user && ps_global->pre390)
      upgrade_old_postponed();

    /*
     * Check to see if they've never run a 4.x version.  If true,
     * and they have expanded-view set, then silently enable
     * combined-* features...
     */
    if(!var_in_pinerc(ps->vars[V_BROWSER].name)){	/* 4.00 */
	char ***alval;

	if(F_ON(F_EXPANDED_FOLDERS, ps_global)){
	    F_SET(F_CMBND_FOLDER_DISP, ps_global, 1);
	    alval = ALVAL(&ps_global->vars[V_FEATURE_LIST], Main);
	    set_feature(alval, "combined-folder-display", 1);
	}

	if(F_ON(F_EXPANDED_ADDRBOOKS, ps_global)){
	    F_SET(F_CMBND_ABOOK_DISP, ps_global, 1);
	    alval = ALVAL(&ps_global->vars[V_FEATURE_LIST], Main);
	    set_feature(alval, "combined-addrbook-display", 1);
	}

	if(F_ON(F_CMBND_FOLDER_DISP, ps_global)
	   || F_ON(F_CMBND_ABOOK_DISP, ps_global))
	  write_pinerc(ps_global, Main);
    }
}


int
nuov_processor(cmd, msgmap, sparms)
    int	      cmd;
    MSGNO_S  *msgmap;
    SCROLL_S *sparms;
{
    int rv = 0;

    switch(cmd){
	/*----------- Print all the help ------------*/
      case MC_PRINTMSG :
	if(open_printer(sparms->text.desc) == 0){
	    print_help(sparms->proc.data.p);
	    close_printer();
	}

	break;

      case MC_RELNOTES :
	helper(h_news, "PINE RELEASE NOTES", 0);
	ps_global->mangled_screen = 1;
	break;

      case MC_EXIT :
	rv = 1;
	break;

      default :
	panic("Unhandled case");
    }

    return(rv);
}



void
upgrade_old_postponed()
{
    int	       i;
    char       file_path[MAXPATH], *status, buf[6];
    STORE_S   *in_so, *out_so;
    CONTEXT_S *save_cntxt = default_save_context(ps_global->context_list);
    STRING     msgtxt;
    gf_io_t    pc, gc;

    /*
     * NOTE: woe to he who redefines things in os.h such that
     * the new and old pastponed folder names are the same.
     * If so, you're on your own...
     */
    build_path(file_path, ps_global->folders_dir, POSTPONED_MAIL,
	       sizeof(file_path));
    if(in_so = so_get(FileStar, file_path, READ_ACCESS)){
	for(i = 0; i < 5 && so_readc((unsigned char *)&buf[i], in_so); i++)
	  ;

	buf[i] = '\0';
	if(strncmp(buf, "From ", 5)){
	    dprint(1, (debugfile,
		       "POSTPONED conversion %s --> <%s>%s\n",
		       file_path,save_cntxt->context,
		       ps_global->VAR_POSTPONED_FOLDER));
	    so_seek(in_so, 0L, 0);
	    if((out_so = so_get(CharStar, NULL, WRITE_ACCESS))
	       && ((folder_exists(save_cntxt,
				ps_global->VAR_POSTPONED_FOLDER) & FEX_ISFILE)
		   || context_create(save_cntxt, NULL,
				     ps_global->VAR_POSTPONED_FOLDER))){
		gf_set_so_readc(&gc, in_so);
		gf_set_so_writec(&pc, out_so);
		gf_filter_init();
		gf_link_filter(gf_local_nvtnl, NULL);
		if(!(status = gf_pipe(gc, pc))){
		    so_seek(out_so, 0L, 0);	/* just in case */
		    INIT(&msgtxt, mail_string, so_text(out_so),
			 strlen((char *)so_text(out_so)));

		    if(context_append(save_cntxt, NULL,
				      ps_global->VAR_POSTPONED_FOLDER,
				      &msgtxt)){
			so_give(&in_so);
			unlink(file_path);
		    }
		    else{
			q_status_message(SM_ORDER | SM_DING, 3, 5,
					"Problem upgrading postponed message");
			dprint(1, (debugfile,
				   "Conversion failed: Can't APPEND\n"));
		    }
		}
		else{
		    q_status_message(SM_ORDER | SM_DING, 3, 5,
				     "Problem upgrading postponed message");
		    dprint(1,(debugfile,"Conversion failed: %s\n",status));
		}

		gf_clear_so_readc(in_so);
		gf_clear_so_writec(out_so);
	    }
	    else{
		q_status_message(SM_ORDER | SM_DING, 3, 5,
				 "Problem upgrading postponed message");
		dprint(1, (debugfile,
			   "Conversion failed: Can't create %s\n",
			   (out_so) ? "new postponed folder"
				    : "temp storage object"));
	    }

	    if(out_so)
	      so_give(&out_so);
	}

	if(in_so)
	  so_give(&in_so);
    }
}



/*----------------------------------------------------------------------
          Quit pine if the user wants to 

    Args: The usual pine structure

  Result: User is asked if she wants to quit, if yes then execute quit.

       Q U I T    S C R E E N

Not really a full screen. Just count up deletions and ask if we really
want to quit.
  ----*/
void
quit_screen(pine_state)
    struct pine *pine_state;
{
    int quit = 0;

    dprint(1, (debugfile, "\n\n    ---- QUIT SCREEN ----\n"));    

    if(F_ON(F_CHECK_MAIL_ONQUIT,ps_global)
       && new_mail(1, VeryBadTime, NM_STATUS_MSG | NM_DEFER_SORT) >= 0
       && (quit = want_to("Quit even though new mail just arrived", 'y', 0,
			  NO_HELP, WT_NORM)) != 'y'){
	refresh_sort(pine_state->msgmap, SRT_VRB);
        pine_state->next_screen = pine_state->prev_screen;
        return;
    }

    if(quit != 'y'
       && F_OFF(F_QUIT_WO_CONFIRM,pine_state)
       && want_to("Really quit pine", 'y', 0, NO_HELP, WT_NORM) != 'y'){
        pine_state->next_screen = pine_state->prev_screen;
        return;
    }

    goodnight_gracey(pine_state, 0);
}



/*----------------------------------------------------------------------
    The nuts and bolts of actually cleaning up and exitting pine

    Args: ps -- the usual pine structure, 
	  exit_val -- what to tell our parent

  Result: This never returns

  ----*/
void
goodnight_gracey(pine_state, exit_val)
    struct pine *pine_state;
    int		 exit_val;
{
    int   i, cur_is_inbox;
    char *final_msg = NULL;
    char  msg[MAX_SCREEN_COLS+1];
    char *pf = "Pine finished";
    extern KBESC_T *kbesc;

    dprint(2, (debugfile, "goodnight_gracey:\n"));    

    completely_done_with_adrbks();

    cur_is_inbox = (pine_state->inbox_stream == pine_state->mail_stream);

    /* clean up open streams */
    dprint(5, (debugfile, "goodnight_gracey: close open streams\n"));    
    if(pine_state->mail_stream)
      expunge_and_close(pine_state->mail_stream, pine_state->context_current,
			pine_state->cur_folder,
			(!pine_state->inbox_stream || cur_is_inbox)
			  ? &final_msg : NULL);
    if(pine_state->msgmap)
      mn_give(&pine_state->msgmap);

    pine_state->redrawer = (void (*)())NULL;

    if(pine_state->inbox_stream && !cur_is_inbox){
	pine_state->mail_stream = pine_state->inbox_stream;
	pine_state->msgmap      = pine_state->inbox_msgmap;
	pine_state->expunge_count = 0L;
	pine_state->dead_stream = pine_state->dead_inbox;
        expunge_and_close(pine_state->inbox_stream, NULL,
			  pine_state->inbox_name, &final_msg);
	mn_give(&pine_state->msgmap);
    }

    pine_state->inbox_stream = pine_state->mail_stream = NULL;

#ifdef _WINDOWS
    if(ps_global->ttyo)
      (void)get_windsize(ps_global->ttyo);
#endif

    dprint(7, (debugfile, "goodnight_gracey: close config files\n"));    

    if(pine_state->prc){
	if(pine_state->prc->outstanding_pinerc_changes)
	  write_pinerc(pine_state, Main);

	if(pine_state->prc->rd)
	  rd_close_remdata(&pine_state->prc->rd);
	
	free_pinerc_s(&pine_state->prc);
    }

    if(pine_state->pconf)
      free_pinerc_s(&pine_state->pconf);

    if(pine_state->post_prc){
	if(pine_state->post_prc->outstanding_pinerc_changes)
	  write_pinerc(pine_state, Post);

	if(pine_state->post_prc->rd)
	  rd_close_remdata(&pine_state->post_prc->rd);
	
	free_pinerc_s(&pine_state->post_prc);
    }

    if(final_msg){
	strcpy(msg, pf);
	strcat(msg, " -- ");
	strncat(msg, final_msg, sizeof(msg)-1-strlen(msg));
	fs_give((void **)&final_msg);
    }
    else
      strcpy(msg, pf);

    end_stream_cache();
    end_screen(msg, exit_val);
    end_titlebar();
    end_keymenu();

    end_keyboard(F_ON(F_USE_FK,pine_state));
    end_tty_driver(pine_state);
#if !defined(DOS) && !defined(OS2)
    kbdestroy(kbesc);
#endif
    end_signals(0);
    if(filter_data_file(0))
      unlink(filter_data_file(0));

    imap_flush_passwd_cache();
    clear_index_cache();
    free_newsgrp_cache();
    mailcap_free();
    close_every_pattern();
    free_extra_hdrs();
    free_contexts(&ps_global->context_list);
    dprint(7, (debugfile, "goodnight_gracey: end_status_review\n"));    
    end_status_review();
    dprint(7, (debugfile, "goodnight_gracey: free more memory\n"));    
#ifdef	ENABLE_LDAP
    free_saved_query_parameters();
#endif

    if(pine_state->hostname != NULL)
      fs_give((void **)&pine_state->hostname);
    if(pine_state->localdomain != NULL)
      fs_give((void **)&pine_state->localdomain);
    if(pine_state->ttyo != NULL)
      fs_give((void **)&pine_state->ttyo);
    if(pine_state->home_dir != NULL)
      fs_give((void **)&pine_state->home_dir);
    if(pine_state->folders_dir != NULL)
      fs_give((void **)&pine_state->folders_dir);
    if(pine_state->ui.homedir)
      fs_give((void **)&pine_state->ui.homedir);
    if(pine_state->ui.login)
      fs_give((void **)&pine_state->ui.login);
    if(pine_state->ui.fullname)
      fs_give((void **)&pine_state->ui.fullname);
    if(pine_state->index_disp_format)
      fs_give((void **)&pine_state->index_disp_format);
    if(pine_state->conv_table){
	if(pine_state->conv_table->table)
	  fs_give((void **) &pine_state->conv_table->table);
	
	if(pine_state->conv_table->from_charset)
	  fs_give((void **) &pine_state->conv_table->from_charset);
	
	if(pine_state->conv_table->to_charset)
	  fs_give((void **) &pine_state->conv_table->to_charset);

	fs_give((void **)&pine_state->conv_table);
    }

    if(pine_state->pinerc)
      fs_give((void **)&pine_state->pinerc);
#if defined(DOS) || defined(OS2)
    if(pine_state->pine_dir)
      fs_give((void **)&pine_state->pine_dir);
    if(pine_state->aux_files_dir)
      fs_give((void **)&pine_state->aux_files_dir);
#endif
#ifdef PASSFILE
    if(pine_state->passfile)
      fs_give((void **)&pine_state->passfile);
#endif /* PASSFILE */

    if(ps_global->hdr_colors)
      free_hdr_colors(&ps_global->hdr_colors);

    if(ps_global->atmts){
	for(i = 0; ps_global->atmts[i].description; i++){
	    fs_give((void **)&ps_global->atmts[i].description);
	    fs_give((void **)&ps_global->atmts[i].number);
	}

	fs_give((void **)&ps_global->atmts);
    }
    
    dprint(7, (debugfile, "goodnight_gracey: free_vars\n"));    
    free_vars(pine_state);

    dprint(2, (debugfile, "goodnight_gracey finished\n"));    

    fs_give((void **)&pine_state);

#ifdef DEBUG
    if(debugfile)
      fclose(debugfile);
#endif    

    exit(exit_val);
}


/*----------------------------------------------------------------------
  Return sequence number based on given index that are search-worthy

    Args: stream --
	  msgmap --
	  msgno --
	  flags -- flags for msgline_hidden

  Result:  0		     : index not search-worthy
	  -1		     : index out of bounds
	   1 - stream->nmsgs : sequence number to flag search

  ----*/
long
flag_search_sequence(stream, msgmap, msgno, flags)
    MAILSTREAM *stream;
    MSGNO_S    *msgmap;
    long	msgno;
    int         flags;
{
    long rawno = msgno;

    return((msgno > stream->nmsgs
	    || msgno <= 0
            || (msgmap && !(rawno = mn_m2raw(msgmap, msgno))))
	      ? -1L		/* out of range! */
	      : ((get_lflag(stream, NULL, rawno, MN_EXLD)
		  || (msgmap && msgline_hidden(stream, msgmap, msgno, flags)))
		    ? 0L		/* NOT interesting! */
		    : rawno));
}



/*----------------------------------------------------------------------
  Perform mail_search based on flag bits

    Args: stream --
	  flags --
	  start --
	  msgmap --

  Result: if no set_* specified, call mail_search to light the searched
	  bit for all the messages matching the given flags.  If set_start
	  specified, it is an index (possibly into set_msgmap) telling
	  us where to search for invalid flag state hence when we
	  return everything with the searched bit is interesting and
	  everything with the valid bit lit is believably valid.

  ----*/
void
flag_search(stream, flags, set_start, set_msgmap, ping)
    MAILSTREAM *stream;
    int		flags;
    MsgNo	set_start;
    MSGNO_S    *set_msgmap;
    long      (*ping) PROTO((MAILSTREAM *));
{
    long	        n, i, new;
    char	       *seq;
    SEARCHPGM	       *pgm;
    SEARCHSET	       *full_set = NULL, **set;
    MESSAGECACHE       *mc;
    extern  MAILSTREAM *mm_search_stream;

    new = ps_global->new_mail_count;

    /* Anything we don't already have flags for? */
    if(set_start){
	/*
	 * Use elt's sequence bit to coalesce runs in ascending
	 * sequence order...
	 */
	for(i = 1L; i <= stream->nmsgs; i++)
	  mail_elt(stream, i)->sequence = 0;

	for(i = set_start;
	    (n = flag_search_sequence(stream, set_msgmap, i, MH_ANYTHD)) >= 0L;
	    (flags & F_SRCHBACK) ? i-- : i++)
	  if(n > 0L && !(mc = mail_elt(stream, n))->valid)
	    mc->sequence = 1;

	/* Unroll searchset in ascending sequence order */
	set = &full_set;
	for(i = 1L; i <= stream->nmsgs; i++)
	  if(mail_elt(stream, i)->sequence){
	      if(*set){
		  if(((*set)->last ? (*set)->last : (*set)->first)+1L == i)
		    (*set)->last = i;
		  else
		    set = &(*set)->next;
	      }

	      if(!*set){
		  *set = mail_newsearchset();
		  (*set)->first = i;
	      }
	  }

	/*
	 * No search-worthy messsages?, prod the server for
	 * any flag updates and clear the searched bits...
	 */
	if(full_set){
	    if(full_set->first == 1
	       && full_set->last == stream->nmsgs
	       && full_set->next == NULL)
	      mail_free_searchset(&full_set);
	}
	else{
	    for(i = 1L; i <= stream->nmsgs; i++)
	      mail_elt(stream, i)->searched = 0;

	    if(ping)
	      (*ping)(stream);	/* prod server for any flag updates */

	    if(new != ps_global->new_mail_count){
		process_filter_patterns(stream, ps_global->msgmap,
					ps_global->new_mail_count);

		refresh_sort(ps_global->msgmap, SRT_NON);
		flag_search(stream, flags, set_start, set_msgmap, ping);
	    }

	    return;
	}
    }

    if((!is_imap_stream(stream) || modern_imap_stream(stream))
       && !(IS_NEWS(stream))){
	pgm = mail_newsearchpgm();

	if(flags & F_SEEN)
	  pgm->seen = 1;

	if(flags & F_UNSEEN)
	  pgm->unseen = 1;

	if(flags & F_DEL)
	  pgm->deleted = 1;

	if(flags & F_UNDEL)
	  pgm->undeleted = 1;

	if(flags & F_ANS)
	  pgm->answered = 1;

	if(flags & F_UNANS)
	  pgm->unanswered = 1;

	if(flags & F_FLAG)
	  pgm->flagged = 1;

	if(flags & F_UNFLAG)
	  pgm->unflagged = 1;

	if(flags & F_RECENT)
	  pgm->recent = 1;

	if(flags & F_OR_SEEN){
	    SEARCHPGM *pgm2 = mail_newsearchpgm();
	    pgm2->or = mail_newsearchor();
	    pgm2->or->first = pgm;
	    pgm2->or->second = mail_newsearchpgm();
	    pgm2->or->second->seen = 1;
	    pgm = pgm2;
	}

	if(flags & F_OR_UNSEEN){
	    SEARCHPGM *pgm2 = mail_newsearchpgm();
	    pgm2->or = mail_newsearchor();
	    pgm2->or->first = pgm;
	    pgm2->or->second = mail_newsearchpgm();
	    pgm2->or->second->unseen = 1;
	    pgm = pgm2;
	}

	if(flags & F_OR_DEL){
	    SEARCHPGM *pgm2 = mail_newsearchpgm();
	    pgm2->or = mail_newsearchor();
	    pgm2->or->first = pgm;
	    pgm2->or->second = mail_newsearchpgm();
	    pgm2->or->second->deleted = 1;
	    pgm = pgm2;
	}

	if(flags & F_OR_UNDEL){
	    SEARCHPGM *pgm2 = mail_newsearchpgm();
	    pgm2->or = mail_newsearchor();
	    pgm2->or->first = pgm;
	    pgm2->or->second = mail_newsearchpgm();
	    pgm2->or->second->undeleted = 1;
	    pgm = pgm2;
	}

	if(flags & F_OR_FLAG){
	    SEARCHPGM *pgm2 = mail_newsearchpgm();
	    pgm2->or = mail_newsearchor();
	    pgm2->or->first = pgm;
	    pgm2->or->second = mail_newsearchpgm();
	    pgm2->or->second->flagged = 1;
	    pgm = pgm2;
	}

	if(flags & F_OR_UNFLAG){
	    SEARCHPGM *pgm2 = mail_newsearchpgm();
	    pgm2->or = mail_newsearchor();
	    pgm2->or->first = pgm;
	    pgm2->or->second = mail_newsearchpgm();
	    pgm2->or->second->unflagged = 1;
	    pgm = pgm2;
	}

	if(flags & F_OR_ANS){
	    SEARCHPGM *pgm2 = mail_newsearchpgm();
	    pgm2->or = mail_newsearchor();
	    pgm2->or->first = pgm;
	    pgm2->or->second = mail_newsearchpgm();
	    pgm2->or->second->answered = 1;
	    pgm = pgm2;
	}

	if(flags & F_OR_UNANS){
	    SEARCHPGM *pgm2 = mail_newsearchpgm();
	    pgm2->or = mail_newsearchor();
	    pgm2->or->first = pgm;
	    pgm2->or->second = mail_newsearchpgm();
	    pgm2->or->second->unanswered = 1;
	    pgm = pgm2;
	}

	if(flags & F_OR_RECENT){
	    SEARCHPGM *pgm2 = mail_newsearchpgm();
	    pgm2->or = mail_newsearchor();
	    pgm2->or->first = pgm;
	    pgm2->or->second = mail_newsearchpgm();
	    pgm2->or->second->recent = 1;
	    pgm = pgm2;
	}

	pgm->msgno = full_set;

	mail_search_full(mm_search_stream = stream, NULL,
			 pgm, SE_NOPREFETCH | SE_FREE);

	if(new != ps_global->new_mail_count){
	    process_filter_patterns(stream, ps_global->msgmap,
				    ps_global->new_mail_count);

	    flag_search(stream, flags, set_start, set_msgmap, ping);
	}
    }
    else{
	if(full_set){
	    /* sequence bits of interesting msgs set */
	    mail_free_searchset(&full_set);
	}
	else{
	    /* light sequence bits of interesting msgs */
	    for(i = 1L;
		(n = flag_search_sequence(stream, set_msgmap, i, MH_ANYTHD)) >= 0L;
		i++)
	      if(n > 0L && !(mc = mail_elt(stream, n))->valid)
		mc->sequence = 1;
	      else
		mc->sequence = 0;
	}

	for(i = 1L; i <= stream->nmsgs; i++)
	  mail_elt(stream, i)->searched = 0;

	if(seq = build_sequence(stream, NULL, NULL)){
	    mail_fetch_flags(stream, seq, 0L);
	    fs_give((void **) &seq);
	}
    }
}



/*----------------------------------------------------------------------
    count messages on stream with specified system flag attributes

  Args: stream -- The stream/folder to look at message status
        flags -- flags on folder/stream to examine

 Result: count of messages flagged as requested

 Task: return count of message flagged as requested while being
       as server/network friendly as possible.

 Strategy: run thru flags to make sure they're all valid.  If any
	   invalid, do a search starting with the invalid message.
	   If all valid, ping the server to let it know we're 
	   receptive to flag updates.  At this 

  ----------------------------------------------------------------------*/
long
count_flagged(stream, flags)
    MAILSTREAM *stream;
    long	flags;
{
    long		n, count;
    MESSAGECACHE       *mc;

    flag_search(stream, flags, 1, NULL, mail_ping);

    /* Paw thru once more since all should be updated */
    for(n = 1L, count = 0L; n <= stream->nmsgs; n++)
      if(((mc = mail_elt(stream, n))->searched
	  || (mc->valid && FLAG_MATCH(flags, mc)))
	 && !get_lflag(stream, NULL, n, MN_EXLD)){	  
	  mc->searched = 1;	/* caller may be interested! */
	  count++;
      }

    return(count);
}



/*----------------------------------------------------------------------
     Find the first message with the specified flags set

  Args: flags -- Flags in messagecache to match on
        stream -- The stream/folder to look at message status

 Result: Message number of first message with specified flags set or the
	 number of the last message if none found.
  ----------------------------------------------------------------------*/
MsgNo
first_sorted_flagged(flags, stream, set_start, opts)
    unsigned long  flags;
    MAILSTREAM    *stream;
    long           set_start;
    int            opts;
{
    MsgNo	  i, n, start_with, winner = 0L;
    MESSAGECACHE *mc;
    int		  last;

    last = (opts & FSF_LAST);

    /* set_start only affects which search bits we light */
    start_with = set_start ? set_start
			   : (flags & F_SRCHBACK)
			       ? mn_get_total(ps_global->msgmap) : 1L;
    flag_search(stream, flags, start_with, ps_global->msgmap, NULL);

    for(i = start_with;
	(n = flag_search_sequence(stream, ps_global->msgmap, i,
				 (opts & FSF_SKIP_CHID) ? 0 : MH_ANYTHD)) >= 0L;
	(flags & F_SRCHBACK) ? i-- : i++)
      if(n && ((mc = mail_elt(stream, n))->searched
	       || (mc->valid && FLAG_MATCH(flags, mc)))){
	  winner = i;
	  if(!last)
	    break;
      }

    if(winner == 0L && flags != F_UNDEL && flags != F_NONE){
	dprint(4, (debugfile,
	   "First_sorted_flagged didn't find a winner, look for undeleted\n"));
	winner = first_sorted_flagged(F_UNDEL, stream, 0L,
		opts | (mn_get_revsort(ps_global->msgmap) ? 0 : FSF_LAST));
    }

    if(winner == 0L && flags != F_NONE){
	dprint(4, (debugfile,
          "First_sorted_flagged didn't find an undeleted, look for visible\n"));
	winner = first_sorted_flagged(F_NONE, stream, 0L,
		opts | (mn_get_revsort(ps_global->msgmap) ? 0 : FSF_LAST));
    }

    dprint(4, (debugfile,
	       "First_sorted_flagged returning winner = %ld\n", winner));
    return(winner ? winner
		  : (mn_get_revsort(ps_global->msgmap)
		      ? 1L : mn_get_total(ps_global->msgmap)));
}



/*----------------------------------------------------------------------
     Find the next message with specified flags set

  Args: flags  -- Flags in messagecache to match on
        stream -- The stream/folder to look at message status
        start  -- Start looking after this message
        opts   -- These bits are both input and output. On input the bit
		  NSF_TRUST_FLAGS tells us whether we need to ping or not.
		  On input, the bit NSF_SEARCH_BACK tells us that we want to
		  know about matches <= start if we don't find any > start.
		  On output, NSF_FLAG_MATCH is set if we matched a message.
  Returns: Message number of the matched message, if any; else the start # or
	   the max_msgno if the mailbox changed dramatically.
  ----------------------------------------------------------------------*/
MsgNo
next_sorted_flagged(flags, stream, start, opts)
    unsigned long  flags;
    MAILSTREAM    *stream;
    long           start;
    int           *opts;
{
    MsgNo	  i, n, dir;
    MESSAGECACHE *mc;
    int           rev, fss_flags = 0;

    /*
     * Search for the next thing the caller's interested in...
     */

    fss_flags = (opts && *opts & NSF_SKIP_CHID) ? 0 : MH_ANYTHD;
    rev = (opts && *opts & NSF_SEARCH_BACK);
    dir = (rev ? -1L : 1L);

    flag_search(stream, flags | (rev ? F_SRCHBACK : 0), start + dir,
		ps_global->msgmap,
		(opts && ((*opts) & NSF_TRUST_FLAGS)) ? NULL : mail_ping);

    for(i = start + dir;
	(n = flag_search_sequence(stream, ps_global->msgmap,
				  i, fss_flags)) >= 0L;
	i += dir)
      if(n && ((mc = mail_elt(stream, n))->searched
	       || (mc->valid && FLAG_MATCH(flags, mc)))){
	  /* actually found a msg matching the flags */
	  if(opts)
	    (*opts) |= NSF_FLAG_MATCH;

	  return(i);
      }
    

    return(min(start, mn_get_total(ps_global->msgmap)));
}



/*----------------------------------------------------------------------
  get the requested LOCAL flag bits for the given pine message number

   Accepts: msgs - pointer to message manipulation struct
            n - message number to get
	    f - bitmap of interesting flags
   Returns: non-zero if flag set, 0 if not set or no elt (error?)

   NOTE: this can be used to test system flags
  ----*/
int
get_lflag(stream, msgs, n, f)
     MAILSTREAM *stream;
     MSGNO_S    *msgs;
     long        n;
     int         f;
{
    MESSAGECACHE *mc;
    unsigned long rawno;

    rawno = msgs ? mn_m2raw(msgs, n) : n;
    if(!stream || rawno < 1L || rawno > stream->nmsgs)
      return(0);

    mc = mail_elt(stream, rawno);
    return((!mc) ? 0 : (!f)
		    ? !(mc->spare || mc->spare2 || mc->spare3 ||
		        mc->spare4 || mc->spare5)
		    : (((f & MN_HIDE) ? mc->spare : 0)
		       || ((f & MN_EXLD) ? mc->spare2 : 0)
		       || ((f & MN_SLCT) ? mc->spare3 : 0)
		       || ((f & MN_STMP) ? mc->spare6 : 0)
		       || ((f & MN_USOR) ? mc->spare7 : 0)
		       || ((f & MN_COLL) ? mc->spare5 : 0)
		       || ((f & MN_CHID) ? mc->spare4 : 0)
		       || ((f & MN_CHID2) ? mc->spare8 : 0)));
}



/*----------------------------------------------------------------------
  set the requested LOCAL flag bits for the given pine message number

   Accepts: msgs - pointer to message manipulation struct
            n - message number to set
	    f - bitmap of interesting flags
	    v - value (on or off) flag should get
   Returns: our index number of first

   NOTE: this isn't to be used for setting IMAP system flags
  ----*/
int
set_lflag(stream, msgs, n, f, v)
     MAILSTREAM *stream;
     MSGNO_S    *msgs;
     long        n;
     int         f, v;
{
    MESSAGECACHE *mc;
    long          rawno = 0L;
    PINETHRD_S   *thrd, *topthrd = NULL;

    if(n < 1L || n > mn_get_total(msgs))
      return(0L);

    if(mc = mail_elt(stream, (rawno=mn_m2raw(msgs, n)))){
	int was_invisible, is_invisible;
	int chk_thrd_cnt = 0, thrd_was_visible, was_hidden, is_hidden;

	was_invisible = (mc->spare || mc->spare4) ? 1 : 0;

	if(chk_thrd_cnt = ((msgs->visible_threads >= 0L)
	   && THRD_INDX_ENABLED() && (f & MN_HIDE) && (mc->spare != v))){
	    thrd = fetch_thread(stream, rawno);
	    if(thrd && thrd->top){
		if(thrd->top == thrd->rawno)
		  topthrd = thrd;
		else
		  topthrd = fetch_thread(stream, thrd->top);
	    }

	    if(topthrd){
		thrd_was_visible = thread_has_some_visible(stream, topthrd);
		was_hidden = mc->spare ? 1 : 0;
	    }
	}

	if((f & MN_HIDE) && mc->spare != v){
	    mc->spare = v;
	    msgs->flagged_hid += (v) ? 1L : -1L;
	}

	if((f & MN_CHID) && mc->spare4 != v){
	    mc->spare4 = v;
	    msgs->flagged_chid += (v) ? 1L : -1L;
	}

	if((f & MN_CHID2) && mc->spare8 != v){
	    mc->spare8 = v;
	    msgs->flagged_chid2 += (v) ? 1L : -1L;
	}

	if((f & MN_COLL) && mc->spare5 != v){
	    mc->spare5 = v;
	    msgs->flagged_coll += (v) ? 1L : -1L;
	}

	if((f & MN_USOR) && mc->spare7 != v){
	    mc->spare7 = v;
	    msgs->flagged_usor += (v) ? 1L : -1L;
	}

	if((f & MN_EXLD) && mc->spare2 != v){
	    mc->spare2 = v;
	    msgs->flagged_exld += (v) ? 1L : -1L;
	}

	if((f & MN_SLCT) && mc->spare3 != v){
	    mc->spare3 = v;
	    msgs->flagged_tmp += (v) ? 1L : -1L;
	}

	if((f & MN_STMP) && mc->spare6 != v){
	    mc->spare6 = v;
	    msgs->flagged_stmp += (v) ? 1L : -1L;
	}

	is_invisible = (mc->spare || mc->spare4) ? 1 : 0;

	if(was_invisible != is_invisible)
	  msgs->flagged_invisible += (v) ? 1L : -1L;
	
	/*
	 * visible_threads keeps track of how many of the max_thrdno threads
	 * are visible and how many are MN_HIDE-hidden.
	 */
	if(chk_thrd_cnt && topthrd
	   && (was_hidden != (is_hidden = mc->spare ? 1 : 0))){
	    if(!thrd_was_visible && !is_hidden){
		/* it is visible now, increase count by one */
		msgs->visible_threads++;
	    }
	    else if(thrd_was_visible && is_hidden){
		/* thread may have been hidden, check */
		if(!thread_has_some_visible(stream, topthrd))
		  msgs->visible_threads--;
	    }
	    /* else no change */
	}
    }

    return(1);
}



/*----------------------------------------------------------------------
  return whether the given flag is set somewhere in the folder

   Accepts: msgs - pointer to message manipulation struct
	    f - flag bitmap to act on
   Returns: number of messages with the given flag set.
	    NOTE: the sum, if multiple flags tested, is bogus
  ----*/
long
any_lflagged(msgs, f)
     MSGNO_S    *msgs;
     int         f;
{
    if(!msgs)
      return(0L);

    if(f == MN_NONE)
      return(!(msgs->flagged_hid || msgs->flagged_exld || msgs->flagged_tmp ||
	       msgs->flagged_coll || msgs->flagged_chid));
    else if(f == (MN_HIDE | MN_CHID))
      return(msgs->flagged_invisible);		/* special non-bogus case */
    else
      return(((f & MN_HIDE)   ? msgs->flagged_hid  : 0L)
	     + ((f & MN_EXLD) ? msgs->flagged_exld : 0L)
	     + ((f & MN_SLCT) ? msgs->flagged_tmp  : 0L)
	     + ((f & MN_STMP) ? msgs->flagged_stmp  : 0L)
	     + ((f & MN_COLL) ? msgs->flagged_coll  : 0L)
	     + ((f & MN_USOR) ? msgs->flagged_usor  : 0L)
	     + ((f & MN_CHID) ? msgs->flagged_chid : 0L)
	     + ((f & MN_CHID2) ? msgs->flagged_chid2 : 0L));
}


/*----------------------------------------------------------------------
  See if stream can be used for a mailbox name

   Accepts: mailbox name
            candidate stream
   Returns: stream if it can be used, else NIL

  This is called to weed out unnecessary use of c-client streams.
  ----*/
MAILSTREAM *
same_stream(name, stream)
    char *name;
    MAILSTREAM *stream;
{
    NETMBX mb_s, mb_n, mb_o;

    dprint(7, (debugfile, "same_stream: %s == %s\n", name,
	       (stream && stream->mailbox) ? stream->mailbox : "NULL"));

    if(stream && stream->mailbox && *stream->mailbox && name && *name
       && !(stream == ps_global->mail_stream && ps_global->dead_stream)
       && !(stream != ps_global->mail_stream
	    && stream == ps_global->inbox_stream && ps_global->dead_inbox)
       && mail_valid_net_parse(stream->mailbox, &mb_s)
       && mail_valid_net_parse(stream->original_mailbox, &mb_o)
       && mail_valid_net_parse(name, &mb_n)
       && !strucmp(mb_n.service, mb_s.service)
       && (!strucmp(mb_n.host, mb_o.host)
	   || !strucmp(canonical_name(mb_n.host), mb_s.host))
       && (!mb_n.port || mb_n.port == mb_s.port)
       && mb_n.anoflag == stream->anonymous
       && (struncmp(mb_n.service, "imap", 4)
	    ? 1
	    : (strcmp(imap_host(stream), ".NO-IMAP-CONNECTION.")
	       && ((mb_n.user && *mb_n.user &&
	            mb_s.user && !strcmp(mb_n.user, mb_s.user))
		   ||
		   ((!mb_n.user || !*mb_n.user)
		    && mb_s.user
		    && ((ps_global->VAR_USER_ID
		         && !strcmp(ps_global->VAR_USER_ID, mb_s.user))
		        ||
		        (!ps_global->VAR_USER_ID
			 && ps_global->ui.login[0]
		         && !strcmp(ps_global->ui.login, mb_s.user))))
		   ||
		   (!((mb_n.user && *mb_n.user) || (mb_s.user && *mb_s.user))
		    && stream->anonymous))))){
	dprint(7, (debugfile, "same_stream: yes\n"));
	return(stream);
    }

    dprint(7, (debugfile, "same_stream: no dice\n"));
    return(NULL);
}


/*----------------------------------------------------------------------
  See if this stream has the named mailbox selected.

   Accepts: mailbox name
            candidate stream
   Returns: stream if it can be used, else NIL
  ----*/
MAILSTREAM *
same_stream_and_mailbox(name, stream)
    char *name;
    MAILSTREAM *stream;
{
    NETMBX mb_s, mb_n;

    dprint(7, (debugfile, "same_stream_and_mailbox: %s == %s\n", name,
	       (stream && stream->mailbox) ? stream->mailbox : "NULL"));

    if(same_stream(name, stream)
       && mail_valid_net_parse(stream->mailbox, &mb_s)
       && mail_valid_net_parse(name, &mb_n)
       && (mb_n.mailbox && mb_s.mailbox
       &&  !strucmp(mb_n.mailbox,mb_s.mailbox))){
	dprint(7, (debugfile, "same_stream_and_mailbox: yes\n"));
	return(stream);
    }

    dprint(7, (debugfile, "same_stream_and_mailbox: no dice\n"));
    return(NULL);
}


/*----------------------------------------------------------------------
   Give hint about Other command being optional.  Some people get the idea
   that it is required to use the commands on the 2nd and 3rd keymenus.
   
   Args: none

 Result: message may be printed to status line
  ----*/
void
warn_other_cmds()
{
    static int other_cmds = 0;

    other_cmds++;
    if(((ps_global->first_time_user || ps_global->show_new_version) &&
	      other_cmds % 3 == 0 && other_cmds < 10) || other_cmds % 20 == 0)
        q_status_message(SM_ASYNC, 0, 9,
			 "Remember the \"O\" command is always optional");
}


/*----------------------------------------------------------------------
    Panic pine - call on detected programmatic errors to exit pine

   Args: message -- message to record in debug file and to be printed for user

 Result: The various tty modes are restored
         If debugging is active a core dump will be generated
         Exits Pine

  This is also called from imap routines and fs_get and fs_resize.
  ----*/
void
panic(message)
    char *message;
{
    char buf[256];

    if(ps_global->ttyo){
	end_screen(NULL, -1);
	end_keyboard(ps_global != NULL ? F_ON(F_USE_FK,ps_global) : 0);
	end_tty_driver(ps_global);
	end_signals(1);
    }
    if(filter_data_file(0))
      unlink(filter_data_file(0));

    dprint(1, (debugfile, "Pine Panic: %s\n", message));

    /* intercept c-client "free storage" errors */
    if(strstr(message, "free storage"))
      sprintf(buf, "No more available memory.\nPine Exiting");
    else
      sprintf(buf, "Problem detected: \"%.200s%s\".\nPine Exiting.",
	      message, strlen(message) > 200 ? "..." : "");

#ifdef _WINDOWS
    /* Put up a message box. */
    mswin_messagebox (buf, 1);
#else
    fprintf(stderr, "\n\n%s\n", buf);
#endif

#ifdef DEBUG
    if(debugfile)
      save_debug_on_crash(debugfile);

    coredump();   /*--- If we're debugging get a core dump --*/
#endif

    exit(-1);
    fatal("ffo"); /* BUG -- hack to get fatal out of library in right order*/
}



/*----------------------------------------------------------------------
    Panic pine - call on detected programmatic errors to exit pine, with arg

  Input: message --  printf styule string for panic message (see above)
         arg     --  argument for printf string

 Result: The various tty modes are restored
         If debugging is active a core dump will be generated
         Exits Pine
  ----*/
void
panic1(message, arg)
    char *message;
    char *arg;
{
    char buf1[1001], buf2[1001];

    sprintf(buf1, "%.*s", max(sizeof(buf1) - 1 - strlen(message), 0), arg);
    sprintf(buf2, message, buf1);
    panic(buf2);
}


/*
 * Pine wrapper around mail_open. If we have the PREFER_ALT_AUTH flag turned
 * on, we need to set the TRYALT flag before trying the open.
 */
MAILSTREAM *
pine_mail_open(stream, mailbox, flags)
    MAILSTREAM *stream;
    char       *mailbox;
    long	flags;
{
    MAILSTREAM *retstream;

    dprint(7, (debugfile,
	       "pine_mail_open: opening \"%s\"%s flag=0x%x %s%s%s%s\n", 
	       mailbox ? mailbox : "(NULL)",
	       stream ? "" : " (stream was NULL)",
	       flags,
	       flags & OP_HALFOPEN ? " OP_HALFOPEN" : "",
	       flags & OP_READONLY ? " OP_READONLY" : "",
	       flags & OP_SILENT ? " OP_SILENT" : "",
	       flags & OP_DEBUG ? " OP_DEBUG" : ""));

#ifdef	DEBUG
    if(ps_global->debug_imap > 3 || ps_global->debugmem)
      flags |= OP_DEBUG;
#endif
    
    if(F_ON(F_PREFER_ALT_AUTH, ps_global)){
	DRIVER *d;

	if((d = mail_valid (NIL, mailbox, (char *) NIL))
	   && !strcmp(d->name, "imap"))
	  flags |= OP_TRYALT;
    }

    /* try to re-use stream during startup */
    if(!stream){
	stream = stream_cache(mailbox);
	if(stream)
	  dprint(9,
		 (debugfile, "pine_mail_open: attempting to re-use stream\n"));
    }

    /*
     * When we pass a stream to mail_open, it will either re-use it or
     * close it.
     */
    retstream = mail_open(stream, mailbox, flags);

    /*
     * When opening a newsgroup, c-client marks the messages up to the
     * last Deleted as Unseen. If the feature news-approximates-new-status
     * is on, we'd rather they be treated as Seen. That way, selecting New
     * messages will give us the ones past the last Deleted. So we're going
     * to change them to Seen. Since Seen is a session flag for news, making
     * this change won't have any permanent effect. C-client also marks the
     * messages after the last deleted Recent, which is the bit of
     * information we'll use to find the  messages we want to change.
     */
    if(F_ON(F_FAKE_NEW_IN_NEWS, ps_global) &&
       retstream && retstream->nmsgs > 0 && IS_NEWS(retstream)){
	char         *seq;
	long          i, mflags = ST_SET;
	MESSAGECACHE *mc;

	/*
	 * Search for !recent messages to set the searched bit for
	 * those messages we want to change. Then we'll flip the bits.
	 */
	(void)count_flagged(retstream, F_UNRECENT);

	for(i = 1L; i <= retstream->nmsgs; i++)
	  if((mc = mail_elt(retstream,i))->searched)
	    mc->sequence = 1;
	  else
	    mc->sequence = 0;

	if(!is_imap_stream(retstream))
	    mflags |= ST_SILENT;
	if((seq = build_sequence(retstream, NULL, NULL)) != NULL){
	    mail_flag(retstream, seq, "\\SEEN", mflags);
	    fs_give((void **)&seq);
	}
    }

    return(retstream);
}


/*
 * Pine wrapper around mail_create. If we have the PREFER_ALT_AUTH flag turned
 * on we don't want to pass a NULL stream to c-client because it will open
 * a non-ssl connection when we want it to be ssl.
 */
long
pine_mail_create(stream, mailbox)
    MAILSTREAM *stream;
    char       *mailbox;
{
    MAILSTREAM *origstream = stream;
    long        return_val;
    char        source[MAILTMPLEN], *target = NULL;

    dprint(7, (debugfile, "pine_mail_create: creating \"%s\"%s\n", 
	       mailbox ? mailbox : "(NULL)",
	       stream ? "" : " (stream was NULL)"));

    if(check_for_move_mbox(mailbox, source, sizeof(source), &target)){
	mailbox = target;
	dprint(7, (debugfile,
		   "pine_mail_create: #move special case, creating \"%s\"\n", 
		   mailbox ? mailbox : "(NULL)"));
    }

    /*
     * We don't really need this anymore, since we are now using IMAPTRYALT.
     * We'll leave it since it works and since it gives us OP_DEBUG.
     */
    if(stream == NULL
       && (F_ON(F_PREFER_ALT_AUTH, ps_global)
           || (ps_global->debug_imap > 3 || ps_global->debugmem))){
	DRIVER *d;

	if((d = mail_valid (NIL, mailbox, (char *) NIL))
	   && !strcmp(d->name, "imap")){
	    long flags = (OP_HALFOPEN | OP_SILENT);

	    if(F_ON(F_PREFER_ALT_AUTH, ps_global))
	      flags |= OP_TRYALT;

#ifdef	DEBUG
	    if(ps_global->debug_imap > 3 || ps_global->debugmem)
	      flags |= OP_DEBUG;
#endif

	    stream = pine_mail_open(NULL, mailbox, flags);
	}
    }

    return_val = mail_create(stream, mailbox);

    if(stream != origstream)
      pine_mail_close(stream);

    return(return_val);
}


/*
 * Pine wrapper around mail_delete.
 */
long
pine_mail_delete(stream, mailbox)
    MAILSTREAM *stream;
    char       *mailbox;
{
    MAILSTREAM *origstream = stream;
    long        return_val;
    char        source[MAILTMPLEN], *target = NULL;

    dprint(7, (debugfile, "pine_mail_delete: deleting \"%s\"%s\n", 
	       mailbox ? mailbox : "(NULL)",
	       stream ? "" : " (stream was NULL)"));

    if(check_for_move_mbox(mailbox, source, sizeof(source), &target)){
	mailbox = target;
	dprint(7, (debugfile,
		   "pine_mail_delete: #move special case, deleting \"%s\"\n", 
		   mailbox ? mailbox : "(NULL)"));
    }

    /*
     * We don't really need this anymore, since we are now using IMAPTRYALT.
     * We'll leave it since it works and since it gives us OP_DEBUG.
     */
    if(stream == NULL
       && (F_ON(F_PREFER_ALT_AUTH, ps_global)
           || (ps_global->debug_imap > 3 || ps_global->debugmem))){
	DRIVER *d;

	if((d = mail_valid (NIL, mailbox, (char *) NIL))
	   && !strcmp(d->name, "imap")){
	    long flags = (OP_HALFOPEN | OP_SILENT);

	    if(F_ON(F_PREFER_ALT_AUTH, ps_global))
	      flags |= OP_TRYALT;

#ifdef	DEBUG
	    if(ps_global->debug_imap > 3 || ps_global->debugmem)
	      flags |= OP_DEBUG;
#endif

	    stream = pine_mail_open(NULL, mailbox, flags);
	}
    }

    return_val = mail_delete(stream, mailbox);

    if(stream != origstream)
      pine_mail_close(stream);

    return(return_val);
}


/*
 * Pine wrapper around mail_append.
 */
long
pine_mail_append_full(stream, mailbox, flags, date, message)
    MAILSTREAM *stream;
    char       *mailbox;
    char       *flags;
    char       *date;
    STRING     *message;
{
    MAILSTREAM *origstream = stream;
    long        return_val;
    char        source[MAILTMPLEN], *target = NULL;

    dprint(7, (debugfile, "pine_mail_append_full: appending to \"%s\"%s\n", 
	       mailbox ? mailbox : "(NULL)",
	       stream ? "" : " (stream was NULL)"));

    if(check_for_move_mbox(mailbox, source, sizeof(source), &target)){
	mailbox = target;
	dprint(7, (debugfile,
	   "pine_mail_append_full: #move special case, appending to \"%s\"\n", 
		   mailbox ? mailbox : "(NULL)"));
    }

    /*
     * We don't really need this anymore, since we are now using IMAPTRYALT.
     * We'll leave it since it works and since it gives us OP_DEBUG.
     */
    if(stream == NULL
       && (F_ON(F_PREFER_ALT_AUTH, ps_global)
           || (ps_global->debug_imap > 3 || ps_global->debugmem))){
	DRIVER *d;

	if((d = mail_valid (NIL, mailbox, (char *) NIL))
	   && !strcmp(d->name, "imap")){
	    long flags = (OP_HALFOPEN | OP_SILENT);

	    if(F_ON(F_PREFER_ALT_AUTH, ps_global))
	      flags |= OP_TRYALT;

#ifdef	DEBUG
	    if(ps_global->debug_imap > 3 || ps_global->debugmem)
	      flags |= OP_DEBUG;
#endif

	    stream = pine_mail_open(NULL, mailbox, flags);
	}
    }

    return_val = mail_append_full(stream, mailbox, flags, date, message);

    if(stream != origstream)
      pine_mail_close(stream);

    return(return_val);
}


/*
 * Pine wrapper around mail_append.
 */
long
pine_mail_append_multiple(stream, mailbox, af, data)
    MAILSTREAM *stream;
    char       *mailbox;
    append_t    af;
    void       *data;
{
    MAILSTREAM *origstream = stream;
    long        return_val;
    char        source[MAILTMPLEN], *target = NULL;

    dprint(7, (debugfile, "pine_mail_append_multiple: appending to \"%s\"%s\n", 
	       mailbox ? mailbox : "(NULL)",
	       stream ? "" : " (stream was NULL)"));

    if(check_for_move_mbox(mailbox, source, sizeof(source), &target)){
	mailbox = target;
	dprint(7, (debugfile,
         "pine_mail_append_multiple: #move special case, appending to \"%s\"\n",
		   mailbox ? mailbox : "(NULL)"));
    }

    if(stream == NULL
       && (F_ON(F_PREFER_ALT_AUTH, ps_global)
           || (ps_global->debug_imap > 3 || ps_global->debugmem))){
	DRIVER *d;

	if((d = mail_valid (NIL, mailbox, (char *) NIL))
	   && !strcmp(d->name, "imap")){
	    long flags = (OP_HALFOPEN | OP_SILENT);

	    if(F_ON(F_PREFER_ALT_AUTH, ps_global))
	      flags |= OP_TRYALT;

#ifdef	DEBUG
	    if(ps_global->debug_imap > 3 || ps_global->debugmem)
	      flags |= OP_DEBUG;
#endif

	    stream = pine_mail_open(NULL, mailbox, flags);
	}
    }

    return_val = mail_append_multiple(stream, mailbox, af, data);

    if(stream != origstream)
      pine_mail_close(stream);

    return(return_val);
}


/*
 * Pine wrapper around mail_copy.
 */
long
pine_mail_copy_full(stream, sequence, mailbox, options)
    MAILSTREAM *stream;
    char       *sequence;
    char       *mailbox;
    long        options;
{
    MAILSTREAM *origstream = stream;
    long        return_val;
    char        source[MAILTMPLEN], *target = NULL;

    dprint(7, (debugfile, "pine_mail_copy_full: copying to \"%s\"%s\n", 
	       mailbox ? mailbox : "(NULL)",
	       stream ? "" : " (stream was NULL)"));

    if(check_for_move_mbox(mailbox, source, sizeof(source), &target)){
	mailbox = target;
	dprint(7, (debugfile,
	   "pine_mail_copy_full: #move special case, copying to \"%s\"\n", 
		   mailbox ? mailbox : "(NULL)"));
    }

    /*
     * We don't really need this anymore, since we are now using IMAPTRYALT.
     * We'll leave it since it works and since it gives us OP_DEBUG.
     */
    if(stream == NULL
       && (F_ON(F_PREFER_ALT_AUTH, ps_global)
           || (ps_global->debug_imap > 3 || ps_global->debugmem))){
	DRIVER *d;

	if((d = mail_valid (NIL, mailbox, (char *) NIL))
	   && !strcmp(d->name, "imap")){
	    long flags = (OP_HALFOPEN | OP_SILENT);

	    if(F_ON(F_PREFER_ALT_AUTH, ps_global))
	      flags |= OP_TRYALT;

#ifdef	DEBUG
	    if(ps_global->debug_imap > 3 || ps_global->debugmem)
	      flags |= OP_DEBUG;
#endif

	    stream = pine_mail_open(NULL, mailbox, flags);
	}
    }

    return_val = mail_copy_full(stream, sequence, mailbox, options);

    if(stream != origstream)
      pine_mail_close(stream);

    return(return_val);
}


/*----------------------------------------------------------------------
  Our mail_close wrapper to clean up anything on the mailstream we may have
  added to it.  mostly in the unused bits of the elt's.
  ----*/
void
pine_mail_close(stream)
    MAILSTREAM *stream;
{
    PARTEX_S **partp;
    long       n;

    if(!stream)
      return;

    for(n = 1L; n <= stream->nmsgs; n++)
      free_pine_elt((PINELT_S **) &mail_elt(stream, n)->sparep);

    dprint(7, (debugfile, "pine_mail_close: closing \"%s\"\n", 
	       stream && stream->mailbox ? stream->mailbox : "(NULL)"));
    
    cache_stream(stream);
}


static MAILSTREAM *streamcache;

MAILSTREAM *
stream_cache(mailbox)
    char *mailbox;
{
    MAILSTREAM *stream = NULL;

    if(same_stream(mailbox, streamcache)){
	stream = streamcache;
	streamcache = NULL;
    }
    else
      end_stream_cache();

    return(stream);
}


void
cache_stream(stream)
    MAILSTREAM *stream;
{
    /*
     * stream caching is intended to save serial opening/closing/opening
     * of folders on the same server as we start up and acquire remote
     * config and such (that is, until we have an established initial
     * mail session open). The general case is problematic in that we
     * don't necessarily want to tie up server resources on the outside chance
     * we may do some activity, so we add the !mail_stream requirement.
     */

    if(!ps_global->mail_stream && !streamcache){
	streamcache = stream;
	dprint(9, (debugfile, "    caching stream for possible re-use\n"));
    }
    else
      mail_close(stream);
}


void
end_stream_cache()
{
    if(streamcache){
	mail_close(streamcache);
	streamcache = NULL;
    }
}


/*----------------------------------------------------------------------
  Call back for c-client to feed us back the progress of network reads

  Input: 

 Result: 
  ----*/
void
pine_read_progress(md, count)
    GETS_DATA     *md;
    unsigned long  count;
{
    gets_bytes += count;			/* update counter */
}



/*----------------------------------------------------------------------
  Function to fish the current byte count from a c-client fetch.

  Input: reset -- flag telling us to reset the count

 Result: Returns the number of bytes read by the c-client so far
  ----*/
unsigned long
pine_gets_bytes(reset)
    int reset;
{
    if(reset)
      gets_bytes = 0L;

    return(gets_bytes);
}


/*----------------------------------------------------------------------
  Function to see if a given MAILSTREAM mailbox is in the news namespace

  Input: stream -- mail stream to test

 Result: 
  ----*/
int
ns_test(mailbox, namespace)
    char *mailbox;
    char *namespace;
{
    if(mailbox){
	switch(*mailbox){
	  case '#' :
	    return(!struncmp(mailbox + 1, namespace, strlen(namespace)));

	  case '{' :
	  {
	      NETMBX mbx;

	      if(mail_valid_net_parse(mailbox, &mbx))
		return(ns_test(mbx.mailbox, namespace));
	  }

	  break;

	  default :
	    break;
	}
    }

    return(0);
}


int
is_imap_stream(stream)
    MAILSTREAM *stream;
{
    return(stream && stream->dtb && !strcmp(stream->dtb->name, "imap"));
}


int
modern_imap_stream(stream)
    MAILSTREAM *stream;
{
    return(is_imap_stream(stream) && LEVELIMAP4rev1(stream));
}


#ifdef	WIN32
char *
pine_user_callback()
{
    if(ps_global->VAR_USER_ID && ps_global->VAR_USER_ID[0]){
	return(ps_global->VAR_USER_ID);
    }
    else{
	/* SHOULD PROMPT HERE! */
      return(NULL);
    }
}
#endif

#ifdef	_WINDOWS
/*
 * windows callback to get/set function keys mode state
 */
int
fkey_mode_callback(set, args)
    int  set;
    long args;
{
    return(F_ON(F_USE_FK, ps_global) != 0);
}


void
imap_telemetry_on()
{
    if(ps_global->mail_stream)
      mail_debug(ps_global->mail_stream);
}


void
imap_telemetry_off()
{
    if(ps_global->mail_stream)
      mail_nodebug(ps_global->mail_stream);
}

char *
pcpine_help_main(title)
    char *title;
{
    if(title)
      strcpy(title, "PC-Pine MAIN MENU Help");

    return(pcpine_help(main_menu_tx));
}


int
pcpine_resize_main()
{
    (void) get_windsize (ps_global->ttyo);
    main_redrawer();
    return(0);
}


int
pcpine_main_cursor(col, row)
    int  col;
    long row;
{
    unsigned char ndmi;

    ndmi = (unsigned char) row - 3;
    if(row >= 3 && !(ndmi & 0x01)
       && ndmi <= (unsigned) MAX_DEFAULT_MENU_ITEM
       && ndmi < ps_global->ttyo->screen_rows - 4 - FOOTER_ROWS(ps_global)
       && !(ndmi & 0x01))
      return(MSWIN_CURSOR_HAND);
    else
      return(MSWIN_CURSOR_ARROW);
}
#endif /* _WINDOWS */
