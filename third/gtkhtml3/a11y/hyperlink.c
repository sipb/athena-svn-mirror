/*  This file is part of the GtkHTML library.
 *
 *  Copyright 2002 Ximian, Inc.
 *
 *  Author: Radek Doulik
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <atk/atkhyperlink.h>

#include "htmltext.h"

#include "object.h"
#include "html.h"
#include "hyperlink.h"

static void html_a11y_hyper_link_class_init    (HTMLA11YHyperLinkClass *klass);
static void html_a11y_hyper_link_init          (HTMLA11YHyperLink *a11y_hyper_link);

static void atk_action_interface_init (AtkActionIface *iface);

static gboolean html_a11y_hyper_link_do_action (AtkAction *action, gint i);
static gint html_a11y_hyper_link_get_n_actions (AtkAction *action);
static G_CONST_RETURN gchar * html_a11y_hyper_link_get_description (AtkAction *action, gint i);
static G_CONST_RETURN gchar * html_a11y_hyper_link_get_name (AtkAction *action, gint i);
static gboolean html_a11y_hyper_link_set_description (AtkAction *action, gint i, const gchar *description);

static AtkObjectClass *parent_class = NULL;

GType
html_a11y_hyper_link_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo tinfo = {
			sizeof (HTMLA11YHyperLinkClass),
			NULL,                                                      /* base init */
			NULL,                                                      /* base finalize */
			(GClassInitFunc) html_a11y_hyper_link_class_init,                /* class init */
			NULL,                                                      /* class finalize */
			NULL,                                                      /* class data */
			sizeof (HTMLA11YHyperLink),                                     /* instance size */
			0,                                                         /* nb preallocs */
			(GInstanceInitFunc) html_a11y_hyper_link_init,                   /* instance init */
			NULL                                                       /* value table */
		};

		static const GInterfaceInfo atk_action_info = {
			(GInterfaceInitFunc) atk_action_interface_init,
			(GInterfaceFinalizeFunc) NULL,
			NULL
		};

		type = g_type_register_static (ATK_TYPE_HYPERLINK, "HTMLA11YHyperLink", &tinfo, 0);
		g_type_add_interface_static (type, ATK_TYPE_ACTION, &atk_action_info);
	}

	return type;
}

static void
atk_action_interface_init (AtkActionIface *iface)
{
	g_return_if_fail (iface != NULL);

	iface->do_action       = html_a11y_hyper_link_do_action;
	iface->get_n_actions   = html_a11y_hyper_link_get_n_actions;
	iface->get_description = html_a11y_hyper_link_get_description;
	iface->get_name        = html_a11y_hyper_link_get_name;
	iface->set_description = html_a11y_hyper_link_set_description;
}

static void
html_a11y_hyper_link_finalize (GObject *obj)
{
	HTMLA11YHyperLink *hl = HTML_A11Y_HYPER_LINK (obj);

	if (hl->a11y)
		g_object_remove_weak_pointer (G_OBJECT (hl->a11y),
					      (gpointer *) &hl->a11y);

	G_OBJECT_CLASS (parent_class)->finalize (obj);
}

static void
html_a11y_hyper_link_class_init (HTMLA11YHyperLinkClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	gobject_class->finalize = html_a11y_hyper_link_finalize;
}

static void
html_a11y_hyper_link_init (HTMLA11YHyperLink *a11y_hyper_link)
{
	a11y_hyper_link->description = NULL;
}

AtkHyperlink * 
html_a11y_hyper_link_new (HTMLA11Y *a11y)
{
	HTMLA11YHyperLink *hl;

	g_return_val_if_fail (G_IS_HTML_A11Y (a11y), NULL);

	hl = HTML_A11Y_HYPER_LINK (g_object_new (G_TYPE_HTML_A11Y_HYPER_LINK, NULL));

	hl->a11y = a11y;
	g_object_add_weak_pointer (G_OBJECT (hl->a11y), (gpointer *) &hl->a11y);

	return ATK_HYPERLINK (hl);
}

/*
 * Action interface
 */

static gboolean
html_a11y_hyper_link_do_action (AtkAction *action, gint i)
{
	HTMLA11YHyperLink *hl;
	gboolean result = FALSE;

	hl = HTML_A11Y_HYPER_LINK (action);

	if (i == 0 && hl->a11y) {
		HTMLText *text = HTML_TEXT (HTML_A11Y_HTML (hl->a11y));
		gchar *url = html_object_get_complete_url (HTML_OBJECT (text), hl->offset);

		if (url && *url) {
			GObject *gtkhtml = GTK_HTML_A11Y_GTKHTML_POINTER
				(html_a11y_get_gtkhtml_parent (HTML_A11Y (hl->a11y)));

			g_signal_emit_by_name (gtkhtml, "link_clicked", url);
			result = TRUE;
		}
		
		g_free (url);
	}

	return result;
}

static gint
html_a11y_hyper_link_get_n_actions (AtkAction *action)
{
	return 1;
}

static G_CONST_RETURN gchar *
html_a11y_hyper_link_get_description (AtkAction *action, gint i)
{
	if (i == 0) {
		HTMLA11YHyperLink *hl;

		hl = HTML_A11Y_HYPER_LINK (action);

		return hl->description;
	}

	return NULL;
}

static G_CONST_RETURN gchar *
html_a11y_hyper_link_get_name (AtkAction *action, gint i)
{
	return i == 0 ? "link click" : NULL;
}

static gboolean
html_a11y_hyper_link_set_description (AtkAction *action, gint i, const gchar *description)
{
	if (i == 0) {
		HTMLA11YHyperLink *hl;

		hl = HTML_A11Y_HYPER_LINK (action);

		g_free (hl->description);
		hl->description = g_strdup (description);

		return TRUE;	
	}

	return FALSE;
}
