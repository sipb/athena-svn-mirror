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
 * Author: Andy Hertzfeld
 */

/* header file for the text view component */

#ifndef NAUTILUS_MUSIC_VIEW_H
#define NAUTILUS_MUSIC_VIEW_H

#include <libnautilus/nautilus-view.h>
#include <gtk/gtkeventbox.h> 
typedef struct _NautilusTextView      NautilusTextView;
typedef struct _NautilusTextViewClass NautilusTextViewClass;

#define NAUTILUS_TYPE_TEXT_VIEW	    (nautilus_text_view_get_type ())
#define NAUTILUS_TEXT_VIEW(obj)	    (GTK_CHECK_CAST ((obj), NAUTILUS_TYPE_TEXT_VIEW, NautilusTextView))
#define NAUTILUS_TEXT_VIEW_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), NAUTILUS_TYPE_TEXT_VIEW, NautilusTextViewClass))
#define NAUTILUS_IS_TEXT_VIEW(obj)	    (GTK_CHECK_TYPE ((obj), NAUTILUS_TYPE_TEXT_VIEW))
#define NAUTILUS_IS_TEXT_VIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), NAUTILUS_TYPE_TEXT_VIEW))

typedef struct _NautilusTextViewDetails NautilusTextViewDetails;

struct _NautilusTextView {
	GtkEventBox parent;
	NautilusTextViewDetails *details;
};

struct _NautilusTextViewClass {
	GtkEventBoxClass parent_class;
};



/* GtkObject support */
GtkType       nautilus_text_view_get_type          (void);

/* Component embedding support */
NautilusView *nautilus_text_view_get_nautilus_view (NautilusTextView *view);

/* URI handling */
void          nautilus_text_view_load_uri          (NautilusTextView *view,
						     						const char *uri);

#endif /* NAUTILUS_TEXT_VIEW_H */
