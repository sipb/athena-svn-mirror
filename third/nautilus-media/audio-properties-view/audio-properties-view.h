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

/* audio-properties-view.h - properties of audio files */

#ifndef AUDIO_PROPERTIES_VIEW_H
#define AUDIO_PROPERTIES_VIEW_H

typedef	struct AudioPropertiesView AudioPropertiesView;


void	audio_properties_view_dispose		(AudioPropertiesView *view);
void	audio_properties_view_load_location	(AudioPropertiesView *view,
			                         const char *location);
GtkWidget *
	audio_properties_view_get_widget	(AudioPropertiesView *view);
AudioPropertiesView *
	audio_properties_view_new		();

#endif /* AUDIO_PROPERTIES_VIEW_H */
