/**************************************************************************
 * LPRng IFHP Filter
 * Copyright 1994-1999 Patrick Powell, San Diego, CA <papowell@astart.com>
 **************************************************************************/
/**** HEADER *****/
#include "patchlevel.h"
 static char *const _id = "$Id: ifhp.c,v 1.2 1999-03-11 22:18:24 mwhitson Exp $"
 " VERSION " PATCHLEVEL;

#include "ifhp.h"
#include "debug.h"

/**** ENDINCLUDE ****/

#define GLYPHSIZE 15
 struct glyph{
    int ch, x, y;   /* baseline location relative to x and y position */
    char bits[GLYPHSIZE];
};

 struct font{
    int height; /* height from top to bottom */
    int width;  /* width in pixels */
    int above;  /* max height above baseline */
    struct glyph *glyph;    /* glyphs */
};

int main(int argc,char *argv[], char *envp[]) ;
void cleanup(int sig) ;
void getargs( int argc, char **argv ) ;
void Init_outbuf() ;
void Put_outbuf_str( char *s ) ;
void Put_outbuf_len( char *s, int len ) ;
void Init_inbuf() ;
void Put_inbuf_str( char *str ) ;
void Put_inbuf_len( char *str, int len ) ;
char *Get_inbuf_str() ;
void Pr_status( char *str ) ;
void Check_device_status( char *line ) ;
void Initialize_parms( struct line_list *list, struct keyvalue *valuelist ) ;
void Dump_parms( char *title, struct keyvalue *v ) ;
void Process_job() ;
void Put_pjl( char *s ) ;
void Put_pcl( char *s ) ;
void Put_ps( char *s ) ;
void Put_fixed( char *s ) ;
int Get_block_io( int fd ) ;
void Set_nonblock_io( int fd ) ;
void Set_block_io( int fd ) ;
int Read_write_timeout( int readfd, int *flag, int readtimeout,
	int writefd, char *buffer, int len, int writetimeout,
	int monitorfd ) ;
int Read_status_line( int fd ) ;
int Write_out_buffer( int outlen, char *outbuf, int maxtimeout ) ;
void Resolve_key_val( char *prefix, char *key_val, Wr_out routine ) ;
int Is_flag( char *s, int *v ) ;
void Resolve_list( char *prefix, struct line_list *l, Wr_out routine ) ;
void Resolve_user_opts( char *prefix, struct line_list *only,
	struct line_list *l, Wr_out routine ) ; char *Find_sub_value( int c, char *id );

char *Fix_option_str( char *str, int remove_ws, int trim ) ;
char *Fix_option_str( char *str, int remove_ws, int trim ) ;
char *Find_sub_value( int c, char *id ) ;
int Builtin( char* prefix, char *id, char *value, Wr_out routine) ;
int Font_download( char* prefix, char *id, char *value, Wr_out routine) ;
void Pjl_job() ;
void Pjl_eoj() ;
void Pjl_console( int start ) ;
int Pjl_setvar(char *prefix, char*id, char *value, Wr_out routine) ;
int Pcl_setvar(char *prefix, char*id, char *value, Wr_out routine ) ;
void Do_sync( int sync_timeout ) ;
void Do_waitend( int waitend_timeout ) ;
int Do_pagecount( int pagecount_timeout ) ;
int Current_pagecount( int pagecount_timeout, int use_pjl, int use_ps ) ;
void Send_job() ;
void Process_OF_mode( int job_timeout ) ;
void Add_val_to_list( struct line_list *v, int c, char *key, char *sep ) ;
void Do_banner( char *line ) ;
void moveto( int x, int y ) ;
void fontsize( int size ) ;
void textline( char *line, int start, int end ) ;
void pcl_banner( char *line, struct line_list *l, struct line_list *rest ) ;
char *ps_str_fix( char *str ) ;
void ps_banner( char *line, struct line_list *l, struct line_list *rest ) ;
void file_banner( char *file ) ;
void Filter_banner( char *filter, char *line ) ;
void close_on_exec( int n ) ;
char *Use_file_util(char *pgm) ;
void Make_stdin_file() ;
int Set_mode_lang( char *s, int *mode, char **language ) ;
int Fd_readable( int fd ) ;
void Init_job( char *language, int mode ) ;
void Term_job( int mode ) ;
void Text_banner(void) ;
void do_char( struct font *font, struct glyph *glyph,
	char *str, int line ) ;
int bigfont( struct font *font, char *line, struct line_list *l, int start ) ;

/*
 * Main program:
 * 1. does setup - checks mode
 * 2. initializes connection
 * 3. initializes printer
 * 4. checks printer for idle status
 * 5. gets pagecount
 * 6.
 *   if IF mode {
 *       determines job format
 *         - binary, PCL, PostScript, Text
 *       if PCL then sets up PCL
 *       if PostScript then sets up PostScript
 *       if Text then
 *          sets up Text
 *          if text converter,  runs converter
 *       puts job out to printer
 *   }
 *   if OF mode {
 *       reads input line;
 *       if banner line and in banner mode
 *          if generating banner, put out banner page
 *          if banner generator, run converter
 *       else if suspend string, then suspend
 *       else pass through
 *   }
 * 7. terminates job
 * 8. checks printer for end of job status
 * 9. gets page count
 */


 char *Outbuf;	/* buffer */
 int Outmax;		/* max buffer size */
 int Outlen;		/* length to output */
 char *Inbuf;	/* buffer */
 int Inmax;		/* max buffer size */
 char *Inplace;	/* current input location */
 int Inlen;		/* total input from Inplace */

 char *PJL_UEL_str =  "\033%-12345X";


int main(int argc,char *argv[], char *envp[])
{
	struct stat statb;
	char *s, *t;
	int i, use_ps, use_pcl, use_text, use;
	struct line_list l;

	Init_line_list(&l);
	Argc = argc;
	Argv = argv;
	Envp = envp;
	if( argc ){
		Name = argv[0];
		if( (s = strrchr( Name, '/' )) ){
			Name = s+1;
		}
	}

	time( &Start_time );
	/* 1. does setup - checks mode */

	/* set up the accounting FD for output */
	if( fstat( 3, &statb ) == 0 ){
		Accounting_fd = 3;
	}

	/* initialize input and output buffers */
	Init_outbuf();
	Init_inbuf();

#if 0
	Debug = 4;
	Trace_on_stderr = 1;
#if 0
	Statusfile = "status";
#endif
#if 0
	Globmatch("*","");
	Globmatch("a","a");
	Globmatch("aa","a");
	Globmatch("a*","a");
	Globmatch("a*","ab");
	Globmatch("*a","a");
	Globmatch("?a","xa");
	Globmatch("a?","xa");
	Globmatch("a?","ax");
	Globmatch("a[b-d]","ax");
	Globmatch("a[b-d]","ac");
	Globmatch("a[b-d]*","ace");
#endif
#if 0
	{
	struct line_list info, names, order;
	Init_line_list(&info); Init_line_list(&names); Init_line_list(&order);
	Config_file = "/tmp/e";
	Read_file_list( &Raw, Config_file, "\n", 0, 0, 0, 1, 1, 1, 1 );
	Dump_line_list("Raw-", &Raw );
	Build_printcap_info( &names, &order, &Model, &Raw );
	Dump_line_list("PC- Model", &Model );
	Dump_line_list("PC- names", &names );
	Dump_line_list("PC- order", &order );
	s = Select_pc_info( &info,&names,&Model,"lw4", 0, "astart4.astart.com");
	}
	exit(0);
#endif
#if 0
	if( DEBUGL1 ){
		logDebug("main: ENV");
		for( i = 0; (s = envp[i]); ++i ){
			logDebug(" [%d] '%s'", i, s );
		}
	}
#endif
#if 0
	/* testing for support routines */
	{
	struct line_list l;
	Init_line_list(&l);
	Split(&l, "v2=val2\nv1=val1\n", "\n", 1, Value_sep, 1, 1, 0, 0 );
	/*Free_line_list(&l); */
	Split(&l, "v1=val3\nv2=val4\n", "\n", 1, Value_sep, 1, 1, 0, 0 );
	Split(&l, "v0=val5\nv6=val6\nv3=val7\n", "\n", 1, Value_sep, 1, 1, 0, 0 );
	exit(0);
	}
#endif
#if 0
	DEBUG1("main: banner '%s'", Outbuf );
	Upperopts['J'-'A'] = "Test Job";
	Text_banner();
	DEBUG1("main: banner");
	Write_fd_str(2,Outbuf);
	exit(0);
#endif
#endif

	/* check the environment variables */
	if( (s = getenv("PRINTCAP_ENTRY")) ){
		DEBUG3("main: PRINTCAP_ENTRY '%s'", s );
		Split( &l, s, ":", 1, Value_sep, 1, 1, 0  );
		if(DEBUGL3)Dump_line_list("main: PRINTCAP_ENTRY", &l );
		if( (s = Find_str_value(&l,"ifhp",Value_sep)) ){
			DEBUG1("main: getting PRINTCAP info '%s'", s );
			Split( &Topts, s, ",", 1, Value_sep, 1, 1, 0  );
		}
		Free_line_list(&l);
		if(DEBUGL3)Dump_line_list("main: PRINTCAP final ifhp info", &Topts );
	}

	/* get the argument lists */
	getargs( argc, argv );
	/* we extract the debug information */
	if( Debug == 0 && (s = Find_str_value( &Topts, "debug", Value_sep )) ){
		Debug = atoi(s);
		DEBUG3("main: Debug '%d'", Debug );
	}
	if( Find_exists_value( &Topts, "trace_on_stderr", Value_sep ) ){
		Trace_on_stderr = Find_flag_value( &Topts, "trace_on_stderr", Value_sep );
	}
	/* check for config file */
	if( (s = Find_str_value(&Topts,"config",Value_sep)) ){
		Config_file = safestrdup( s,__FILE__,__LINE__);
		DEBUG3("main: setting Config_file '%s'", Config_file );
	}
	if( Find_flag_value(&Topts,"version",Value_sep)
		|| Find_flag_value(&Zopts,"version",Value_sep) ){
		char buffer[128];
		plp_snprintf(buffer,sizeof(buffer),
			"Version %s - %s\n", PATCHLEVEL, _id );
			Write_fd_str(2,buffer);
	}

	if( !Config_file ) Config_file = CONFIG_FILE;
		
	Read_file_list( &Raw, Config_file, "\n", 0, 0, 0, 0, 1, 0, 1 );
	if(DEBUGL5)Dump_line_list("main: Raw", &Raw );
	Model_id = Select_model_info( &Model, &Topts, Model_id );
	Model_id = Select_model_info( &Model, &Raw, Model_id );
	DEBUG1("main: model id '%s'", Model_id );
	if(DEBUGL5)Dump_line_list("main: Model after Raw", &Model );
	Merge_list( &Model, &Topts, Value_sep, 1, 1 );
	if(DEBUGL5)Dump_line_list("main: Model after Topts", &Model );

	if( (s = Find_str_value( &Model, "user_opts", Value_sep)) ){
		Split( &User_opts, s, List_sep, 1, Value_sep, 1, 1, 0  );
		if(DEBUGL4)Dump_line_list("main: user_opts", &Pjl_user_opts);
		for( i = 0; i < User_opts.count; ++i ){
			s = User_opts.list[i];
			DEBUG4("main: checking for -Z option '%s'", s );
			if( (t = Find_exists_value( &Zopts, s, Value_sep )) ){
				if( strcmp(s,"logall") ){
					Logall = atoi(t);
				}
			}
		}
	}

	if( Debug == 0 && (s = Find_str_value( &Model, "debug", Value_sep )) ){
		Debug = atoi(s);
		DEBUG3("main: Debug '%d'", Debug );
	}

	if(DEBUGL3) Dump_line_list( "main: combined list", &Model );
	Initialize_parms(&Model, Valuelist );
	/* allow user to capture error or other output of the printer */
	if( Logall == 0 && (s = Find_exists_value(&Zopts,"logall",Value_sep))){
		Logall = atoi(s);
	}

	/*
	 * set some parameters that might neet to be done if not on
	 * command line
	 */
	if( !Banner_only ){
		Banner_only = Find_exists_value(&Zopts,"banner_only",Value_sep );
	}
	if( !Banner_only ){
		Banner_only = Find_exists_value(&Model,"banner_only",Value_sep );
	}
	if( Banner_only || Banner_name ){
		Status = 0;
		OF_Mode = 1;
	}
	if( OF_Mode ){
		use_text = use_ps = use_pcl = use = 0;
		if( !Banner && Banner_only ){
			Banner = Banner_only;
		}
		if( !Banner ){
			Banner = Find_exists_value(&Model,"banner",Value_sep );
		}
		/* now we check to see if there is a generate banner flag */
		if( Banner && Banner_user && Banner_suppressed
			&& !Find_flag_value( &Zopts, "banner", Value_sep) ){
			Banner = 0;
		}
		if( Banner && !Is_flag( Banner, &use ) ){
			if( !strcasecmp(Banner,"ps") ){
				use_ps = 1;
			}
			if( !strcasecmp(Banner,"pcl") ){
				use_pcl = 1;
			}
			if( !strcasecmp(Banner,"text") ){
				use_text = 1;
			}
			use = 1;
		} else if( use ){
			use_ps = Ps;
			use_pcl = Pcl;
			use_text = Text;
		} else {
			Banner = 0;
		}
		if( use_ps && !Ps ) use_ps = 0;
		if( use_pcl && !Pcl) use_pcl = 0;
		if( use_text && !Text) use_text = 0;
		if( use && !use_pcl && !use_ps && !use_text ){
			Errorcode = JABORT;
			fatal("main: no banner type available" );
		}
		if( use_pcl ){
			Banner = "pcl";
		} else if( use_ps ){
			Banner = "ps";
		} else if( use_text ){
			Banner = "text";
		}
	}
	DEBUG3("main: Banner '%s', Banner_only '%s'", Banner, Banner_only );
	
	if( (s = Loweropts['s'-'a']) ) Statusfile = s;
	if( Statusfile == 0 && (s = Find_str_value(&Model,"statusfile",Value_sep))){
		Statusfile = s;
	}
	if( Status_fd >= 0 ){
		close( Status_fd );
		Status_fd = -2;
	}

	DEBUG1("main: Debug level %d", Debug );
	if(DEBUGL3) Dump_parms( "main- from list", Valuelist );
	if(DEBUGL3) Dump_line_list( "main: Model", &Model );

#if 0
	{
	char b[SMALLBUFFER], *s, *t;
	s = "testing line 1\n testing line 2";
	t = Fix_option_str(s,0,1); plp_snprintf(b,sizeof(b),"'%s'='%s'\n",s,t); Write_fd_str(1,b);
	t = Fix_option_str(s,1,1); plp_snprintf(b,sizeof(b),"'%s'='%s'\n",s,t); Write_fd_str(1,b);
	s = "\\033X\\r\\n";
	t = Fix_option_str(s,0,1); plp_snprintf(b,sizeof(b),"'%s'='%s'\n",s,t); Write_fd_str(1,b);
	t = Fix_option_str(s,1,1); plp_snprintf(b,sizeof(b),"'%s'='%s'\n",s,t); Write_fd_str(1,b);
	s = "\\%{pjl},\\%d{pjl},\\%05.2f{pjl}";
	t = Fix_option_str(s,1,1); plp_snprintf(b,sizeof(b),"'%s'='%s'\n",s,t); Write_fd_str(1,b);
	exit(0);
	}
#endif

	(void)signal(SIGINT, cleanup);
	(void)signal(SIGHUP, cleanup);
	(void)signal(SIGTERM, cleanup);
	(void)signal(SIGQUIT, cleanup);
	(void)signal(SIGPIPE, SIG_IGN);

	/* 2. initializes connection */
	if( Device ){
		Open_device( Device );
	}
	if( Force_status ){
		Status = 1;
		DEBUG1( "main: forcing status reading" );
	} else if( Status && !Fd_readable(1) ){
		DEBUG1( "main: device cannot provide status" );
		Status = 0;
	}

	Process_job();
	Errorcode = 0;
	cleanup(0);
	return(0);
}

void cleanup(int sig)
{
    DEBUG3("cleanup: Signal '%s', Errorcode %d", Sigstr(sig), Errorcode );
    exit(Errorcode);
}

/*
 * getargs( int argc, char **argv )
 *  - get the options from the argument list
 */

void getargs( int argc, char **argv )
{
	int i, flag;
	char *arg, *s;

	/* log("testing"); */
	DEBUG3("getargs: starting, debug %d", Debug);
	for( i = 1; i < argc; ++i ){
		arg = argv[i];
		if( *arg++ != '-' ) break;
		flag = *arg++;
		if( flag == '-' ){
			++i;
			break;
		}
		if( *arg == 0 && flag != 'c' ){
			if( i < argc){
				arg = argv[++i];
			} else {
				fatal( "missing argument for flag '%c'", flag );
			}
		}
		/* we duplicate the strings */
		DEBUG3("getargs: flag '%c'='%s'", flag, arg);
		if( islower(flag) ){
			if( (s = Loweropts[flag-'a'] )) free(s);
			Loweropts[flag-'a'] = safestrdup(arg,__FILE__,__LINE__);
		} else if( isupper(flag) ){
			s = Upperopts[flag-'A'];
			switch( flag ){
			case 'T': case 'Z':
				if( s ){
					Upperopts[flag-'A'] = 
						safestrdup3(s,",",arg,__FILE__,__LINE__);
				} else {
					Upperopts[flag-'A'] = safestrdup(arg,__FILE__,__LINE__);
				}
				break;
			default:
				Upperopts[flag-'A'] = safestrdup(arg,__FILE__,__LINE__);
				break;
			}
			if( s ) free(s);
		}
	}
	if( i < argc ){
		Accountfile = argv[i];
	} else if( (s = Loweropts['a'-'a']) ){
		Accountfile = s;
	}
	if( ((s = Upperopts['F'-'A']) && *s == 'o')
		|| strstr( Name, "of" ) ){
		OF_Mode = 1;
	}
	if( strstr( Name, "banner" ) ){
		Banner_name = 1;
	}
	if( (s = Upperopts['T'-'A'] ) ){
		Split( &Topts, s, ",", 1, Value_sep, 1, 1, 0 );
	}
	if( (s = Upperopts['Z'-'A'] ) ){
		Split( &Zopts, s, ",", 1, Value_sep, 1, 1, 0 );
	}
}

/*
 * Output buffer management
 *  Set up and put values into an output buffer for
 *  transmission at a later time
 */
void Init_outbuf()
{
	DEBUG1("Init_outbuf: Outbuf 0x%x, Outmax %d, Outlen %d",
		Outbuf, Outmax, Outlen );
	if( Outmax <= 0 ) Outmax = OUTBUFFER;
	if( Outbuf == 0 ) Outbuf = realloc_or_die( Outbuf, Outmax+1,__FILE__,__LINE__);
	Outlen = 0;
	Outbuf[0] = 0;
}

void Put_outbuf_str( char *s )
{
	if( s && *s ) Put_outbuf_len( s, strlen(s) );
}

void Put_outbuf_len( char *s, int len )
{
	DEBUG4("Put_outbuf_len: starting- Outbuf 0x%x, Outmax %d, Outlen %d, len %d",
		Outbuf, Outmax, Outlen, len );
	if( s == 0 || len <= 0 ) return;
	if( Outmax - Outlen <= len ){
		Outmax += ((OUTBUFFER + len)/1024)*1024;
		Outbuf = realloc_or_die( Outbuf, Outmax+1,__FILE__,__LINE__);
		DEBUG4("Put_outbuf_len: update- Outbuf 0x%x, Outmax %d, Outlen %d, len %d",
		Outbuf, Outmax, Outlen, len );
		if( !Outbuf ){
			Errorcode = JABORT;
			logerr_die( "Put_outbuf_len: realloc %d failed", len );
		}
	}
	memcpy( Outbuf+Outlen, s, len );
	Outlen += len;
	Outbuf[Outlen] = 0;
}

/*
 * Input buffer management
 *  Set up and put values into an input buffer for
 *  scanning purposes
 */
