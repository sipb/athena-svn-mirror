/********************************************************************/
/*                                                                  */
/*                      F R O N T E N D                             */
/*                                                                  */
/*                  Mainline for IDL compilers.                     */
/*                                                                  */
/********************************************************************/

/*
 * ========================================================================== 
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 *
 * Apollo Computer Inc. reserves all rights, title and interest with respect 
 * to copying, modification or the distribution of such software programs and
 * associated documentation, except those rights specifically granted by Apollo
 * in a Product Software Program License, Source Code License or Commercial
 * License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between Apollo and 
 * Licensee.  Without such license agreements, such software programs may not
 * be used, copied, modified or distributed in source or object code form.
 * Further, the copyright notice must appear on the media, the supporting
 * documentation and packaging as set forth in such agreements.  Such License
 * Agreements do not grant any rights to use Apollo Computer's name or trademarks
 * in advertising or publicity, with respect to the distribution of the software
 * programs without the specific prior written permission of Apollo.  Trademark 
 * agreements may be obtained in a separate Trademark License Agreement.
 * ========================================================================== 
 */



#include <stdio.h>
#include <signal.h>
#include <string.h>

#ifdef vms
#  include <types.h>
#  include <stat.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include "fcntl.h"
#endif

#include "idl_base.h"
#include "files.h"
#include "nametbl.h"
#include "ast.h"
#include "astp.h"
#include "frontend.h"
#include "errors.h"
#include "backend.h"
#include "utils.h"
#include "getflags.h"
#include "sysdep.h"

extern void exit __PROTOTYPE((int));
extern int yyparse __PROTOTYPE((void));
extern void AST_dump_interface __PROTOTYPE((interface_t *));
    
/*--------------------------------------------------------------------*/

extern  binding_t * the_interface;
extern int  yynerrs;
extern int  yydebug;       
extern int  yyprevious;       

/*--------------------------------------------------------------------*/


#ifdef UNIX
# ifdef apollo 
#  define CPP "/usr/lib/cpp " 
# else
#  define CPP "/lib/cpp "
# endif
# define SUFFIXES "_cstub.c,_sstub.c,_cswtch.c"
#endif

#ifdef MSDOS
# define SUFFIXES "C.C,S.C,W.C"
static int max_suffix_len;
#endif

#ifdef VMS     
# define SUFFIXES "_CSTUB.C,_SSTUB.C,_CSWTCH.C"
#endif

#ifndef MAX_IMPORT_DIRECTORIES
# define MAX_IMPORT_DIRECTORIES 10
#endif 

#ifndef MAX_DEF_STRINGS
# define MAX_DEF_STRINGS 10
#endif 

static boolean confirm_flags = false;
static boolean dump_ast = false;
static boolean dump_after = false;
#if 0
/* This doesn't seem to be referenced anwywhere */
static boolean ins_files_only = false;
#endif
static char *out_dir = "";
#ifdef UNIX
static boolean do_cpp = true;
#endif 
static boolean do_def_idir = true;
static boolean ansi_c = false;
static boolean pascal_header = false;
static boolean ftn_client = false;
static boolean ftn_server = false;
static boolean mmmv = false;
static boolean not_mmmv = false;
#ifdef UNIX
static char *my_cpp = CPP;
#endif
static boolean nametable_dump = false;
static char *extentions = SUFFIXES;

/* allow two  extra slots for the implicit -idir CD_DIR DEFAULT_IDIR and one for the sentinel */
static char *(import_directories[MAX_IMPORT_DIRECTORIES + 2 + 1]); 
static char *(def_strings[MAX_DEF_STRINGS + 1]);
static boolean space_opt = false;
static boolean version_request = false;
static boolean emit_env = false;
static boolean do_stubs = true ;
#define NUM_BUGS 1
static int do_bug[NUM_BUGS];
static int do_not_do_bug[NUM_BUGS];

boolean support_bug[NUM_BUGS + 1] = {true, false};
boolean no_warnings = false;
boolean graceful_backend_exit = true;

