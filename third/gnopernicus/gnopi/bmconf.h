/* bmconf.h
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
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

#ifndef __BRAILLE_MONITOR__
#define __BRAILLE_MONITOR__ 0


#include <glib.h>
#include "libsrconf.h"

gboolean	bmconf_gconf_client_init (void);
void 		bmconf_load_default_settings (void);


void		bmconf_size_get (gint *line, gint *column);
gchar*		bmconf_position_get (void);
gchar*		bmconf_display_mode_get (void);
gboolean	bmconf_use_theme_color_get (void);
void		bmconf_colors_get (gchar **dot7, gchar **dot8, gchar **dot78);
gint		bmconf_font_size_get (void);

void		bmconf_size_set (gint line, gint column);
void		bmconf_position_set (const gchar *position);
void		bmconf_display_mode_set (const gchar *mode);
void		bmconf_colors_set (const gchar *dot7, const gchar *dot8, const gchar *dot78);
void		bmconf_use_theme_color_set (gboolean use_theme);
void		bmconf_font_size_set (gint font_size);
#endif