void Init_inbuf()
{
	Inmax = LARGEBUFFER;
	Inbuf = realloc_or_die( Inbuf, Inmax+1,__FILE__,__LINE__);
	Inbuf[Inmax] = 0;
	Inplace = Inbuf;
	Inlen = 0;
	Inbuf[0] = 0;
}

void Put_inbuf_str( char *str )
{
	int len = strlen( str );
	Put_inbuf_len( str, len );
}

void Put_inbuf_len( char *str, int len )
{
	Inlen = strlen(Inplace);
	if( Inplace != Inbuf ){
		/* we readjust the locations */
		memmove( Inbuf, Inplace, Inlen+1 );
		Inplace = Inbuf;
	}
	if( Inmax - Inlen <= (len+1) ){
		Inmax += ((LARGEBUFFER + len+1)/1024)*1024;
		Inbuf = realloc_or_die( Inbuf, Inmax+1,__FILE__,__LINE__);
		if( !Inbuf ){
			Errorcode = JABORT;
			logerr_die( "Put_outbuf_len: realloc %d failed", len );
		}
	}
	memmove( Inplace+Inlen, str, len+1 );
	Inlen += len;
	Inbuf[Inlen] = 0;
	DEBUG4("Put_inbuf_len: buffer '%s'", Inbuf );
}

/*
 * char *Get_inbuf_str()
 * return a pointer to a complete line in the input buffer
 *   This will, in effect, remove the line from the
 *   buffer, and it should be saved.
 * Any of the Put_inbuf or Get_inbuf calls will destroy
 *   this line.
 */
char *Get_inbuf_str()
{
	char *s = 0;
	/* DEBUG4("Get_inbuf_str: buffer '%s'", Inplace); */
	Inlen = strlen(Inplace);
	if( Inplace != Inbuf ){
		/* we readjust the locations */
		memmove( Inbuf, Inplace, Inlen+1 );
		Inplace = Inbuf;
	}
	/* remove \r */
	while( (s = strchr( Inbuf, '\r' ) ) ){
		memmove( s, s+1, strlen(s)+1 );
	}
	s = strpbrk( Inbuf, Line_ends );
	if( s ){
		*s++ = 0;
		Inplace = s;
		s = Inbuf;
		/* DEBUG4("Get_inbuf_str: found '%s', buffer '%s'", s, Inplace); */
		Pr_status( s );
	}
	Inlen = strlen(Inplace);
	return( s );
}

/*
 * Printer Status Reporting
 * PJL:
 *  We extract info of the form:
 *  @PJL XXXX P1 P2 ...
 *  V1
 *  V2
 *  0
 *    we then create a string 
 *  P1.P2.P3=xxxx
 *  - we put this into the 'current status' array
 * PostScript:
 *  We extract info of the form:
 *  %%[ key: value value key: value value ]%%
 *    we then create strings
 *     key=value value
 *  - we put this into the 'current status' array
 *
 */

void Pr_status( char *str )
{
	char *s, *t, *end = 0;
	int len, i, start;
	struct line_list l;
	static char *name;

	DEBUG4("Pr_status: start str '%s', name '%s'", str, name);

	Init_line_list(&l);
	if( str && *str && name == 0 ){
		Split(&l, str, Whitespace, 0, 0, 0, 0, 0 );
		if(DEBUGL5)Dump_line_list("Pr_status", &l);
		/* we do PJL coordination */
		if( l.count > 1 && !strcasecmp(l.list[0], "@PJL" ) ){
			DEBUG5("Pr_status: doing PJL status");
			s = l.list[1];
			start = 1;
			if( !strcasecmp( s, "ECHO" ) ){
				/* we have the echo value */
				if( Logall ){
					log( "Pr_status: printer status '%s'", str );
				}
				name = safestrdup2("echo=", str,__FILE__,__LINE__);
				DEBUG5("Pr_status: found echo '%s'", name );
				Add_line_list( &Devstatus, name, Value_sep, 1, 1 );
				free( name ); name = 0;
			} else {
				name = safestrdup(str,__FILE__,__LINE__);
			}
		} else if( strstr( str,"%%[" ) ){
			int c = 0;
			/* we do Postscipt status */
			DEBUG5("Pr_status: doing PostScript status");
			if( Logall ){
				log( "Pr_status: printer status '%s'", str );
			}
			for( i = 0; i < l.count; ++i ){
				s = l.list[i];
				DEBUG5("Pr_status: list [%d]='%s', name %s", i, s, name );
				if( strstr( s, "]%%" ) || strstr( s, "%%[" ) ){
					if( name ){
						DEBUG5("Pr_status: found ps status '%s'", name );
						Check_device_status(name);
						Add_line_list( &Devstatus, name, Value_sep, 1, 1 );
						free( name ); name = 0;
					}
					c = 0;
				} else if( (t = strchr(s,':')) && t[1] == 0 ){
					if( name ){
						DEBUG5("Pr_status: found ps status '%s'", name );
						Check_device_status(name);
						Add_line_list( &Devstatus, name, Value_sep, 1, 1 );
						free( name ); name = 0;
					}
					name = safestrdup(s,__FILE__,__LINE__);
					if( (t = strchr(name,':')) ){
						*t++ = '=';
					}
					lowercase(name);
					c = 0;
				} else if( name ){
					t = name;
					name = safestrdup3(name,c?" ":"",s,__FILE__,__LINE__);
					free(t);
					c = 1;
				}
			}
			if( name ){
				DEBUG5("Pr_status: found ps status '%s'", name );
				Check_device_status(name);
				Add_line_list( &Devstatus, name, Value_sep, 1, 1 );
				free( name ); name = 0;
			}
			c = 0;
		} else if( Logall ) {
			log( "Pr_status: printer status '%s'", str );
		}
	} else if( str && *str && name && *name  ){
		DEBUG5("Pr_status: append '%s' '%s'", name, str );
		t = name;
		name = safestrdup3(name,"\n",str,__FILE__,__LINE__);
		free(t);
		DEBUG5("Pr_status: adding value result '%s'", name );
	} else if( name && *name ){
		DEBUG5("Pr_status: found '%s'", name );
		if( Logall ){
			log( "Pr_status: printer status '%s'", str );
		}
		/* now we need to find the type of information */ 
		Free_line_list( &l );
		if( (end = strchr(name,'\n'))) *end = 0;
		Split(&l, name, Whitespace, 0, 0, 0, 0, 0 );
		/* now we check to see what type of PJL */
		if(DEBUGL5)Dump_line_list("Pr_status- first line", &l );
		if( end && l.count > 2 ){
			s = l.list[1];
			start = 1;
			if( !strcasecmp( s, "INFO" ) ){
				/* solicted information */
				start = 2;
			}
			if( !strcasecmp( s, "USTATUS" ) ){
				/* solicted information */
				start = 2;
			}
			len = 0;
			for( i = start; i < l.count; ++i ){
				len += strlen(l.list[i])+1;
			}
			len += 3;
			t = malloc_or_die(len+1,__FILE__,__LINE__);
			t[0] = 0;
			for( s = t, i = start; i < l.count; ++i ){
				if( *t ) *s++ = '.';
				strcpy(s, l.list[i] );
				s += strlen(s);
			}
			*s++ = '=';
			*s = 0;
			s = safestrdup2( t, end?end+1:"", __FILE__,__LINE__);
			free(t);
			DEBUG5("Pr_status: modified status '%s'", s );
			Free_line_list( &l );
			Split(&l, s, "\n", 0, 0, 0, 0, 0 );
			free(s);
			for( i = 0; i < l.count; ++i ){
				s = l.list[i];
				DEBUG5( "Pr_status: PJL status entry '%s'", s );
				Check_device_status(s);
				Add_line_list( &Devstatus, s, Value_sep, 1, 1 );
				if( i == 0 && (t = strchr(s,'=')) ){
					DEBUG5( "Pr_status: checking substatus entry '%s'", t+1 );
					if( strchr(t+1,'=') ){
						DEBUG5( "Pr_status: found substatus entry '%s'", t+1 );
						Check_device_status(t+1);
						Add_line_list( &Devstatus, t+1, Value_sep, 1, 1 );
					}
				}
			}
		}
		if( name ){ free( name ); name = 0; }
	}
	Free_line_list( &l );
}

/*
 * Check_device_status( char *line )
 *  - the line has to have the form DEVICE="nnn"
 *  We log the message IF it is different from the last one
 */

void Check_device_status( char *line )
{
	char *old, *end, *t;

	line = safestrdup(line,__FILE__,__LINE__);
	if( (end = strpbrk( line, Value_sep )) ) *end++ = 0;

	if( end && *end ){
		lowercase( line );
		if( !strcasecmp(line,"CODE") ){
			DEBUG4("Check_device_status: found device '%s'", line, end );
			old = Find_str_value( &Devstatus, line, Value_sep );
			DEBUG4("Check_device_status: old code '%s', new '%s'", old, end );
			if( !old || strcmp(old,end) ){
				/* we now have to check to see if we log this */
				if( Find_first_key( &Pjl_quiet_codes, line, Value_sep, 0 ) ){
					/* ok, we log it */
					t = Check_code( end );
					DEBUG4("Check_device_status: code '%s' = '%s'", line, t);
					log("Check_device_status: device = '%s'", t );
				}
			}
		} else if( strstr(line,"ustatus") ){
			;
		} else if( strstr(line,"status") || strstr(line,"error") || strstr(line,"offending") ){
			DEBUG4("Check_device_status: found '%s'='%s'", line, end );
			old = Find_str_value( &Devstatus, line, Value_sep );
			DEBUG4("Check_device_status: old status '%s', new '%s'", old, end );
			if( !old || strcmp(old,end) ){
				log("Check_device_status: %s = '%s'", line, end );
			}
		}
	}
	if( line ){ free(line); line = 0; }
}


/*
 * void Initialize_parms( struct line_list *list, struct keyvalue *valuelist )
 *  Initialize variables from values in a parameter list
 *  This list has entries of the form key=value
 *  If the variable is a flag,  then entry of the form 'key' will set
 *    it,  and key@ will clear it.
 *
 *  list = key list, i.e. - strings of form key=value
 *  count = number of entries in key list
 *  valuelist = variables and keys to use to set them
 */

void Initialize_parms( struct line_list *list, struct keyvalue *valuelist )
{
	struct keyvalue *v;
	char *arg, *convert;
	int n;
	if( valuelist == 0 ) return;
	for( v = valuelist; v->key; ++v ){
		if( v->key[0] == 0 ) continue;
		if( Find_first_key( list, v->key, Value_sep, 0) ) continue;
		arg = Find_exists_value( list, v->key, Value_sep );
		DEBUG4("Initialize_parms: key '%s'='%s'", v->key, arg );
		if( arg ) switch( v->kind ){
		case STRV: *(char **)v->var = arg; break;
		case INTV:
		case FLGV:
			convert = arg;
			n = strtol( arg, &convert, 10 );
			if( convert != arg ){
				*(int *)v->var = n;
			} else {
				*(int *)v->var = ( !strcasecmp( arg, "yes" )
				|| !strcasecmp( arg, "on" ));
			}
			break;
		}
	}
}

/*
 * Dump_parms( char *title, struct keyvalue *v )
 *  Dump the names, config file tags, and current values of variables
 *   in a value list.
 */
void Dump_parms( char *title, struct keyvalue *v )
{
	logDebug( "Dump_parms: %s", title );
	for( v = Valuelist; v->key; ++v ){
		switch( v->kind ){
		case STRV:
			logDebug( " '%s' (%s) STRV = '%s'", v->varname, v->key, *(char **)v->var ); break;
		case INTV:
			logDebug( " '%s' (%s) INTV = '%d'", v->varname, v->key, *(int *)v->var ); break;
		case FLGV:
			logDebug( " '%s' (%s) FLGV = '%d'", v->varname, v->key, *(int *)v->var ); break;
		}
	}
}

/*
 * Process the job  - we do this for both IF and OF modes 
 */
void Process_job()
{
	char *s;
	struct line_list l;
	time_t current_t;
	int elapsed, startpagecount = 0, endpagecount = 0;

	Init_line_list( &l );
	Init_outbuf();

	log( "Process_job: setting up printer");

	if( (s = Find_str_value( &Model, "pjl_only", Value_sep)) ){
		Split( &Pjl_only, s, List_sep, 1, 0, 1, 1, 0 );
		if(DEBUGL4)Dump_line_list("Process_job: Pjl_only", &Pjl_only);
	}
	if( (s = Find_str_value( &Model, "pjl_except", Value_sep)) ){
		Split( &Pjl_except, s, List_sep, 1, 0, 1, 1, 0 );
		if(DEBUGL4)Dump_line_list("Process_job: Pjl_except", &Pjl_except);
	}
	if( (s = Find_str_value( &Model, "pjl_vars_set", Value_sep)) ){
		Split( &Pjl_vars_set, s, List_sep, 1, Value_sep, 1, 1, 0 );
		if(DEBUGL4)Dump_line_list("Process_job: Pjl_vars_set", &Pjl_vars_set);
	}
	if( (s = Find_str_value( &Model, "pjl_vars_except", Value_sep)) ){
		Split( &Pjl_vars_except, s, List_sep, 1, Value_sep, 1, 1, 0 );
		if(DEBUGL4)Dump_line_list("Process_job: Pjl_vars_except", &Pjl_vars_except);
	}
	if( (s = Find_str_value( &Model, "pjl_user_opts", Value_sep)) ){
		Split( &Pjl_user_opts, s, List_sep, 1, Value_sep, 1, 1, 0 );
		if(DEBUGL4)Dump_line_list("Process_job: Pjl_user_opts", &Pjl_user_opts);
	}
	if( (s = Find_str_value( &Model, "pcl_user_opts", Value_sep)) ){
		Split( &Pcl_user_opts, s, List_sep, 1, Value_sep, 1, 1, 0 );
		if(DEBUGL4)Dump_line_list("Process_job: Pcl_user_opts", &Pcl_user_opts);
	}
	if( (s = Find_str_value( &Model, "ps_user_opts", Value_sep)) ){
		Split( &Ps_user_opts, s, List_sep, 1, Value_sep, 1, 1, 0 );
		if(DEBUGL4)Dump_line_list("Process_job: Ps_user_opts", &Ps_user_opts);
	}
	if( (s = Find_str_value( &Model, "pjl_error_codes", Value_sep)) ){
		Split( &Pjl_error_codes, s, "\n", 1, Value_sep, 1, 1, 0 );
		if(DEBUGL4)Dump_line_list("Process_job: Pjl_error_codes", &Pjl_error_codes);
	}
	if( (s = Find_str_value( &Model, "pjl_quiet_codes", Value_sep)) ){
		Split( &Pjl_quiet_codes, s, List_sep, 1, Value_sep, 1, 1, 0 );
		if(DEBUGL4)Dump_line_list("Process_job: Pjl_quiet_codes", &Pjl_quiet_codes);
	}


	DEBUG1("Process_job: sync and pagecount");
	if( Status ){
		Do_sync(Sync_timeout);
		startpagecount = Do_pagecount(Pagecount_timeout);
	}
	time( &current_t );
	elapsed = current_t - Start_time;

	if( !Banner_only ) Do_accounting(1, elapsed, startpagecount, 0 );

	DEBUG1("Process_job: doing 'init'");

	Init_outbuf();
	if( !Find_first_key( &Model, "init", Value_sep, 0) ){
		s  = Find_str_value( &Model, "init", Value_sep);
		DEBUG1("Process_job: 'init'='%s'", s);
		Split( &l, s, List_sep, 0, 0, 0, 1, 0 );
		Resolve_list( "", &l, Put_pcl );
		Free_line_list( &l );
	}
	if( Pjl ){
		DEBUG1("Process_job: doing pjl");
		Put_outbuf_str( PJL_UEL_str );
		Pjl_job();
		Pjl_console(1);
		if( !Find_first_key( &Model, "pjl_init", Value_sep, 0) ){
			s  = Find_str_value( &Model, "pjl_init", Value_sep);
			DEBUG1("Process_job: 'pjl_init'='%s'", s);
			Split( &l, s, List_sep, 0, 0, 0, 1, 0 );
			Resolve_list( "pjl_", &l, Put_pjl );
			Free_line_list( &l );
		}
		if( !OF_Mode ){
			DEBUG1("Process_job: 'pjl' and Zopts");
			Resolve_user_opts( "pjl_", &Pjl_user_opts, &Zopts, Put_pjl );
			DEBUG1("Process_job: 'pjl' and Topts");
			Resolve_user_opts( "pjl_", &Pjl_user_opts, &Topts, Put_pjl );
		}
	}
	if( Write_out_buffer( Outlen, Outbuf, Job_timeout ) ){
		Errorcode = JFAIL;
		fatal("Process_job: timeout");
	}
	Init_outbuf();
	if( Banner_only ){
		Do_banner("");	
		Write_fd_str(1, Outbuf );
	} else if( OF_Mode ){
		log( "Process_job: starting OF mode passthrough" );
		Process_OF_mode( Job_timeout );
		log( "Process_job: ending OF mode passthrough" );
	} else {
		log( "Process_job: sending job file" );
		Send_job();
		log( "Process_job: sent job file" );
	}
	log( "Process_job: doing cleanup");
	Init_outbuf();
	if( Pjl ){
		DEBUG1("Process_job: doing pjl at end");
		Put_outbuf_str( PJL_UEL_str );
		Pjl_eoj();
		Pjl_console(0);
		if( !Find_first_key( &Model, "pjl_term", Value_sep, 0) ){
			s  = Find_str_value( &Model, "pjl_term", Value_sep);
			DEBUG1("Process_job: 'pjl_term'='%s'", s);
			Split( &l, s, List_sep, 0, 0, 0, 1, 0 );
			Resolve_list( "pjl_", &l, Put_pjl );
			Free_line_list( &l );
		}
	}
	if( !Find_first_key( &Model, "term", Value_sep, 0) ){
		s  = Find_str_value( &Model, "term", Value_sep);
		DEBUG1("Process_job: 'term'='%s'", s);
		Split( &l, s, List_sep, 0, 0, 0, 1, 0 );
		Resolve_list( "", &l, Put_pcl );
		Free_line_list( &l );
	}
	if( Write_out_buffer( Outlen, Outbuf, Job_timeout ) ){
		Errorcode = JFAIL;
		fatal("Process_job: timeout");
	}
	Init_outbuf();
	DEBUG1("Process_job: end sync and pagecount");
	if( Status ){
		Do_waitend(Job_timeout);
		endpagecount = Do_pagecount(Job_timeout);
	}

	time( &current_t );
	elapsed = current_t - Start_time;
	if( !Banner_only ) Do_accounting(0, elapsed, endpagecount, endpagecount - startpagecount );
	log( "Process_job: done" );
}

/*
 * Put_pjl( char *s )
 *  write pjl out to the output buffer
 *  check to make sure that the line is PJL code only
 *  and that the PJL option is in a list
 */

void Put_pjl( char *s )
{
	struct line_list l, wl;
	int i;

	Init_line_list(&l);
	Init_line_list(&wl);

	DEBUG4("Put_pjl: orig '%s'", s );
	if( s == 0 || *s == 0 ) return;
	s = Fix_option_str( s, 0, 1 );
	Split( &l, s, "\n", 0, 0, 0, 1, 0 );
	if( s ){ free(s); s = 0; }
	for( i = 0; i < l.count; ++i ){
		/* check for valid PJL */
		Split( &wl, l.list[i], Whitespace, 0, 0, 0, 0, 0 );
		if( wl.count == 0 || strcmp("@PJL", wl.list[0]) ){
			continue;
		}
		/* key must be in PJL_ONLY list if supplied, and NOT in
		 * PJL_ACCEPT
		 */
		s = wl.list[1];
		DEBUG4("Put_pjl: trying '%s'", s );
		if( wl.count > 1 &&
			(  Find_first_key( &Pjl_only, s, 0, 0)
			||  !Find_first_key( &Pjl_except, s, 0, 0)
			) ){
			continue;
		}
		DEBUG4("Put_pjl: '%s'", l.list[i] );
		Put_outbuf_str( l.list[i] );
		Put_outbuf_str( "\n" );
		Free_line_list( &wl );
	}
	Free_line_list(&l);
}