static char cstub_suffix[32],
            sstub_suffix[32],
            cswtch_suffix[32];

#ifdef VMS
#define NIDLTMP "nidl.tmp"
static char *processed_file = "nidl.tmp";
#endif


#define FD(x) (FLAGDEST) &x
#define FDV(x) (FLAGDEST) x

OPTIONS option_table[]={
    "ansi",         ASSERTARG|HIDARG,                FD(ansi_c),
    "bug",          VINTARG(NUM_BUGS),               FD(do_bug[0]),
    "confirm",      ASSERTARG|HIDARG,                FD(confirm_flags),
#ifdef UNIX
    "cpp",          STRARG,                          FD(my_cpp),
#endif
    "def",          VSTRARG(MAX_DEF_STRINGS),        FDV(def_strings),
    "dump_after",   ASSERTARG|HIDARG,                FD(dump_after),
    "dump_ast",     ASSERTARG|HIDARG,                FD(dump_ast),
    "env",          ASSERTARG|HIDARG,                FD(emit_env),
    "exts",         STRARG,                          FD(extentions),
    "f77c",         ASSERTARG,                       FD(ftn_client),
    "f77s",         ASSERTARG,                       FD(ftn_server),
    "idir",         VSTRARG(MAX_IMPORT_DIRECTORIES), FD(import_directories[0]),
    "m",            ASSERTARG,                       FD(mmmv),
    "nametbl",      ASSERTARG|HIDARG,                FD(nametable_dump),
    "no_bug",       VINTARG(NUM_BUGS),               FD(do_not_do_bug[0]),
#ifdef UNIX
    "no_cpp",       DENYARG,                         FD(do_cpp),
#endif
    "no_def_idir",  DENYARG,                         FD(do_def_idir),
    "no_stubs",     DENYARG,                         FD(do_stubs),
    "no_warn",      ASSERTARG,                       FD(no_warnings),
    "out",          STRARG,                          FD(out_dir),
    "pascal",       ASSERTARG,                       FD(pascal_header),
    "s",            ASSERTARG,                       FD(not_mmmv),
    "space_opt",    ASSERTARG,                       FD(space_opt),
    "ungraceful",   DENYARG|HIDARG,                  FD(graceful_backend_exit),
    "version",      ASSERTARG,                       FD(version_request),
    "yydebug",      ASSERTARG|HIDARG,                FD(yydebug),
    0,              0,                               0};

/*--------------------------------------------------------------------*/

/*
 *  Function: Sends the source file through CPP before
 *            giving it to lex.
 *
 *  Inputs:  source - source file name.
 *
 *  Outputs:
 */

#ifdef UNIX
#ifdef __STDC__
static boolean cpp (char *file_name, char **def_strings)
#else
static boolean cpp (file_name, def_strings)
char   *file_name;
char   **def_strings;
#endif
{
    extern  FILE * yyin;
    extern  FILE * popen ();
    extern  FILE * fopen ();
    char    cmd[max_string_len];

    cmd[0] = '\0';
    strcpy (cmd, CPP);

    strcpy (cmd, my_cpp);
    strcat (cmd, " ");
    strcat (cmd, file_name);

    while (*def_strings) {
        strcat (cmd, " -D");
        strcat (cmd, *def_strings++);
        }
    if ((yyin = popen (cmd, "r")) == 0)
        error ("Couldn't invoke /usr/lib/cpp");
}
#endif

/*--------------------------------------------------------------------*/

/*
 *  Function:   Invokes the parser on the specified file.
 *
 *  Inputs:     source_file - pathname of the file to parse
 *
 *              error_proc  - a function to call if the source 
 *                            could not be opened.
 *
 *
 *  Outputs:
 *
 *  Functional value:   a pointer to the interface binding if the file
 *                      was parsed, 0 otherwise.
 *
 */

