/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-activation-context.c: A global activation interface
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#ifndef _BONOBO_ACTIVATION_CONTEXT_H_
#define _BONOBO_ACTIVATION_CONTEXT_H_

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

typedef struct _BonoboActivationContextPrivate BonoboActivationContextPrivate;

typedef struct {
	BonoboObject parent;

	BonoboActivationContextPrivate *priv;
} BonoboActivationContext;

BonoboObject *bonobo_activation_context_new (void);

END_GNOME_DECLS

#endif /* _BONOBO_ACTIVATION_CONTEXT_H_ */

