/* main.h
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

#ifndef __MAIN_H__
#define __MAIN_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cspi/spi.h>
#include <libspi/Accessibility.h>
#include <libspi/accessible.h>
#include <libspi/application.h>
#include <login-helper/Accessibility_LoginHelper.h>
#include <login-helper/login-helper.h>
#include "gok-keyboard.h"

typedef struct _GokApplication GokApplication;

/* GOK Bonobo type info and #defines */
#define GOK_TYPE_APPLICATION        (gok_application_get_type ())
#define GOK_APPLICATION(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), GOK_TYPE_APPLICATION, GokApplication))
#define GOK_APPLICATION_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST ((k), GOK_TYPE_APPLICATION, GokApplicationClass))

struct _GokApplication {
  LoginHelper     parent;
  gboolean        safe;
};

typedef struct _GokApplicationClass {
  LoginHelperClass     parent_class;
} GokApplicationClass;

/* public */
GType      gok_application_get_type (void) G_GNUC_CONST;

GtkWidget* gok_main_create_window (gboolean is_dock);
void gok_main_set_cursor (GdkCursor *cursor);
gint gok_main_open(gint argc, gchar *argv[]);
void gok_main_raise_window (void);
const gchar *gok_main_get_custom_compose_kbd_name (void);
GokKeyboard *gok_main_keyboard_find_byname (const gchar* NameKeyboard);
gboolean gok_main_display_scan (GokKeyboard* pKeyboard, gchar* nameKeyboard, KeyboardTypes typeKeyboard, KeyboardLayouts layout, KeyboardShape shape);
gboolean gok_main_display_scan_previous (void);
gboolean gok_main_display_scan_previous_premade (void);
gboolean gok_main_display_scan_reset (void);
gboolean gok_main_ds (GokKeyboard* pKeyboard);
GokKeyboard* gok_main_get_first_keyboard (void);
void gok_main_set_first_keyboard (GokKeyboard* pKeyboard);
GokKeyboard* gok_main_get_current_keyboard (void);
GtkWidget* gok_main_get_main_window (void);
Accessible* gok_main_get_foreground_accessible (void);
void gok_main_resize_window (GtkWidget* pWindow, GokKeyboard *pKeyboard, gint Width, gint Height);
void gok_main_store_window_center (void);
Accessible* gok_main_get_foreground_window_accessible (void);
void gok_main_read_keyboards (void);
GokKeyboard* gok_main_read_keyboards_from_dir (const char *directory, GokKeyboard *first);
void gok_main_on_window_position_change (void);
void gok_main_get_our_window_size (gint* pWidth, gint* pHeight);
void gok_main_display_error (gchar* ErrorString);
void gok_main_display_gconf_error (void);
gboolean gok_main_window_contains_pointer (void);
Display *gok_main_display (void);
gchar* gok_main_get_scan_override (void);
gchar* gok_main_get_select_override (void);
gchar* gok_main_get_access_method_override (void);
gdouble gok_main_get_valuatorsensitivity_override (void);
gboolean gok_main_get_extras (void);
gboolean gok_main_get_login (void);
gchar* gok_main_get_inputdevice_name (void);
void gok_main_check_accessibility (void);
void gok_main_warn_if_corepointer_mode (gchar *message, gboolean always);
gboolean gok_main_safe_mode (void);

/* private */
void gok_main_app_change_listener (Accessible* pAccessible);
void gok_main_window_change_listener (Accessible* pAccessible);
void gok_main_motion_listener (gint n_axes, int *motion_data, long mods, long timestamp);
void gok_main_button_listener (gint button, gint state, long mods, long timestamp);
void gok_main_mousebutton_listener (gint button, gint state, long mods, long timestamp);
void gok_main_close (void);
gboolean gok_main_get_use_geometry (void);
void gok_main_get_geometry (GdkRectangle* pRectangle);
void gok_main_update_struts (gint width, gint height, gint x, gint y);
void gok_main_set_wm_dock (gboolean is_dock);

#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* #ifndef __MAIN_H__ */
