/*
 * This file is part of the OLC On-Line Consulting System.
 * It declares functions to customize the client for different services.
 *
 *      bert Dvornik
 *      MIT Athena Software Service
 *
 * Copyright (C) 1996 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: incarnate.h,v 1.2 1999-01-22 23:13:39 ghudson Exp $
 */

#include <mit-copyright.h>

#ifndef __olc_incarnate_h
#define __olc_incarnate_h

#include <olc/olc.h>

ERRCODE incarnate (const char *client_hint, const char *cfg_path);

ERRCODE incarnate_hardcoded (char *client, char *service, char *server);

char   *client_name(void);
char   *client_service_name (void);
char   *client_hardcoded_server (void);
char   *client_nl_service_name (void);

char   *client_default_prompt (void);
char   *client_default_consultant_title(void);

char    client_is_consulting_client (void);
#define client_is_user_client()   (! client_is_consulting_client())
char    client_has_hours (void);
char    client_has_answers (void);
char    client_has_help (void);

char   *client_help_directory (void);
char   *client_help_primary_file (void);
char   *client_help_ext (void);

char   *client_SA_directory (void);
char   *client_SA_browser_program (void);
char  **client_SA_attach_commands (void);
char   *client_SA_magic_file (void);

#endif /* __olc_incarnate_h */
