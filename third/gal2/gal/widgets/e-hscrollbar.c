/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-hscrollbar.c
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Ettore Perazzoli <ettore@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/* This is just a version of GtkHScrollbar that returns TRUE for button_press
   and button_release events.  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gal/util/e-util.h"

#include "e-hscrollbar.h"


#define PARENT_TYPE gtk_hscrollbar_get_type ()
static GtkHScrollbarClass *parent_class = NULL;


static int
impl_button_press_event (GtkWidget *widget,
			 GdkEventButton *event)
{
	(* GTK_WIDGET_CLASS (parent_class)->button_press_event) (widget, event);

	return TRUE;
}

static int
impl_button_release_event (GtkWidget *widget,
			   GdkEventButton *event)
{
	(* GTK_WIDGET_CLASS (parent_class)->button_release_event) (widget, event);

	return TRUE;
}


static void
class_init (EHScrollbarClass *klass)
{
	GtkWidgetClass *widget_class;

	parent_class = g_type_class_ref (PARENT_TYPE);

	widget_class = GTK_WIDGET_CLASS (klass);

	widget_class->button_press_event   = impl_button_press_event;
	widget_class->button_release_event = impl_button_release_event;
}

static void
init (EHScrollbar *hscrollbar)
{
}


GtkWidget *
e_hscrollbar_new (GtkAdjustment *adjustment)
{
	EHScrollbar *new;

	new = E_HSCROLLBAR (g_object_new (E_TYPE_HSCROLLBAR, "adjustment", adjustment, NULL));
	
	return GTK_WIDGET (new);
}


E_MAKE_TYPE (e_hscrollbar, "EHScrollbar", EHScrollbar, class_init, init, PARENT_TYPE)
