/* gok-control.h
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#ifndef __GOKCONTROL_H__
#define __GOKCONTROL_H__

#include <gnome.h>

/* types of controls */
/* if you modify this array, update ArrayControlTypeNames in gok-control.c */
typedef enum {
CONTROL_TYPE_LABEL,
CONTROL_TYPE_HBOX,
CONTROL_TYPE_VBOX,
CONTROL_TYPE_COMBOBOX,
CONTROL_TYPE_SEPERATOR,
CONTROL_TYPE_FRAME,
CONTROL_TYPE_BUTTON,
CONTROL_TYPE_CHECKBUTTON,
CONTROL_TYPE_RADIOBUTTON,
CONTROL_TYPE_SPINBUTTON,
MAX_CONTROL_TYPES /* keep this as the last entry in the enum */
} ControlTypes;

typedef enum {
CONTROL_FILLWITH_ACTIONS,
CONTROL_FILLWITH_FEEDBACKS,
CONTROL_FILLWITH_SOUNDS,
CONTROL_FILLWITH_OPTIONS
} ControlFillwiths;

/* handlers for controls */
/* if you modify this array, update ArrayControlHandlerNames in gok-control.c */
typedef enum {
CONTROL_HANDLER_BROWSESOUND,
CONTROL_HANDLER_ADVANCED,
MAX_CONTROL_HANDLERS /* keep this as the last entry in the enum */
} ControlHandlers;
	
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct GokControl {
	gchar* Name;
	gint Type;
	gchar* String;
	gchar* StringBackup;
	gint Size;
	gint Border;
	gint Spacing;
	gint Fillwith;
	gint Qualifier;
	gboolean bGroupStart;
	gint Value;
	gint ValueBackup;
	gint Min;
	gint Max;
	gint StepIncrement;
	gint PageIncrement;
	gint PageSize;
	gint Handler;
	GtkWidget* pWidget;
	gchar* NameAssociatedControl;
	gboolean bAssociatedStateActive;
	struct GokControl* pControlChild;	
	struct GokControl* pControlNext;	
} GokControl;

typedef struct GokControlCallback {
	GtkWidget* pWidget;
	gint HandlerType;
	struct GokControlCallback* pControlCallbackNext;
} GokControlCallback;

GokControl* gok_control_new (void);
void gok_control_delete_all (GokControl* pControl);
gint gok_control_get_control_type (gchar* NameControlType);
gint gok_control_get_handler_type (gchar* NameControlType);
void gok_control_button_handler (GtkButton* button, gpointer user_data);
void gok_control_add_handler (GtkWidget* pWidget, gint HandlerType);
void gok_control_button_callback_open (void);
void gok_control_button_callback_close (void);
GokControl* gok_control_find_by_widget (GtkWidget* pWidget, GokControl* pControl);
GokControl* gok_control_find_by_name (gchar* Name, GokControl* pControl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKCONTROL_H__ */
