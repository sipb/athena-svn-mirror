/* presconf.h
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
#ifndef __PRES_CONF__
#define __PRES_CONF__

#include <glib.h>

gboolean 	presconf_gconf_init		(void);
void 		presconf_gconf_terminate 	(void);
void		presconf_set_defaults 		(void);
GSList*		presconf_all_settings_get 	(void);
gboolean	presconf_check_if_setting_in_list (GSList *list, const gchar *name, GSList **ret);
gchar*		presconf_active_setting_get 	(void);
void		presconf_active_setting_set 	(const gchar *setting_name);


#endif
