/* fmtutil.c: this program emulates the fmtutil shell script from tetex.

Copyright (C) 1998 Fabrice POPINEAU.

Time-stamp: <99/05/10 17:59:51 popineau>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* Maximum number of lines per usage message. */
#define MAX_LINES 16

#define SEPARATORS " \t\n\""

#include <signal.h>

#include <kpathsea/kpathsea.h>
extern KPSEDLL char* kpathsea_version_string;

#include "fileutils.h"
#include "stackenv.h"
#include "variables.h"
#include "fmtutil.h"

#ifdef _WIN32
#define isatty _isatty
#else
#include <glob.h>
#endif

extern string fmtutil_version_string;

/* 
   Global Variables found in the fmtutil script
*/

static string texmfmain;
static string tempenv;
static string cwd;

static string mode, bdpi;

/* fmtutil.cnf */

char tmpdir[PATH_MAX]; /* Working temporary directory */
string output = "astdout";
FILE *fout = NULL;
FILE *fnul = NULL;
static boolean downcase_names;
char cmd_buf[1024];			/* Hope it is enough for handling for command */
char empty_str[] = "(empty)";
char cmd_sys[1024];		/* system() calls */

static int errstatus;

void usage(void);

/* Test whether getopt found an option ``A''.
   Assumes the option index is in the variable `option_index', and the
   option table in a variable `long_options'.  */
#define ARGUMENT_IS(a) STREQ (long_options[option_index].name, a)

static struct option long_options [] = {
    { "all",               0, 0, 0},
    { "quiet",             0, 0, 0},
    { "test",              0, 0, 0},
    { "force",             0, 0, 0},
    { "dolinks",           0, 0, 0},
    { "missing",           0, 0, 0},
    { "byfmt",             1, 0, 0},
    { "byhyphen",          1, 0, 0},
    { "showhyphen",        1, 0, 0},
    { "help",              0, 0, 0},
    { "cnffile",           1, 0, 0},
    { "fmtdir",            1, 0, 0},
    { "debug",             1, 0, 0},
    {0, 0, 0, 0}
};

enum {
  NONE, ALL, MISSING, BYFMT, BYHYPHEN, SHOWHYPHEN
};

#define MAX_FMTS 128

typedef struct fmtdesc {
  string format;		/* e.g.: latex.fmt */
  string engine;		/* e.g.: tex */
  string progname;		/* e.g.: latex */
  string hyphenation;		/* e.g.: - */
  string texargs;
  string inifile;
} fmtdesc;

static fmtdesc fmts[MAX_FMTS];
static int nb_formats = 0;

string unrmable_engines[] = {
  "tex", "pdftex", "etex", "pdfetex", "omega", NULL
};

/*
  First  argument takes progname;
  Other arguments take no arguments.
  */
static string usage_msg[MAX_LINES] = { 
	"Usage: fmtutil [option] ... cmd [argument]\n\n",
	"Valid options:\n",
	"  --cnffile file\n",
	"  --fmtdir directory\n",
	"  --quiet                    no output except error messages\n",
	"  --test                     only print what would be done\n",
	"  --dolinks                  link engine to format\n\n",
	"  --force                    force links even if target exists\n\n",
	"Valid commands:\n",
	"  --all                      recreate all format files\n",
	"  --missing                  create all missing format files\n",
	"  --byfmt formatname         (re)create format for `formatname'\n",
	"  --byhyphen hyphenfile      (re)create formats that depend on `hyphenfile'\n",
	"  --showhyphen formatname    print name of hyphenfile for format `formatname'\n",
	"  --help                     show this message\n",
};

string progname = NULL;
static string fmtutil_version_string = "Revision: 0.2";

static string cnf_default = "fmtutil.cnf";
static string cnf_file = NULL;
static string arg = NULL;
static string destdir = NULL;
static int cmd= NONE;
static boolean quiet = false;
static boolean norebuild = false;
static boolean dolinks = false;
static boolean force = false;

#define MAX_LINKS 128

