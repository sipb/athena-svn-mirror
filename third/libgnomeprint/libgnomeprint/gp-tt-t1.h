/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * TrueType to Type1 converter
 *
 * Authors:
 *   Akira TAGOH <tagoh@redhat.com>
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Please notice, that the actual code has very long copyright
 * owners list, and has advertisement clause. Read accompanying
 * source file for more information.
 *
 * Copyright (C) 2001 Akira Tagoh
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#ifndef __GP_TT_T1_H__
#define __GP_TT_T1_H__

#include <glib.h>

G_BEGIN_DECLS

#include <freetype/freetype.h>

/*
 * Convert loaded Freetype TTF face to Type1
 *
 * ft_face has to be TrueType or TrueType collection
 * embeddedname is the name for resulting PS font
 * glyphmask is bit array of used glyphs
 *
 */

guchar *ttf2pfa (FT_Face face, const guchar *embeddedname, guint32 *glyphmask);

G_END_DECLS

#endif /* __PARSETT_H__ */
