/*
 * Copyright 2001 Sun Microsystems Inc.
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

#ifndef __HTML_LINK_ACCESSIBLE_H__
#define __HTML_LINK_ACCESSIBLE_H__

#include <atk/atk.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define HTML_TYPE_LINK_ACCESSIBLE              (html_link_accessible_get_type ())
#define HTML_LINK_ACCESSIBLE(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), HTML_TYPE_LINK_ACCESSIBLE, HtmlLinkAccessible))
#define HTML_LINK_ACCESSIBLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), HTML_TYPE_LINK_ACCESSIBLE, HtmlLinkAccessibleClass))
#define HTML_IS_LINK_ACCESSIBLE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), HTML_TYPE_LINK_ACCESSIBLE))
#define HTML_IS_LINK_ACCESSIBLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), HTML_TYPE_LINK_ACCESSIBLE))
#define HTML_LINK_ACCESSIBLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), HTML_TYPE_LINK_ACCESSIBLE, HtmlLinkAccessibleClass))

typedef struct _HtmlLinkAccessible        HtmlLinkAccessible;
typedef struct _HtmlLinkAccessibleClass   HtmlLinkAccessibleClass;

struct _HtmlLinkAccessible
{
	AtkHyperlink parent;

	AtkObject *obj;

	gchar *click_description;
	guint  action_idle_handler;
};

GType
html_link_accessible_get_type (void);

struct _HtmlLinkAccessibleClass
{
	AtkHyperlinkClass parent_class;
};

AtkHyperlink* html_link_accessible_new (AtkObject *obj);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __HTML_LINK_ACCESSIBLE_H__ */