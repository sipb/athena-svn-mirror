/*
 * glade-bonobo.c: support for bonobo widgets in libglade.
 *
 * Authors:
 *      Michael Meeks (michael@ximian.com)
 *      Jacob Berkman (jacob@ximian.com>
 *
 * Copyright (C) 2000,2001 Ximian, Inc., 2001,2002 James Henstridge.
 */
#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <libbonoboui.h>
#include <glade/glade-init.h>
#include <glade/glade-build.h>

#define INT(s)   (strtol ((s), NULL, 0))
#define UINT(s)  (strtoul ((s), NULL, 0))
#define BOOL(s)  (g_ascii_tolower (*(s)) == 't' || g_ascii_tolower (*(s)) == 'y' || INT (s))
#define FLOAT(s) (g_strtod ((s), NULL))

static void
dock_allow_floating (GladeXML *xml, GtkWidget *widget,
		     const char *name, const char *value)
{
	bonobo_dock_allow_floating_items (BONOBO_DOCK (widget), BOOL (value));
}

static void
dock_item_set_shadow_type (GladeXML *xml, GtkWidget *widget,
			   const char *name, const char *value)
{
	bonobo_dock_item_set_shadow_type (
		BONOBO_DOCK_ITEM (widget),
		glade_enum_from_string (GTK_TYPE_SHADOW_TYPE, value));
}

static void
dock_item_set_behavior (GladeXML *xml, GtkWidget *widget,
			const char *name, const char *value)
{
	BonoboDockItem *dock_item = BONOBO_DOCK_ITEM (widget);
	gchar *old_name;

	old_name = dock_item->name;
	dock_item->name = NULL;
	bonobo_dock_item_construct (dock_item, old_name,
				    glade_flags_from_string (
					BONOBO_TYPE_DOCK_ITEM_BEHAVIOR,
					value));
	g_free (old_name);
}

static GtkWidget *
dock_item_build (GladeXML *xml, GType widget_type,
		 GladeWidgetInfo *info)
{
	GtkWidget *w;

	w = glade_standard_build_widget (xml, widget_type, info);

	g_free(BONOBO_DOCK_ITEM (w)->name);
	BONOBO_DOCK_ITEM (w)->name = g_strdup (info->name);

	return w;
}


static GtkWidget *
glade_bonobo_widget_new (GladeXML        *xml,
			 GType            widget_type,
			 GladeWidgetInfo *info)
{
	const gchar *control_moniker = NULL;
	GtkWidget *widget;
	GObjectClass *oclass;
	BonoboControlFrame *cf;
	Bonobo_PropertyBag pb;
	gint i;

	for (i = 0; i < info->n_properties; i++) {
		if (!strcmp (info->properties[i].name, "moniker")) {
			control_moniker = info->properties[i].value;
			break;
		}
	}

	if (!control_moniker) {
		g_warning (G_STRLOC " BonoboWidget doesn't have moniker property");
		return NULL;
	}
	widget = bonobo_widget_new_control (
		control_moniker, CORBA_OBJECT_NIL);

	if (!widget) {
		g_warning (G_STRLOC " unknown bonobo control '%s'", control_moniker);
		return NULL;
	}

	oclass = G_OBJECT_GET_CLASS (widget);

	cf = bonobo_widget_get_control_frame (BONOBO_WIDGET (widget));

	if (!cf) {
		g_warning ("control '%s' has no frame", control_moniker);
		gtk_widget_unref (widget);
		return NULL;
	}

	pb = bonobo_control_frame_get_control_property_bag (cf, NULL);
	if (pb == CORBA_OBJECT_NIL)
		return widget;

	for (i = 0; i < info->n_properties; i++) {
		const gchar *name = info->properties[i].name;
		const gchar *value = info->properties[i].value;
		GParamSpec *pspec;

		if (!strcmp (name, "moniker"))
			continue;

		pspec = g_object_class_find_property (oclass, name);
		if (pspec) {
			GValue gvalue = { 0 };

			if (glade_xml_set_value_from_string(xml, pspec, value, &gvalue)) {
				g_object_set_property(G_OBJECT(widget), name, &gvalue);
				g_value_unset(&gvalue);
			}
		} else if (pb != CORBA_OBJECT_NIL) {
			CORBA_TypeCode tc =
				bonobo_property_bag_client_get_property_type (pb, name, NULL);

			switch (tc->kind) {
			case CORBA_tk_boolean:
				bonobo_property_bag_client_set_value_gboolean (pb, name,
									       value[0] == 'T' || value[0] == 'y', NULL);
				break;
			case CORBA_tk_string:
				bonobo_property_bag_client_set_value_string (pb, name, value,
									     NULL);
				break;
			case CORBA_tk_long:
				bonobo_property_bag_client_set_value_glong (pb, name,
									    strtol (value, NULL,0), NULL);
				break;
			case CORBA_tk_float:
				bonobo_property_bag_client_set_value_gfloat (pb, name,
									     strtod (value, NULL), NULL);
				break;
			case CORBA_tk_double:
				bonobo_property_bag_client_set_value_gdouble (pb, name,
									      strtod (value, NULL),
									      NULL);
				break;
			default:
				g_warning ("Unhandled type %d for `%s'", tc->kind, name);
				break;
			}
		} else {
			g_warning ("could not handle property `%s'", name);
		}
	}

	bonobo_object_release_unref (pb, NULL);

	return widget;
}

