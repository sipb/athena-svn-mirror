/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gp-truetype-utils.h: utility functions for TrueType
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
 *  Authors:
 *    Lauris Kaplinski <lauris@ariman.ee>
 *
 *  Copyright (C) 1999-2002 Ximian, Inc.
 */

#ifndef __GP_TRUETYPE_UTILS_H__
#define __GP_TRUETYPE_UTILS_H__

#include <glib/gmacros.h>

G_END_DECLS

#include <glib.h>

GSList *gp_tt_split_file (const guchar *buf, guint len);

G_END_DECLS

#endif

