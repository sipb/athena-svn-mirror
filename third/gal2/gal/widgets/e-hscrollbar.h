/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-hscrollbar.h
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

#ifndef _E_HSCROLLBAR_H_
#define _E_HSCROLLBAR_H_

#include <gtk/gtkhscrollbar.h>

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#define E_TYPE_HSCROLLBAR			(e_hscrollbar_get_type ())
#define E_HSCROLLBAR(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), E_TYPE_HSCROLLBAR, EHScrollbar))
#define E_HSCROLLBAR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), E_TYPE_HSCROLLBAR, EHScrollbarClass))
#define E_IS_HSCROLLBAR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), E_TYPE_HSCROLLBAR))
#define E_IS_HSCROLLBAR_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((obj), E_TYPE_HSCROLLBAR))


typedef struct _EHScrollbar        EHScrollbar;
typedef struct _EHScrollbarPrivate EHScrollbarPrivate;
typedef struct _EHScrollbarClass   EHScrollbarClass;

struct _EHScrollbar {
	GtkHScrollbar parent;
};

struct _EHScrollbarClass {
	GtkHScrollbarClass parent_class;
};


GtkType    e_hscrollbar_get_type (void);
GtkWidget *e_hscrollbar_new      (GtkAdjustment *adjustment);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _E_HSCROLLBAR_H_ */
