/*
 * This file is part of the OLC On-Line Consulting System.
 * It declares functions that are used in reading configuration files.
 *
 *      bert Dvornik
 *      MIT Athena Software Service
 *
 * Copyright (C) 1996 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: configure.h,v 1.2 1999-01-22 23:13:37 ghudson Exp $
 */

#include <mit-copyright.h>
#include <stdio.h>

#ifndef __cfgfile_configure_h
#define __cfgfile_configure_h

/* Declaration of the type for configuration parsing functions */
typedef char* (*config_set)(char*, void*, long, int);

/* The following structure contains instructions for parsing of a
 * single keyword in the configuration file.
 */
typedef struct config_keyword_data {
  char      *keyword;		/* string containing the config. keyword */
  config_set parse;		/* function to parse the argument(s) if any */
  void      *placement;		/* ptr to where the data should be stored */
  long       value;		/* additional data used by some functions */
} config_keyword;
  
/* prototypes */
FILE *cfg_fopen_in_path(const char *name, const char *ext,
			const char *path);

void cfg_read_config(const char* name, FILE* cf, config_keyword *keywords);

/* parsing functions (of type config_set) */
char *config_set_quoted_string (char*, void*, long, int);
char *config_set_string_list (char*, void*, long, int);
char *config_set_char_constant (char*, void*, long, int);

#endif /* __cfgfile_configure_h */
