/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions used by "common.a".
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Id: common.h,v 1.14 1999-06-28 22:52:27 ghudson Exp $
 */

#include <mit-copyright.h>

#ifndef __olc_common_h
#define __olc_common_h

/* we can't include <olc/olc.h> because that needs STATUS which is defined
 * in this file.  I say we should completely rearrange the header files.
 * Until they glow.
 */
#include "olc/macros.h"
#include "olc/structs.h"

#include <time.h>
#include <stdarg.h>   /* my_snprintf uses a variable argument list */

typedef struct tSTATUS
{
  int status;
  char label[TITLE_SIZE];
} STATUS;

typedef void (*olc_perror_type) (const char*);

/* io.c */
int send_dbinfo (int fd , DBINFO *dbinfo );
int read_dbinfo (int fd , DBINFO *dbinfo );
ERRCODE send_response (int fd , ERRCODE response );
ERRCODE read_response (int fd , ERRCODE *response );
ERRCODE write_int_to_fd (int fd , int response );
ERRCODE read_int_from_fd (int fd , int *response );
ERRCODE read_text_into_file (int fd , char *filename );
ERRCODE read_file_into_text (char *filename, char **bufp );
ERRCODE write_file_to_fd (int fd , char *filename );
ERRCODE write_text_to_fd (int fd , char *buf );
char *read_text_from_fd (int fd );
int sread (int fd , char *buf , int nbytes );
int swrite (int fd , char *buf , int nbytes );

/* perror.c */
char *format_time (char *time_buf , struct tm *time_info );
void time_now (char *time_buf );
extern olc_perror_type olc_perror;
olc_perror_type set_olc_perror(olc_perror_type perr);

/* status.c */
ERRCODE OGetStatusString (int status , char *string );
ERRCODE OGetStatusCode (char *string , int *status );

/* string_utils.c */
void uncase (char *string );
void upcase_string (char *str);
void downcase_string (char *str);
char *cap (char *string );
int isnumber (char *string );
char *get_next_word (char *line , char *buf , int (*func )(int c));
int IsAlpha (int c );
int NotWhiteSpace (int c );
void make_temp_name (char *name );

/* my_vsnprintf.c */
int my_vsnprintf (char *str, size_t n, const char *fmt, va_list ap);
int my_snprintf (char *str, size_t n, const char *fmt, ...);

/* kaboom.c */
void stash_olc_corefile (void);
void dump_current_core_image (void);

/* env.c */
void set_env_var (const char *var, const char *value);

#endif /* __olc_common_h */
