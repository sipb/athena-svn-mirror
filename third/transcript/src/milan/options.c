/* 
 *  Copyright Milan Technology Inc. 1991, 1992
 */

/* @(#)options.c	2.1 10/15/92 */

#include "std.h"
#include "dp.h"
#include "errors.h"


extern char *g_filter_name; /* The name by which this program was invoked */
extern char *VERSION;

#ifdef ANSI
void setDefaults(s_options* opt)
#else
void setDefaults(opt)
s_options* opt;
#endif
{
   memset(opt, (char)0, sizeof(s_options));
   
   opt->dataport         = 2000;
   opt->adobe.banner_first = 1;
}

/* ADOBE: checks for those environment variables used by transcript
 * and sets the relevant option in the global option struct.
 */

/* envget is a getenv
 * 	if the variable is not present in the environment or
 *	it has the null string as value envget returns NULL
 *	otherwise it returns the value from the environment
 */

#ifdef ANSI
char *envget(char *var)
#else
char *envget(var)
char *var;
#endif
{
    char *val;
    if (!((val = (char*)getenv(var))) || (*val == 0))
    	return((char *) 0);
    else 
       return(val);
}

void parseAdobeEnvironment()
{
   char *test = 0;

   test = envget("BANNERFIRST");
   if (test) {
      g_opt.adobe.banner_first = atoi(test);
      g_opt.dobanner = 1;
   }

   test = envget("BANNERLAST");
   if (test) {
      g_opt.adobe.banner_last = atoi(test);
      g_opt.dobanner = 1;
   }

   test = envget("VERBOSELOG");
   if (test)
      g_opt.adobe.verbose_log = atoi(test);
}

#ifdef ANSI
void handleAdobeArgs(int argc, char **argv)
#else
void handleAdobeArgs(argc, argv)
int argc;
char **argv;
#endif
{
   /* In Transcript we are called like this:
    *
    * $PSCOMM -P $pname -p $prog -n $user -h $host $afile
    *
    * pscomm gets called with:
    *    stdin	== the file to print (may be a pipe!)
    *    stdout	== the printer
    *    stderr	== the printer log file
    *    cwd	== the spool directory
    *    argv	== set up by interface shell script:
    *   filtername	-P printer -p filtername [-r] (don't ever reverse)
    *              -n login -h host [accntfile]
    *
    */
   
   /* parse command-line arguments 
    * the argv (see header comments) comes from the spooler daemon
    * itself, so it should be canonical, but at least one 4.2-based
    * system uses -nlogin -hhost (insead of -n login -h host) so I 
    * check for both 
    */

   register char *cp;
   char **av = argv;
   g_opt.adobe.prog = *av;
   
   while (--argc) {
      if (*(cp = *++av) == '-') {
	 switch (*(cp + 1)) {
	 case 'P':	/* printer name */
	    argc--;
	    g_opt.adobe.pname = *(++av);
	    break;
	    
	 case 'n':	/* user name */
	    argc--;
	    g_opt.adobe.name = *(++av);
	    break;
	    
	 case 'h':	/* host */
	    argc--;
	    g_opt.adobe.host = *(++av);
	    break;
	    
	 case 'p':	/* prog */
	    argc--;
	    g_opt.adobe.prog = *(++av);
	    break;
	    
	 default:	/* unknown */
	    fprintf(stderr, "%s: unknown option: %s\n", g_opt.adobe.prog, cp);
	    break;
	 }
      }
      else
	 g_opt.adobe.accountingfile = cp;
   }

   /* ADOBE:
    * If g_opt.adobe.pname is set then we want to add this
    * printer to the list of printers.  But we want this to be
    * the lowest priority printer (ask Dan why) and put it last
    * after those found in .fpconfig.
    * So this routine better be called after the .fpconfig
    * file is parsed.
    */
   if (g_opt.adobe.pname)
   g_opt.prt_list = 
      form_printer_list(g_opt.adobe.pname, g_opt.prt_list, PARALLEL, APPEND);   
}

