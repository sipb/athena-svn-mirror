#ifndef DEEPEE_H
#define DEEPEE_H
/*
 * Copyright Milan Technology Inc., 1991, 1992
 */

/* @(#)dp.h	2.1 10/9/92 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#ifdef XPG3
#include <unistd.h>
#endif

#include <signal.h>

#include <fcntl.h>
#include <sgtty.h>
#include <sys/file.h>
#include <sys/socket.h>

#ifdef LPD
#include <sys/un.h>
#endif

#ifdef ATT
#include <sys/in.h>

#else
#include <netinet/in.h>
#endif

#include <netdb.h>

#ifdef INTERACTIVE
#include <net/errno.h>
#endif
#include <errno.h>

extern int errno;
#include <sys/time.h>
#include <syslog.h>

#ifdef SCO
#include <poll.h>
#endif

#ifdef _IBMR2
#include <sys/select.h>
#endif

#ifdef XPG3
#define bcopy(a, b, c) memcpy(b, a, c)
#define rindex(a, b)   strrchr(a, b)
#define bzero(a, c) memset(a, (char)0, c)
#endif

#if !defined( _strcasecmp )
#define strcasecmp(a,b) strcmp(a,b)
#endif

#define SERIAL 2001
#define PARALLEL 2000

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#define MAX_MAGIC_HEADER 100 
#define LINGERVAL 1000
#define ASCII 0
#define POSTSCRIPT 1
#define CAT 2
#define MAX_BUFFER 2000
#define MAXFILELEN 1024
#define MAXSTRNGLEN 256
#define MAXNAMELEN  256
#define MAXLINELEN  256
#define APPEND 0
#define PREPEND 1

#define TRUE  1
#define FALSE 0

#define readEnd 0
#define writeEnd 1

/* hsw == Host SoftWare */
typedef struct hsw_CONFIG {
   char printer_name[MAXNAMELEN];
   char hostname[MAXNAMELEN];
   int ptype; /* used for printer classes */
   struct hsw_CONFIG  *next_printer;
} hsw_PCONFIG;

typedef struct file_obj {
   char file_name[MAXNAMELEN];
   struct file_obj *next;
} file_obj_t, *file_obj_ptr;


/* Structure used for notification purposes */

typedef struct notification {
   int mail;                  /* Should I notify thru mail ? */
   int file;                  /* Should I notify thru file ? */
   int syslog;                /* Should I notify thru syslog file ? */
   int program;               /* notify thru execing a prog */
   char user[MAXNAMELEN];     /* User name to send mail to   */
   char filename[MAXNAMELEN]; /* Name of such a file */
   char prog_name[MAXNAMELEN];/* name of prog to exec */
} s_notification;

typedef struct Adobe {
   char *prog;
   char *pname;
   char *name;
   char *host;
   char *accountingfile;
   int banner_first, banner_last, verbose_log;
} s_Adobe;

/* Struct to hold options and other "global" values */
typedef struct options {
   int use_control_d;
   int mapflg;
   int ff_flag;
   int dobanner;
   int check_postscript;
   int dataport;
   int send_startfile;
   int send_endfile;
   int real_filter;
   int closewait  ;
   int use_printer_classes;
   char *current_dir;
   char *start_string;
   char *end_string;
   char *asciifilter;
   char asciiname[MAXNAMELEN];
   char start_file[MAXFILELEN];
   char end_file[MAXFILELEN];
   char *printer_str;

   file_obj_ptr file_list;

   s_Adobe adobe;
   s_notification notify_type;
   hsw_PCONFIG *prt_list;    /* List of serial and parallel prts*/
   int acctg;
} s_options;

/* Function prototypes */

#if defined(__cplusplus)
#define EXTERNAL_FUNCTION( funName, params ) extern "C" { funName params; }
#else
#if defined(ANSI) || defined(__STDC__)
#define EXTERNAL_FUNCTION( funName, params ) funName params
#else
#define EXTERNAL_FUNCTION( funName, params ) funName()
#endif
#endif

EXTERNAL_FUNCTION(void systemNoWait,
		  (char* cmd));
EXTERNAL_FUNCTION(void sig_child,
		  ());
EXTERNAL_FUNCTION(int parseCommandLineArgs,
		  (int argc, char **argv));
EXTERNAL_FUNCTION(void expand_char,
		  ());
EXTERNAL_FUNCTION(void get_printername, 
		  (char *printer));
EXTERNAL_FUNCTION(void error_notify, 
		  (int err_no, char* message));
EXTERNAL_FUNCTION(void error_protect, 
		  (int err_no));
EXTERNAL_FUNCTION(hsw_PCONFIG* form_printer_list, 
		  (char* string, hsw_PCONFIG *ptr_ptr, int ptype, int where));
EXTERNAL_FUNCTION(char* parse_string, 
		  (char* str));
EXTERNAL_FUNCTION(void add_file, 
		  (file_obj_ptr *flist, char *file_to_add));
EXTERNAL_FUNCTION(int sendfile, 
		  (int sock, char *file_to_send, int output_dest, 
		   int check_postscript, char *printer));
EXTERNAL_FUNCTION(void check_input, 
		  (int sock, int output_dest, int timeout));
EXTERNAL_FUNCTION(void send_control_d,
		  (int sock));
EXTERNAL_FUNCTION(void check_alarm, 
		  ());
EXTERNAL_FUNCTION(char *expand_buffer,
		  (char *, int, int *));
EXTERNAL_FUNCTION(void send_banner,
		  ());
EXTERNAL_FUNCTION(void update_status_file,(char *message,int status_num,int style));
EXTERNAL_FUNCTION(void write_buffer,(int sock, char *buffer, int length,char *printer_name,int output_dest));

EXTERNAL_FUNCTION(void check_write,(int sock));
EXTERNAL_FUNCTION(void do_acctg,(char *file));

#endif /* DEEPEE_H */

#define APPEND 0
#define CREATE 1

extern s_options g_opt;
