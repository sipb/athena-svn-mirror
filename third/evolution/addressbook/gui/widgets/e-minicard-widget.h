/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* e-minicard-widget.h
 * Copyright (C) 2000  Ximian, Inc.
 * Author: Chris Lahey <clahey@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __E_MINICARD_WIDGET_H__
#define __E_MINICARD_WIDGET_H__

#include <gal/widgets/e-canvas.h>
#include "addressbook/backend/ebook/e-card.h"

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

/* EMinicardWidget - A card displaying information about a contact.
 *
 * The following arguments are available:
 *
 * name		type		read/write	description
 * --------------------------------------------------------------------------------
 */

#define E_TYPE_MINICARD_WIDGET			(e_minicard_widget_get_type ())
#define E_MINICARD_WIDGET(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), E_TYPE_MINICARD_WIDGET, EMinicardWidget))
#define E_MINICARD_WIDGET_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), E_TYPE_MINICARD_WIDGET, EMinicardWidgetClass))
#define E_IS_MINICARD_WIDGET(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), E_TYPE_MINICARD_WIDGET))
#define E_IS_MINICARD_WIDGET_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((obj), E_TYPE_MINICARD_WIDGET))


typedef struct _EMinicardWidget       EMinicardWidget;
typedef struct _EMinicardWidgetClass  EMinicardWidgetClass;

struct _EMinicardWidget
{
	ECanvas parent;
	
	/* item specific fields */
	GnomeCanvasItem *item;

	GnomeCanvasItem *rect;
	ECard *card;
};

struct _EMinicardWidgetClass
{
	ECanvasClass parent_class;
};


GtkWidget *e_minicard_widget_new(void);
GType      e_minicard_widget_get_type (void);

void e_minicard_widget_set_card (EMinicardWidget *, ECard *);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __E_MINICARD_WIDGET_H__ */