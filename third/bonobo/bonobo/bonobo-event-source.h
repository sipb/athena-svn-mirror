/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-event-source.h: Generic event emitter.
 *
 * Author:
 *	Alex Graveley (alex@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#ifndef _BONOBO_EVENT_SOURCE_H_
#define _BONOBO_EVENT_SOURCE_H_

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#define BONOBO_EVENT_SOURCE_TYPE        (bonobo_event_source_get_type ())
#define BONOBO_EVENT_SOURCE(o)          (GTK_CHECK_CAST ((o), BONOBO_EVENT_SOURCE_TYPE, BonoboEventSource))
#define BONOBO_EVENT_SOURCE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_EVENT_SOURCE_TYPE, BonoboEventSourceClass))
#define BONOBO_IS_EVENT_SOURCE(o)       (GTK_CHECK_TYPE ((o), BONOBO_EVENT_SOURCE_TYPE))
#define BONOBO_IS_EVENT_SOURCE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_EVENT_SOURCE_TYPE))

typedef struct _BonoboEventSourcePrivate BonoboEventSourcePrivate;

typedef struct {
	BonoboObject              parent;
	BonoboEventSourcePrivate *priv;
} BonoboEventSource;

typedef struct {
	BonoboObjectClass parent_class;
} BonoboEventSourceClass;

GtkType            bonobo_event_source_get_type         (void);
BonoboEventSource *bonobo_event_source_new              (void);
BonoboEventSource *bonobo_event_source_construct        (BonoboEventSource *event_source, 
							 Bonobo_EventSource corba_event_source);
void               bonobo_event_source_notify_listeners (BonoboEventSource *event_source,
							 const char        *event_name,
							 const CORBA_any   *value,
							 CORBA_Environment *opt_ev);

void          bonobo_event_source_notify_listeners_full (BonoboEventSource *event_source,
							 const char        *path,
							 const char        *type,
							 const char        *subtype,
							 const CORBA_any   *value,                          
							 CORBA_Environment *opt_ev);

POA_Bonobo_EventSource__epv *bonobo_event_source_get_epv   (void);
Bonobo_EventSource bonobo_event_source_corba_object_create (BonoboObject *object);
/* You don't want this routine */
void               bonobo_event_source_ignore_listeners    (BonoboEventSource *event_source);

END_GNOME_DECLS

#endif /* _BONOBO_EVENT_SOURCE_H_ */