struct _links {
  string src;
  string dst;
} to_link[MAX_LINKS];
int nb_links = 0;

void link_formats();

/*
  First part: fmtutil.opt
  */
int fmtutil_opt(int argc, char *argv[])
{
  int g; /* getopt return code */
  int i;
  int option_index;

  for(;;) {
    g = getopt_long_only (argc, argv, "", long_options, &option_index);

    if (g == EOF)
      break;

    if (g == '?')
      return 1;  /* Unknown option.  */

    /* assert (g == 0); */ /* We have no short option names.  */
    
    if (ARGUMENT_IS ("debug")) {
      kpathsea_debug |= atoi (optarg);
    }
    else if (ARGUMENT_IS ("quiet")) {
      quiet = true;
    }
    else if (ARGUMENT_IS ("test")) {
      norebuild = true;
    }
    else if (ARGUMENT_IS ("dolinks")) {
      dolinks = true;
    }
    else if (ARGUMENT_IS ("force")) {
      force = true;
    }
    else if (ARGUMENT_IS ("help")) {
      usage();
      exit(0);
    }
    else if (ARGUMENT_IS ("version")) {
      fprintf(stderr, "%s (version %s) of %s.\n", progname, 
	      fmtutil_version_string,
	      kpathsea_version_string);
      exit(0);
    }
    else if (ARGUMENT_IS("all")) {
      cmd = ALL;
    }
    else if (ARGUMENT_IS("missing")) {
      cmd = MISSING;
    }
    else if (ARGUMENT_IS("byfmt")) {
      cmd = BYFMT;
      arg = xstrdup(optarg);
    }
    else if (ARGUMENT_IS("byhyphen")) {
      cmd = BYHYPHEN;
      arg = xstrdup(optarg);
    }
    else if (ARGUMENT_IS("showhyphen")) {
      cmd=SHOWHYPHEN;
      arg = xstrdup(optarg);
    }
    else if (ARGUMENT_IS("cnffile")) {
      cnf_file = xstrdup(optarg);
    }
    else if (ARGUMENT_IS("fmtdir")) {
      destdir = xstrdup(optarg);
    }
  }

  /* shifting options from argv[] list */
  for (i = 1; optind < argc; i++, optind++)
    argv[i] = argv[optind];
  argv[i] = NULL;

  argc = i;
#if 0
  fprintf(stderr, "New args [%d] : ", argc);
  for (i = 0; i < argc; i++)
    fprintf(stderr, "%s ", argv[i]);
  fprintf(stderr, "\n");
  if (argc < program->arg_min) {
    fprintf (stderr, "%s: Missing argument(s).\nTry `%s --help' for more information.\n", progname, kpse_program_name);
    exit(1);
  }
#endif
  if (argc > 1) {
    fprintf(stderr, "%s: Extra arguments", progname);
    fprintf (stderr, "\nTry `%s --help' for more information.\n",
	     kpse_program_name);
    exit(1);
  }

  return argc;
}
  
