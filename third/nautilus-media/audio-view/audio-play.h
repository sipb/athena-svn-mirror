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

/* audio-play.h - play files using AudioPlay object
 */


/* it's a hack to define NM_DEBUG here if necessary, but every other code file
 * pulls it in, so it's all good for now */

#ifndef AUDIO_PLAY_H
#define AUDIO_PLAY_H

#ifdef DEBUG
#define NM_DEBUG( ...) g_print (  __VA_ARGS__ )
#else
#define NM_DEBUG(x,...) {}
#endif

#define AUDIO_PLAY_ERROR           audio_play_error_quark ()

#define AUDIO_PLAY_TYPE            (audio_play_get_type ())
#define AUDIO_PLAY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), AUDIO_PLAY_TYPE, AudioPlay))
#define AUDIO_PLAY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AUDIO_PLAY_TYPE, AudioPlayClass))
#define IS_AUDIO_PLAY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AUDIO_PLAY_TYPE))
#define IS_AUDIO_PLAY_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE ((klass), AUDIO_PLAY_TYPE))
#define AUDIO_PLAY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), AUDIO_PLAY_TYPE, AudioPlayClass))

typedef struct _AudioPlay          AudioPlay;
typedef struct _AudioPlayClass     AudioPlayClass;

typedef struct AudioPlayPriv AudioPlayPriv;

typedef void (* AudioPlayErrorHandler) (const gchar *message, gpointer *data);

struct _AudioPlay
{
	GObject parent;
	AudioPlayPriv *priv;
};

struct _AudioPlayClass
{
	GObjectClass parent_class;

	/* signals */
	void (*eos)	(AudioPlay *play);
	void (*tick)	(AudioPlay *play, guint64 nanosecs);
	void (*length)	(AudioPlay *play, guint64 nanosecs);
};

GType		audio_play_get_type		(void);
AudioPlay *	audio_play_new			(GError **error);

gboolean	audio_play_set_location		(AudioPlay *play,
		                                 const char *uri,
						 GError **error);

gboolean	audio_play_set_state		(AudioPlay *play,
		                                 GstElementState state,
						 GError **error);
GstElementState	audio_play_get_state		(AudioPlay *play);

guint64		audio_play_get_length		(AudioPlay *play);
void		audio_play_seek_to_pos		(AudioPlay *play, double value);
void		audio_play_seek_to_time		(AudioPlay *play,
		                                 guint64 nanosecs);

void		audio_play_set_sink		(AudioPlay *play,
						 GstElement *element);
void		audio_play_set_error_handler	(AudioPlay *play,
						 AudioPlayErrorHandler handler,
						 gpointer data);

#endif /* AUDIO_PLAY_H */
