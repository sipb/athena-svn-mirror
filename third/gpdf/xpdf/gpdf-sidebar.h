/*
 *  Copyright (C) 2000, 2001, 2002 Marco Pesenti Gritti
 *                2003 Remi Cohen-Scali
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: gpdf-sidebar.h,v 1.1.1.1 2004-10-06 18:43:43 ghudson Exp $
 */

#ifndef GPDF_SIDEBAR_H
#define GPDF_SIDEBAR_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkvbox.h>
	
G_BEGIN_DECLS

typedef struct GPdfSidebarClass GPdfSidebarClass;

#define GPDF_SIDEBAR_TYPE             (gpdf_sidebar_get_type ())
#define GPDF_SIDEBAR(obj)             (GTK_CHECK_CAST ((obj), GPDF_SIDEBAR_TYPE, GPdfSidebar))
#define GPDF_SIDEBAR_CLASS(klass)     (GTK_CHECK_CLASS_CAST ((klass), GPDF_SIDEBAR_TYPE, GPdfSidebarClass))
#define GPDF_IS_SIDEBAR(obj)          (GTK_CHECK_TYPE ((obj), GPDF_SIDEBAR_TYPE))
#define GPDF_IS_SIDEBAR_CLASS(klass)  (GTK_CHECK_CLASS_TYPE ((klass), GPDF_SIDEBAR))
#define GPDF_SIDEBAR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GPDF_SIDEBAR_TYPE, GPdfSidebarClass))

typedef struct GPdfSidebar GPdfSidebar;
typedef struct GPdfSidebarPrivate GPdfSidebarPrivate;

struct GPdfSidebar 
{
        GtkVBox parent;
        GPdfSidebarPrivate *priv;
};

struct GPdfSidebarClass
{
        GtkVBoxClass parent_class;

	void (* close_requested) (GPdfSidebar *sidebar);
	
	void (* page_changed) (GPdfSidebar *sidebar,
			       const char *page_id);
	
	void (* remove_requested) (GPdfSidebar *sidebar,
			           const char *page_id);
};

GType          gpdf_sidebar_get_type            (void);

GtkWidget     *gpdf_sidebar_new	                (void);

void           gpdf_sidebar_add_page	        (GPdfSidebar *sidebar,
					         const char *title,
					         const char *page_id,
                                                 GtkWidget *tools_menu, 
					         gboolean can_remove);

gboolean       gpdf_sidebar_remove_page	        (GPdfSidebar *sidebar,
					         const char *page_id);

gboolean       gpdf_sidebar_select_page         (GPdfSidebar *sidebar,
			    		         const char *page_id);

void           gpdf_sidebar_set_content         (GPdfSidebar *sidebar,
					         GObject *content);

const gchar   *gpdf_sidebar_get_current_page_id (GPdfSidebar *sidebar);

/* Set tools menu for one page and returns old menu (if exists) */
GtkWidget*     gpdf_sidebar_set_page_tools_menu	(GPdfSidebar *sidebar,
                                                 const char *page_id,
                                                 GtkWidget *tools_menu); 

G_END_DECLS

#endif