/* Reading fmtutil.cnf */
boolean parse_line(string line)
{
  string format, engine, hyphenation, texargs, inifile;
  string p;

  format = strtok(line, "\t ");
  if (!format || !*format)
    return false;
  engine = strtok(NULL, "\t ");
  if (!engine || !*engine)
    return false;
  hyphenation = strtok(NULL, "\t ");
  if (!hyphenation || !*hyphenation)
    return false;
  texargs = strtok(NULL, "\n\0");
  if (!texargs || !*texargs) 
    return false;
  for (p = texargs+strlen(texargs) - 1; 
       !isspace(*p) && (*p != '*') && (p>=texargs); p--);
  inifile = ++p;

#if 0
  if (strcmp(inifile, "pdflatex.ini")== 0) {
        kpathsea_debug=-1; 
  }
#endif
  kpse_reset_program_name(format);
#if 0
  /* kpathsea 3.3beta5 should correct this */
  kpse_format_info[kpse_tex_format].cnf_path = kpse_cnf_get("TEXINPUTS");
#endif
  if (!kpse_find_file(inifile, kpse_tex_format, false)) {
    fprintf(stderr, "%s: ini file %s not found\n", cnf_file, inifile);
    return false;
  }
  kpse_reset_program_name("fmtutil");
#if 0
  /* kpathsea 3.3beta5 should correct this */
  kpse_format_info[kpse_tex_format].cnf_path = kpse_cnf_get("TEXINPUTS");
  kpathsea_debug=0;
#endif
  fmts[nb_formats].progname = xstrdup(format);

  if (STREQ(engine, "etex")
      || STREQ(engine, "pdfetex")) {
    fmts[nb_formats].format = concat(format, ".efmt");
  }
  else {
    fmts[nb_formats].format = concat(format, ".fmt");
  }
  fmts[nb_formats].engine = xstrdup(engine);
  fmts[nb_formats].hyphenation = xstrdup(hyphenation);
  fmts[nb_formats].texargs = xstrdup(texargs);
  fmts[nb_formats].inifile = xstrdup(inifile);
  nb_formats++;

  return true;
}

void read_fmtutilcnf()
{
  FILE *f;
  string line;
#if 0
  __asm int 3;
#endif
  if (test_file('n', cnf_file) && test_file('r', cnf_file)) {
    f = fopen(cnf_file, "r");
    if (KPSE_DEBUG_P(FMTUTIL_DEBUG)) {
      fprintf(stderr, "Reading fmtutil_cnf file: %s\n", cnf_file);
    }
    while ((line  = read_line(f)) != NULL) {
      /* skip comments */
      if (*line == '#' || *line == '%' || isspace(*line)
	  || *line == '\0' || *line == '\n')
	continue;
      
      if (!parse_line(line))
	fprintf(stderr, "%s: syntax error: %s\n", cnf_file, line);
      free(line);
    }
    fclose(f);
    if (KPSE_DEBUG_P(FMTUTIL_DEBUG)) {
      int i;
      for (i = 0; i < nb_formats; i++) {
	fprintf(stderr, "Line %d: fmt = %s engine = %s progname = %s hyph = %s tex = %s ini = %s\n",
		i, fmts[i].format, fmts[i].engine, fmts[i].progname,
		fmts[i].hyphenation, fmts[i].texargs, fmts[i].inifile);
      }
    }

  }
}

/* If non-negative, records the handle to redirect STDOUT
   in `output_and_cleanup'.  */
static int redirect_stdout = -1;

/* Record the handle where STDOUT is to be redirected
   for printing the names of the generated files before we exit.  */
void record_output_handle(int fd)
{
  redirect_stdout = fd;	/* FIXME: cannot be nested! */
}

void output_and_cleanup(int code)
{
  /* FIXME : what cleanup ? */
  if (test_file('d', tmpdir))
    rec_rmdir(tmpdir);
}

void usage()
{
  int i;
  fprintf(stderr, "%s of %s\n", progname, kpathsea_version_string);
  fprintf(stderr, "Fmtutil version %s\n", fmtutil_version_string);
  fprintf(stderr,usage_msg[0], progname );
  fputs("\n", stderr);
  for(i = 1; usage_msg[i]; ++i)
    fputs(usage_msg[i], stderr);
}


