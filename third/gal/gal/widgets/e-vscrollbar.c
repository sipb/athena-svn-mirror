/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-vscrollbar.c
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

/* This is just a version of GtkVScrollbar that returns TRUE for button_press
   and button_release events.  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gal/util/e-util.h"

#include "e-vscrollbar.h"


#define PARENT_TYPE gtk_vscrollbar_get_type ()
static GtkVScrollbarClass *parent_class = NULL;


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
class_init (EVScrollbarClass *klass)
{
	GtkWidgetClass *widget_class;

	parent_class = gtk_type_class (PARENT_TYPE);

	widget_class = GTK_WIDGET_CLASS (klass);

	widget_class->button_press_event   = impl_button_press_event;
	widget_class->button_release_event = impl_button_release_event;
}

static void
init (EVScrollbar *vscrollbar)
{
}


GtkWidget *
e_vscrollbar_new (GtkAdjustment *adjustment)
{
	EVScrollbar *new;

	new = E_VSCROLLBAR (gtk_object_new (e_vscrollbar_get_type (), "adjustment", adjustment, NULL));
	
	return GTK_WIDGET (new);
}


E_MAKE_TYPE (e_vscrollbar, "EVScrollbar", EVScrollbar, class_init, init, PARENT_TYPE)