void Put_pcl( char *s )
{
	DEBUG4("Put_pcl: orig '%s'", s );
	s = Fix_option_str( s, 1, 1 );
	DEBUG4("Put_pcl: final '%s'", s );
	Put_outbuf_str( s );
	if(s)free(s);s=0;
}


void Put_ps( char *s )
{
	DEBUG4("Put_fixed: orig '%s'", s );
	s = Fix_option_str( s, 0, 0 );
	DEBUG4("Put_fixed: final '%s'", s );
	Put_outbuf_str( s );
	if(s)free(s);s=0;
	Put_outbuf_str( "\n" );
}


void Put_fixed( char *s )
{
	DEBUG4("Put_fixed: orig '%s'", s );
	s = Fix_option_str( s, 0, 0 );
	DEBUG4("Put_fixed: final '%s'", s );
	Put_outbuf_str( s );
	if(s)free(s);s=0;
}


/*
 * Set_non_block_io(fd)
 * Set_block_io(fd)
 *  Set blocking or non-blocking IO
 *  Dies if unsuccessful
 * Get_block_io(fd)
 *  Returns NONBLOCK flag value
 */

int Get_block_io( int fd )
{
	int mask;
	/* we set IO to non-blocking on fd */

	if( (mask = fcntl( fd, F_GETFL, 0 ) ) == -1 ){
		Errorcode = JABORT;
		logerr_die( "ifhp: fcntl fd %d F_GETFL failed", fd );
	}
	mask &= NONBLOCK;
	return( mask );
}

void Set_nonblock_io( int fd )
{
	int mask;
	/* we set IO to non-blocking on fd */

	if( (mask = fcntl( fd, F_GETFL, 0 ) ) == -1 ){
		Errorcode = JABORT;
		logerr_die( "ifhp: fcntl fd %d F_GETFL failed", fd );
	}
	mask |= NONBLOCK;
	if( (mask = fcntl( fd, F_SETFL, mask ) ) == -1 ){
		Errorcode = JABORT;
		logerr_die( "ifhp: fcntl fd %d F_SETFL failed", fd );
	}
}

void Set_block_io( int fd )
{
	int mask;
	/* we set IO to blocking on fd */

	if( (mask = fcntl( fd, F_GETFL, 0 ) ) == -1 ){
		Errorcode = JABORT;
		logerr_die( "ifhp: fcntl fd %d F_GETFL failed", fd );
	}
	mask &= ~NONBLOCK;
	if( (mask = fcntl( fd, F_SETFL, mask ) ) == -1 ){
		Errorcode = JABORT;
		logerr_die( "ifhp: fcntl fd %d F_SETFL failed", fd );
	}
}


/*
 * Read_write_timeout( int readfd, int *flag, int readtimeout,
 *	 int writefd, char *buffer, int len, int writetimeout )
 *  Write the contents of a buffer to the file descriptor
 *  int readfd: 
 *  int writefd: write file descriptor
 *  char *buffer, int len: buffer and number of bytes
 *  int timeout:  timeout in seconds
 *  Returns: numbers of bytes left to write
 *  - if we are writing (len > 0), then end of write terminates
 *  - if we are only reading (len == 0, flag != 0),
 *    then we wait for the readtimeout length
 */

int Read_write_timeout( int readfd, int *flag, int readtimeout,
	int writefd, char *buffer, int len, int writetimeout,
	int monitorfd )
{
	time_t start_t, current_t;
	int elapsed, blocking = 0, m, err;
	struct timeval timeval, *tp;
    fd_set readfds, writefds; /* for select() */

	DEBUG4(
	"Read_write_timeout: write(fd %d, len %d, timeout %d) read(fd %d, timeout %d) monitor %d",
		writefd, len, writetimeout, readfd, readtimeout,monitorfd);

	time( &start_t );

	if( flag ) *flag = 0;
	if( len == 0 && flag && readfd >= 0 ){
		while(1){
			if( readtimeout > 0 ){
				time( &current_t );
				elapsed = current_t - start_t;
				if( readtimeout > 0 && elapsed >= readtimeout ){
					break;
				}
				memset( &timeval, 0, sizeof(timeval) );
				timeval.tv_sec = m = readtimeout - elapsed;
				tp = &timeval;
				DEBUG4("Read_write_timeout: read timeout now %d", m );
			} else {
				tp = 0;
			}
			FD_ZERO( &readfds );
			m = 0;
			if( flag && readfd >= 0 ){
				FD_SET( readfd, &readfds );
				if( m <= readfd ) m = readfd+1;
			}
			if( flag && monitorfd >= 0 ){
				FD_SET( monitorfd, &readfds );
				if( m <= monitorfd ) m = monitorfd+1;
			}
			DEBUG4("Read_write_timeout: starting read select" );
			m = select( m,
				FD_SET_FIX((fd_set *))&readfds,
				FD_SET_FIX((fd_set *))0,
				FD_SET_FIX((fd_set *))0, tp );
			err = errno;
			DEBUG4("Read_write_timeout: reading only, select returned %d", m );
			if( m < 0 ){
				/* error */
				if( err != EINTR ){
					Errorcode = JFAIL;
					logerr_die( "Read_write_timeout: select error" );
				}
			} else if( m == 0 ){
				/* timeout */
				break;
			}
			if( readfd >= 0 && FD_ISSET( readfd, &readfds ) ){
				*flag = 1;
				break;
			}
			if( monitorfd >= 0 && FD_ISSET( monitorfd, &readfds ) ){
				*flag = 1;
				break;
			}
		}
		DEBUG4("Read_write_timeout: read wait flag %d", *flag );
		return( len );
	}

	if( writefd>=0 && (blocking = Get_block_io( writefd ))){
		Set_nonblock_io( writefd );
	}
	while( len > 0 ){
		if( writetimeout > 0 ){
			time( &current_t );
			elapsed = current_t - start_t;
			if( writetimeout > 0 && elapsed >= writetimeout ){
				break;
			}
			memset( &timeval, 0, sizeof(timeval) );
			timeval.tv_sec = m = writetimeout - elapsed;
			tp = &timeval;
			DEBUG4("Read_write_timeout: timeout now %d", m );
		} else {
			tp = 0;
		}
		FD_ZERO( &writefds );
		FD_ZERO( &readfds );
		m = 0;
		if( writefd >= 0 ){
			FD_SET( writefd, &writefds );
			if( m <= writefd ) m = writefd+1;
		}
		if( flag && readfd >= 0 ){
			FD_SET( readfd, &readfds );
			if( m <= readfd ) m = readfd+1;
		}
		if( flag && monitorfd >= 0 ){
			FD_SET( monitorfd, &readfds );
			if( m <= monitorfd ) m = monitorfd+1;
		}
		DEBUG4("Read_write_timeout: starting select" );
        m = select( m,
            FD_SET_FIX((fd_set *))&readfds,
            FD_SET_FIX((fd_set *))&writefds,
            FD_SET_FIX((fd_set *))0, tp );
		err = errno;
		DEBUG4("Read_write_timeout: reading and writing, select returned %d", m );
		if( m < 0 ){
			/* error */
			if( err != EINTR ){
				Errorcode = JFAIL;
				logerr_die( "Read_write_timeout: select error" );
			}
		} else if( m == 0 ){
			/* timeout */
			break;
		}
		if( writefd>=0 && FD_ISSET( writefd, &writefds ) ){
			DEBUG4("Read_write_timeout: write possible on fd %d", writefd );
			m = write( writefd, buffer, len );
			DEBUG4("Read_write_timeout: wrote %d", m );
			if( m < 0 ){
				logerr( "Read_write_timeout: write error" );
				len = -1;
				break;
			} else if( m == 0 ){
				/* we have EOF on the file descriptor */
				len = -1;
				break;
			} else {
				len -= m;
				buffer += m;
			}
		}
		if( readfd >=0 && FD_ISSET( readfd, &readfds ) ){
			DEBUG4("Read_write_timeout: read possible on fd %d", readfd );
			*flag = 1;
			break;
		}
		if( monitorfd >=0 && FD_ISSET( monitorfd, &readfds ) ){
			DEBUG4("Read_write_timeout: monitor possible on fd %d", monitorfd );
			*flag = 1;
			break;
		}
	}
	if(writefd >= 0 && blocking){
		Set_block_io( writefd );
	}
	DEBUG4("Read_write_timeout: returning %d, flag %d",
		len, flag?*flag:0 );
	return( len );
}

/*
 * Read status information from FD
 */
int Read_status_line( int fd )
{
	int blocking, count;
	char inbuff[SMALLBUFFER];

	inbuff[0] = 0;
	if((blocking = Get_block_io( fd ))){
		Set_nonblock_io( fd );
	}
	/* we read from stdout */
	count = read( fd, inbuff, sizeof(inbuff) - 1 );
	if(blocking){
		Set_block_io( fd );
	}
	if( count > 0 ){
		inbuff[count] = 0;
		Put_inbuf_str( inbuff );
	}
	DEBUG2("Read_status_line: read fd %d, count %d, '%s'",
		fd, count, inbuff );
	return( count );
}

/*
 * int Write_out_buffer( int outlen, char *outbuf, int maxtimeout )
 * We output the buffer, but we also read the status line
 * information as well.
 */

int Write_out_buffer( int outlen, char *outbuf, int maxtimeout )
{
	time_t current_time;
	int elapsed, timeout, len, flag, read_fd = 1;

	if( Status == 0 ) read_fd = -1;

	DEBUG1("Write_out_buffer: write len %d, read_fd %d", outlen, read_fd );
	while( outlen > 0 && outbuf ){ 
		time( &current_time );
		elapsed = current_time - Start_time;
		timeout = 0;
		if( maxtimeout > 0 ){
			timeout = maxtimeout - elapsed; 
			if( timeout <= 0 ) break;
		}
		len = Read_write_timeout( read_fd, &flag, 0,
			1, outbuf, outlen, timeout, -1 );
		DEBUG1("Write_out_buffer: left to write %d", len );
		if( len < 0 ){
			outlen = len;
		} else if( len >= 0 ){
			outbuf += (outlen - len);
			outlen = len;
		}
		if( flag ){
			if( Read_status_line( 1 ) <= 0 ){
				logerr("Write_out_buffer: read from printer failed");
				break;
			}
			/* get the status */
			while( Get_inbuf_str() );
		}
	}
	DEBUG1("Write_out_buffer: done, returning %d", outlen );
	return( outlen );
}

/*
 * void Resolve_key_val( char *prefix, char *key_val,
 *	Wr_out routine )
 * - determine what to do for the specified key and value
 *   we first check to see if it is a builtin
 *   if it is, then we do not need to process further
 */

 struct line_list setvals;

void Resolve_key_val( char *prefix, char *key_val, Wr_out routine )
{
	char *value, *id = 0, *prefix_id = 0, *s;
	int c = 0, done = 0;
	int depth = setvals.count;


	if( (value = strpbrk( key_val, Value_sep )) ){ c = *value; *value = 0; }
	id = safestrdup( key_val,__FILE__,__LINE__);
	if( value ) *value++ = c;
	if( value && *value == 0 ) value = 0;
	if( value ){
		Check_max(&setvals,1);
		setvals.list[setvals.count++] = safestrdup(key_val,__FILE__,__LINE__);
	}
	lowercase( id );
	/* decide if it is a built-in function or simply forcing a string out */
	DEBUG4("Resolve_key_val: prefix '%s', id '%s'='%s'",
		prefix,id,value);
	if( !done ){
		done = Builtin( prefix, id, value, routine);
		DEBUG4("Resolve_key_val: '%s' is builtin '%d'", id, done );
	}
	if( !done && !strncasecmp(prefix,"pjl", 3) ){
		done = Pjl_setvar( prefix, id, value, routine);
		DEBUG4("Resolve_key_val: '%s' is PJL '%d'", id, done );
	}
	if( !done ){
		prefix_id = safestrdup2( prefix, id,__FILE__,__LINE__);
		if( (s = Find_str_value( &Model, prefix_id, Value_sep )) ){
			DEBUG4("Resolve_key_val: string value of id '%s'='%s'", id, s);
			if( *s == '[' ){
				struct line_list l;

				Init_line_list( &l );
				Split( &l, s, List_sep, 0, 0, 0, 1, 0 );
				Resolve_list( prefix, &l, routine );
				Free_line_list( &l );
				done = 1;
			} else {
				routine( s );
				done = 1;
			}
		}
	}
	if( !done && value && *value && !Is_flag(value, 0) ){
		routine( Fix_val(value) );
		done = 1;
	}
	for( c = depth; c < setvals.count; ++c ){
		if( (s = setvals.list[c]) ) free(s);
		setvals.list[c] = 0;
	}
	setvals.count = depth;
	if(id)free(id); id = 0;
	if(prefix_id)free(prefix_id); prefix_id = 0;
}

int Is_flag( char *s, int *v )
{
	int n = 0;
	if( s && *s && strlen(s) == 1 && isdigit(cval(s)) &&
		(n = (*s == '0' || *s == '1' || *s == '@')) && v ){
		if( *s == '@' ){
			*v = 0;
		} else {
			*v = *s - '0';
		}
	}
	return( n );
}

/*
 * void Resolve_list( char *prefix, char **list, int count, W_out routine )
 * Resolve the actions specified by a list
 * List has the form [ xx xx xx or [xx xx xx
 */

void Resolve_list( char *prefix, struct line_list *l, Wr_out routine )
{
	int i;
	char *s;
	DEBUG4("Resolve_list: prefix '%s', count %d", prefix, l->count );
	if(DEBUGL4)Dump_line_list("Resolve_list",l);
	for( i = 0; i < l->count; ++i ){
		s = l->list[i];
		DEBUG4("Resolve_list: prefix '%s', [%d]='%s'", prefix, i, s );
		if( (*s == '[') || (*s == ']')) ++s;
		if( *s ){
			Resolve_key_val( prefix, s, routine );
		}
	}
}


/*
 * void Resolve_user_opts( char *prefix, struct line_list *only, *opts,
 *		W_out routine )
 * Resolve the actions specified by a the user and allowed as options
 * 
 */

void Resolve_user_opts( char *prefix, struct line_list *only,
	struct line_list *l, Wr_out routine )
{
	int i, c = 0, cmp;
	char *id = 0, *value;
	DEBUG4("Resolve_user_opts: prefix '%s'", prefix );
	if(DEBUGL4)Dump_line_list("Resolve_user_opts - only",only);
	if(DEBUGL4)Dump_line_list("Resolve_user_opts - list",l);
	for( i = 0; i < l->count; ++i ){
		id = l->list[i];
		if( (value = strpbrk( id, Value_sep )) ){ c = *value; *value = 0; }
		cmp = Find_first_key( only, id, Value_sep, 0 );
		DEBUG4("Resolve_user_opts: id '%s', Find_first=%d, value '%s'",
			id, cmp, value );
		if( cmp == 0 ){
			Resolve_key_val( prefix, l->list[i], routine );
		}
		if( value ){ *value = c; }
	}
}


/*
 * Fix_option_str( char *s, int remove_ws )
 *  if( remove_ws ) then remove all isspace() characters
 *  find and remove all white space before and after \n
 *  do escape substitutions
 */
char *Find_sub_value( int c, char *id );

char *Fix_option_str( char *str, int remove_ws, int trim )
{
	char *s, *t, *value;
	char num[127];
	char fmt[127];
	int c, insert, rest, len;
	char *sq, *eq;

	if( str == 0 ){ return str; }
	str = safestrdup( str,__FILE__,__LINE__);
	DEBUG5("Fix_option_str: orig '%s', remove_ws %d", str, remove_ws );
	if( remove_ws ){
		for( s = t = str; (*t = *s); ++s ){
			if( !isspace(cval(t)) ) ++t;
		}
		*t = 0;
	}
	if( trim ){
		for( s = str; (s = strpbrk( s, "\n" )); s = t ){
			for( t = s; t > str && isspace(cval(t-1)); --t );
			if( t != s ){
				memmove( t, s, strlen(s)+1 );
				s = t;
			}
			for( t = s+1; isspace(cval(t)); ++t );
			if( t != s+1 ){
				memmove( s+1, t, strlen(t)+1 );
				t = s+1;
			}
		}
	}
	DEBUG5("Fix_option_str: trimmed and ws removed '%s'", str );
	/* now we do escape sequences */
	for( s = str; (s = strchr( s, '\\' )); ){
		c = s[1];
		len = 1;
		insert = 0;

		DEBUG4("Fix_option_str: escape at '%s', '%c'", s, c );
		s[0] = c;
		switch( c ){
		case 'r': s[0] = '\r'; break;
		case 'n': s[0] = '\n'; break;
		case 't': s[0] = '\t'; break;
		case 'f': s[0] = '\f'; break;
		}
		if( isdigit( c ) ){
			for( len = 0;
				len < 3 && (num[len] = s[1+len]) && isdigit(cval(num+len));
				++len );
			num[len] = 0;
			s[0] = rest = strtol( num, 0, 8 );
			DEBUG5("Fix_option_str: number '%s', len %d, convert '%d'",
				num, len, rest );
		}
		memmove( s+1, s+len+1, strlen(s+len)+1 );
		s += 1;
		DEBUG5("Fix_option_str: after simple escape c '%c', '%s', next '%s'",
			c, str, s );
		/* we have \%fmt{key} */
		if( c == '%' ){
			--s;
			DEBUG5("Fix_option_str: var sub into '%s'", s );
			if( (sq = strpbrk( s, "[{" )) ){
				c = sq[0];
				*sq++ = 0;
				eq = strpbrk( sq, "]}" );
				if( eq ){
					*eq++ = 0;
				} else {
					eq = sq + strlen(sq);
				}
				len = eq - s;
				value = Find_sub_value( c, sq );
				plp_snprintf( fmt, sizeof(fmt), "%%%s", s+1 );
				DEBUG5("Fix_option_str: trying fmt '%s'", fmt );
				t = fmt+strlen(fmt)-1;
				c = *t;
				if( !islower(cval(t)) ){
					strcpy(fmt,"%d");
					c = 'd';
				}
				DEBUG5("Fix_option_str: using fmt '%s', type '%c', value '%s'",
					fmt, c, value );
				num[0] = 0;
				switch( c ){
				case 'o': case 'd': case 'x':
					plp_snprintf( num, sizeof(num), fmt, strtol(value, 0, 0) );
					break;
				case 'f': case 'g': case 'e':
					plp_snprintf( num, sizeof(num), fmt, strtod(value, 0));
					break;
				case 's':
					plp_snprintf( num, sizeof(num), fmt, value );
					break;
				}
				*s = 0;
				len = strlen(str)+strlen(num);
				DEBUG5("Fix_option_str: first '%s', num '%s', rest '%s'",
					str, num, eq );
				s = str;
				str = safestrdup3(str,num,eq,__FILE__,__LINE__);
				free(s);
				s = str+len;
				DEBUG5("Fix_option_str: result '%s', next '%s'", str, s );
			}
		}
	}
	DEBUG4("Fix_option_str: returning '%s'", str );
	return( str );
}

/*
 * char *Find_sub_value( int c, char *id, char *sep )
 *  if c == {, try the user -Z opts
 *  try user -T opts
 *  if id is single character try options
 *  try Model config
 *  default value is 0
 */