int main(int argc, char* argv[])
{
  string texinputs;

#if defined(WIN32)
  /* if _DEBUG is not defined, these macros will result in nothing. */
   SETUP_CRTDBG;
   /* Set the debug-heap flag so that freed blocks are kept on the
    linked list, to catch any inadvertent use of freed memory */
   SET_CRT_DEBUG_FIELD( _CRTDBG_DELAY_FREE_MEM_DF );
#endif

  if (!progname)
    progname = argv[0];
  kpse_set_program_name (progname, NULL);

  /* initialize the symbol table */
  init_vars();

  /* `kpse_init_prog' will put MODE and DPI into environment as values of
     the two variables below.  We need to leave these variables as they are. */
  mode = getval("MAKETEX_MODE");
  bdpi = getval("MAKETEX_BASE_DPI");

  /* NULL for no fallback font. */
  kpse_init_prog (uppercasify (progname), 
		  bdpi && atoi(bdpi) ? atoi(bdpi) : 600, mode, NULL);

  /* fmtutil_opt may modify argc and shift argv */
  argc = fmtutil_opt(argc, argv);

  setval_default("DPI", getval("BDPI"));
  setval_default("MAG", "1.0");

  /* if no cnf_file from command-line, look it up with kpsewhich: */
  if (test_file('z', cnf_file)) {
    cnf_file = kpse_find_file(cnf_default, kpse_web2c_format, true);
    if (test_file('z', cnf_file)) {
      fprintf(stderr, "%s: fatal error, configuration file not found.\n",
	      progname);
      exit(1);
    }
  }
  if (!test_file('f', cnf_file)) {
    fprintf(stderr, "%s: config file `%s' not found.\n", progname, cnf_file);
    exit(1);
  }

  /* read the fmtutil.cnf file */
  read_fmtutilcnf();

  if (cmd == SHOWHYPHEN) {
    exit(show_hyphen_file(arg));
  }

  cache_vars();

  texmfmain = getval("MT_TEXMFMAIN");
  if (test_file('z', texmfmain) || !test_file('d', texmfmain)) {
    fprintf(stderr, "%s: $TEXMFMAIN is undefined or points to a non-existent directory;\n%s: check your installation.\n", progname, progname);
    exit(1);	/* not mt_exit, since temporary files were not created yet */
  }

  /* setup dest dir */
  if (test_file('z', destdir)) {
    setval_default("VARTEXMF", expand_var("$VARTEXMF"));
    if (test_file('z', getval("VARTEXMF")))
      setval("VARTEXMF", expand_var("$MT_TEXMFMAIN"));
    destdir = concat(getval("VARTEXMF"), "/web2c");
  }
  if (!test_file('d', destdir)) {
    fprintf(stderr, "%s: format directory `%s' does not exist.\n",
	    progname, destdir);
    exit(1);
  }

  /* Catch signals, so we clean up if the child is interrupted.
     This emulates "trap 'whatever' 1 2 15".  */
#ifdef _WIN32
  SetConsoleCtrlHandler((PHANDLER_ROUTINE)sigint_handler, TRUE);
#else
# ifdef SIGINT
  signal (SIGINT, sigint_handler);
# endif
# ifdef SIGHUP
  signal (SIGHUP, sigint_handler);
# endif
# ifdef SIGTERM
  signal (SIGTERM, sigint_handler);
# endif
#endif

  tempenv = getval("TMPDIR");
  if (test_file ('z', tempenv))
    tempenv = getval("TEMP");
  if (test_file ('z', tempenv))
    tempenv = getval("TMP");
#ifdef DOSISH
  if (test_file ('z', tempenv))
    tempenv = "c:/tmp";	/* "/tmp" is not good if we are on a CD-ROM */
#else
  if (test_file ('z', tempenv))
    tempenv = "/tmp";
#endif
  sprintf(tmpdir, "%s/fmXXXXXX", tempenv);
  mktemp(tmpdir);
  if (do_makedir(tmpdir)) {
    perror(tmpdir);
    exit(1);
  }
  setval("TEMPDIR", tmpdir);
  setval("STDOUT", concat_pathes(tmpdir, "astdout"));
  setval("KPSE_DOT", cwd = xgetcwd());
  /* export KPSE_DOT */
  xputenv("KPSE_DOT", cwd);

#if 0
  texinputs = getval("TEXINPUTS");
  if (test_file('z', texinputs)) {
    xputenv("TEXINPUTS", concat(cwd, ENV_SEP_STRING));
  }
  else {
    xputenv("TEXINPUTS", concatn(cwd, ENV_SEP_STRING, 
				 texinputs, ENV_SEP_STRING, NULL));
  }
#endif
  pushd(tmpdir);
  
  if ((fout = fopen(getval("STDOUT"), "w")) == NULL) {
    perror(output);
    mt_exit(1);
  }
  
  /* make local pathes absolute */
  if (! kpse_absolute_p(destdir, FALSE) ) {
    destdir = concat3(cwd, DIR_SEP_STRING, destdir);
  }
  if (! kpse_absolute_p(cnf_file, FALSE) ) {
    cnf_file = concat3(cwd, DIR_SEP_STRING, cnf_file);
  }


  /* umask = 0 */
  umask(0);	/* for those platforms who support it */

  errstatus = 0;

  switch (cmd) {
  case ALL:
    errstatus = recreate_all();
    break;
  case MISSING:
    errstatus = create_missing();
    break;
  case BYFMT:
    errstatus = create_one_format(arg);
    break;
  case BYHYPHEN:
    errstatus = recreate_by_hyphenfile(arg);
    break;
  }
#if 0
  __asm int 3;
#endif
  /* Install the log files and format files */
  process_multiple_files("*.log", move_log);
  process_multiple_files("*.fmt", move_fmt);
  process_multiple_files("*.efmt", move_fmt);

  if (dolinks)
    link_formats();

  mt_exit(errstatus);

}