binding_t * parse (source_file, error_proc, impdirs, def_strings) 
STRTAB_str_t source_file;
void (*error_proc) __PROTOTYPE((char *msg));
char **impdirs;
char **def_strings;
{
    extern  FILE * yyin;
    extern int  yylineno;
    char   *sf;
    struct stat stat_buf ;
    char    full_path_name[max_string_len] ;
    STRTAB_str_t full_pn_id ;

    STRTAB_str_to_string (source_file, &sf);
    set_name_for_errors (sf);

    if  (! lookup(source_file, impdirs, &stat_buf, full_path_name)) {
        (*error_proc)(sf) ;
        return NULL ;
    }

#ifdef UNIX
    if (do_cpp)
        cpp (full_path_name, def_strings);
    else
#endif
        if ((yyin = fopen(full_path_name, "r")) == 0) {
            (*error_proc)(sf) ;
            return NULL ;
            }

        yylineno = 1;
        yyprevious = (int)'\n';
        (void) yyparse ();

#ifdef UNIX
    if (do_cpp)
        pclose(yyin) ;
#endif

        if (yynerrs == 0 && the_interface != NULL) {
            full_pn_id = STRTAB_add_string(full_path_name) ;
            the_interface->binding->interface.source_file = full_pn_id ;
            return the_interface;
        }
        else
                return 0;

}

/*--------------------------------------------------------------------*/

/*
 *  Function: Create a file based on the specified interface name and suffix.
 *
 *  Inputs:   interface_name - the NAMETABLE_id of the interface.
 *            suffix         - a string to be appended with the interface name.
 *          
 *  Outputs:  
 *
 *  Function value: The file pointer of the newly created file.
 *
 */
#ifdef __STDC__
FILE * create_file (STRTAB_str_t source_name, char *suffix)
#else
FILE * create_file (source_name, suffix)
STRTAB_str_t source_name;
char   *suffix;
#endif
{

  char leaf_name[max_string_len],
       ext[max_string_len],
       file_name[max_string_len];
  FILE * file_p;
  char    error_message[max_string_len];
  
  file_name[0] = '\0';

  if (*out_dir)
    {
      strcpy (file_name, out_dir);
#ifndef vms
      strcat (file_name, "/");
#endif
    }

  parse_path_name (source_name, (char *)NULL, leaf_name, ext);
  strcat (file_name, leaf_name);
  strcat (file_name, suffix);
  if ((file_p = fopen (file_name, "w")) == 0)
    {
      sprintf (error_message, "unable to open %s.\n", file_name);
      error (error_message);
    }
  return file_p;
}

/*--------------------------------------------------------------------*/

/*
 *  Function:   issues the "file not found message" when the specified
 *              IDL file does not exist, and then exits.
 *
 *  Inputs:     file_name - pointer to the source file name.
 *
 *  Outputs:    
 *
 *  Functional value:
 *
 */

#ifdef __STDC__
void file_not_found (char *file_name)
#else
void file_not_found (file_name)
char   *file_name;
#endif
{
    char    message[max_string_len];
    sprintf (message, "File: %s not found", file_name);
    error (message);
}

/*--------------------------------------------------------------------*/

/*
 *  Function:   Explains command line arguments.
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Globals:
 *
 *  Functional value:
 *
 */

#ifdef __STDC__
void explain_args (void)
#else
void explain_args ()
#endif
{
    fprintf (stderr, "usage: NIDL fileName [flags]\n\n");
    printflags(option_table);
}
/*--------------------------------------------------------------------*/

#ifdef __STDC__
boolean stubs_required(interface_t *interface_ptr)
#else
boolean stubs_required(interface_ptr)
interface_t * interface_ptr ;
#endif
{   
    binding_t * export_ptr ;

    if (!do_stubs || interface_ptr->local_only)
        return false ;

    for (export_ptr = interface_ptr->exports; export_ptr; export_ptr = export_ptr->next)
        if (export_ptr->kind == routine_k)
            return true ;

    return false ;
}

/*--------------------------------------------------------------------*/
                                    