char *Find_sub_value( int c, char *id )
{
	char *s = 0;
	int i;
	DEBUG4("Find_sub_value: type '%c', id '%s'", c, id );
	if( s == 0 && !strcasecmp(id,"pid") ){
		static char pid[32];
		plp_snprintf(pid, sizeof(pid),"%d",getpid());
		s = pid;
	}
	if( s == 0 && setvals.count ){
		char *t, *u;
		int n;
		n = strlen(id);
		for( i = 0; i < setvals.count; ++i ){
			if( !strncasecmp( (t = setvals.list[i]), id, n) ){
				u = strpbrk(t,Value_sep);
				if( (u - t) == n ){
					s = u+1;
					DEBUG4("Find_sub_value: from setvals '%s', value '%s'",
						u,s);
					break;
				}
			}
		}
	}
	if( s == 0 && c == '{' ){
		s = Find_str_value( &Zopts, id, Value_sep );
		DEBUG4("Find_sub_value: from Zopts '%s'", s );
	}
	if( s == 0 ){
		s = Find_str_value( &Model, id, Value_sep );
		DEBUG4("Find_sub_value: from Model '%s'", s );
	}
	if( s == 0 && strlen(id) == 1 ){
		int n = id[0];
		if( isupper(n) ){
			s = Upperopts[n-'A'];
		} else if( islower(n) ){
			s = Loweropts[n-'a'];
		}
		DEBUG4("Find_sub_value: from opts '%s'", s );
	}
	if( s == 0 && c == '{' ){
		s = Find_exists_value( &Zopts, id, Value_sep );
		DEBUG4("Find_sub_value: from Zopts '%s'", s );
	}
	if( s == 0 ){
		s = Find_value( &Model, id, Value_sep );
		DEBUG4("Find_sub_value: from Model '%s'", s );
	}
	DEBUG4("Find_sub_value: type '%c', id '%s'='%s'", c, id, s );
	return( s );
}

/*
 * int Builtin( char* prefix, char *pr_id, char *value, Wr_out routine)
 *  - check to see if the pr_id is for a builtin operation
 *  - call the built-in with the parameters
 *  - if found, the invoked return should return return 1 
 *        so no further processing is done
 *    if not found or the return value is 0, then further processing is
 *    done.
 */

int Builtin( char* prefix, char *id, char *value, Wr_out routine)
{
	int cmp=1;
	char *s, *prefix_id = 0;
	Builtin_func r;
	struct keyvalue *v;

	DEBUG4("Builtin: looking for '%s'", id );
	for( v = Builtin_values;
		(s = v->key) && (cmp = strcasecmp(s, id));
		++v );
	if( cmp && prefix && *prefix ){
		prefix_id = safestrdup2( prefix, id,__FILE__,__LINE__);
		DEBUG4("Builtin: looking for '%s'", prefix_id );
		for( v = Builtin_values;
			(s = v->key) && (cmp = strcasecmp(s, prefix_id));
			++v );
	}
	if( cmp == 0 ){
		DEBUG4("Builtin: found '%s'", s );
		r = (Builtin_func)(v->var);
		cmp = (r)( prefix, id, value, routine);
	} else {
		cmp = 0;
	}
	DEBUG4("Builtin: returning '%d'", cmp );
	if( prefix_id){ free(prefix_id); prefix_id = 0; }
	return( cmp );
}

int Font_download( char* prefix, char *id, char *value, Wr_out routine)
{
	char *dirname = 0, *filename, *s;
	char buffer[LARGEBUFFER];
	int n, fd, i;
	struct line_list fontnames;

	DEBUG2("Font_download: prefix '%s', id '%s', value '%s'",prefix, id, value );

	Init_line_list(&fontnames);
	s = safestrdup2(prefix,"fontdir",__FILE__,__LINE__);
	DEBUG4("Font_download: directory name '%s'", s );
	dirname = Find_str_value( &Model, s, Value_sep );
	if(s) free(s); s = 0;
	if( !dirname ){
		logerr("Font_download: no value for %sfontdir", prefix);
		return(1);
	}

	/* if there is a value, then use it */
	if( (filename = Fix_option_str(value,1,1)) ){
		DEBUG2("Font_download: fixed value '%s'", filename );
		Split(&fontnames,filename,Filesep,0,0,0,0,0);
		if( filename ) free(filename); filename = 0;
	} else {
		/* if there is a 'font' value, then use it */
		filename = Find_sub_value( '{', "font" );
		Split(&fontnames,filename,Filesep,0,0,0,0,0);
	}
	if( fontnames.count == 0 ){
		log("Font_download: no 'font' value");
		return(1);
	}
	filename = 0;
	for( i = 0; i < fontnames.count; ++i ){
		if( filename ) free(filename); filename = 0;
		if( (s = strrchr(fontnames.list[i],'/')) ){
			++s;
		} else {
			s = fontnames.list[i];
		}
		filename = safestrdup3(dirname,"/",s,__FILE__,__LINE__);
		/* get rid of // */
		DEBUG4("Font_download: filename '%s'", filename );
		if( (fd = open( filename, O_RDONLY )) >= 0 ){
			while( (n = read(fd, buffer,sizeof(buffer))) >0 ){
				Put_outbuf_len( buffer, n );
			}
			close(fd);
		} else {
			logerr("Font_download: cannot open '%s'", filename);
		}
	}
	if( filename ) free(filename); filename = 0;
	Free_line_list(&fontnames);
	return(1);
}

/*
 * int Pjl_job()
 *  - put out the JOB START string
 */

 char *Jobstart_str="@PJL JOB NAME = \"%s\"";
 char Jobname[SMALLBUFFER];
 char *Jobend_str="@PJL EOJ NAME = \"%s\"";

void Pjl_job()
{
	char *str, *s, buffer[SMALLBUFFER];
	int n = 0;

	if( (s = Find_value(&Model,"pjl_job",Value_sep )) ){
		Is_flag(s, &n);
	}
	if( n && ( Find_first_key( &Pjl_only, "JOB", 0, 0)
			|| !Find_first_key( &Pjl_except, "JOB", 0, 0)) ){
		n = 0;
	}
	DEBUG2("Pjl_job: Pjl %d, job '%s', flag %d", Pjl, s, n );
	if( Pjl == 0 || n == 0 ){
		return;
	}
	s = Loweropts['n'-'a'];
	if( s == 0 ) s="";
	plp_snprintf( Jobname, sizeof(Jobname), "%s%sPID %d",
		s,*s?" ":"",getpid() );
	plp_snprintf( buffer, sizeof(buffer), Jobstart_str, Jobname );
	str = safestrdup( buffer,__FILE__,__LINE__);
	if( (s = Find_exists_value( &Topts, "startpage", Value_sep)) ){
		n = atoi( s );
		if( n > 0 ){
			plp_snprintf(buffer, sizeof(buffer), " START = %d", n );
			s = str;
			str = safestrdup2( str, buffer,__FILE__,__LINE__);
			free(s);
		}
	}
	if( (s = Find_exists_value( &Topts, "endpage", Value_sep)) ){
		n = atoi( s );
		if( n > 0 ){
			plp_snprintf(buffer, sizeof(buffer), " END = %d", n );
			s = str;
			str = safestrdup2( str, buffer,__FILE__,__LINE__);
			free(s);
		}
	}
	Put_pjl( str );
	free( str ); str = 0;
}

/*
 * int Pjl_eoj(char *prefix, char*id, char *value, Wr_out routine)
 *  - put out the JOB EOJ string
 */

void Pjl_eoj()
{
	char buffer[SMALLBUFFER];
	char *s;
	int n = 0;
	if( (s = Find_value(&Model,"pjl_job",Value_sep )) ){
		n = atoi(s);
	}
	DEBUG2("Pjl_eoj: Pjl %d, job '%s', flag %d", Pjl, s, n );
	if( Pjl == 0 || n == 0 ){
		return;
	}
	plp_snprintf( buffer, sizeof(buffer), Jobend_str, Jobname );
	Put_pjl( buffer );
}

/*
 * PJL RDYMSG option
 *  console    enables messages on console
 *  console@   disables or erases messages on console
 */

 char *PJL_RDYMSG_str = "@PJL RDYMSG DISPLAY = \"%s\"";

void Pjl_console( int start )
{
	char *s, buffer[SMALLBUFFER], name[SMALLBUFFER];
	int n = 0;

	DEBUG2( "Pjl_console: start %d", start );
	if( (s = Find_value(&Model,"pjl_console",Value_sep )) ){
		n = atoi(s);
	}
	DEBUG2("Pjl_console: Pjl %d, console '%s', flag %d", Pjl, s, n );
	if( Pjl == 0 || n == 0 ){
		return;
	}
	s = "";
	if( start && ((s = Loweropts['n'-'a']) == 0 || *s == 0) ){
		s = name;
		plp_snprintf(name,sizeof(name), "PID %d", getpid());
	}
	plp_snprintf( buffer, sizeof(buffer), PJL_RDYMSG_str, s );
	Put_pjl( buffer );
}

/*
 * PJL SET VAR=VALUE
 *  1. The variable must be in the Pjl_vars_set list
 *     This list has entries of the form:
 *        var=value var=value
 *        var@  -> var=OFF  
 *        var   -> var=ON  
 *  2. The variable must not be in the Pjl_vars_except list
 *     This list has entries of the form
 *        var  var var
 */

 char *PJL_SET_str = "@PJL SET %s = %s";

int Pjl_setvar(char *prefix, char*id, char *value, Wr_out routine)
{
	int found = 0, i;
	char buffer[SMALLBUFFER], *s;

	DEBUG3( "Pjl_setvar: prefix '%s', id '%s', value '%s'",
		prefix, id, value );
	if( !Find_first_key( &Pjl_vars_set, id, Value_sep, &i ) ){
		s = strpbrk( Pjl_vars_set.list[i], Value_sep );
		DEBUG3( "Pjl_setvar: found id '%s'='%s'", id, s );
		if( Find_first_key( &Pjl_vars_except, id, Value_sep, 0 ) ){
			DEBUG3( "Pjl_setvar: not in the except list" );
			if( value == 0 || *value == 0 ){
				value = s;
			}
			if( value == 0 || *value == 0 ){
				value = "ON";
			} else if( *value =='@' ){
				value = "OFF";
			} else if( *value == '=' || *value == '#' ){
				/* skip over = and # */
				++value;
			}
			DEBUG3( "Pjl_setvar: setting '%s' = '%s'", id, value );
			plp_snprintf(buffer,sizeof(buffer),PJL_SET_str,id,value);
			uppercase(buffer);
			routine(buffer);
			found = 1;
		}
	}
	return(found);
}

int Pcl_setvar(char *prefix, char*id, char *value, Wr_out routine )
{
	int found = 0, i;
	char *s;

	DEBUG3( "Pcl_setvar: prefix '%s', id '%s', value '%s'",
		prefix, id, value );
	if( !Find_first_key( &Pcl_vars_set, id, Value_sep, &i ) ){
		s = strpbrk( Pcl_vars_set.list[i], Value_sep );
		DEBUG3( "Pcl_setvar: found id '%s'='%s'", id, s );
		if( Find_first_key( &Pcl_vars_except, id, Value_sep, 0 ) ){
			DEBUG3( "Pcl_setvar: not in the except list" );
			value = s;
			if( value == 0 || *value == 0 ){
				value = 0;
			} else if( *value =='@' ){
				value = 0;
			} else if( *value == '=' || *value == '#' ){
				/* skip over = and # */
				++value;
			}
			DEBUG3( "Pcl_setvar: setting '%s' = '%s'", id, value );
			if( value ){
				routine(value);
			}
			found = 1;
		}
	}
	return(found);
}

/*
 * Do_sync()
 *  actual work is done here later
 *  sync=pjl  enables pjl
 *  sync=ps   enables postscript
 *  sync@     disables sync
 *  default is to use the prefix value
 */

 char *PJL_ECHO_str = "@PJL ECHO %s";
 char *CTRL_T = "\024";
 char *CTRL_D = "\004";

 char *PS_status_start = "%!PS-Adobe-2.0\n( %%[ echo: ";
 char *PS_status_end = " ]%% ) print () = flush\n";

/*
 * void Do_sync( int sync_timeout )
 * - get a response from the printer
 */

void Do_sync( int sync_timeout )
{
	char buffer[SMALLBUFFER], name[SMALLBUFFER], *s, *sync_str;
	int len, flag, elapsed, timeout, sync, use, use_ps, use_pjl;
	time_t start_t, current_t, interval_t;

	time( &start_t );
 again:

	sync_str = s = Find_value(&Model,"sync",Value_sep);
	DEBUG2("Do_sync: sync is '%s'", s );

	/* we see if we can do sync */
	use = use_pjl = use_ps = 0;
	if( !Is_flag( s, &use ) ){
		/* we check to see if the string is specifying */
		if( !strcasecmp(s,"pjl") && Pjl ){
			use_pjl = 1;
		}
		if( !strcasecmp(s,"ps") && Ps ){
			use_ps = 1;
		}
		use = 1;
	} else if( use ){
		use_pjl = Pjl;
		use_ps = Ps;
	} else {
		return;
	}
	if( use_pjl
		&& ( Find_first_key( &Pjl_only, "ECHO", 0, 0)
			|| !Find_first_key( &Pjl_except, "ECHO", 0, 0)) ){
		use_pjl = 0;
	}
	if( !use_ps && !use_pjl ){
		Errorcode = JABORT;
		fatal("Do_sync: sync '%s' and method not supported", s );
	}
	if( use_pjl ) use_ps = 0;
	s = use_pjl?"pjl":"ps";

	sync_str = s;
	log("Do_sync: getting sync using '%s'", sync_str );

	/* get start time */
	sync = 0;
	time( &interval_t );
	Init_outbuf();
	plp_snprintf( name, sizeof(name), "%d@%s", getpid(), Time_str(0,0) );
	if( use_pjl ){
		Put_outbuf_str( PJL_UEL_str );
		plp_snprintf( buffer, sizeof(buffer),
			PJL_ECHO_str, name );
		Put_pjl( buffer );
		Put_outbuf_str( PJL_UEL_str );
	} else if( use_ps ){
		if(Pjl){
			Put_outbuf_str( PJL_UEL_str );
			if( Pjl_enter ){
				Put_outbuf_str( "@PJL ENTER LANGUAGE = POSTSCRIPT\n" );
			}
		}
		Put_outbuf_str( CTRL_T );
		Put_outbuf_str( PS_status_start );
		Put_outbuf_str( name );
		Put_outbuf_str( PS_status_end );
		Put_outbuf_str( CTRL_D );
		if(Pjl){
			Put_outbuf_str( PJL_UEL_str );
		}
	} else {
		Errorcode = JABORT;
		fatal("Do_sync: no way to synchronize printer, need PJL or PS" );
	}

	while( !sync ){
		flag = 0;
		/* write the string */
		/* check to see if we need to read */
		time( &current_t );
		elapsed = current_t - start_t;

		/* check for master timeout */
		if( sync_timeout > 0 ){
			if( elapsed >= sync_timeout ){
				break;
			}
			timeout = sync_timeout - elapsed;
		} else {
			timeout = 0;
		}
		if( Sync_interval > 0 ){
			elapsed = current_t - interval_t;
			if( elapsed > Sync_interval ){
				goto again;
			}
			if( timeout == 0 || timeout > Sync_interval ){
				timeout = Sync_interval;
			}
		}
		DEBUG3("Do_sync: waiting for sync, timeout %d", timeout );
		len = Read_write_timeout( 1, &flag, timeout,
			1, Outbuf, Outlen, timeout, -1 );
		DEBUG3("Do_sync: read/write len '%d'", len );
		if( len < 0 ){
			break;
		} else if( len >= 0 ){
			memmove(Outbuf, Outbuf+(Outlen - len), len+1 );
			Outlen = len;
		}
		if( flag ){
			if( Read_status_line( 1 ) <= 0 ){
				logerr("Do_sync: read from printer failed");
				break;
			}
			/* get the status */
			while( Get_inbuf_str() );
			if( (s = Find_str_value( &Devstatus, "echo", Value_sep))
				&& strstr( s, name ) ){
				sync = 1;
			}
		}
	}
	DEBUG2("Do_sync: sync %d", sync );
	if( sync == 0 ){
		Errorcode = JFAIL;
		fatal("Do_sync: no response from printer" );
	}
	log("Do_sync: sync done");
}

/*
 * void Do_waitend( int sync_timeout )
 * - get the job status from the printer
 *   and wait for the end of job
 */

 char *PJL_USTATUS_JOB_str = "@PJL USTATUS JOB = ON";

void Do_waitend( int waitend_timeout )
{
	char *s, *t, buffer[SMALLBUFFER];
	int len, flag, elapsed, timeout, waitend,
		use, use_pjl, use_ps, use_job;
	time_t start_t, current_t, interval_t;

	time( &start_t );
 again:
	s = Find_value(&Model,"sync",Value_sep);
	if( s && Find_exists_value(&Model,"waitend",Value_sep) ){
		s = Find_value(&Model,"waitend",Value_sep);
	}
	DEBUG3("Do_waitend: getting end using '%s'", s );

	/* we see if we can do sync */
	use = use_pjl = use_ps = use_job = 0;
	if( !Is_flag( s, &use ) ){
		/* we check to see if the string is specifying */
		if( !strcasecmp(s,"pjl") ){
			use_pjl = Pjl;
		}
		if( !strcasecmp(s,"ps") ){
			use_ps = Ps;
		}
		use = 1;
	} else if( use ){
		use_job = use_pjl = Pjl;
		use_ps = Ps;
	} else {
		return;
	}

	if( use_pjl
		&& ( Find_first_key( &Pjl_only, "JOB", 0, 0)
			|| !Find_first_key( &Pjl_except, "JOB", 0, 0)
			) ){
		use_job = 0;
	}
	if( use_pjl
		&& ( Find_first_key( &Pjl_only, "JOB", 0, 0)
			|| !Find_first_key( &Pjl_except, "JOB", 0, 0)
			) ){
		use_job = 0;
	}
	if( use_pjl && use_job == 0
		&& ( Find_first_key( &Pjl_only, "ECHO", 0, 0)
			|| !Find_first_key( &Pjl_except, "ECHO", 0, 0)
			) ){
		use_pjl = 0;
	}
	if( !use_ps && !use_pjl ){
		Errorcode = JABORT;
		fatal("Do_waitend: waitend '%s' and method not supported", s );
	}
	if( use_pjl ) use_ps = 0;
	s = use_pjl?"pjl":"ps";

	log("Do_waitend: getting end using '%s'", s );

	waitend = 0;

	/* find if we need to periodically send the waitend value */
	DEBUG3("Do_waitend: use_pjl %d, use_job %d, use_ps %d, timeout %d, interval %d",
		use_pjl, use_job, use_ps, waitend_timeout, Waitend_interval );

	Add_line_list( &Devstatus, "job=", Value_sep, 1, 1 );
	Add_line_list( &Devstatus, "name=", Value_sep, 1, 1 );
	Add_line_list( &Devstatus, "echo=", Value_sep, 1, 1 );

	Init_outbuf();
	plp_snprintf( Jobname, sizeof(Jobname), "%s PID %d",
		Time_str(1,0), getpid() );
	if( use_pjl ){
		Put_outbuf_str( PJL_UEL_str );
		s = Loweropts['n'-'a'];
		if( s == 0 ) s="";
		plp_snprintf( buffer, sizeof(buffer), Jobstart_str, Jobname );
		Put_pjl(buffer);
		if( use_job ){
		Put_pjl( PJL_USTATUS_JOB_str );
			plp_snprintf( buffer, sizeof(buffer), Jobend_str, Jobname );
			Put_pjl(buffer);
		} else {
			plp_snprintf( buffer, sizeof(buffer), PJL_ECHO_str, Jobname );
			Put_pjl(buffer);
		}
		Put_outbuf_str( PJL_UEL_str );
	} else if( use_ps ){
		if(Pjl){
			Put_outbuf_str( PJL_UEL_str );
			if( Pjl_enter ){
				Put_outbuf_str( "@PJL ENTER LANGUAGE = POSTSCRIPT\n" );
			}
		}
		Put_outbuf_str( CTRL_T );
		if(Pjl){
			Put_outbuf_str( PJL_UEL_str );
		}
	}

	time( &interval_t );

	DEBUG3("Do_waitend: sending '%s'", Outbuf );
	while( !waitend ){
		flag = 0;
		/* write the string */
		/* check to see if we need to read */
		time( &current_t );
		elapsed = current_t - start_t;

		/* check for master timeout */
		if( waitend_timeout > 0 ){
			if( elapsed >= waitend_timeout ){
				break;
			}
			timeout = waitend_timeout - elapsed;
		} else {
			timeout = 0;
		}
		if( Waitend_interval > 0 ){
			elapsed = current_t - interval_t;
			if( elapsed > Waitend_interval ){
				goto again;
			}
			if( timeout == 0 || timeout > Waitend_interval ){
				timeout = Waitend_interval;
			}
		}
		len = Read_write_timeout( 1, &flag, timeout,
			1, Outbuf, Outlen, timeout, -1 );
		DEBUG3("Do_waitend: write len %d", len );
		if( len < 0 ){
			break;
		} else if( len >= 0 ){
			memmove(Outbuf, Outbuf+(Outlen - len), len);
			Outlen = len;
		}
		if( flag ){
			if( Read_status_line( 1 ) <= 0 ){
				logerr("Do_waitend: read from printer failed");
				break;
			}
			/* get the status */
			while( Get_inbuf_str() );
			if(DEBUGL4)Dump_line_list("Do_waitend - Devstatus", &Devstatus );
			if( use_pjl ){
				s = Find_str_value( &Devstatus, "job", Value_sep);
				t = Find_str_value( &Devstatus, "name", Value_sep);
				DEBUG4("Do_waitend: job '%s', name '%s', Jobname '%s'", s, t, Jobname );
				if( s && strstr(s,"END") && t && strstr(t,Jobname) ){
					waitend = 1;
				}
				s = Find_str_value( &Devstatus, "echo", Value_sep);
				DEBUG4("Do_waitend: echo '%s'", s );
				if( s && strstr(s,Jobname) ){
					waitend = 1;
				}
			} else if( use_ps ){
				if( (s = Find_str_value( &Devstatus, "status", Value_sep)) ){
					s = safestrdup(s,__FILE__,__LINE__);
					DEBUG4("Do_waitend: status '%s'", s );
					lowercase( s );
					if( !strstr(s, "printing") ){
						waitend = 1;
					}
					free(s); s = 0;
				}
			}
		}
	}
	if( waitend == 0 ){
		Errorcode = JFAIL;
		fatal("Do_waitend: no response from printer" );
	}
	DEBUG3("Do_waitend: end of print job" );
}

