/* gnopiui.h
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

#ifndef __GNOPIUI__
#define __GNOPIUI__ 
#include <glade/glade.h>

void		gn_iconify_gnopernicus 		(void);
gboolean	gn_load_gnopi			(void);
gboolean 	gn_load_io_settings 		(void);
gboolean        gn_load_configure 		(void);
GladeXML* 	gn_load_interface 		(const gchar *glade_file, 
						 const gchar *window);
void		gn_load_help 			(const gchar *section);
void 		gn_show_message			(const gchar *msg);
#endif
