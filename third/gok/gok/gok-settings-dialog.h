/* gok-settings-dialog.h
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto
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

#ifndef __GOKSETTINGSDIALOG_H__
#define __GOKSETTINGSDIALOG_H__

#include <gnome.h>
#include <glade/glade.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*page numbers - change these values if you change the order of the pages */
#define PAGE_NUM_APPEARANCE 0
#define PAGE_NUM_ACTIONS 1
#define PAGE_NUM_FEEDBACKS 2
#define PAGE_NUM_ACCESS_METHODS 3
#define PAGE_NUM_PREDICTION 4

gboolean gok_settingsdialog_open (gboolean bShow);
void gok_settingsdialog_close (void);
gboolean gok_settingsdialog_show (void);
void gok_settingsdialog_hide (void);
void on_button_try (GtkButton* button, gpointer user_data);
void on_button_revert (GtkButton* button, gpointer user_data);
void on_button_ok (GtkButton* button, gpointer user_data);
void on_button_cancel (GtkButton* button, gpointer user_data);
void on_button_help (GtkButton* button, gpointer user_data);
void gok_settingsdialog_refresh (void);
void gok_settingsdialog_backup_settings (void);
GtkWidget* gok_settingsdialog_get_window (void);
GladeXML* gok_settingsdialog_get_xml (void);
gint gok_settingsdialog_sort (gconstpointer pItem1, gconstpointer pItem2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKSETTINGSDIALOG_H__ */
