/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions used by "common.a".
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 *
 *	$Id: common.h,v 1.13 1999-03-06 16:48:24 ghudson Exp $
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

#ifdef __STDC__
# define        P(s) s
#else
# define P(s) ()
#endif

typedef void (*olc_perror_type) P((const char*));

/* io.c */
int send_dbinfo P((int fd , DBINFO *dbinfo ));
int read_dbinfo P((int fd , DBINFO *dbinfo ));
ERRCODE send_response P((int fd , RESPONSE response ));
ERRCODE read_response P((int fd , RESPONSE *response ));
ERRCODE write_int_to_fd P((int fd , int response ));
ERRCODE read_int_from_fd P((int fd , int *response ));
ERRCODE read_text_into_file P((int fd , char *filename ));
ERRCODE read_file_into_text P((char *filename, char **bufp ));
ERRCODE write_file_to_fd P((int fd , char *filename ));
ERRCODE write_text_to_fd P((int fd , char *buf ));
char *read_text_from_fd P((int fd ));
int sread P((int fd , char *buf , int nbytes ));
int swrite P((int fd , char *buf , int nbytes ));

/* perror.c */
char *format_time P((char *time_buf , struct tm *time_info ));
void time_now P((char *time_buf ));
extern olc_perror_type olc_perror;
olc_perror_type set_olc_perror(olc_perror_type perr);

/* status.c */
ERRCODE OGetStatusString P((int status , char *string ));
ERRCODE OGetStatusCode P((char *string , int *status ));

/* string_utils.c */
void uncase P((char *string ));
void upcase_string P((char *str));
void downcase_string P((char *str));
char *cap P((char *string ));
int isnumber P((char *string ));
char *get_next_word P((char *line , char *buf , int (*func )(int c)));
int IsAlpha P((int c ));
int NotWhiteSpace P((int c ));
void make_temp_name P((char *name ));

/* my_vsnprintf.c */
int my_vsnprintf P((char *str, size_t n, const char *fmt, va_list ap));
int my_snprintf P((char *str, size_t n, const char *fmt, ...));

/* kaboom.c */
void stash_olc_corefile P((void));
void dump_current_core_image P((void));

/* env.c */
void set_env_var P((const char *var, const char *value));

#undef P

#endif /* __olc_common_h */