#ifdef __STDC__
void set_stub_suffixes(char *arg, char *client, char *server, char *swtch)
#else
void set_stub_suffixes(arg, client, server, swtch)
char *arg;
char *client;
char *server;
char *swtch;
#endif
{
  if(!*arg) return;
  while(*arg)
    {
      if(*arg == ',') break;
      *client++ = *arg++;
    }
  *client++ = 0;
  if(!*arg) return;
  arg++;
  while(*arg)
    {
      if(*arg == ',') break;
      *server++ = *arg++;
    }
  *server++ = 0;
  if(!*arg) return;
  arg++;
  while(*arg)
    {
      if(*arg == ',') break;
      *swtch++ = *arg++;
    }
  *swtch++ = 0;
  return;
}
/*--------------------------------------------------------------------*/

void NIDL_init()
{

    NAMETABLE_init() ;
    AST_init() ;
    CHECKER_init() ;
}

/*--------------------------------------------------------------------*/

char *NIDL_parse_args(argc, argv)
    int argc ;
    char **argv ;
{
    FileType out_dir_type;
    char message[256] ;
    int i, j;

    /* 
     *  Check for no args
     */

    if (argc == 0) {
        explain_args ();
        exit (pgm_$error);
    }
    
    import_directories[0] = NULL;

    getflags(argc, argv, option_table) ;

    if (version_request) {
        printf ("nidl version 1.5.1\n");
        exit (pgm_$ok);
        }

    if (ansi_c) 
        printf ("-ansi is no longer needed.\n");

    if (!mmmv && !not_mmmv)
        printf ("Should specify -s or -m; assuming -s.\n");

    if (mmmv && not_mmmv) {
        printf ("Cannot specify both -m and -s.\n");
        exit (pgm_$error);
        }

    if (do_def_idir) {
        for (i = 0; import_directories[i] != NULL; i++)
            ;
        import_directories[i+2] = NULL;
        import_directories[i+1] = DEFAULT_IDIR;
        for ( ; i > 0; i--)
            import_directories[i] = import_directories[i-1];
        import_directories[0] = CD_IDIR;
        flags_incr_count (option_table, "idir", 2);
    }



    for (i = flags_option_count(option_table, "bug") - 1; i >= 0; i--)
        if ((do_bug[i] < 1) || (do_bug[i] > NUM_BUGS)) {
            printf ("-bug %d is not known.\n", do_bug[i]);
            exit (pgm_$error);
            }

    for (i = flags_option_count(option_table, "no_bug") - 1; i >= 0; i--)
        if ((do_not_do_bug[i] < 1) || (do_not_do_bug[i] > NUM_BUGS)) {
            printf ("-no_bug %d is not known.\n", do_bug[i]);
            exit (pgm_$error);
            }

    for (i = flags_option_count(option_table, "bug") - 1; i >= 0; i--)
        for (j = flags_option_count(option_table, "no_bug") - 1; j >= 0; j--)
            if (do_bug[i] == do_not_do_bug[j]) {
                printf ("-bug %d conflicts with -no_bug %d.\n", do_bug[i], do_bug[i]);
                exit (pgm_$error);
                }

    for (i = flags_option_count(option_table, "bug") - 1; i >= 0; i--)
        support_bug[do_bug[i]] = true;

    for (i = flags_option_count(option_table, "no_bug") - 1; i >= 0; i--)
        support_bug[do_not_do_bug[i]] = false;

    if (confirm_flags) {
        printflags (option_table) ;
        exit (pgm_$ok) ;
    } 

        
    set_stub_suffixes(extentions, cstub_suffix, sstub_suffix, cswtch_suffix);
#ifdef MSDOS
    {
#define extlen(s) (strchr(s, '.') == NULL) ? strlen(s) : (strchr(s, '.') - s)
    int tmp;
    max_suffix_len = extlen(cstub_suffix);
    if ((tmp = extlen(sstub_suffix)) > max_suffix_len) max_suffix_len = tmp;
    if ((tmp = extlen(cswtch_suffix)) > max_suffix_len) max_suffix_len = tmp;
    }
#endif

    if(flags_other_count() != 1)
      {
        explain_args ();
        exit (pgm_$error);
      }
    
#ifndef vms
    if (*out_dir) {
      out_dir_type = file_type (out_dir);
      if (out_dir_type != FileDirectory) {
        sprintf (message, "File not a directory: %s\n", out_dir);
        error (message);
      };
    }
#endif
    
#ifdef MSDOS
    {
    STRTAB_str_t src_file_str;
    char *src_file_name;
    char leafname [32];

    src_file_str = STRTAB_add_string(flags_other(0));
    parse_path_name (src_file_str, (char *) NULL, leafname, (char *) NULL);
    if (strlen(leafname) + max_suffix_len > 8) {
        printf ("Length of source file name and stub suffix exceed 8 chars.\n");
        exit (pgm_$error);
        }
    }
#endif
    return (flags_other(0));

}

