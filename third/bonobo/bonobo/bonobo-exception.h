/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-exception.c: a generic exception -> user string converter.
 *
 * Authors:
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */

#ifndef _BONOBO_EXCEPTION_H_
#define _BONOBO_EXCEPTION_H_

#include <glib.h>
#include <bonobo/Bonobo.h>

#define BONOBO_EX(ev)         ((ev) && (ev)->_major != CORBA_NO_EXCEPTION)

#define BONOBO_USER_EX(ev,id) ((ev) && (ev)->major == CORBA_USER_EXCEPTION &&	\
			       (ev)->_repo_id != NULL && !strcmp ((ev)->_repo_id, id))

#define BONOBO_RET_EX(ev)		\
	G_STMT_START			\
		if (BONOBO_EX (ev))	\
			return;		\
	G_STMT_END

#define BONOBO_RET_VAL_EX(ev,v)		\
	G_STMT_START			\
		if (BONOBO_EX (ev))	\
			return (v);	\
	G_STMT_END

typedef char *(*BonoboExceptionFn)     (CORBA_Environment *ev, gpointer user_data);

char *bonobo_exception_get_text        (CORBA_Environment *ev);

void  bonobo_exception_add_handler_str (const char *repo_id,
					const char *str);

void  bonobo_exception_add_handler_fn  (const char *repo_id,
					BonoboExceptionFn fn,
					gpointer          user_data,
					GDestroyNotify    destroy_fn);

#endif /* _BONOBO_EXCEPTION_H_ */