/*
 * int Do_pagecount(void)
 * - get the pagecount from the printer
 *  We can use the PJL or PostScript method
 *  If we use PJL, then we send pagecount command, and read status
 *   until we get the value.
 *  If we use PostScript,  we need to set up the printer for PostScript,
 *   send the PostScript code,  and then get the response.
 *
 * INFO PAGECOUNT Option
 * pagecount=pjl   - uses PJL (default)
 * pagecount=ps    - uses PS
 * pagecount@
 *
 */

int Do_pagecount( int pagecount_timeout )
{
	int len, flag, elapsed, timeout, pagecount = 0, new_pagecount = -1,
		use, use_ps, use_pjl;
	time_t start_t, current_t;
	char *s;

	s = Find_value(&Model,"pagecount",Value_sep);
	DEBUG4("Do_pagecount: getting pagecount using '%s'", s );

	/* we see if we can do sync */
	use = use_pjl = use_ps = 0;
	if( !Is_flag( s, &use ) ){
		/* we check to see if the string is specifying */
		if( !strcasecmp(s,"pjl") ){
			use_pjl = 1;
		}
		if( !strcasecmp(s,"ps") ){
			use_ps = 1;
		}
		use = 1;
	} else if( use ){
		use_pjl = Pjl;
		use_ps = Ps;
	} else {
		return 0;
	}
	if(use_ps && !Pagecount_ps_code){
		use_ps = 0;
	}
	if(use_pjl
		&& (Find_first_key( &Pjl_only, "INFO", 0, 0)
			||  !Find_first_key( &Pjl_except, "INFO", 0, 0)) ){
		use_pjl = 0;
	}
	if(use_ps && !Pagecount_ps_code){
		use_ps = 0;
	}

	if( !use_ps && !use_pjl ){
		Errorcode = JABORT;
		fatal("Do_pagecount: pagecount '%s' and method not supported", s );
	}
	if( use_pjl ) use_ps = 0;
	s = use_pjl?"pjl":"ps";

	log("Do_pagecount: getting pagecount using '%s'", s );

	pagecount = Current_pagecount( pagecount_timeout, use_pjl, use_ps );
	if( pagecount >= 0 && Pagecount_poll > 0 ){
		time( &start_t );
		new_pagecount = pagecount;
		log("Do_pagecount: pagecount %d, polling in %d seconds",
			pagecount, Pagecount_poll );
		do{
			pagecount = new_pagecount;
			while( 1 ){
				time( &current_t );
				elapsed = current_t - start_t;
				timeout = Pagecount_poll - elapsed;
				if( timeout <= 0 ){
					break;
				}
				DEBUG1("Do_pagecount: pagecount poll %d, waiting %d",
					Pagecount_poll, timeout );
				len = Read_write_timeout( 1, &flag, timeout,
					-1, 0, 0, 0, -1 );
				DEBUG1("Do_pagecount: waiting, read len %d", len );
				if( len < 0 ){
					break;
				}
				if( flag ){
					if( Read_status_line( 1 ) <= 0 ){
						logerr("Do_pagecount: read from printer failed");
						break;
					}
					while( Get_inbuf_str() );
				}
			}
			DEBUG1("Do_pagecount: polling, getting count again");
			new_pagecount = Current_pagecount(pagecount_timeout,
				use_pjl, use_ps);
		} while( new_pagecount != pagecount );
	}
	log("Do_pagecount: final pagecount %d", pagecount);
	return( pagecount );
}

 char *PJL_INFO_PAGECOUNT_str = "@PJL INFO PAGECOUNT \n";

int Current_pagecount( int pagecount_timeout, int use_pjl, int use_ps )
{
	int i, len, flag, elapsed, timeout, page, pagecount = 0, found;
	time_t start_t, current_t, interval_t;
	char *s;

	/* get start time */
	time( & start_t );
 again:

	DEBUG1("Current_pagecount: starting, use_pjl %d, use_ps %d, timeout %d",
		use_pjl, use_ps, pagecount_timeout );

	/* remove the old status */
	Add_line_list( &Devstatus, "pagecount=", Value_sep, 1, 1 );

	Init_outbuf();
	if( use_pjl ){
		Put_outbuf_str( PJL_UEL_str );
		Put_pjl( PJL_INFO_PAGECOUNT_str );
		Put_outbuf_str( PJL_UEL_str );
	} else if( use_ps ){
		if( Pagecount_ps_code && *Pagecount_ps_code ){
			if( Pjl ){
				Put_outbuf_str( PJL_UEL_str );
				if( Pjl_enter ){
					Put_outbuf_str( "@PJL ENTER LANGUAGE = POSTSCRIPT\n" );
				}
			}
			Put_outbuf_str( CTRL_D );
			Put_outbuf_str( Pagecount_ps_code );
			Put_outbuf_str( "\n" );
			Put_outbuf_str( CTRL_D );
			if( Pjl ) Put_outbuf_str( PJL_UEL_str );
		} else {
			Errorcode = JABORT;
			fatal("Current_pagecount: no pagecount_ps_code config info");
		}
	}

	page = 0;
	found = 0;
	while(!page){
		time( & interval_t );
		flag = 0;
		/* write the string */
		/* check to see if we need to read */
		time( &current_t );
		elapsed = current_t - start_t;
		if( pagecount_timeout > 0 ){
			if( elapsed >= pagecount_timeout ){
				break;
			}
			timeout = pagecount_timeout - elapsed;
		} else {
			timeout = 0;
		}
		if( Pagecount_interval > 0 ){
			elapsed = current_t - interval_t;
			if( elapsed > Pagecount_interval ){
				goto again;
			}
			if( timeout == 0 || timeout > Pagecount_interval ){
				timeout = Pagecount_interval;
			}
		}
		len = Read_write_timeout( 1, &flag, timeout,
			1, Outbuf, Outlen, timeout, -1 );
		DEBUG1("Current_pagecount: write len %d", len );
		if( len < 0 ){
			break;
		} else if( len >= 0 ){
			memmove(Outbuf, Outbuf+(Outlen - len), len);
			Outlen = len;
		}
		if( flag ){
			if( Read_status_line( 1 ) <= 0 ){
				logerr("Current_pagecount: read from printer failed");
				break;
			}
			while( (s = Get_inbuf_str() ) );
		}
		if( !Find_first_key( &Devstatus, "pagecount", Value_sep, &i )){
			s = Find_str_value( &Devstatus, "pagecount", Value_sep );
			if( s && isdigit(cval(s)) ){
				pagecount = atoi( s );
				page = 1;
			}
		}
	}
	DEBUG1("Current_pagecount: page %d, pagecount %d", page, pagecount );
	if( page == 0 ){
		Errorcode = JFAIL;
		fatal("Current_pagecount: no response from printer" );
	}
	return( pagecount );
}


/*
 * Send_job
 * 1. we read an input buffer
 * 2. we then check to see what type it is
 * 3. we then set up the various configuration
 * 4. we then send the job to the remote end
 * Note: there is a problem when you send language switch commands
 *  to the HP 4M+, and other printers.
 * two functions.
 *
 * The following comments from hp.com try to explain the situation:
 *
 *"What is happening is the printer buffer fills with your switch
 * command and the first parts of your job. The printer does not
 * "prescan" the input looking for the <Esc>%-1234... but instead gets
 * around to it after some time has expired. During the switch, the
 * data that you have already trasmitted is discarded and is picked
 * up whereever your driver happens to be. For PostScript that's a
 * messy situation.
 * 
 * What you need to do is isolate the switch command. You can do this
 * with nulls if you have no control over the timing. 8K of them will
 * fill the largest allocated I/O buffer and then hold off the UNIX
 * system. The switch will be made and the initial remaining nulls
 * will be discarded. If you can control the timing of the data, you'll
 * have to experiment with the correct time.
 * 
 */

 char *PJL_ENTER_str = "@PJL ENTER LANGUAGE = %s\n";
 char *PCL_EXIT_str = "\033E";

void Send_job()
{
	plp_status_t status;
	int len = 0, i, c, pid = 0, n, p[2], cnt;
	char *s, *pgm = 0;
	int done = 0;
	char *save_outbuf;
	int save_outlen, save_outmax;
	struct line_list l;
	int mode;
	char *language;
	struct stat statb;
	int progress_last, total_size, is_file, progress_pc, progress_k,
		progress_total;

	Init_line_list( &l );
	Init_outbuf();
	
	log( "Send_job: starting transfer" );
	DEBUG2( "Send_job: want %d", Outmax );
	while( Outlen < Outmax 
		&& (len = read( 0, Outbuf+Outlen, Outmax - Outlen )) > 0 ){
		Outlen += len;
	}
	Outbuf[Outlen] = 0;
	DEBUG2( "Send_job: read %d from stdin", Outlen );
	if( len < 0 ){
		Errorcode = JFAIL;
		logerr_die( "Send_job: read error on stdin" );
	}
	if( Outlen == 0 ){
		log( "Send_job: zero length job file" );
		return;
	}
	/* defaults */
	if( !(s = Find_str_value(&Model,"default_language",Value_sep)) ){
		s = "unknown";
	}
	if( Set_mode_lang( s,  &mode, &language ) ){
		Errorcode = JABORT;
		fatal( "Send_job: bad initial value '%s'", s );
	}

	pgm = 0;
	/* decide on the job type */
	if( Loweropts['c'-'a'] || Autodetect ){
		mode = RAW;
		language = 0;
	} else if( (s = Find_str_value(&Zopts,"language",Value_sep)) ){
		if( Set_mode_lang( s, &mode, &language ) ){
			Errorcode = JABORT;
			fatal( "Send_job: bad '-Zlanguage=%s' option", s );
		}
	} else if( Outbuf[0] == '\033' ){
		mode = PCL;
		language = "PCL";
		/* remove \033E at start */
		if( Outbuf[1] == 'E' ){
			memmove(Outbuf,Outbuf+2,Outlen-2);
			Outlen -= 2;
		}
	} else if( Outbuf[0] == '\004' ){
		/* remove the ^D at input */
		mode = PS;
		language = "POSTSCRIPT";
		memmove(Outbuf,Outbuf+1,Outlen-1);
		--Outlen;
	} else if( !memcmp(Outbuf, "%!", 2) ){
		mode = PS;
		language = "POSTSCRIPT";
	} else if( mode == UNKNOWN
		&& (pgm = Find_str_value( &Model, "file_util_path", Value_sep ))){
		DEBUG2( "Send_job: file_util_path '%s'", pgm );
		s = Use_file_util(pgm);
		if( Set_mode_lang(s, &mode, &language) ){
			Errorcode = JABORT;
			fatal("Send_job: bad file type '%s'", s);
		}
		if( mode == UNKNOWN ){
			Errorcode = JABORT;
			fatal("Send_job: unknown file type '%s'", s);
		}
		pgm = 0;
	}

	if( mode == TEXT ){
		s = Find_str_value( &Model,
			"text_converter_output", Value_sep );
		if( s && Set_mode_lang( s, &mode, &language ) ){
			Errorcode = JABORT;
			fatal( "Send_job: bad default_job_mode '%s'", s );
		}
		pgm = Find_str_value( &Model, "text_converter", Value_sep );
		if( mode == TEXT && !Text && !pgm ){
			Errorcode = JABORT;
			fatal("Send_job: job is text, and no converter available");
		}
	}


	if( mode == PCL && !Pcl ){
		Errorcode = JABORT;
		fatal("Send_job: job is PCL and PCL not supported");
	}
	if( mode == PS && !Ps ){
		Errorcode = JABORT;
		fatal("Send_job: job is PostScript and PostScript not supported");
	}
	if( mode == TEXT && !Text ){
		Errorcode = JABORT;
		fatal("Send_job: job is Text and Text not supported");
	}

	if( pgm ){
		DEBUG2( "Send_job: text_converter '%s'", pgm );
		/* set up pipe */
		Make_stdin_file();
		if( pipe(p) == -1 ){
			Errorcode = JABORT;
			logerr_die("Send_job: pipe()) failed");
		}
		/* fork the process */
		if( (pid = fork()) == -1 ){
			Errorcode = JABORT;
			logerr_die("Send_job: fork failed");
		} else if( pid == 0 ){
			Init_line_list(&l);
			Split(&l,pgm,Whitespace,0,0,0,0, 0 );
			Check_max(&l,1);
			l.list[l.count] = 0;
			for( i = 0; i < l.count; ++i ){
				l.list[i] = Fix_option_str( l.list[i], 0, 1 );
			}
			if( dup2(p[1],1) == -1 ){
				Errorcode = JABORT;
				logerr_die("Send_job: dup2 failed");
			}
			close_on_exec(3);
			execve( l.list[0], l.list, Envp );
			/* now we exec the filter process */
			Errorcode = JABORT;
			logerr_die( "Send_job: execv failed" );
		}
		log("Send_job: started converter");
		if( dup2(p[0],0) == -1 ){
			Errorcode = JABORT;
			logerr_die("Send_job: dup2 p[0] failed");
		}
		/* close the pipes */
		close(p[1]); close(p[0]);
	}


	save_outbuf = Outbuf; Outbuf = 0;
	save_outlen = Outlen; Outlen = 0;
	save_outmax = Outmax; Outmax = LARGEBUFFER;
	Init_outbuf();

	Init_job( language, mode );

	/* You can now work with the big buffer again */
	if( Outbuf ){ free(Outbuf); };
	Outbuf = save_outbuf;
	Outlen = save_outlen;
	Outmax = save_outmax;

	DEBUG1("Send_job: doing file transfer");
	if( fstat( 0, &statb ) < 0 ){
		Errorcode = JABORT;
		logerr_die( "Send_job: cannot fstat fd 0" );
	}
	is_file = ((statb.st_mode & S_IFMT) == S_IFREG);
	total_size = statb.st_size;

	progress_total = progress_last =  progress_pc = progress_k = 0;
	if( (s = Find_str_value(&Model,"progress_pc",Value_sep)) ){
		progress_pc = atoi(s);
	}
	if( (s = Find_str_value(&Model,"progress_k",Value_sep)) ){
		progress_k = atoi(s);
	}
	if( !is_file || progress_pc < 0 || progress_pc > 100 ){
		progress_pc = 0;
	}
	if( progress_k <= 0 ){
		progress_k = 0;
	}
	if( progress_pc == 0 && progress_k == 0 ){
		progress_k = 100;
	}
	DEBUG2( "Send_job: input stat 0%06o, is_file %d, size %d, progress_pc %d, progress_k %d",
		(int)(statb.st_mode & S_IFMT),
		is_file, total_size, progress_pc, progress_k );

	if( mode == PS && Remove_ctrl ){
		for( i = 0; i < strlen(Remove_ctrl); ++i ){
			Remove_ctrl[i] &= 0x1F;
		}
	}

	do{
		len = 0;
		progress_total += Outlen;
		if( mode == PS && Tbcp ){
			DEBUG4("Send_job: tbcp");
			for( cnt = 0; cnt < Outlen; ){
				len = 0; c = 0;
				for( i = cnt; c == 0 && i < Outlen; ){
					switch( (c = ((unsigned char *)Outbuf)[i]) ){
					case 0x01: case 0x03: case 0x04: case 0x05:
					case 0x11: case 0x13: case 0x14: case 0x1C:
						break;
					default: c = 0; ++i;
						break;
					}
				}
				/* we now write the string from cnt to count+i-1 */
				n = i - cnt;
				DEBUG1("Send_job: tbcp writing %d", len );
				if( (len = Write_out_buffer( n, Outbuf+cnt, Job_timeout )) ){
					break;
				}
				if( c ){
					char b[2];
					DEBUG1("Send_job: tbcp escape 0x%02x", c );
					b[0] = '\001'; b[1] = c ^ 0x40;
					if( (len = Write_out_buffer( 2, b, Job_timeout )) ){
						break;
					}
					n += 1;
				}
				cnt += n;
			}
		} else if( mode == PS && Remove_ctrl ){
			for( i = 0; i < Outlen; ){
				c = cval(Outbuf+i);
				if( strchr( Remove_ctrl,c ) ){
					memmove( Outbuf+i, Outbuf+i+1,Outlen-i);
					--Outlen;
				} else {
					++i;
				}
			}
			len = Write_out_buffer( Outlen, Outbuf, Job_timeout );
		} else if( mode != RAW && Crlf ){
			DEBUG4("Send_job: crlf");
			for( cnt = 0; cnt < Outlen; ){
				len = 0; c = 0;
				for( i = cnt; c == 0 && i < Outlen; ){
					switch( (c = ((unsigned char *)Outbuf)[i]) ){
					case '\n':
						break;
					default: c = 0; ++i;
						break;
					}
				}
				/* we now write the string from cnt to count+i-1 */
				n = i - cnt;
				DEBUG1("Send_job: crlf writing %d", n );
				if( (len = Write_out_buffer( n, Outbuf+cnt, Job_timeout )) ){
					break;
				}
				if( c ){
					if( (len = Write_out_buffer( 2, "\r\n", Job_timeout )) ){
						break;
					}
					n += 1;
				}
				cnt += n;
			}
		} else {
			DEBUG1("Send_job: writing %d", Outlen );
			len = Write_out_buffer( Outlen, Outbuf, Job_timeout );
		}
		if( len > 0 ){
			Errorcode = JFAIL;
			fatal( "Send_job: job timed out" );
		} else if( len < 0 ){
			Errorcode = JFAIL;
			logerr_die( "Send_job: job failed during copy" );
		}
		Init_outbuf();
		DEBUG1("Send_job: total writen %d", progress_total );
		/*
		 * now we do progress report
		 */
		if( progress_pc ){
			i = ((double)(progress_total)/total_size)*100;
			DEBUG1("Send_job: pc total %d, new %d", progress_total, i );
			if( i > progress_last ){
				log("Send_job: %d percent done", i);
				progress_last = i;
			}
		} else if( progress_k ){
			i = ((double)(progress_total)/(1024 * (double)progress_k));
			DEBUG1("Send_job: k total %d, new %d", progress_total, i );
			if( i > progress_last ){
				log("Send_job: %d Kbytes done", i);
				progress_last = i;
			}
		}
		if( Outlen == 0 && !done ){
				DEBUG1("Send_job: read %d", len );
			while( Outlen < Outmax 
					&& (len = read( 0, Outbuf+Outlen, Outmax - Outlen )) > 0 ){
				DEBUG1("Send_job: read %d", len );
				Outlen += len;
			}
			Outbuf[Outlen] = 0;
			DEBUG1("Send_job: read total %d", Outlen );
			if( len < 0 ){
				Errorcode = JFAIL;
				logerr_die( "Send_job: read error on stdin" );
			} else if( len == 0 ){
				done = 1;
			}
		}
	} while( Outlen > 0 );
	/* we are done on STDIN */
	/* close(0); */
	if( pgm ){
		while( (n = waitpid(pid,&status,0)) != pid );
		if( WIFEXITED(status) && (n = WEXITSTATUS(status)) ){
			Errorcode = JABORT;
			fatal("Send_job: text process exited with status %d", n);
		} else if( WIFSIGNALED(status) ){
			Errorcode = JABORT;
			n = WTERMSIG(status);
			fatal("Send_job: text process died with signal %d, '%s'",
				n, Sigstr(n));
		}
		log("Send_job: converter done");
	}
	DEBUG1( "Send_job: finished file transfer" );

	Term_job( mode );

	log( "Send_job: finished writing file, cleaning up" );
}

