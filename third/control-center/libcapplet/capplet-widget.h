/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* capplet-widget.h
 * Copyright (C) 2000 Helix Code, Inc.
 * Copyright (C) 1998 Red Hat Software, Inc.
 *
 * Written by Bradford Hovinen (hovinen@helixcode.com),
 *            Jonathon Blandford (jrb@redhat.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef __CAPPLET_WIDGET_H__
#define __CAPPLET_WIDGET_H__

#include <gtk/gtk.h>
#include <gnome.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CAPPLET_WIDGET(obj)          GTK_CHECK_CAST (obj, capplet_widget_get_type (), CappletWidget)
#define CAPPLET_WIDGET_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, capplet_widget_get_type (), CappletWidgetClass)
#define IS_CAPPLET_WIDGET(obj)       GTK_CHECK_TYPE (obj, capplet_widget_get_type ())

typedef struct _CappletWidget		        CappletWidget;
typedef struct _CappletWidgetClass		CappletWidgetClass;

struct _CappletWidget
{
	GtkFrame		frame;
        gboolean 		changed;

        GnomeDialog            *dialog;
        GtkMenuItem            *undo_item;

        gint                    capid;
};

struct _CappletWidgetClass
{
	GtkFrameClass 		parent_class;
        gchar                   buffer[sizeof (GtkPlugClass) - 
                                      sizeof (GtkFrameClass)];

        void (* try) 		(CappletWidget *capplet);
        void (* revert) 	(CappletWidget *capplet);
        void (* ok) 		(CappletWidget *capplet);
        void (* cancel)		(CappletWidget *capplet);
        void (* help) 		(CappletWidget *capplet);
        void (* new_multi_capplet) 	(CappletWidget *capplet);
        void (* page_hidden)	(CappletWidget *capplet);
        void (* page_shown)	(CappletWidget *capplet);
};

guint           capplet_widget_get_type       	(void);

GtkWidget*      capplet_widget_new            	(void);
void            capplet_widget_destroy          (GtkObject *object);
GtkWidget*      capplet_widget_multi_new       	(gint capid);

void		capplet_gtk_main  		(void);

/* returns 0 upon successful initialization.
   returns 1 if --init-session-settings was passed on the cmdline
   returns 2 if --ignore was passed on the cmdline
   returns 3 if --get was passed on the cmdline
   returns 4 if --set was passed on the cmdline
   returns -1 upon error
*/

gint            gnome_capplet_init              (const char *app_id, 
                                                 const char *app_version,
                                                 int argc, 
                                                 char **argv, 
                                                 struct poptOption *options,
                                                 unsigned int flags, 
                                                 poptContext *return_ctx);

void 		capplet_widget_state_changed 	(CappletWidget *cap, 
                                                 gboolean undoable);

gint            capplet_widget_class_get_capid  (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CAPPLET_WIDGET_H__ */
