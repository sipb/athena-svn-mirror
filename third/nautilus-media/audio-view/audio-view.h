/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* 
 * Copyright (C) 2002 Thomas Vander Stichele
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
 * Author: Thomas Vander Stichele <thomas at apestaart dot org>
 */

/* audio-view.h - audio view code to be shared among implementations
 */

#ifndef AUDIO_VIEW_H
#define AUDIO_VIEW_H
	
typedef struct AudioView AudioView;

AudioView *	audio_view_new			();
GtkWidget *	audio_view_get_widget		(AudioView *view);

void		audio_view_load_location	(AudioView *view,
						 const char *location);
const gchar *	audio_view_get_error		(AudioView *view);

static void audio_view_set_playing (AudioView *view, GstMediaInfoStream *info,
		                    GtkTreePath *path);
/* updates the view with info from the uri */

static void
audio_view_update (AudioView *view);

/* set up the tree view based on the AudioView */
static void
set_up_tree_view (AudioView *view);


#endif /* AUDIO_VIEW_H */