/*
 * Builtins
 */

 struct keyvalue Builtin_values[] = {
/* we get the various UEL strings */
{"Font_download","font_download",(void *)Font_download},
{0,0,0}
};


/*
 * Process_OF_mode()
 *  We read from STDIN on a character by character basis.
 *  We then send the output to STDOUT until we are done
 *  If we get a STOP sequence, we suspend ourselves.
 *
 *  We actually have to wait on 3 FD's -
 *  0 to see if we have input
 *  1 to see if we have output
 *  2 to see if there is an error
 *
 *  We will put the output in the Outbuf buffer
 *  Note:  if we need to suspend,  we do this AFTER we have
 *   written the output
 */

 static char stop[] = "\031\001";    /* sent to cause filter to suspend */

void Process_OF_mode( int job_timeout )
{
    fd_set readfds, writefds; /* for select() */
	struct timeval timeval, *tp;
	time_t current_t;
	int input, output, monitor, state = 0, suspend = 0,
		inputblocking, outputblocking, monitorblocking,
		elapsed, count = 0, i, m, err, c, banner_done = 0;
	char *s;
	char buffer[LARGEBUFFER];

	/* check to see if we can print banner
	 * we need to have
	 *  banner=type
	 */

	/* do we do banner processing? */

	count = 0;
	buffer[count] = 0;

	DEBUG2("Process_OF_mode: starting, Banner_only '%s', Banner '%s'",
		Banner_only, Banner);

	input = 0; output = 1; monitor = 1;
	inputblocking = outputblocking =  monitorblocking = 0;
	if( Status == 0 ){
		monitor = -1;
	}
	
	Init_outbuf();

	if( (inputblocking = Get_block_io( input ))){
		Set_nonblock_io( input );
	}
	if( (outputblocking = Get_block_io( output ))){
		Set_nonblock_io( output );
	}
	if( monitor >= 0 && (monitorblocking = Get_block_io( monitor ))){
		Set_nonblock_io( monitor );
	}
	while( input >= 0 || Outlen > 0 ){
		if( job_timeout > 0 ){
			time( &current_t );
			elapsed = current_t - Start_time;
			if( job_timeout > 0 && elapsed >= job_timeout ){
				break;
			}
			memset( &timeval, 0, sizeof(timeval) );
			timeval.tv_sec = m = job_timeout - elapsed;
			tp = &timeval;
			DEBUG4("Process_OF_mode: timeout now %d", m );
		} else {
			tp = 0;
		}
		FD_ZERO( &readfds );
		FD_ZERO( &writefds );
		if( !suspend && input >= 0 ){
			FD_SET( input, &readfds );
		}
		if( output >= 0 && Outlen > 0 ){
			FD_SET( output, &writefds );
		}
		if( monitor >= 0 ){
			FD_SET( monitor, &readfds );
		}
		m = 4;
		DEBUG4("Process_OF_mode: starting read select" );
		m = select( m,
			FD_SET_FIX((fd_set *))&readfds,
			FD_SET_FIX((fd_set *))&writefds,
			FD_SET_FIX((fd_set *))0, tp );
		err = errno;
		DEBUG4("Process_OF_mode: select returned %d", m );
		if( m < 0 ){
			/* error */
			if( err != EINTR ){
				Errorcode = JFAIL;
				logerr_die( "Process_OF_mode: select error" );
			}
		} else if( m == 0 ){
			/* timeout */
			break;
		}
		if( input >= 0 && FD_ISSET( input, &readfds ) ){
			m = read( input, buffer+count, 1 );
			/* we have EOF on input */
			if( m <= 0 ){
				close( input );
				if( (m = open( "/dev/null", O_RDONLY )) < 0 ){
					Errorcode = JABORT;
					logerr_die( "cannot open /dev/null" );
				}
				if( m != input && dup2(m, input) == -1 ){
					Errorcode = JABORT;
					logerr_die( "cannot dup2 /dev/null" );
				}
				input = -1;
			} else {
				buffer[count+1] = 0;
				DEBUG2("Proces_OF_mode: read from input '%s'", buffer);
				/* now we work the state stuff */
				c = buffer[count];
				if( c && c == stop[state] ){
					/* we consume the escape sequence */
					++state;
					if( stop[state] == 0 ){
						state = 0;
						suspend = 1;
					}
				} else {
					if( state ){
						for( i = 0; i < state; ++i ){
							buffer[count++] = stop[i];
						}
					}
					state = 0;
					buffer[count++] = c;
					buffer[count] = 0;
				}
			}
			DEBUG4("Process_OF_mode: read %d, total %d '%s'",m,count,buffer );
			if( Banner ){
				if( banner_done ){
					count = 0;
					buffer[0] = 0;
				} else if( (s = strpbrk( buffer, "\n\f" )) ){
					/* we have a line, and it should be a banner */
					*s++ = 0;
					DEBUG4("Process_OF_mode: banner line '%s'", buffer );
					if( !Banner_suppressed && !banner_done ){
						Do_banner( buffer );
					}
					banner_done = 1;
					count = 0;
					buffer[0] = 0;
				} else if( count >= sizeof(buffer)-1 ){
					banner_done = 1;
					count = 0;
					buffer[0] = 0;
				}
			} else {
				Put_outbuf_len( buffer, count );
				count = 0;
			}
		}
		if( monitor >=0 && FD_ISSET( monitor, &readfds ) ){
			/* we have data available on STDIN */
			if( Read_status_line( monitor ) <= 0 ){
				Errorcode = JFAIL;
				logerr_die("Process_OF_mode: read from printer failed");
			}
			while( (s = Get_inbuf_str()) );
		}
		if( Outlen > 0 && output>=0 && FD_ISSET( output, &writefds ) ){
			Outbuf[Outlen] = 0;
			DEBUG4("Process_OF_mode: writing '%s'", Outbuf);
			m = write( output, Outbuf, Outlen );
			DEBUG4("Process_OF_mode: wrote '%d'", m);
			if( m <= 0 ){
				/* output is closed */
				Errorcode = JFAIL;
				logerr_die("Process_OF_mode: cannot write to printer");
			} else {
				/* move the strings down */
				memmove( Outbuf, Outbuf+m, Outlen-m );
				Outlen -= m;
				Outbuf[Outlen] = 0;
				DEBUG4("Process_OF_mode: %d left to write '%s'",
					Outlen, Outbuf);
			}
		}
		/* we get a line from the Outbuf and process it */
		if( Outlen == 0 && suspend ){
			log( "Process_OF_mode: suspending");
			kill(getpid(), SIGSTOP);
			log( "Process_OF_mode: active again");
			suspend = 0;
		}
	}
	if(monitor >= 0 && monitorblocking ){
		Set_nonblock_io( monitor );
	}
	if(output >= 0 && outputblocking ){
		Set_nonblock_io( output );
	}
	DEBUG4("Process_OF_mode: done" );
}

void Add_val_to_list( struct line_list *v, int c, char *key, char *sep )
{
	char *s = 0;
	if( isupper(c) ) s = Upperopts[c-'A'];
	if( islower(c) ) s = Loweropts[c-'a'];
	if( s ){
		s = safestrdup3(key,sep,s,__FILE__,__LINE__);
		DEBUG3("Add_val_to_list: '%s'",s );
		Add_line_list( v, s, sep, 1, 1 );
		free(s); s = 0;
	}
}

/*
 * Do_banner
 *  - we put a banner into the Outbuffer
 *
 * parse the banner input line (read from stdin) and get the 
 * banner title values.  This has the form:
 *   class:name key:value key:value ...
 * or
 *   key:value  key:value ...
 */

void Do_banner( char *line )
{
	struct line_list l, v;
	char *t, *s, *u, *last_key = 0, *language = 0;
	int i, mode = 0;

	DEBUG2("Do_banner: Banner '%s' input '%s'", Banner, line );
	/* we can parse the banner line */
	Init_line_list( &l );
	Init_line_list( &v );
	Split(&l, line, Whitespace, 0, 0, 0, 1, 0 );
	if(DEBUGL3)Dump_line_list("Do_banner- raw", &l ); 
	Check_max( &v, l.count+1 );
	for( i = 0; i < l.count; ++i ){
		s = l.list[i];
		DEBUG4("Do_banner: last_key '%s'", last_key );
		if( (t = strchr( s, ':' )) && (i == 0 || t[1] == 0 ) ){
			*t = '=';
			DEBUG4("Do_banner: adding '%s'", s );
			Add_line_list( &v, s, Value_sep, 1, 1 );
			if(last_key ){free(last_key); last_key=0;}
			last_key = safestrdup(s,__FILE__,__LINE__);
			if( i==0 ){
				*t = 0;
				u = safestrdup2("class=",s,__FILE__,__LINE__);
				DEBUG4("Do_banner: adding '%s'", u );
				Add_line_list( &v, u, Value_sep, 1, 1 );
				free(u);
				u = safestrdup2("user=",t+1,__FILE__,__LINE__);
				DEBUG4("Do_banner: adding '%s'", u );
				Add_line_list( &v, u, Value_sep, 1, 1 );
				free(u);
				u = 0;
			}
			*t = ':';
		} else if( last_key ){
			t = last_key;
			last_key = safestrdup3(last_key,s," ",__FILE__,__LINE__);
			Add_line_list( &v, last_key, Value_sep, 1, 1 );
			free(t);
			DEBUG4("Do_banner: appending '%s'", last_key );
		}
	}
	if( last_key ){
		Add_line_list( &v, last_key, Value_sep, 1, 1 );
	}
	if( last_key ){ free(last_key); last_key=0;}
	Add_val_to_list( &v, 'A', "ident", "=");
	Add_val_to_list( &v, 'C', "class", "=");
	Add_val_to_list( &v, 'F', "format", "=");
	Add_val_to_list( &v, 'H', "fqdnhost", "=");
	Add_val_to_list( &v, 'J', "job", "=");
	Add_val_to_list( &v, 'L', "user", "=");
	Add_val_to_list( &v, 'N', "filename", "=");
	Add_val_to_list( &v, 'P', "printer", "=");
	Add_val_to_list( &v, 'R', "accntname", "=");
	Add_val_to_list( &v, 'S', "pr_commnt", "=");
	Add_val_to_list( &v, 'j', "seq", "=");
	Add_val_to_list( &v, 'c', "literal", "="),
	Add_val_to_list( &v, 'h', "host", "=");
	Add_val_to_list( &v, 'i', "indent", "="),
	Add_val_to_list( &v, 'k', "controlfile", "=");
	Add_val_to_list( &v, 'l', "length", "="),
	Add_val_to_list( &v, 'n', "login", "=");
	Add_val_to_list( &v, 's', "statusfile", "=");
	Add_val_to_list( &v, 'w', "width", "="),
	Add_val_to_list( &v, 'x', "xwidth", "="),
	Add_val_to_list( &v, 'y', "ylength", "=");
	if( line && *line ){
		s = safestrdup2("line=",line,__FILE__,__LINE__);
		Add_line_list( &v, s, Value_sep, 1, 1 );
		if(s){free(s);s=0;}
	}

	if(DEBUGL3)Dump_line_list("Do_banner- fixed", &v ); 
	Set_mode_lang( Banner, &mode, &language );
	if( !strcasecmp(Banner,"pcl") ){
		Init_job(language, mode );
		pcl_banner( line, &v, &l );
		Term_job( mode );
	} else if( !strcasecmp(Banner,"ps") ){
		Init_job(language, mode );
		ps_banner( line, &v, &l );
		Term_job( mode );
	} else if( !strcasecmp(Banner,"text") ){
		Text_banner();
	} else if( Banner[0] == '/' ){
		file_banner( Banner );
	} else if( Banner[0] == '|' ){
		Filter_banner(Banner, line );
	}
	Free_line_list(&v);
	Free_line_list(&l);
}

 int xpos = 50, ypos = 100, incr;

/******************************************************************************
 *	PCL Banner Description strings
 *****************************************************************************/

 static char *margin = "\\033&l0u0Z";
 /* light grey bar at 30 X 50 Y */
 static char *lightbar = "\\033*c1800a100b45g2P";
 /* dark grey bar at 30 X 560 Y */
 static char *darkbar  = "\\033*c1800a100b25g2P";
 static char *fontchange = "\\033(8U\\033(s1p%dv0s0b4148T";
 static char *position = "\\033*p%dx%dY";  /* position to  (X Y) */
 static char *pclexit = "\\033E";
 static char *crlfstr = "\\033&k2G";

void moveto( int x, int y )
{
	char sendline[SMALLBUFFER];
	plp_snprintf( sendline, sizeof(sendline)-1, position, x, y );
	Put_fixed( sendline );
}

void fontsize( int size )
{
	/* size is in points (72nds of inch ) */
	char sendline[SMALLBUFFER];
	incr = (size*300*1.1)/72;
	plp_snprintf( sendline, sizeof(sendline)-1, fontchange, size );
	Put_fixed( sendline );
}

void textline( char *line, int start, int end )
{
	DEBUG4("textline: '%s'", line );
	if( start ) moveto( xpos, ypos );
	Put_fixed( line );
	if( end ) ypos += incr;
}

void pcl_banner( char *line, struct line_list *l, struct line_list *rest )
{
	char *s;
	int i = 0;

	DEBUG4("pcl_banner: starting");

	i = 0;
	Put_fixed( pclexit );
	Put_fixed( crlfstr );
	Put_fixed( margin );
	moveto( xpos, ypos );
	Put_fixed( lightbar );
	ypos += 100;
	/* set font size */
	fontsize( 24 );
	ypos += incr;
	moveto( xpos, ypos );
	s = Find_exists_value( l, "file", Value_sep );
	if( s == 0 || *s == 0 ) s = Find_exists_value( l, "job", Value_sep );
	if( s && *s ){
		textline( s, 1, 1 );
	}
	s = Find_exists_value( l, "user", Value_sep );
	if( s == 0 || *s == 0 ) s = Find_exists_value( l, "login", Value_sep );
	if( s && *s ){
		textline( "User: ", 1, 0 );
		textline( s, 0, 1 );
	}
	s = Find_exists_value( l, "host", Value_sep);
	if( s && *s ){
		textline( "Host: ", 1, 0 );
		textline( s, 0, 1 );
	}
	s = Find_exists_value( l, "printer", Value_sep);
	if( s && *s ){
		textline( "Printer: ", 1, 0 );
		textline( s, 0, 1 );
	}
	fontsize( 12 );
	s = Find_exists_value( l, "class", Value_sep);
	if( s && *s ){
		textline( "Class: ", 1, 0 );
		textline( s, 0, 1 );
	}
	textline( "Date: ", 1, 0 );
	textline( Time_str(0,0), 0, 1 );
	fontsize( 12 );
	textline( "" , 1, 1 );
	textline( line , 1, 1 );
	textline( "" , 1, 1 );

	moveto( xpos, ypos );
	Put_fixed( darkbar );


	Put_fixed( "\\f" );
	Put_fixed( pclexit );
	DEBUG4("pcl_banner: done");
}

/*
  We generate a PS banner by
  1. Outputting some Postscript
     i.e.-
  %!PS-Adobe-2.0
  /Seq (number) def
  /Job (banner) def  (with '()\' escaped to '\(\)\\' )
  /Host (HOST) def
  /Class (CLASS) def
  /User (USER) def
  /Date (DATE) def
  /Name (NAME) def
  /Line (LINE) def
  /A    (A Value) def

  This is obtained from the input line which has the form
   class:name tag:value tag:value

 2. appending the contents of a PostScript file
     i.e.-  ps_banner_file
 */

char *ps_str_fix( char *str )
{
	char *s, buffer[3];
	int i = 0;

	if( str == 0 ) str = "";
	str = safestrdup(str,__FILE__,__LINE__);
	strcpy(buffer,"\\x");
	for( s = str; (s = strpbrk(s,"()\\")); ){
		i = s - str;
		buffer[1] = *s;
		*s = 0;
		str = safestrdup3(str,buffer,s+1,__FILE__,__LINE__);
		s = str+i+2;
	}
	return(str);
}

void ps_banner( char *line, struct line_list *l, struct line_list *rest )
{
	char *s, *t, *u;
	int c = 0, n, i, fd;
	char value[LARGEBUFFER];

	if( !Banner_file ){
		Errorcode = JABORT;
		fatal("missing ps_banner_file information");
	}
	if( (fd = open(Banner_file,O_RDONLY)) == -1 ){
		Errorcode = JABORT;
		logerr_die("cannot open banner_file '%s'", Banner_file );
		return;
	}

	Put_outbuf_str("%!PS-Adobe-2.0\n");
	Put_outbuf_str("/Seq (SEQ) def\n");
	Put_outbuf_str("/Job (JOB) def\n");
	Put_outbuf_str("/Host (HOST) def\n");
	Put_outbuf_str("/Class (CLASS) def\n");
	Put_outbuf_str("/User (USER) def\n");
	Put_outbuf_str("/Date (DATE) def\n");
	Put_outbuf_str("/Name (NAME) def\n");
	Put_outbuf_str("/Line (LINE) def\n");

	for( i = 0; i < l->count; ++i ){
		s = l->list[i];
		if( (t = strpbrk(s,Value_sep)) ){
			c = *t; *t = 0;
			for( u = t+1; isspace(cval(u)); ++u );
			u = ps_str_fix(u);
			n = cval(s);
			if( islower(n) ) *s = toupper(n);
			plp_snprintf(value,sizeof(value),"/%s (%s) def\n", s, u );
			Put_outbuf_str(value);
			*t = c;
			*s = n;
			free(u);
		}
	}
	{
		for( i = 'A'; i <= 'Z'; ++i ){
			if( !(s = Upperopts[i-'A']) ) s = "";
			s = ps_str_fix(s);
			plp_snprintf(value,sizeof(value),"/%c (%s) def\n", i, s );
			Put_outbuf_str(value);
			free(s); s = 0;
		}
		for( i = 'a'; i <= 'z'; ++i ){
			if( !(s = Loweropts[i-'a']) ) s = "";
			s = ps_str_fix(s);
			plp_snprintf(value,sizeof(value),"/%c (%s) def\n", i, s );
			Put_outbuf_str(value);
			free(s); s = 0;
		}
	}
	DEBUG3("ps_banner: header '%s'", Outbuf);
	while( (n = read(fd, value, sizeof(value))) > 0 ){
		Put_outbuf_len( value, n );
	}
}

void file_banner( char *file )
{
	char value[LARGEBUFFER];
	int fd, n;

	if( (fd = open(file,O_RDONLY)) == -1 ){
		log("cannot open ps_banner_file '%s' - %s", file, Errormsg(errno));
		return;
	}
	while( (n = read(fd, value, sizeof(value))) > 0 ){
		Put_outbuf_len( value, n );
	}
}

