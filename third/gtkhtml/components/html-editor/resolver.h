/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef RESOLVER_H_
#define RESOLVER_H_

#include <libgnome/gnome-defs.h>
#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#define EDITOR_RESOLVER_TYPE        (htmleditor_resolver_get_type ())
#define EDITOR_RESOLVER(o)          (GTK_CHECK_CAST ((o), EDITOR_RESOLVER_TYPE, HTMLEditorResolver))
#define EDITOR_RESOLVER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), EDITOR_RESOLVER_TYPE, HTMLResolverClass))
#define IS_EDITOR_RESOLVER(o)       (GTK_CHECK_TYPE ((o), EDITOR_RESOLVER_TYPE))
#define IS_EDITOR_RESOLVER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), EDITOR_RESOLVER_TYPE))

typedef struct {
	BonoboObject parent;

	char *instance_data;
} HTMLEditorResolver;

typedef struct {
	BonoboObjectClass parent_class;
} HTMLEditorResolverClass;

GtkType                             htmleditor_resolver_get_type   (void);
HTMLEditorResolver                 *htmleditor_resolver_construct  (HTMLEditorResolver        *resolver,
								    GNOME_GtkHTML_Editor_Resolver  corba_resolver);
HTMLEditorResolver                 *htmleditor_resolver_new        (void);
POA_GNOME_GtkHTML_Editor_Resolver__epv *htmleditor_resolver_get_epv    (void);

END_GNOME_DECLS

#endif /* RESOLVER_H_ */








