/* gok-settings-page-accessmethod.h
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

#ifndef __GOK_PAGE_ACCESSMETHOD_H__
#define __GOK_PAGE_ACCESSMETHOD_H__

#include <gnome.h>
#include <glade/glade.h>
#include "gok-scanner.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

gboolean gok_page_accessmethod_initialize (GladeXML* xml);
void gok_page_accessmethod_close (void);
gboolean gok_page_accessmethod_apply (void);
gboolean gok_page_accessmethod_apply_controls (GokControl* pControl, gchar* NameAccessMethod);
gboolean gok_page_accessmethod_revert (void);
void gok_page_accessmethod_backup (void);
void gok_page_accessmethod_change_controls (gchar* pNameAccessMethod);
void gok_page_accessmethod_draw_controls (gchar* NameAccessMethod, GokControl* pControlParent, GokControl* pControl, gboolean bShow);
void gok_page_accessmethod_update_controls (gchar* NameAccessMethod, GokControl* pControl);
gchar* gok_page_accessmethod_get_displayname (gchar* NameAccessMethod);
gchar* gok_page_accessmethod_get_name (gchar* DisplayNameAccessMethod);
void gok_page_accessmethod_page_active (void);
void gok_page_accessmethod_fill_combos (gboolean bRefill);
void gok_page_accessmethod_update_effects (void);
void gok_page_accessmethod_method_changed (GtkEditable* pEditControl);
void gok_page_accessmethod_checkbox_changed (GtkButton* pButton, gpointer data);
void gok_page_accessmethod_update_associated (GokAccessMethod* pAccessMethod);
void gok_page_accessmethod_update_associated_loop (GokControl* pControl);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOK_PAGE_ACCESSMETHOD_H__ */
