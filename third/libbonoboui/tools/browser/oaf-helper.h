/* Bonobo component browser
 *
 * AUTHORS:
 *      Dan Siemon <dan@coverfire.com>
 *      Rodrigo Moya <rodrigo@gnome-db.org>
 *      Patanjali Somayaji <patanjali@morelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __OAF_HELPER_H__
#define __OAF_HELPER_H__

#include <bonobo-activation/bonobo-activation.h>
#include <bonobo-activation/Bonobo_Activation_types.h>
#include <bonobo/bonobo-i18n.h>

/*
 *      BonoboComponentInfo is used to store the data read from
 *      oaf about each component
 */

typedef enum {
        PROPERTY_TYPE_STRING,
        PROPERTY_TYPE_DOUBLE,
        PROPERTY_TYPE_BOOLEAN,
        PROPERTY_TYPE_STRINGLIST
} BonoboComponentPropertyType;

typedef struct {
        gchar *property_name;
	BonoboComponentPropertyType property_type;
        union {
                gchar *value_string;
		double value_double;
		gboolean value_boolean;
		GList *value_stringlist;
        } property_value;
} BonoboComponentProperty;

typedef struct {
        Bonobo_ImplementationID component_iid;
        gchar *component_type;
        gchar *component_location;
        gchar *component_username;
        gchar *component_hostname;
        gchar *component_domain;
        gchar *component_description;
        gchar *component_name;
        GList *component_properties;
	gboolean active;
} BonoboComponentInfo;

BonoboComponentInfo *bonobo_component_info_new (void);

void bonobo_component_info_set_values (
	BonoboComponentInfo *info,
	Bonobo_ImplementationID iid,
	const gchar *type,
	const gchar *location,
	const gchar *username,
	const gchar *hostname,
	const gchar *domain,
	const gchar *description,
	const gchar *name,
	GList *properties);

void bonobo_component_info_free (BonoboComponentInfo *info);

GList *bonobo_browser_get_components_list (const gchar *query);

void bonobo_browser_free_components_list (GList *list);

void bonobo_component_list_print (GList *list);

GList *bonobo_component_get_repoids (BonoboComponentInfo *info);

#endif