/* sets g_opt.current_dir if specified on command line (-D) */
#ifdef ANSI
void checkForCurrentDir(int argc, char **argv)
#else
void checkForCurrentDir(argc, argv)
int argc;
char **argv;
#endif
{
   int i = 1;
   while (i < argc) {
    if (argv[i][0] != '-') {
	  i++;
	  continue;
    }
      if (argv[i][1] == 'D') {
	 g_opt.current_dir = argv[i+1];
	 break;
      }
      i++;
   }
}


/* parses the command line args and returns "output_dest" flag */

/* We first look into command line option. If a host has been
 * specified, then we add the host to the printer list first. Then we look
 * at host specified in the '.fpconfig' file. We add that host to the list.
 * Finally we look at S_CLASS and P_CLASS options and add the hosts
 * accordingly.
 */

#ifdef ANSI
int parseCommandLineArgs(int argc, char **argv)
#else
int parseCommandLineArgs(argc, argv)
int argc;
char **argv;
#endif
{
   int iflag=0, wflag = 0, nflag = 0, lflag = 0, i;
   char               *host = 0;

   if ((g_filter_name  = (char *)rindex(argv[0], '/')))
      g_filter_name++;
   else 
      g_filter_name = argv[0];
   
   if ( ! (strcmp(g_filter_name, "fpcomm"))) {
      /* If this is called from Adobe Transcript then we skip the
       * rest of the command line handling and just handle the
       * regular options that goto pscomm:
       */
      parseAdobeEnvironment();
      handleAdobeArgs(argc, argv);
      /* return 2 for stderr so that any output back from the
       * socket goes to transcript log file.
       */
      return 2;
   }

  if ( ! (strcmp(g_filter_name, "fpcomm_s"))) {
       /* If this is called from Adobe Transcript then we skip the
        * rest of the command line handling and just handle the
        * regular options that goto pscomm:
        */
       g_opt.dataport     = SERIAL ;
       parseAdobeEnvironment();
       handleAdobeArgs(argc, argv);
       /* Make sure that the printer type is set to serial */
       if (g_opt.prt_list) {
               g_opt.prt_list->ptype = SERIAL;
       }
      return 2;
   }

   if ( ! (strcmp(g_filter_name, "parfilter"))) {
      g_opt.dataport = PARALLEL;
      g_opt.use_control_d = 0;
      g_opt.real_filter = 1;
   } 
   else if ( ! (strcmp(g_filter_name, "pardfilter"))) {
      g_opt.dataport = PARALLEL;
      g_opt.use_control_d = 1;
      g_opt.real_filter = 1;
   } 
   else if ( ! (strcmp(g_filter_name, "serfilter"))) {
      g_opt.dataport = SERIAL;
      g_opt.use_control_d = 0;
      g_opt.real_filter = 1;
   } 
   else if ( ! (strcmp(g_filter_name, "serdfilter"))) {
      g_opt.dataport = SERIAL;
      g_opt.use_control_d = 1;
      g_opt.real_filter = 1;
   } 
   else if ( ! (strcmp(g_filter_name, "psparfilter"))) {
      g_opt.dataport = PARALLEL;
      g_opt.real_filter = 1;
      g_opt.check_postscript = 1;
   } 
   else if ( ! (strcmp(g_filter_name, "pspardfilter"))) {
      g_opt.dataport = PARALLEL;
      g_opt.use_control_d = 1;
      g_opt.real_filter = 1;
      g_opt.check_postscript = 1;
   } 
   else if ( ! (strcmp(g_filter_name, "psserfilter"))) {
      g_opt.dataport = SERIAL;
      g_opt.real_filter = 1;
      g_opt.check_postscript = 1;
   } 
   else if ( ! (strcmp(g_filter_name, "psserdfilter"))) {
      /* code folded from here */
      g_opt.dataport = SERIAL;
      g_opt.use_control_d = 1;
      g_opt.real_filter = 1;
      g_opt.check_postscript = 1;
      /* unfolding */
   }
   
   i = 1;
   while ((i < argc) && (argv[i][0] == '-')) {
      switch (argv[i][1]) {
      case 'P':
	 if (! strcasecmp(argv[i], "-P_CLASS")) 
	    g_opt.prt_list = 
	       form_printer_list(argv[i+1], g_opt.prt_list, PARALLEL, APPEND);
	 else
	    host = argv[i + 1];
	 i++;
	 break;
	 
      case 'S':
	 if (! strcasecmp(argv[i], "-S_CLASS")) 
	    g_opt.prt_list = 
	       form_printer_list(argv[i+1], g_opt.prt_list, SERIAL, APPEND);
	 i++;
	 break;
      case 'c':
	 if (! strcasecmp(argv[i], "-closewait")) 
		g_opt.closewait = 1;
	 break;

      case 'D':
	 g_opt.current_dir = argv[i+1];
	 i++;
	 break;
	 
      case 'A':
	 g_opt.asciifilter = argv[i + 1];
	 g_opt.check_postscript = 1;
	 i++;
	 break;
	 
      case 'i':
	 iflag++;
	 break;
      case 'w':
	 wflag++;
	 break;
      case 'l':
	 lflag++;
	 break;
      case 'd':
#ifdef DATAGEN
	 if (!strcmp(argv[i], "-dataport")) {
	    g_opt.dataport = atoi(argv[i + 1]);
	    i++;
	    break;
	 }
#endif
	 g_opt.use_control_d = 1;
	 break;
      case 'b':
	 g_opt.dobanner = 1;
	 break;
      case 's':
	 if (!strcmp(argv[i], "-startstring")) {
	    g_opt.start_string = parse_string(argv[i + 1]);
	    i++;
	 }
	 else if (! strcmp(argv[i], "-startfile")) {
	    g_opt.send_startfile = 1;
	    strcpy(g_opt.start_file, argv[i + 1]);
	    i++;
	 }
	 else if (! strcasecmp(argv[i], "-sys"))
	    g_opt.notify_type.syslog = 1;
	 else 
	    g_opt.dataport = SERIAL;
	 break;
      case 'e':
	 if (!strcmp(argv[i], "-endstring")) {
	    g_opt.end_string = parse_string(argv[i + 1]);
	    i++;
	 }
	 else if (! strcmp(argv[i], "-endfile")) {
	    g_opt.send_endfile = 1;
	    strcpy(g_opt.end_file, argv[i + 1]);
	    i++;
	 }
	 else if (! strcasecmp(argv[i], "-errorfile"))  {
	    g_opt.notify_type.file = 1;
	    strcpy(g_opt.notify_type.filename, argv[i+1]);
	    i++;
	 }
	 break;
      case 'x':
      case 'y':
	 break;
      case 'm':
	 if (!strcmp(argv[i], "-mail")) {
	    g_opt.notify_type.mail = 1;
	    strcpy(g_opt.notify_type.user, argv[i+1]);
	    i++;
	 }
	 else
	    g_opt.mapflg++;	    
	 break;
      case 'n':
	 i++;
	 nflag++;
	 break;
      case 'h':
	 i++;
	 break;
      case 'f':
	 g_opt.ff_flag = 1;
	 break;
      case 'p':
	 if (! strcasecmp(argv[i], "-program"))  {
	    g_opt.notify_type.program = 1; /* Notify thru program */
	    strcpy(g_opt.notify_type.prog_name, argv[i+1]);
	    i++;
	 }
	 else
	    g_opt.check_postscript = 1;
	 break;
      case 'V':
	 fprintf(stderr, "version = %s\n", VERSION);
	 break;
      default:
	 break;
      }
      i++;
   }
   
   if (! nflag ) {
      for (; i < argc; i++) {
	 if (! access(argv[i], R_OK))
	    /* files to read and send across */
	    add_file(&g_opt.file_list, argv[i]);
	 else
	    error_notify(ERR_ILLFILE,0);
      }
   }
   
   /* If a host has been specified on the command line 
    * then add this to the FRONT of the list of printers.
    */
   if (host) 
      g_opt.prt_list = 
	 form_printer_list(host, g_opt.prt_list, g_opt.dataport, PREPEND);
   
   return (iflag || wflag || lflag ) ? 2 : 1;   
}

