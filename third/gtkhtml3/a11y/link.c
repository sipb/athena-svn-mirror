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

#include "htmllinktext.h"

#include "html.h"
#include "hyperlink.h"
#include "link.h"

static void html_a11y_link_class_init    (HTMLA11YLinkClass *klass);
static void html_a11y_link_init          (HTMLA11YLink *a11y_link);

static void atk_hyper_text_interface_init (AtkHypertextIface *iface);

static AtkHyperlink * html_a11y_link_get_link (AtkHypertext *hypertext, gint link_index);
static gint html_a11y_link_get_n_links (AtkHypertext *hypertext);
static gint html_a11y_link_get_link_index (AtkHypertext *hypertext, gint char_index);

static AtkObjectClass *parent_class = NULL;

GType
html_a11y_link_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo tinfo = {
			sizeof (HTMLA11YLinkClass),
			NULL,                                                      /* base init */
			NULL,                                                      /* base finalize */
			(GClassInitFunc) html_a11y_link_class_init,                /* class init */
			NULL,                                                      /* class finalize */
			NULL,                                                      /* class data */
			sizeof (HTMLA11YLink),                                     /* instance size */
			0,                                                         /* nb preallocs */
			(GInstanceInitFunc) html_a11y_link_init,                   /* instance init */
			NULL                                                       /* value table */
		};

		static const GInterfaceInfo atk_hyper_text_info = {
			(GInterfaceInitFunc) atk_hyper_text_interface_init,
			(GInterfaceFinalizeFunc) NULL,
			NULL
		};

		type = g_type_register_static (G_TYPE_HTML_A11Y_TEXT, "HTMLA11YLink", &tinfo, 0);
		g_type_add_interface_static (type, ATK_TYPE_HYPERTEXT, &atk_hyper_text_info);
	}

	return type;
}

static void
atk_hyper_text_interface_init (AtkHypertextIface *iface)
{
	g_return_if_fail (iface != NULL);

	iface->get_link = html_a11y_link_get_link;
	iface->get_n_links = html_a11y_link_get_n_links;
	iface->get_link_index = html_a11y_link_get_link_index;
}

static void
html_a11y_link_finalize (GObject *obj)
{
}

static void
html_a11y_link_initialize (AtkObject *obj, gpointer data)
{
	/* printf ("html_a11y_link_initialize\n"); */

	if (ATK_OBJECT_CLASS (parent_class)->initialize)
		ATK_OBJECT_CLASS (parent_class)->initialize (obj, data);
}

static void
html_a11y_link_class_init (HTMLA11YLinkClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	AtkObjectClass *atk_class = ATK_OBJECT_CLASS (klass);

	parent_class = g_type_class_peek_parent (klass);

	atk_class->initialize = html_a11y_link_initialize;
	gobject_class->finalize = html_a11y_link_finalize;
}

static void
html_a11y_link_init (HTMLA11YLink *a11y_link)
{
}

AtkObject* 
html_a11y_link_new (HTMLObject *html_obj)
{
	GObject *object;
	AtkObject *accessible;

	g_return_val_if_fail (HTML_IS_LINK_TEXT (html_obj), NULL);

	object = g_object_new (G_TYPE_HTML_A11Y_LINK, NULL);

	accessible = ATK_OBJECT (object);
	atk_object_initialize (accessible, html_obj);

	accessible->role = ATK_ROLE_TEXT;

	/* printf ("created new html accessible link object\n"); */

	return accessible;
}

/*
 * AtkHyperLink interface
 */

static AtkHyperlink *
html_a11y_link_get_link (AtkHypertext *hypertext, gint link_index)
{
	return html_a11y_hyper_link_new (HTML_A11Y (hypertext));
}

static gint
html_a11y_link_get_n_links (AtkHypertext *hypertext)
{
	return 1;
}

static gint
html_a11y_link_get_link_index (AtkHypertext *hypertext, gint char_index)
{
	HTMLObject *obj = HTML_A11Y_HTML (hypertext);

	return HTML_TEXT (obj)->text_len > 0 && char_index >= 0 && char_index < HTML_TEXT (obj)->text_len ? 0 : -1;
}