void Filter_banner( char *filter, char *line )
{
	char value[LARGEBUFFER], *s;
	struct line_list l;
	int p0[2], p1[2], pid, i, n;
	plp_status_t status;
	
	/* we need two pipes, and do a fork */ 
	Init_line_list( &l );
	if( pipe(p0) == -1 || pipe(p1) == -1 ){
		Errorcode = JABORT;
		logerr_die("Filter_banner: pipe() failed");
	}
	if( (pid = fork()) == -1 ){
		Errorcode = JABORT;
		logerr_die("Filter_banner: fork failed");
	} else if( pid == 0 ){
		/* child */
		/* grab the passed arguments of the program */
		Check_max( &l, Argc+1 );
		for( i = 0; i < Argc; ++i ){
			l.list[i] = Argv[i];
		}
		l.list[0] = filter;
		/* set up fd 0, fd 1 */
		if( dup2(p0[0],0) == -1
			|| dup2(p1[1],1) == -1 ){
			Errorcode = JABORT;
			logerr_die("Filter_banner: child dup2 failed");
		}
		close_on_exec(3);
		execve( l.list[0], l.list, Envp );
		Errorcode = JABORT;
		logerr_die("Filter_banner: child execve failed");
	}
	close(p0[0]); close(p1[1]);
	i = n = strlen( line );
	for( s = line; n > 0 && (i = write(p0[1], s, n )) > 0;
		s += i, n -= i );
	if( i < 0 ){
		Errorcode = JABORT;
		logerr_die("Filter_banner: could not write to child");
	}
	close(p0[1]);
	
	while( (n = read(p1[0], value, sizeof(value))) > 0 ){
		Put_outbuf_len( value, n );
	}
	close(p1[0]);
	/* ugly, but effective */
	while( (n =  waitpid( pid, &status, 0 )) != pid );
	if( WIFEXITED(status) && (n = WEXITSTATUS(status)) ){
		Errorcode = JABORT;
		fatal("Filter_banner: text process exited with status %d", n);
	} else if( WIFSIGNALED(status) ){
		Errorcode = JABORT;
		n = WTERMSIG(status);
		fatal("Filter_banner: text process died with signal %d, '%s'",
			n, Sigstr(n));
	}
	log("Filter_banner: converter done");
}

/*
 * set the close on exec flag for a reasonable range of FD's
 */
void close_on_exec( int n )
{
    int fd, max = getdtablesize();

    for( fd = n; fd < max; fd++ ){
		fcntl(fd, F_SETFD, 1);
	}
}

/*
 * Use_file_util(char *pgm, int *mode, char **language );
 *
 * We will need to use the file utility
 */

char *Use_file_util(char *pgm)
{
	int p0[2], p1[2], i, pid, n;
	plp_status_t status;
	char value[LARGEBUFFER], *s, *t;
	struct line_list l;

	Init_line_list( &l );
	if( pipe(p0) == -1 || pipe(p1) == -1 ){
		Errorcode = JABORT;
		logerr_die("Use_file_util: pipe() failed");
	}
	if( (pid = fork()) == -1 ){
		Errorcode = JABORT;
		logerr_die("Use_file_util: fork failed");
	} else if( pid == 0 ){
		/* child */
		/* grab the passed arguments of the program */
		Split(&l,pgm,Whitespace,0,0,0,0, 0 );
		Check_max(&l,1);
		l.list[l.count] = 0;
		for( i = 0; i < l.count; ++i ){
			l.list[i] = Fix_option_str( l.list[i], 0, 1 );
		}
		if( dup2(p0[0],0) == -1 || dup2(p1[1],1) == -1
			|| dup2(p1[1],2) == -1 ){
			Errorcode = JABORT;
			logerr_die("Use_file_util: dup2 failed");
		}
		close_on_exec(3);
		execve( l.list[0], l.list, Envp );
		Errorcode = JABORT;
		logerr_die("Use_file_util: child execve failed");
	}
	close(p1[1]); close(p0[0]);
	for( n = i = 0;
		(n = write(p0[1], Outbuf+i, Outlen-i)) > 0;
		i += n );
	close( p0[1] );
	/* we put this in value */
	for( n = i = 0;
		(n = read(p1[0], value+i, sizeof(value)-i-1 )) > 0;
		i += n );
	close(p1[0]);
	value[i] = 0;
	/* ugly, but effective */
	while( (n =  waitpid( pid, &status, 0 )) != pid );
	if( WIFEXITED(status) && (n = WEXITSTATUS(status)) ){
		Errorcode = JABORT;
		fatal("Use_file_util: text process exited with status %d", n);
	} else if( WIFSIGNALED(status) ){
		Errorcode = JABORT;
		n = WTERMSIG(status);
		fatal("Use_file_util: text process died with signal %d, '%s'",
			n, Sigstr(n));
	}
	DEBUG4("Use_file_util: file util done, '%s'", value );
	for( s = value; (s = strpbrk(s,Whitespace)); ++s ){
		*s = ' ';
		while( isspace(cval(s+1)) ) memmove(s, s+1, strlen(s)+1);
	}
	lowercase(value);
	if( (t = strpbrk( value, ":" )) ) ++t; else t = value;
	while( isspace(cval(t)) ) ++t;
	memmove( value, t, strlen(t)+1);

	DEBUG4("Use_file_util: file identified as type '%s'", value );
	if( strstr( value, "postscript" ) ){
		s = "ps";
	} else if( strstr( value, "printer job language" ) ){
		s = "raw";
	} else if( strstr( value, "pcl" ) ){
		s = "pcl";
	} else if( strstr( value, "text" ) ){
		s = "text";
	} else {
		s = "unknown";
	}
	log("Use_file_util: file identified as '%s', assigned type '%s'", value, s );
	return(s);
}

void Make_stdin_file()
{
	int is_file, fd, in, n, i;
	struct stat statb;
	char *s, *tempfile;

	if( fstat( 0, &statb ) < 0 ){
		Errorcode = JABORT;
		logerr_die( "Make_stdin_file: cannot fstat fd 1" );
	}
	is_file = ((statb.st_mode & S_IFMT) == S_IFREG);
	DEBUG2( "Make_stdin_file: input is_file %d, size %d",
		is_file, (int)statb.st_size );
	if( !is_file ){
		if( !(tempfile = Find_str_value( &Model,"text_tempfile", Value_sep ))){
			Errorcode = JABORT;
			fatal(
		"Make_stdin_file: missing 'text_tempfile' needed for conversion");
		}
		tempfile = safestrdup2(tempfile,".XXXXXX",__FILE__,__LINE__);
		DEBUG2( "Make_stdin_file: tempfile '%s'", tempfile );
		if( (fd = mkstemp(tempfile)) == -1 ){
			Errorcode = JABORT;
			logerr_die("Make_stdin_file: could not open '%s'", tempfile);
		}
		DEBUG2( "Make_stdin_file: new tempfile '%s', fd %d", tempfile, fd );
		free(tempfile); tempfile = 0;
		in = 1;
		do{
			n = 0;
			for( i = Outlen, s = Outbuf;
				i > 0 && (n = write(fd,s,i)) > 0;
				i -= n, s += n);
			if( n < 0 ){
				Errorcode = JABORT;
				logerr_die( "Make_stdin_file: write to tempfile fd %d failed", fd );
			}
			Outlen = 0;
			if( in > 0 ){
				n = 0;
				for( Outlen = 0;
					Outlen < Outmax
					&& (n = read(0,Outbuf+Outlen,Outmax-Outlen)) > 0;
					Outlen+=n);
				if( n < 0 ){
					Errorcode = JABORT;
					logerr_die( "Make_stdin_file: read from stdin" );
				} else if( n == 0 ){
					in = 0;
				}
			}
		} while( Outlen > 0 );
		if( dup2(fd,0) == -1 ){
			Errorcode = JABORT;
			logerr_die( "Make_stdin_file: read from stdin" );
		}
		if( fd != 0 ) close(fd);
		is_file = 1;
	}
	if( lseek(0,0,SEEK_SET) == -1 ){
		Errorcode = JABORT;
		logerr_die("Make_stdin_file: lseek failed");
	}
}

int Set_mode_lang( char *s, int *mode, char **language )
{
	int i = 0;
	if(!strcasecmp( s, "ps" )){ *mode = PS; *language = "POSTSCRIPT"; }
	else if(!strcasecmp( s, "pcl" )){ *mode = PCL; *language = "PCL"; }
	else if(!strcasecmp( s, "text" )){ *mode = TEXT; *language = "PCL"; }
	else if(!strcasecmp( s, "raw" )){ *mode = RAW; *language = 0; }
	else if(!strcasecmp( s, "unknown" )){ *mode = UNKNOWN; *language = 0; }
	else i = 1;
	return i;
}

int Fd_readable( int fd )
{
	int m;
	struct stat statb;
    fd_set readfds;
	struct timeval timeval;

	if( fstat( fd, &statb ) < 0 ){
		Errorcode = JABORT;
		logerr_die( "Fd_readable: cannot fstat fd %d", fd );
	}
	DEBUG2( "Fd_readable: fd %d device 0%o", fd, statb.st_mode & S_IFMT );
	memset(&timeval,0,sizeof(timeval));

	FD_ZERO( &readfds );
	FD_SET( fd, &readfds );
	m = fd+1;

	errno = 0;
	m = select( m,
		FD_SET_FIX((fd_set *))&readfds,
		FD_SET_FIX((fd_set *))0,
		FD_SET_FIX((fd_set *))0, &timeval );
	DEBUG2( "Fd_readable: select %d, with error '%s'", m, Errormsg(errno) );

	m = (m >= 0);
	if( m ){
		m = ((statb.st_mode & S_IFMT) != S_IFREG);
	}
	DEBUG2( "Fd_readable: final value %d", m );
	return( m );
}

void Init_job( char *language, int mode )
{
	char buffer[SMALLBUFFER];
	struct line_list l;
	int len, i;
	char *s;

	Init_line_list(&l);

	DEBUG2("Init_job: language '%s'", language );
	if( Pjl && Pjl_enter && language ){
		plp_snprintf( buffer, sizeof(buffer),
			PJL_ENTER_str, language );
		Put_pjl( buffer );
		memset( buffer, 0, sizeof(buffer) );
		/* 
		 * force nulls to be output
		 */
		for( i = Null_pad_count; i > 0; i -= len){
			len = sizeof(buffer);
			if( i < len ) len = i;
			Put_outbuf_len( buffer, len );
		}
	}
	DEBUG2("Init_job: after PJL '%s'", Outbuf );

	/* now we handle the various initializations */
	if( Pcl && (mode == PCL || mode == TEXT) ){
		DEBUG1("Init_job: doing pcl init");
		if( !No_PCL_EOJ ) Put_outbuf_str( PCL_EXIT_str );
		if( (s = Find_str_value( &Model, "pcl_init", Value_sep)) ){
			DEBUG1("Init_job: 'pcl_init'='%s'", s);
			Free_line_list( &l );
			Split( &l, s, List_sep, 1, 0, 1, 1, 0 );
			Resolve_list( "pcl_", &l, Put_pcl );
			Free_line_list( &l );
		}
		DEBUG1("Init_job: 'pcl' and Topts");
		Resolve_user_opts( "pcl_", &Pcl_user_opts, &Topts, Put_pcl );
		DEBUG1("Init_job: 'pcl' and Zopts");
		Resolve_user_opts( "pcl_", &Pcl_user_opts, &Zopts, Put_pcl );
	} else if( Ps && mode == PS ){
		DEBUG1("Init_job: doing ps init");
		if( !No_PS_EOJ ) Put_outbuf_str(CTRL_D);
		if( Tbcp ){
			DEBUG3("Init_job: doing TBCP");
			Put_outbuf_str("\001M");
		}
		if( (s = Find_str_value(&Model, "ps_level_str",Value_sep)) ){
			Put_ps(s);
		} else {
			Put_ps("%!");
		}
		if( !Find_first_key( &Model, "ps_init", Value_sep, 0) ){
			s  = Find_str_value( &Model, "ps_init", Value_sep);
			DEBUG1("Init_job: 'ps_init'='%s'", s);
			Free_line_list( &l );
			Split( &l, s, List_sep, 1, 0, 1, 1, 0 );
			Resolve_list( "ps_", &l, Put_ps );
			Free_line_list( &l );
		}
		DEBUG1("Init_job: 'ps' and Topts");
		Resolve_user_opts( "ps_", &Ps_user_opts, &Topts, Put_ps );
		DEBUG1("Init_job: 'ps' and Zopts");
		Resolve_user_opts( "ps_", &Ps_user_opts, &Zopts, Put_ps );
	}
	DEBUG2("Init_job: final '%s'", Outbuf );
	len = Write_out_buffer( Outlen, Outbuf, Job_timeout );
	if( len ){
		Errorcode = JFAIL;
		fatal( "Init_job: job timed out language selection" );
	}
	Init_outbuf();
	Free_line_list(&l);
}

void Term_job( int mode )
{
	int len;

	DEBUG2("Term_job: mode %d", mode );
	if( Pcl && ( mode == PCL || mode == TEXT ) ){
		Put_outbuf_str( PCL_EXIT_str );
	} else if( Ps && mode == PS ){
		Put_outbuf_str( CTRL_D );
	}
	len = Write_out_buffer( Outlen, Outbuf, Job_timeout );
	if( len ){
		Errorcode = JFAIL;
		fatal( "Term_job: job error on cleanup strings" );
	}
	Init_outbuf();
}

 extern struct font Font9x8;

void Text_banner(void)
{
	int linecount = 0, pagewidth = 0;
	struct line_list l;
	char buffer[SMALLBUFFER];
	char *s, *t;
	int i, n, pb;

	DEBUG4("Text_banner: starting");
	/* check the defaults, then the overrides */
	Init_line_list(&l);
	if( (s = Loweropts['l'-'a']) ){
		linecount = atoi(s);
	}
	if( linecount == 0 ){
		linecount = Find_flag_value(&Model,"page_length",Value_sep);
	}
	if( linecount == 0 ) linecount = 60;

	if( (s = Loweropts['w'-'a']) ){
		pagewidth = atoi(s);
	}
	if( pagewidth == 0 ){
		pagewidth = Find_flag_value(&Model,"page_width",Value_sep);
	}
	if( pagewidth == 0 ) pagewidth = 80;

	Check_max(&l,linecount+1);
	for( i = 0; i < linecount; ++i ){
		Add_line_list(&l,"",0,0,0);
	}

	/* we add top and bottom breaks */

	for( i = 0; i < sizeof(buffer)-1; ++i ){
		buffer[i] = '*';
	}
	buffer[i] = 0;
	if( pagewidth < sizeof(buffer)-1 ) buffer[pagewidth] = 0;
	if( !(pb = Find_flag_value(&Model,"page_break",Value_sep)) ){
		pb = 3;
	}
	
	for( i = 0; i < pb; ++i ){
		if( i < l.count ){
			free(l.list[i]);
			l.list[i] = safestrdup(buffer,__FILE__,__LINE__);
		}
		n = l.count - i - 1;
		if( n >= 0 ){
			free(l.list[n]);
			l.list[n] = safestrdup(buffer,__FILE__,__LINE__);
		}
	}
	DEBUG4("Text_banner: page width %d, page length %d", pagewidth, linecount);
	if(DEBUGL4)Dump_line_list("Text_banner- pagebreaks", &l);

	n = pb + 2;
	/*
	 * First, the job number, name, then the user name, then the other info
	 */
	if( (s = Loweropts['j'-'a']) ){
		n = bigfont(&Font9x8, s, &l, n);
		n += 2;
	}
	if( (s = Upperopts['J'-'A']) ){
		n = bigfont(&Font9x8, s, &l, n);
		n += 2;
	}

	if(DEBUGL4)Dump_line_list("Text_banner- subject", &l);

	if( (s = Upperopts['L'-'A']) || (s = Upperopts['P'-'A']) ){
		n = bigfont(&Font9x8, s, &l, n);
		n += 2;
	}
	DEBUG4("Text_banner: adding lines at %d", n );

	for( i = 'a'; n < l.count && i <= 'z'; ++i ){
		s = Loweropts[i-'a'];
		t = 0;
		switch(i){
			case 'j': t = "Seq"; break;
			case 'h': t = "Host"; break;
		}
		DEBUG4("Text_banner: '%c' - %s = %s", i, t, s );
		if( s && t ){
			plp_snprintf(buffer,sizeof(buffer),"%-7s: %s",t, s);
			free(l.list[n]);
			l.list[n] = safestrdup(buffer,__FILE__,__LINE__);
			n = n+1;
		}
	}
	for( i = 'A'; n < l.count && i <= 'Z'; ++i ){
		s = Upperopts[i-'A'];
		t = 0;
		switch(i){
			case 'C': t = "Class"; break;
			case 'D': t = "Date"; break;
			case 'H': t = "Host"; break;
			case 'J': t = "Job"; break;
			case 'L': t = "Banner User Name"; break;
			case 'N': t = "Person"; break;
			case 'Q': t = "Queue Name"; break;
			case 'R': t = "Account"; break;
		}
		DEBUG4("Text_banner: '%c' - %s = %s", i, t, s );
		if( s && t ){
			plp_snprintf(buffer,sizeof(buffer),"%-7s: %s",t, s);
			free(l.list[n]);
			l.list[n] = safestrdup(buffer,__FILE__,__LINE__);
			n = n+1;
		}
	}
	if(DEBUGL4)Dump_line_list("Text_banner",&l);
	for( i = 0; i < l.count; ++i ){
		Put_outbuf_str( l.list[i] );
		Put_outbuf_str( "\n" );
	}
	Free_line_list(&l);
}

/***************************************************************************
 * bigfornt( struct font *font, char * line, struct line_list *l,
 *  int startline )
 * print the line in big characters
 * for i = topline to bottomline do
 *  for each character in the line do
 *    get the scan line representation for the character
 *    foreach bit in the string do
 *       if the bit is set, print X, else print ' ';
 *        endfor
 *  endfor
 * endfor
 *
 ***************************************************************************/
void do_char( struct font *font, struct glyph *glyph,
	char *str, int line )
{
	int chars, i, j;
	char *s;

	/* if(debug)fprintf(stderr,"do_char: '%c', width %d\n", glyph->ch ); */
	chars = (font->width+7)/8;	/* calculate the row */
	s = &glyph->bits[line*chars];	/* get start of row */
	for( i = 0; i < chars; ++i ){	/* for each byte in row */
		for( j = 7; j >= 0; --j ){	/* from most to least sig bit */
			if( *s & (1<<j) ){		/* get bit value */
				*str = 'X';
			}
			++str;
		}
		++s;
	}
}


int bigfont( struct font *font, char *line, struct line_list *l, int start )
{
	int i, j, k, len;                   /* ACME Integers, Inc. */
	char *s = 0;

	DEBUG4("bigfont: '%s'", line );
	len = strlen(line);
	for( i = 0; start < l->count && i < font->height; ++i ){
		k = font->width * len;
		free(l->list[start]);
		s = malloc_or_die(k+1,__FILE__,__LINE__);
		l->list[start++] = s;
		memset(s,' ',k);
		s[k] = 0;
		for( j = 0; j < len; ++j){
			do_char( font, &font->glyph[line[j]-' '],
				&s[j * font->width], i );
		}
	}
	if( start < l->count ) ++start;
	return(start);
}

/*
 The font information is provided as entries in a data structure.

 The struct font{} entry specifies the character heights and widths,
 as well as the number of lines needed to display the characters.
 The struct glyph{} array is the set of glyphs for each character.

    
	{ 
	X__11___,
	X__11___,
	X__11___,
	X__11___,
	X__11___,
	X_______,
	X_______,
	X__11___,
	cX_11___},			/ * ! * /

     ^ lower left corner, i.e. - on baseline - x = 0, y = 8

	{
	X_______,
	X_______,
	X_______,
	X_111_1_,
	X1___11_,
	X1____1_,
	X1____1_,
	X1___11_,
	cX111_1_,
	X_____1_,
	X1____1_,
	X_1111__},			/ * g * /

     ^ lower left corner, i.e. - on baseline - x = 0, y = 8

 ***************************************************************************/

