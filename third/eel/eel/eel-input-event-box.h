/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef __EEL_INPUT_EVENT_BOX_H__
#define __EEL_INPUT_EVENT_BOX_H__


#include <gdk/gdk.h>
#include <gtk/gtkbin.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define EEL_TYPE_INPUT_EVENT_BOX           (eel_input_event_box_get_type ())
#define EEL_INPUT_EVENT_BOX(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), EEL_TYPE_INPUT_EVENT_BOX, EelInputEventBox))
#define EEL_INPUT_EVENT_BOX_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass), EEL_TYPE_INPUT_EVENT_BOX, EelInputEventBoxClass))
#define EEL_IS_INPUT_EVENT_BOX(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EEL_TYPE_INPUT_EVENT_BOX))
#define EEL_IS_INPUT_EVENT_BOX_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE ((klass), EEL_TYPE_INPUT_EVENT_BOX))
#define EEL_INPUT_EVENT_BOX_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), EEL_TYPE_INPUT_EVENT_BOX, EelInputEventBoxClass))


typedef struct _EelInputEventBox	  EelInputEventBox;
typedef struct _EelInputEventBoxClass  EelInputEventBoxClass;

struct _EelInputEventBox
{
  GtkBin bin;
  GdkWindow *input_window;
};

struct _EelInputEventBoxClass
{
  GtkBinClass parent_class;
};

GType	       eel_input_event_box_get_type  (void) G_GNUC_CONST;
GtkWidget*     eel_input_event_box_new	     (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __EEL_INPUT_EVENT_BOX_H__ */
