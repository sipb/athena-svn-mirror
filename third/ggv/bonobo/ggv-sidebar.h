/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * ggv-sidebar.h
 *
 * Author:  Jaka Mocnik  <jaka@gnu.org>
 *
 * Copyright (c) 2002, Free Software Foundation
 */

#ifndef _GGV_SIDEBAR_H_
#define _GGV_SIDEBAR_H_

#include <ggv-postscript-view.h>

G_BEGIN_DECLS
 
#define GGV_SIDEBAR_TYPE           (ggv_sidebar_get_type ())
#define GGV_SIDEBAR(o)             (GTK_CHECK_CAST ((o), GGV_SIDEBAR_TYPE, GgvSidebar))
#define GGV_SIDEBAR_CLASS(k)       (GTK_CHECK_CLASS_CAST((k), GGV_SIDEBAR_TYPE, GgvSidebarClass))

#define GGV_IS_SIDEBAR(o)          (GTK_CHECK_TYPE ((o), GGV_SIDEBAR_TYPE))
#define GGV_IS_SIDEBAR_CLASS(k)    (GTK_CHECK_CLASS_TYPE ((k), GGV_SIDEBAR_TYPE))

typedef struct _GgvSidebar              GgvSidebar;
typedef struct _GgvSidebarClass         GgvSidebarClass;
typedef struct _GgvSidebarPrivate       GgvSidebarPrivate;
typedef struct _GgvSidebarClassPrivate  GgvSidebarClassPrivate;

struct _GgvSidebar {
	
BonoboControl control;
	GgvSidebarPrivate *priv;
};

struct _GgvSidebarClass {
	BonoboControlClass parent_class;

	GgvSidebarClassPrivate *priv;
};

GtkType        ggv_sidebar_get_type  (void);
GgvSidebar    *ggv_sidebar_new       (GgvPostScriptView *ps_view);
GgvSidebar    *ggv_sidebar_construct (GgvSidebar *sidebar,
									  GgvPostScriptView *ps_view);
gint          *ggv_sidebar_get_active_list(GgvSidebar *sidebar);
void           ggv_sidebar_create_page_list(GgvSidebar *sidebar);
void           ggv_sidebar_update_coordinates(GgvSidebar *sidebar,
											  gfloat xcoord, gfloat ycoord);
void           ggv_sidebar_page_changed(GgvSidebar *sidebar, gint page);
GtkWidget     *ggv_sidebar_get_checklist(GgvSidebar *sidebar);

G_END_DECLS

#endif /* _GGV_SIDEBAR */
