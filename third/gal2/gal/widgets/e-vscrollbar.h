/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * e-vscrollbar.h
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

#ifndef _E_VSCROLLBAR_H_
#define _E_VSCROLLBAR_H_

#include <gtk/gtkvscrollbar.h>

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#define E_TYPE_VSCROLLBAR			(e_vscrollbar_get_type ())
#define E_VSCROLLBAR(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), E_TYPE_VSCROLLBAR, EVScrollbar))
#define E_VSCROLLBAR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), E_TYPE_VSCROLLBAR, EVScrollbarClass))
#define E_IS_VSCROLLBAR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), E_TYPE_VSCROLLBAR))
#define E_IS_VSCROLLBAR_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((obj), E_TYPE_VSCROLLBAR))


typedef struct _EVScrollbar        EVScrollbar;
typedef struct _EVScrollbarPrivate EVScrollbarPrivate;
typedef struct _EVScrollbarClass   EVScrollbarClass;

struct _EVScrollbar {
	GtkVScrollbar parent;
};

struct _EVScrollbarClass {
	GtkVScrollbarClass parent_class;
};


GtkType    e_vscrollbar_get_type (void);
GtkWidget *e_vscrollbar_new      (GtkAdjustment *adjustment);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _E_VSCROLLBAR_H_ */
