/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-item-handler.h: a generic ItemContainer handler for monikers.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 * Copyright 1999, 2000 Miguel de Icaza
 */

#ifndef _BONOBO_ITEM_HANDLER_H_
#define _BONOBO_ITEM_HANDLER_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_ITEM_HANDLER_TYPE        (bonobo_item_handler_get_type ())
#define BONOBO_ITEM_HANDLER(o)          (GTK_CHECK_CAST ((o), BONOBO_ITEM_HANDLER_TYPE, BonoboItemHandler))
#define BONOBO_ITEM_HANDLER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_ITEM_HANDLER_TYPE, BonoboItemHandlerClass))
#define BONOBO_IS_ITEM_HANDLER(o)       (GTK_CHECK_TYPE ((o), BONOBO_ITEM_HANDLER_TYPE))
#define BONOBO_IS_ITEM_HANDLER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_ITEM_HANDLER_TYPE))

typedef struct _BonoboItemHandlerPrivate BonoboItemHandlerPrivate;
typedef struct _BonoboItemHandler        BonoboItemHandler;

typedef Bonobo_ItemContainer_ObjectList *(*BonoboItemHandlerEnumObjectsFn)
	(BonoboItemHandler *h, gpointer data, CORBA_Environment *);

typedef Bonobo_Unknown (*BonoboItemHandlerGetObjectFn)
	(BonoboItemHandler *h, const char *item_name, gboolean only_if_exists,
	 gpointer data, CORBA_Environment *ev);

struct _BonoboItemHandler {
	BonoboObject base;

	BonoboItemHandlerEnumObjectsFn enum_objects;
	BonoboItemHandlerGetObjectFn   get_object;
	BonoboItemHandlerPrivate *priv;
	gpointer user_data;
};

typedef struct {
	BonoboObjectClass parent_class;
} BonoboItemHandlerClass;

GtkType              bonobo_item_handler_get_type    (void);
BonoboItemHandler   *bonobo_item_handler_new         (BonoboItemHandlerEnumObjectsFn enum_objects,
						      BonoboItemHandlerGetObjectFn get_object,
						      gpointer user_data);

BonoboItemHandler   *bonobo_item_handler_construct   (BonoboItemHandler *handler,
						      Bonobo_ItemContainer corba_handler,
						      BonoboItemHandlerEnumObjectsFn enum_objects,
						      BonoboItemHandlerGetObjectFn get_object,
						      gpointer user_data);

POA_Bonobo_ItemContainer__epv *bonobo_item_handler_get_epv (void);

END_GNOME_DECLS

#endif