static GtkWidget *
bonobo_window_find_internal_child (GladeXML    *xml,
				   GtkWidget   *parent,
				   const gchar *childname)
{
	if (!strcmp (childname, "vbox")) {
		GtkWidget *ret;

		if ((ret = bonobo_window_get_contents (
			BONOBO_WINDOW (parent))))
			return ret;

		else {
			GtkWidget *box;

			box = gtk_vbox_new (FALSE, 0);
			
			bonobo_window_set_contents (
				BONOBO_WINDOW (parent), box);

			return box;
		}
	}

    return NULL;
}

static void
add_dock_item (GladeXML *xml, 
	       GtkWidget *parent,
	       GladeWidgetInfo *info,
	       GladeChildInfo *childinfo)
{
	BonoboDockPlacement placement;
	guint band, offset;
	int position;
	int i;
	GtkWidget *child;
	
	band = offset = position = 0;
	placement = BONOBO_DOCK_TOP;
	
	for (i = 0; i < childinfo->n_properties; i++) {
		const char *name  = childinfo->properties[i].name;
		const char *value = childinfo->properties[i].value;
		
		if (!strcmp (name, "placement"))
			placement = glade_enum_from_string (
				BONOBO_TYPE_DOCK_PLACEMENT,
				value);
		else if (!strcmp (name, "band"))
			band = UINT (value);
		else if (!strcmp (name, "position"))
			position = INT (value);
		else if (!strcmp (name, "offset"))
			offset = UINT (value);
	}

	child = glade_xml_build_widget (xml, childinfo->child);

	bonobo_dock_add_item (BONOBO_DOCK (parent),
			      BONOBO_DOCK_ITEM (child),
			      placement, band, position, offset, 
			      FALSE);
}
				

static void
dock_build_children (GladeXML *xml, GtkWidget *w, GladeWidgetInfo *info)
{
	int i;
	GtkWidget *child;
	GladeChildInfo *childinfo;

	for (i = 0; i < info->n_children; i++) {
		childinfo = &info->children[i];

		if (!strcmp (childinfo->child->classname, "BonoboDockItem")) {
			add_dock_item (xml, w, info, childinfo);
			continue;
		}
		
		if (bonobo_dock_get_client_area (BONOBO_DOCK (w)))
			g_warning ("Multiple client areas for BonoboDock found.");
		
		child = glade_xml_build_widget (xml, childinfo->child);
		bonobo_dock_set_client_area (BONOBO_DOCK (w), child);
	}
}

/* this macro puts a version check function into the module */
GLADE_MODULE_CHECK_INIT

void
glade_module_register_widgets (void)
{
	glade_require ("gtk");

	glade_register_custom_prop (BONOBO_TYPE_DOCK, "allow_floating", dock_allow_floating);
	glade_register_custom_prop (BONOBO_TYPE_DOCK_ITEM, "shadow_type", dock_item_set_shadow_type);
	glade_register_custom_prop (BONOBO_TYPE_DOCK_ITEM, "behavior", dock_item_set_behavior);

	glade_register_widget (BONOBO_TYPE_WIDGET,
			       glade_bonobo_widget_new,
			       NULL, NULL);
	glade_register_widget (BONOBO_TYPE_WINDOW,
			       NULL, glade_standard_build_children,
			       bonobo_window_find_internal_child);
	glade_register_widget (BONOBO_TYPE_DOCK,
			       NULL, dock_build_children,
			       NULL);
	glade_register_widget (BONOBO_TYPE_DOCK_ITEM,
			       dock_item_build, glade_standard_build_children, NULL);
	glade_provide ("bonobo");
}