/*--------------------------------------------------------------------*/

#ifndef __STDC__
static void terminate ()
#else
static void terminate (void)
#endif
{
#ifndef HASPOPEN
    sysdep_cleanup_temp();
#endif
    exit (pgm_$error);
}


binding_t *NIDL_parse(source_file)
    char    *source_file ;
{
    binding_t *interface_binding_p ;
    char *null_p = NULL;
    char version_template [max_string_len];

    interface_binding_p = parse(STRTAB_add_string (source_file),
                  file_not_found,
                  &null_p,
                  def_strings );

    /* terminate if there were any syntax errors in the top level idl file */
    if (interface_binding_p == NULL)
        terminate();

    /* If multiple version command line option was specified, strop the
       version number onto the name bound to the interface. */
    if (mmmv) {
        sprintf (version_template, "%%s_v%d",
                 interface_binding_p->binding->interface.interface_version);
        interface_binding_p->name =
            NAMETABLE_add_derived_name (interface_binding_p->name, version_template);
        }

    if(nametable_dump)
        NAMETABLE_dump_tab();    
    if (dump_ast)
        AST_dump_interface (&interface_binding_p->binding->interface);

    CHECKER_check_interface (interface_binding_p, import_directories, def_strings);    

    /* terminate if there were semantic errors in the top level idl file or
       syntactic or semantic errors in any imported idl files */
    if (error_count)
        terminate();

    if (dump_after) {
        AST_dump_interface (&interface_binding_p->binding->interface);
    }

    return (interface_binding_p) ;
}


/*--------------------------------------------------------------------*/

void NIDL_gen_hdrs(interface_binding_p)
    binding_t *interface_binding_p;
{
    FILE * c_hdr_fd;
    FILE * pas_hdr_fd;

    c_hdr_fd = create_file (interface_binding_p->binding->interface.source_file, ".h");

    if (pascal_header) {
        pas_hdr_fd = create_file (interface_binding_p->binding->interface.source_file, ".ins.pas");
        PSPELL_gen_pascal_ins_file (interface_binding_p, pas_hdr_fd, emit_env, mmmv);
    }
    CSPELL_gen_c_ins_file (interface_binding_p, c_hdr_fd, emit_env, mmmv);
}

/*--------------------------------------------------------------------*/

void NIDL_gen_stubs(interface_binding_p)
    binding_t *interface_binding_p ;
{                                                                     
    FILE *c_cswtch_fd,
         *c_cstub_fd,
         *c_sstub_fd;

    if (! stubs_required(&interface_binding_p->binding->interface))
        return ;

    /*
     * Create the files into which the stubs will be emitted
     */
    
    c_cswtch_fd = create_file (interface_binding_p->binding ->
                   interface.source_file, 
                   cswtch_suffix);
    c_cstub_fd = create_file (interface_binding_p->binding->
                  interface.source_file,
                  cstub_suffix);
    c_sstub_fd = create_file (interface_binding_p->binding->
                  interface.source_file, 
                  sstub_suffix);
    /*
     * Generate the stubs.
     */

    BACKEND_gen_c_stubs (interface_binding_p,
                         c_cswtch_fd,
                         c_cstub_fd,
                         c_sstub_fd,
                         ftn_client,
                         ftn_server,
                         space_opt,
                         mmmv);

}

