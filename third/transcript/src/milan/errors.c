/* 
 *  Copyright Milan Technology Inc. 1991, 1992
 */

/* @(#)errors.c	2.0 10/9/92 */

#include "dp.h"
#define G_ERRORS
#include "errors.h"
extern s_options g_opt;
extern char *g_filter_name; /* The name by which this program was invoked */
extern FILE *g_errorLog;      /* The fd for the error file */


/* Error handling routines.
 *
 * 7/04/92 Stripped from fpfilter.c
 */

#ifdef ANSI
void Send_to_file(char* string)
#else
void Send_to_file(string)
char *string;
#endif
{
   if (g_opt.notify_type.file) 
      fprintf(g_errorLog, "%s\n", string);
}

#ifdef ANSI
void Send_to_syslog(char* string, int type)
#else
void Send_to_syslog(string, type)
char *string;
int type;
#endif
{
    if (g_opt.notify_type.syslog)
       syslog(type, "%s", string);
}

#ifdef ANSI
void Send_to_tty(char *string)
#else
void Send_to_tty(string)
char *string;
#endif
{
      fprintf(stderr,"%s \n", string);
      fflush(stderr);
}

#ifdef ANSI
void Send_to_mail(char* string)
#else
void Send_to_mail(string)
char *string;
#endif
{
   char cmd[MAXSTRNGLEN];
   static int inited = 0, fd_write;

   if (g_opt.notify_type.mail) {
      if (!inited) {
	 sprintf(cmd, "mail %s", g_opt.notify_type.user);
	 fd_write = initPipes(cmd);
	 inited = 1;
      }
      write(fd_write, string, strlen(string));
   }
}

#ifdef ANSI
void Send_to_program(char* string)
#else
void Send_to_program(string)
char *string;
#endif
{
   static int inited = 0, fd_write;

  if (g_opt.notify_type.program) {
      if (!inited) {
	 fd_write = initPipes(g_opt.notify_type.prog_name);
	 inited = 1;
      }
      write(fd_write, string, strlen(string));
   }
}

/* error_notify sends the error message to the appropriate place.
 * Then it exits unless err_no is 12 or 14 (ERR_GENERIC or ERR_ASCII).
 * ERR_GENERIC means print a generic message (passed down), ERR_ASCII
 * indicates that the user has not specified ascii -> ps conversion.
 */

#ifdef ANSI
void error_notify(int err_no, char* generic_message)
#else
void error_notify(err_no, generic_message)
int err_no;
char *generic_message;
#endif
{
   if (err_no == ERR_GENERIC) {
      Send_to_file(generic_message);
      Send_to_syslog(generic_message, LOG_INFO);
      Send_to_mail(generic_message);
      Send_to_tty(generic_message);
      Send_to_program(generic_message);
   }
   else {
      Send_to_file(g_errors[err_no]);
      Send_to_syslog(g_errors[err_no], LOG_ERR);
      Send_to_mail(g_errors[err_no]);
      Send_to_tty(g_errors[err_no]);
      Send_to_program(g_errors[err_no]);

      if (err_no == ERR_READING) {
	 char usage[MAXSTRNGLEN];
   
	 sprintf(usage,
		 "usage: %s -P <hostname> [-s] [-m] [-SS] [-ES] [-A] [-SF] [-EF] [-p] [-b] files \n",
		 g_filter_name);
   	 Send_to_file(usage);
	 Send_to_syslog(usage, LOG_ERR);
	 Send_to_mail(usage);
	 Send_to_tty(usage);
	 Send_to_program(usage);
      }
   }

   if ((err_no == ERR_GENERIC ) || (err_no == ERR_ASCII)) return;

   exit(12); /* What does 12 mean? */
}

/* Trys to report an error, but turns off the mail and program options */

#ifdef ANSI
void error_protect(int err_no)
#else
void error_protect(err_no)
int err_no;
#endif
{
   g_opt.notify_type.program = 0;
   g_opt.notify_type.mail = 0;
   error_notify(errno, (char*)0);
}
