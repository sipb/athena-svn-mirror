/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gp-tt-t1.h:  TrueType to Type1 converter
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Copyright:
 *  **NOTE: that the actual code has very long copyright
 *          owners list, and has advertisement clause. Read
 *          accompanying source file for more information.
 */

#ifndef __GP_TT_T1_H__
#define __GP_TT_T1_H__

#include <glib.h>

G_BEGIN_DECLS

#include <freetype/freetype.h>

guchar *ttf2pfa (FT_Face face, const guchar *embeddedname, guint32 *glyphmask);

G_END_DECLS

#endif /* __GP_TT_T1_H__ */
