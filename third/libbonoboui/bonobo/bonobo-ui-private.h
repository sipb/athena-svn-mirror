/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo UI internal prototypes / helpers
 *
 * Author:
 *   Michael Meeks (michael@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef _BONOBO_UI_PRIVATE_H_
#define _BONOBO_UI_PRIVATE_H_

#include <gtk/gtkmisc.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtkimage.h>
#include <bonobo/bonobo-ui-node.h>
#include <bonobo/bonobo-ui-engine.h>
#include <bonobo/bonobo-ui-node-private.h>
#include <bonobo/bonobo-ui-toolbar-control-item.h>

G_BEGIN_DECLS

/* To dump lots of sequence information */
#define noDEBUG_UI

/* To debug render issues in plug/socket/control */
#define noDEBUG_CONTROL

void       bonobo_socket_add_id           (BonoboSocket   *socket,
					   GdkNativeWindow xid);
int        bonobo_ui_preferences_shutdown (void);
void       bonobo_ui_image_set_pixbuf     (GtkImage     *image,
					   GdkPixbuf    *pixbuf);
void       bonobo_ui_util_xml_set_image   (GtkImage     *image,
					   BonoboUINode *node,
					   BonoboUINode *cmd_node,
					   GtkIconSize   icon_size);
void       bonobo_ui_engine_dispose       (BonoboUIEngine *engine);
GtkWidget *bonobo_ui_toolbar_button_item_get_image (BonoboUIToolbarButtonItem *item);

GtkWidget *bonobo_ui_internal_toolbar_new (void);

GList *bonobo_ui_internal_toolbar_get_children (GtkWidget *toolbar);

#ifndef   DEBUG_UI

static inline void dprintf (const char *format, ...) { };

#else  /* DEBUG_UI */

#include <stdio.h>

#define dprintf(format...) fprintf(stderr, format)

#endif /* DEBUG_UI */

G_END_DECLS

#endif /* _BONOBO_UI_PRIVATE_H_ */