#define X_______ 0
#define X______1 01
#define X_____1_ 02
#define X____1__ 04
#define X____11_ 06
#define X___1___ 010
#define X___1__1 011
#define X___1_1_ 012
#define X___11__ 014
#define X__1____ 020
#define X__1__1_ 022
#define X__1_1__ 024
#define X__11___ 030
#define X__111__ 034
#define X__111_1 035
#define X__1111_ 036
#define X__11111 037
#define X_1_____ 040
#define X_1____1 041
#define X_1___1_ 042
#define X_1__1__ 044
#define X_1_1___ 050
#define X_1_1__1 051
#define X_1_1_1_ 052
#define X_11____ 060
#define X_11_11_ 066
#define X_111___ 070
#define X_111__1 071
#define X_111_1_ 072
#define X_1111__ 074
#define X_1111_1 075
#define X_11111_ 076
#define X_111111 077
#define X1______ 0100
#define X1_____1 0101
#define X1____1_ 0102
#define X1____11 0103
#define X1___1__ 0104
#define X1___1_1 0105
#define X1___11_ 0106
#define X1__1___ 0110
#define X1__1__1 0111
#define X1__11_1 0115
#define X1__1111 0117
#define X1_1____ 0120
#define X1_1___1 0121
#define X1_1_1_1 0125
#define X1_1_11_ 0126
#define X1_111__ 0134
#define X1_1111_ 0136
#define X11____1 0141
#define X11___1_ 0142
#define X11___11 0143
#define X11_1___ 0150
#define X11_1__1 0151
#define X111_11_ 0166
#define X1111___ 0170
#define X11111__ 0174
#define X111111_ 0176
#define X1111111 0177

 struct glyph g9x8[] = {
	{ ' ', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______}},			/* */

	{ '!', 0, 8, {
	X__11___,
	X__11___,
	X__11___,
	X__11___,
	X__11___,
	X_______,
	X_______,
	X__11___,
	X__11___}},			/* ! */

	{ '"', 0, 8, {
	X_1__1__,
	X_1__1__,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______}},			/* " */

	{ '#', 0, 8, {
	X_______,
	X__1_1__,
	X__1_1__,
	X1111111,
	X__1_1__,
	X1111111,
	X__1_1__,
	X__1_1__,
	X_______}},			/* # */

	{ '$', 0, 8, {
	X___1___,
	X_11111_,
	X1__1__1,
	X1__1___,
	X_11111_,
	X___1__1,
	X1__1__1,
	X_11111_,
	X___1___}},			/* $ */

	{ '%', 0, 8, {
	X_1_____,
	X1_1___1,
	X_1___1_,
	X____1__,
	X___1___,
	X__1____,
	X_1___1_,
	X1___1_1,
	X_____1_}},			/* % */

	{ '&', 0, 8, {
	X_11____,
	X1__1___,
	X1___1__,
	X_1_1___,
	X__1____,
	X_1_1__1,
	X1___11_,
	X1___11_,
	X_111__1}},			/* & */

	{ '\'', 0, 8, {
	X___11__,
	X___11__,
	X___1___,
	X__1____,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______}},			/* ' */

	{ '(', 0, 8, {
	X____1__,
	X___1___,
	X__1____,
	X__1____,
	X__1____,
	X__1____,
	X__1____,
	X___1___,
	X____1__}},			/* ( */

	{ ')', 0, 8, {
	X__1____,
	X___1___,
	X____1__,
	X____1__,
	X____1__,
	X____1__,
	X____1__,
	X___1___,
	X__1____}},			/* ) */

	{ '*', 0, 8, {
	X_______,
	X___1___,
	X1__1__1,
	X_1_1_1_,
	X__111__,
	X_1_1_1_,
	X1__1__1,
	X___1___,
	X_______}},			/* * */

	{ '+', 0, 8, {
	X_______,
	X___1___,
	X___1___,
	X___1___,
	X1111111,
	X___1___,
	X___1___,
	X___1___,
	X_______}},			/* + */

	{ ',', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X__11___,
	X__11___,
	X__1____,
	X_1_____,
	X_______}},			/* , */

	{ '-', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_______,
	X1111111,
	X_______,
	X_______,
	X_______,
	X_______}},			/* - */

	{ '.', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X__11___,
	X__11___}},			/* . */

	{ '/', 0, 8, {
	X_______,
	X______1,
	X_____1_,
	X____1__,
	X___1___,
	X__1____,
	X_1_____,
	X1______,
	X_______}},			/* / */

	{ '0', 0, 8, {
	X_11111_,
	X1_____1,
	X1____11,
	X1___1_1,
	X1__1__1,
	X1_1___1,
	X11____1,
	X1_____1,
	X_11111_}},			/* 0 */

	{ '1', 0, 8, {
	X___1___,
	X__11___,
	X_1_1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X_11111_}},			/* 1 */

	{ '2', 0, 8, {
	X_11111_,
	X1_____1,
	X______1,
	X_____1_,
	X__111__,
	X_1_____,
	X1______,
	X1______,
	X1111111}},			/* 2 */

	{ '3', 0, 8, {
	X_11111_,
	X1_____1,
	X______1,
	X______1,
	X__1111_,
	X______1,
	X______1,
	X1_____1,
	X_11111_}},			/* 3 */

	{ '4', 0, 8, {
	X_____1_,
	X____11_,
	X___1_1_,
	X__1__1_,
	X_1___1_,
	X1____1_,
	X1111111,
	X_____1_,
	X_____1_}},			/* 4 */

	{ '5', 0, 8, {
	X1111111,
	X1______,
	X1______,
	X11111__,
	X_____1_,
	X______1,
	X______1,
	X1____1_,
	X_1111__}},			/* 5 */

	{ '6', 0, 8, {
	X__1111_,
	X_1_____,
	X1______,
	X1______,
	X1_1111_,
	X11____1,
	X1_____1,
	X1_____1,
	X_11111_}},			/* 6 */

	{ '7', 0, 8, {
	X1111111,
	X1_____1,
	X_____1_,
	X____1__,
	X___1___,
	X__1____,
	X__1____,
	X__1____,
	X__1____}},			/* 7 */

	{ '8', 0, 8, {
	X_11111_,
	X1_____1,
	X1_____1,
	X1_____1,
	X_11111_,
	X1_____1,
	X1_____1,
	X1_____1,
	X_11111_}},			/* 8 */

	{ '9', 0, 8, {
	X_11111_,
	X1_____1,
	X1_____1,
	X1_____1,
	X_111111,
	X______1,
	X______1,
	X1_____1,
	X_1111__}},			/* 9 */

	{ ':', 0, 8, {
	X_______,
	X_______,
	X_______,
	X__11___,
	X__11___,
	X_______,
	X_______,
	X__11___,
	X__11___}},			/* : */


	{ ';', 0, 8, {
	X_______,
	X_______,
	X_______,
	X__11___,
	X__11___,
	X_______,
	X_______,
	X__11___,
	X__11___,
	X__1____,
	X_1_____}},			/* ; */

	{ '<', 0, 8, {
	X____1__,
	X___1___,
	X__1____,
	X_1_____,
	X1______,
	X_1_____,
	X__1____,
	X___1___,
	X____1__}},			/* < */

	{ '=', 0, 8, {
	X_______,
	X_______,
	X_______,
	X1111111,
	X_______,
	X1111111,
	X_______,
	X_______,
	X_______}},			/* = */

	{ '>', 0, 8, {
	X__1____,
	X___1___,
	X____1__,
	X_____1_,
	X______1,
	X_____1_,
	X____1__,
	X___1___,
	X__1____}},			/* > */

	{ '?', 0, 8, {
	X__1111_,
	X_1____1,
	X_1____1,
	X______1,
	X____11_,
	X___1___,
	X___1___,
	X_______,
	X___1___}},			/* ? */

	{ '@', 0, 8, {
	X__1111_,
	X_1____1,
	X1__11_1,
	X1_1_1_1,
	X1_1_1_1,
	X1_1111_,
	X1______,
	X_1____1,
	X__1111_}},			/* @ */

	{ 'A', 0, 8, {
	X__111__,
	X_1___1_,
	X1_____1,
	X1_____1,
	X1111111,
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1}},			/* A */

	{ 'B', 0, 8, {
	X111111_,
	X_1____1,
	X_1____1,
	X_1____1,
	X_11111_,
	X_1____1,
	X_1____1,
	X_1____1,
	X111111_}},			/* B */

	{ 'C', 0, 8, {
	X__1111_,
	X_1____1,
	X1______,
	X1______,
	X1______,
	X1______,
	X1______,
	X_1____1,
	X__1111_}},			/* C */

	{ 'D', 0, 8, {
	X11111__,
	X_1___1_,
	X_1____1,
	X_1____1,
	X_1____1,
	X_1____1,
	X_1____1,
	X_1___1_,
	X11111__}},			/* D */

	{ 'E', 0, 8, {
	X1111111,
	X1______,
	X1______,
	X1______,
	X111111_,
	X1______,
	X1______,
	X1______,
	X1111111}},			/* E */

	{ 'F', 0, 8, {
	X1111111,
	X1______,
	X1______,
	X1______,
	X111111_,
	X1______,
	X1______,
	X1______,
	X1______}},			/* F */

	{ 'G', 0, 8, {
	X__1111_,
	X_1____1,
	X1______,
	X1______,
	X1______,
	X1__1111,
	X1_____1,
	X_1____1,
	X__1111_}},			/* G */

	{ 'H', 0, 8, {
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1,
	X1111111,
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1}},			/* H */

	{ 'I', 0, 8, {
	X_11111_,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X_11111_}},			/* I */

	{ 'J', 0, 8, {
	X__11111,
	X____1__,
	X____1__,
	X____1__,
	X____1__,
	X____1__,
	X____1__,
	X1___1__,
	X_111___}},			/* J */

	{ 'K', 0, 8, {
	X1_____1,
	X1____1_,
	X1___1__,
	X1__1___,
	X1_1____,
	X11_1___,
	X1___1__,
	X1____1_,
	X1_____1}},			/* K */

	{ 'L', 0, 8, {
	X1______,
	X1______,
	X1______,
	X1______,
	X1______,
	X1______,
	X1______,
	X1______,
	X1111111}},			/* L */

	{ 'M', 0, 8, {
	X1_____1,
	X11___11,
	X1_1_1_1,
	X1__1__1,
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1}},			/* M */

	{ 'N', 0, 8, {
	X1_____1,
	X11____1,
	X1_1___1,
	X1__1__1,
	X1___1_1,
	X1____11,
	X1_____1,
	X1_____1,
	X1_____1}},			/* N */

	{ 'O', 0, 8, {
	X__111__,
	X_1___1_,
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1,
	X_1___1_,
	X__111__}},			/* O */

	{ 'P', 0, 8, {
	X111111_,
	X1_____1,
	X1_____1,
	X1_____1,
	X111111_,
	X1______,
	X1______,
	X1______,
	X1______}},			/* P */

	{ 'Q', 0, 8, {
	X__111__,
	X_1___1_,
	X1_____1,
	X1_____1,
	X1_____1,
	X1__1__1,
	X1___1_1,
	X_1___1_,
	X__111_1}},			/* Q */

	{ 'R', 0, 8, {
	X111111_,
	X1_____1,
	X1_____1,
	X1_____1,
	X111111_,
	X1__1___,
	X1___1__,
	X1____1_,
	X1_____1}},			/* R */

	{ 'S', 0, 8, {
	X_11111_,
	X1_____1,
	X1______,
	X1______,
	X_11111_,
	X______1,
	X______1,
	X1_____1,
	X_11111_}},			/* S */

	{ 'T', 0, 8, {
	X1111111,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___}},			/* T */

	{ 'U', 0, 8, {
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1,
	X_11111_}},			/* U */

	{ 'V', 0, 8, {
	X1_____1,
	X1_____1,
	X1_____1,
	X_1___1_,
	X_1___1_,
	X__1_1__,
	X__1_1__,
	X___1___,
	X___1___}},			/* V */

	{ 'W', 0, 8, {
	X1_____1,
	X1_____1,
	X1_____1,
	X1_____1,
	X1__1__1,
	X1__1__1,
	X1_1_1_1,
	X11___11,
	X1_____1}},			/* W */

	{ 'X', 0, 8, {
	X1_____1,
	X1_____1,
	X_1___1_,
	X__1_1__,
	X___1___,
	X__1_1__,
	X_1___1_,
	X1_____1,
	X1_____1}},			/* X */

	{ 'Y', 0, 8, {
	X1_____1,
	X1_____1,
	X_1___1_,
	X__1_1__,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___}},			/* Y */

	{ 'Z', 0, 8, {
	X1111111,
	X______1,
	X_____1_,
	X____1__,
	X___1___,
	X__1____,
	X_1_____,
	X1______,
	X1111111}},			/* Z */

	{ '[', 0, 8, {
	X_1111__,
	X_1_____,
	X_1_____,
	X_1_____,
	X_1_____,
	X_1_____,
	X_1_____,
	X_1_____,
	X_1111__}},			/* [ */

	{ '\\', 0, 8, {
	X_______,
	X1______,
	X_1_____,
	X__1____,
	X___1___,
	X____1__,
	X_____1_,
	X______1,
	X_______}},			/* \ */

	{ ']', 0, 8, {
	X__1111_,
	X_____1_,
	X_____1_,
	X_____1_,
	X_____1_,
	X_____1_,
	X_____1_,
	X_____1_,
	X__1111_}},			/* ] */

	{ '^', 0, 8, {
	X___1___,
	X__1_1__,
	X_1___1_,
	X1_____1,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______}},			/* ^ */

	{ '_', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X1111111,
	X_______}},			/* _ */

	{ '`', 0, 8, {
	X__11___,
	X__11___,
	X___1___,
	X____1__,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______}},			/* ` */

	{ 'a', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_1111__,
	X_____1_,
	X_11111_,
	X1_____1,
	X1____11,
	X_1111_1}},			/* a */

	{ 'b', 0, 8, {
	X1______,
	X1______,
	X1______,
	X1_111__,
	X11___1_,
	X1_____1,
	X1_____1,
	X11___1_,
	X1_111__}},			/* b */

	{ 'c', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_1111__,
	X1____1_,
	X1______,
	X1______,
	X1____1_,
	X_1111__}},			/* c */

	{ 'd', 0, 8, {
	X_____1_,
	X_____1_,
	X_____1_,
	X_111_1_,
	X1___11_,
	X1____1_,
	X1____1_,
	X1___11_,
	X_111_1_}},			/* d */

	{ 'e', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_1111__,
	X1____1_,
	X111111_,
	X1______,
	X1____1_,
	X_1111__}},			/* e */

	{ 'f', 0, 8, {
	X___11__,
	X__1__1_,
	X__1____,
	X__1____,
	X11111__,
	X__1____,
	X__1____,
	X__1____,
	X__1____}},			/* f */

	{ 'g', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_111_1_,
	X1___11_,
	X1____1_,
	X1____1_,
	X1___11_,
	X_111_1_,
	X_____1_,
	X1____1_,
	X_1111__}},			/* g */

	{ 'h', 0, 8, {
	X1______,
	X1______,
	X1______,
	X1_111__,
	X11___1_,
	X1____1_,
	X1____1_,
	X1____1_,
	X1____1_}},			/* h */

	{ 'i', 0, 8, {
	X_______,
	X___1___,
	X_______,
	X__11___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X__111__}},			/* i */

	{ 'j', 0, 8, {
	X_______,
	X_______,
	X_______,
	X____11_,
	X_____1_,
	X_____1_,
	X_____1_,
	X_____1_,
	X_____1_,
	X_____1_,
	X_1___1_,
	X__111__}},			/* j */

	{ 'k', 0, 8, {
	X1______,
	X1______,
	X1______,
	X1___1__,
	X1__1___,
	X1_1____,
	X11_1___,
	X1___1__,
	X1____1_}},			/* k */

	{ 'l', 0, 8, {
	X__11___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X__111__}},			/* l */

	{ 'm', 0, 8, {
	X_______,
	X_______,
	X_______,
	X1_1_11_,
	X11_1__1,
	X1__1__1,
	X1__1__1,
	X1__1__1,
	X1__1__1}},			/* m */

	{ 'n', 0, 8, {
	X_______,
	X_______,
	X_______,
	X1_111__,
	X11___1_,
	X1____1_,
	X1____1_,
	X1____1_,
	X1____1_}},			/* n */

	{ 'o', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_1111__,
	X1____1_,
	X1____1_,
	X1____1_,
	X1____1_,
	X_1111__}},			/* o */


	{ 'p', 0, 8, {
	X_______,
	X_______,
	X_______,
	X1_111__,
	X11___1_,
	X1____1_,
	X1____1_,
	X11___1_,
	X1_111__,
	X1______,
	X1______,
	X1______}},			/* p */

	{ 'q', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_111_1_,
	X1___11_,
	X1____1_,
	X1____1_,
	X1___11_,
	X_111_1_,
	X_____1_,
	X_____1_,
	X_____1_}},			/* q */

	{ 'r', 0, 8, {
	X_______,
	X_______,
	X_______,
	X1_111__,
	X11___1_,
	X1______,
	X1______,
	X1______,
	X1______}},			/* r */

	{ 's', 0, 8, {
	X_______,
	X_______,
	X_______,
	X_1111__,
	X1____1_,
	X_11____,
	X___11__,
	X1____1_,
	X_1111__}},			/* s */

	{ 't', 0, 8, {
	X_______,
	X__1____,
	X__1____,
	X11111__,
	X__1____,
	X__1____,
	X__1____,
	X__1__1_,
	X___11__}},			/* t */

	{ 'u', 0, 8, {
	X_______,
	X_______,
	X_______,
	X1____1_,
	X1____1_,
	X1____1_,
	X1____1_,
	X1___11_,
	X_111_1_}},			/* u */

	{ 'v', 0, 8, {
	X_______,
	X_______,
	X_______,
	X1_____1,
	X1_____1,
	X1_____1,
	X_1___1_,
	X__1_1__,
	X___1___}},			/* v */

	{ 'w', 0, 8, {
	X_______,
	X_______,
	X_______,
	X1_____1,
	X1__1__1,
	X1__1__1,
	X1__1__1,
	X1__1__1,
	X_11_11_}},			/* w */

	{ 'x', 0, 8, {
	X_______,
	X_______,
	X_______,
	X1____1_,
	X_1__1__,
	X__11___,
	X__11___,
	X_1__1__,
	X1____1_}},			/* x */

	{ 'y', 0, 8, {
	X_______,
	X_______,
	X_______,
	X1____1_,
	X1____1_,
	X1____1_,
	X1____1_,
	X1___11_,
	X_111_1_,
	X_____1_,
	X1____1_,
	X_1111__}},			/* y */

	{ 'z', 0, 8, {
	X_______,
	X_______,
	X_______,
	X111111_,
	X____1__,
	X___1___,
	X__1____,
	X_1_____,
	X111111_}},			/* z */

	{ '}', 0, 8, {
	X___11__,
	X__1____,
	X__1____,
	X__1____,
	X_1_____,
	X__1____,
	X__1____,
	X__1____,
	X___11__}},			/* } */

	{ '|', 0, 8, {
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___,
	X___1___}},			/* | */

	{ '}', 0, 8, {
	X__11___,
	X____1__,
	X____1__,
	X____1__,
	X_____1_,
	X____1__,
	X____1__,
	X____1__,
	X__11___}},			/* } */

	{ '~', 0, 8, {
	X_11____,
	X1__1__1,
	X____11_,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______,
	X_______}},			/* ~ */

	{ 'X', 0, 8, {
	X_1__1__,
	X1__1__1,
	X__1__1_,
	X_1__1__,
	X1__1__1,
	X__1__1_,
	X_1__1__,
	X1__1__1,
	X__1__1_}}			/* rub-out */
};

/*
  9 by 8 font:
  12 rows high, 8 cols wide, 9 lines above baseline
 */
 struct font Font9x8 = {
	12, 8, 9, g9x8 
};
