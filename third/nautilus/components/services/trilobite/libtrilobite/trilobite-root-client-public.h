/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* 
 * GtkObject definition for TrilobiteRootClient.
 *
 * Copyright (C) 2000 Eazel, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors: Robey Pointer <robey@eazel.com>
 *
 */

#ifndef _TRILOBITE_ROOT_CLIENT_PUBLIC_H_
#define _TRILOBITE_ROOT_CLIENT_PUBLIC_H_

#include "trilobite-service.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define TRILOBITE_TYPE_ROOT_CLIENT \
	(trilobite_root_client_get_type ())
#define TRILOBITE_ROOT_CLIENT(obj) \
	(GTK_CHECK_CAST ((obj), TRILOBITE_TYPE_ROOT_CLIENT, TrilobiteRootClient))
#define TRILOBITE_ROOT_CLIENT_CLASS(klass) \
	(GTK_CHECK_CLASS_CAST ((klass), TRILOBITE_TYPE_ROOT_CLIENT, TrilobiteRootClientClass))
#define TRILOBITE_IS_ROOT_CLIENT(obj) \
	(GTK_CHECK_TYPE ((obj), TRILOBITE_TYPE_ROOT_CLIENT))
#define TRILOBITE_IS_ROOT_CLIENT_CLASS(klass) \
	(GTK_CHECK_CLASS_TYPE ((obj), TRILOBITE_TYPE_ROOT_CLIENT))

typedef struct _TrilobiteRootClient TrilobiteRootClient;
typedef struct _TrilobiteRootClientClass TrilobiteRootClientClass;
typedef struct _TrilobiteRootClientPrivate TrilobiteRootClientPrivate;
typedef struct _TrilobiteRootClientClassPrivate TrilobiteRootClientClassPrivate;

struct _TrilobiteRootClientClass
{
	BonoboObjectClass parent_class;

	gpointer servant_init;
	gpointer servant_fini;
	gpointer servant_vepv;
};

struct _TrilobiteRootClient
{
	BonoboObject parent;
	TrilobiteRootClientPrivate *private;
};

/* prototypes for autogenerated functions */
GtkType trilobite_root_client_get_type (void);
gboolean trilobite_root_client_construct (TrilobiteRootClient *root_client,
					  Trilobite_PasswordQueryClient corba_trilobite);
TrilobiteRootClient *trilobite_root_client_new (void);
POA_Trilobite_PasswordQueryClient__epv *trilobite_root_client_get_epv (void);
void trilobite_root_client_unref (GtkObject *object);

Trilobite_PasswordQueryClient trilobite_root_client_get_passwordqueryclient (TrilobiteRootClient *root_client);
gboolean trilobite_root_client_attach (TrilobiteRootClient *root_client, BonoboObjectClient *service);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* _TRILOBITE_ROOT_CLIENT_PUBLIC_H_ */
