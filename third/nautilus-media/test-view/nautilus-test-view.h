/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* 
 * Copyright (C) 2000 Eazel, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Maciej Stachowiak <mjs@eazel.com>
 */

/* nautilus-test-view.h - Test view component. This
 * component just displays a simple message and includes a menu item
 * and a toolbar button else. It should be a good basis for writing
 * Nautilus view components.
 */

/* WHAT YOU NEED TO CHANGE: You should be able to leave this header
 * pretty much unchanged except for renaming everything to match your
 * component.
 */

#ifndef NAUTILUS_TEST_VIEW_H
#define NAUTILUS_TEST_VIEW_H

#include <libnautilus/nautilus-view.h>

#define NAUTILUS_TYPE_TEST_VIEW	     	   (nautilus_test_view_get_type ())
#define NAUTILUS_TEST_VIEW(obj)	     	   (GTK_CHECK_CAST ((obj), NAUTILUS_TYPE_TEST_VIEW, NautilusTestView))
#define NAUTILUS_TEST_VIEW_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), NAUTILUS_TYPE_TEST_VIEW, NautilusTestViewClass))
#define NAUTILUS_IS_TEST_VIEW(obj)	   (GTK_CHECK_TYPE ((obj), NAUTILUS_TYPE_TEST_VIEW))
#define NAUTILUS_IS_TEST_VIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), NAUTILUS_TYPE_TEST_VIEW))

typedef struct NautilusTestViewDetails NautilusTestViewDetails;

typedef struct {
	NautilusView parent;
	NautilusTestViewDetails *details;
} NautilusTestView;

typedef struct {
	NautilusViewClass parent;
} NautilusTestViewClass;

GType nautilus_test_view_get_type (void);

#endif /* NAUTILUS_TEST_VIEW_H */