/*
###############################################################################
# cache_vars()
#   locate files / kpathsea variables and export variables to environment
#    this speeds up future calls to e.g. mktexupd
###############################################################################
*/
void cache_vars()
{
  setval_default("MT_VARTEXFONTS", expand_var("$VARTEXFONTS"));
  setval_default("MT_TEXMFMAIN", kpse_path_expand("$TEXMFMAIN"));
#if 0
  setval_default("MT_MKTEXNAM", kpse_find_file("mktexnam.exe", kpse_web2c_format, false));
  setval_default("MT_MKTEXNAM_OPT", kpse_find_file("mktexnam.opt", kpse_web2c_format, false));
  setval_default("MT_MKTEXDIR", kpse_find_file("mktexdir.exe", kpse_web2c_format, false));
  setval_default("MT_MKTEXDIR_OPT", kpse_find_file("mktexnam.opt", kpse_web2c_format, false));
  setval_default("MT_MKTEXUPD", kpse_find_file("mktexupd.exe", kpse_web2c_format, false));
#endif
  setval_default("MT_MKTEX_CNF", kpse_find_file("mktex.cnf", kpse_web2c_format, false));
  setval_default("MT_MKTEX_OPT", kpse_find_file("mktex.opt", kpse_web2c_format, false));
  /* This should give us the ls-R default path */
  if (!kpse_format_info[kpse_db_format].type) /* needed if arg was numeric */
    kpse_init_format (kpse_db_format);
  setval_default("MT_LSR_PATH", kpse_format_info[kpse_db_format].path);
}

void do_format(int f)
{
  string cmdfmt = "%s -ini %s=%s -progname=%s %s <nul";
  string fmtswitch;

  if (FILESTRCASEEQ(fmts[f].engine, "etex")
      || FILESTRCASEEQ(fmts[f].engine, "pdfetex")) {
    fmtswitch = "-efmt";
  }
  else {
    fmtswitch = "-fmt";
  }
  if (sprintf(cmd_sys, cmdfmt, fmts[f].engine, fmtswitch, fmts[f].progname,
	      fmts[f].progname, fmts[f].texargs) > 1023) {
    fprintf(stderr, "Warning: do_format buffer overrun!\n");
    fprintf(stderr, "no command run.\n");
    return;
  }
  
  /* run discarding stdout */
  start_redirection(quiet);

  fprintf(stderr, "Running: %s\n", cmd_sys);

  if (!norebuild && system(cmd_sys) == -1) {
    fprintf(stderr, "%s: command `%s' failed.\n", progname, cmd_sys);
    unlink(fmts[f].format);
  }

  pop_fd();

  if (dolinks) {
    to_link[nb_links].src = fmts[f].engine;
    to_link[nb_links].dst = fmts[f].progname;
    nb_links++;
  }
}

