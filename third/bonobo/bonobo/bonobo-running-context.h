/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-running-context.c: An interface to track running objects
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#ifndef _BONOBO_RUNNING_CONTEXT_H_
#define _BONOBO_RUNNING_CONTEXT_H_

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

typedef struct _BonoboRunningContextPrivate BonoboRunningContextPrivate;

typedef struct {
	BonoboObject parent;

	BonoboRunningContextPrivate *priv;
} BonoboRunningContext;

typedef struct {
	BonoboObjectClass parent;

	void (*last_unref) (void);
} BonoboRunningContextClass;

BonoboObject *bonobo_running_context_new (void);

/*
 *   This interface is private, and purely for speed
 * of impl. of the context.
 */
void          bonobo_running_context_add_object    (CORBA_Object object);
void          bonobo_running_context_remove_object (CORBA_Object object);
void          bonobo_running_context_ignore_object (CORBA_Object object);

END_GNOME_DECLS

#endif /* _BONOBO_RUNNING_CONTEXT_H_ */

