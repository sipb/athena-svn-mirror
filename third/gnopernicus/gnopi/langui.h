/* langui.h
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

#ifndef __LANGUI_H__
#define __LANGUI_H__
#include <glib.h>
#include <libgnomeui/libgnomeui.h>

typedef struct {
	gchar *id;
	gchar *country;
	gboolean supported;
	} LanguageType;    
	
void	lng_load_language (GtkWidget *parent_window);
gint	lng_set_envirom   (gchar *id);

#endif