int recreate_all()
{
  int i;

  for (i = 0; i < nb_formats; i++) {
    do_format(i);
  }

  return 0;
}

int create_missing()
{
  int i;
  string destfmt;

  for (i = 0; i < nb_formats; i++) {
    destfmt = concat_pathes(destdir, fmts[i].format);
    if (! test_file('f', destfmt)) {
      do_format(i);
    }
    free(destfmt);
  }
  return 0;
}

int create_one_format(string fmtname)
{
  int i;

  for (i = 0; i < nb_formats; i++) {
    if (FILESTRCASEEQ(fmtname, fmts[i].progname)) {
      do_format(i);
      return 0;
    }
  }
  fprintf(stderr, "%s: format %s not found in fmtutil.cnf.\n",
	  progname, fmtname);
  return 0;
}

int recreate_by_hyphenfile(string hyphenfile)
{
  int i;

  for (i = 0; i < nb_formats; i++) {
    if (FILESTRCASEEQ(hyphenfile, fmts[i].hyphenation)) {
      do_format(i);
      return 0;
    }
  }
  return 0;
}

int show_hyphen_file(string format)
{
  int i;
  string hyphenfile;

  for (i = 0; i < nb_formats; i++) {
    if (FILESTRCASEEQ(format, fmts[i].progname)) {
      if (STREQ(fmts[i].hyphenation, "-")) {
	printf("%s\n", fmts[i].hyphenation);
	return 0;
      }
      kpse_reset_program_name(format);
      kpse_format_info[kpse_tex_format].cnf_path = kpse_cnf_get("TEXINPUTS");
      if (!(hyphenfile 
	    = kpse_find_file(fmts[i].hyphenation, kpse_tex_format, false))) {
	fprintf(stderr, "%s: hyphen file %s not found\n", progname, 
		fmts[i].hyphenation);
	return -1;
      }
      printf("%s\n", hyphenfile);
      free(hyphenfile);
      return 0;
    }
  }
  fprintf(stderr, "no info for format %s\n", format);
  return 0;
}

/*
  This function expects a globbing file specification.
  There is no recursive call here.
*/
void process_multiple_files(string filespec, process_fn fn)
{

  char path[PATH_MAX];
  int path_len;
#if defined(_WIN32)
  WIN32_FIND_DATA find_file_data;
  HANDLE hnd;
#else
  glob_t gl;
#endif

  strcpy(path, filespec);

  /* If it is a directory, then get all the files there */
  if (test_file('d', path)) {
    path_len = strlen(path);
    strcat(path, "/*");
  }
  else {
    path_len = strlen(path) - strlen(xbasename(path)) -1;
  }

#if defined(_WIN32)
  hnd = FindFirstFile(path, &find_file_data);
  while (hnd != INVALID_HANDLE_VALUE) {
    if(!strcmp(find_file_data.cFileName, ".")
       || !strcmp(find_file_data.cFileName, "..")) 
	continue;
    path[path_len+1] = '\0';
    strcat(path, find_file_data.cFileName);
    if (KPSE_DEBUG_P(FMTUTIL_DEBUG)) {
      fprintf(stderr, "Processing %s\n", path);
    }
    (*fn)(path);
      if (FindNextFile(hnd, &find_file_data) == FALSE)
	break;
  }
  path[path_len+1] = '\0';
  FindClose(hnd);

#else /* ! WIN32 */

  switch  (glob(path, GLOB_NOCHECK, NULL, &gl)) {
  case 0:			/* success */
      for (i = 0; i < gl.gl_pathc; i++) {
	(*fn)(gl.gl_pathv[i]);
      }
      globfree(&gl);
      break;

  case GLOB_NOSPACE:
    fprintf(stderr, "%s: Out of memory while globbing %s.\n",
	    kpse_program_name, filespec);
    mt_exit(1);
    break;

  default:
    break;
  }
#endif

}

void move_log(string logname)
{
  string logdest;
  logdest = concat3(destdir, DIR_SEP_STRING, logname);
  unlink(logdest);
  mvfile(logname, logdest);
  free(logdest);
}

void move_fmt(string fmtname)
{
  string fmtdest;
  string texfmt, plainfmt;
  fmtdest = concat3(destdir, DIR_SEP_STRING, fmtname);
  unlink(fmtdest);
  if (KPSE_DEBUG_P(FMTUTIL_DEBUG)) {
    fprintf(stderr, "Moving %s to %s\n", fmtname, fmtdest);
  }
  mvfile(fmtname, fmtdest);
  free(fmtdest);
  texfmt = concat3(destdir, DIR_SEP_STRING, "tex.fmt");
  plainfmt = concat3(destdir, DIR_SEP_STRING, "plain.fmt");
  if (test_file('f', texfmt) && !test_file('f', plainfmt)) {
    catfile(texfmt, plainfmt, FALSE);
  }
  free(texfmt);
  texfmt = concat3(destdir, DIR_SEP_STRING, "tex.efmt");
  plainfmt = concat3(destdir, DIR_SEP_STRING, "plain.efmt");
  if (test_file('f', texfmt) && !test_file('f', plainfmt)) {
    catfile(texfmt, plainfmt, FALSE);
  }
  free(texfmt);
}

void link_formats()
{
  int i, j;
  boolean skip;
  string src, srcdll, srcexe, dstexe;

  for (i = 0; i < nb_links; i++) {
    src = expand_var(concat("$SELFAUTOLOC/", to_link[i].src));
    /* Check that the destination is not among the ones that should
     *never* be removed */
    skip = false;
    for (j = 0; unrmable_engines[j] != NULL; j++) {
      if (FILESTRCASEEQ(to_link[i].dst, unrmable_engines[j])) {
	fprintf(stderr, 
		"%s: %s is a basic engine, won't remove it... skipping.\n",
		kpse_program_name, unrmable_engines[j]);
	skip = true;
	break;
      }
    }
    /* Any smarter way to do that ? */
    if (skip)
      break;

    dstexe = expand_var(concatn("$SELFAUTOLOC/", to_link[i].dst, ".exe", NULL));
    if (test_file('f', dstexe) && force == false) {
      fprintf(stderr, "%s: destination %s already exists, skipping.\n", kpse_program_name, dstexe);
      free(dstexe);
      continue;
    }
    srcexe = concat(src, ".exe");
#ifdef WIN32
    srcdll = concat(src, ".dll");
    if (FILESTRCASEEQ(srcexe, dstexe)) {
      printf("same files: %s and %s, doing nothing\n", srcexe, dstexe);
    }
    else if (test_file('f', srcdll)) {
      if (norebuild) {
	printf("copy %s to %s\n", srcexe, dstexe);
      }
      else if (catfile(srcexe, dstexe, false) == false) {
	fprintf(stderr, "%s: failed to copy %s to %s\n", 
		kpse_program_name, srcexe, dstexe);
      }
    }
    else {
      sprintf(cmd_sys,"lnexe %s %s", srcexe, dstexe);
      if (norebuild) {
	printf("lnexe %s to %s\n", srcexe, dstexe);
      }
      else if (system(cmd_sys) == -1) {
	fprintf(stderr, "%s: failed to lnexe %s to %s\n", 
		kpse_program_name, srcexe, dstexe);
      }
    }
    free(srcdll);
#else
    srcexe = concat(src, ".exe");
    if (FILESTRCASEEQ(srcexe, dstexe)) {
      printf("same files: %s and %s, doing nothing\n");
    }
    else if (norebuild) {
      printf("copy %s to %s\n", srcexe, dstexe);
    }
    else if (catfile(srcexe, dstexe, false) == false) {
      fprintf(stderr, "%s: failed to copy %s to %s\n", 
	      kpse_program_name, srcexe, dstexe);
    }
#endif
    free(srcexe);
    free(dstexe);
  }
}
