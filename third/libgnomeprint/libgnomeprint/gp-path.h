/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  gp-path.c:
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
 *  Copyrithg (C) 1999-2000 Authors
 *
 */

/*
 *
 * This is mostly like GnomeCanvasBpathDef, but with added functionality:
 * - can be constructed from scratch, from existing bpath of from static bpath
 * - Path is always terminated with ART_END
 * - Has closed flag
 * - has concat, split and copy methods
 *
 */

#ifndef __GP_PATH_H__
#define __GP_PATH_H__

#include <glib.h>

G_END_DECLS

#include <libart_lgpl/art_bpath.h>

typedef struct _GPPath GPPath;

/* Constructors */
GPPath * gp_path_new (void);
GPPath * gp_path_new_sized (gint length);
GPPath * gp_path_new_from_bpath (ArtBpath * bpath);
GPPath * gp_path_new_from_static_bpath (ArtBpath * bpath);
GPPath * gp_path_new_from_foreign_bpath (const ArtBpath * bpath);

void gp_path_ref (GPPath * path);
void gp_path_unref (GPPath * path);

void gp_path_reset (GPPath * path);
void gp_path_finish (GPPath * path);
void gp_path_ensure_space (GPPath * path, gint space);

/*
 * Misc constructors
 * All these return NEW path, not unrefing old
 * Also copy and duplicate force bpath to be private (otherwise you
 * would use ref :)
 */
GPPath * gp_path_copy (GPPath * dst, const GPPath * src);
GPPath * gp_path_duplicate (const GPPath * path);
GPPath * gp_path_concat (const GSList * list);
GSList * gp_path_split (const GPPath * path);
GPPath * gp_path_open_parts (const GPPath * path);
GPPath * gp_path_closed_parts (const GPPath * path);
GPPath * gp_path_close_all (const GPPath * path);


/* Drawing methods */
void gp_path_moveto    (GPPath * path, gdouble x, gdouble y);
void gp_path_lineto    (GPPath * path, gdouble x, gdouble y);
void gp_path_curveto   (GPPath * path, gdouble x0, gdouble y0,gdouble x1, gdouble y1, gdouble x2, gdouble y2);
void gp_path_closepath (GPPath * path);
/* Does not create new ArtBpath, but simply changes last lineto position */
void gp_path_lineto_moving (GPPath * path, gdouble x, gdouble y);

/* Does not draw new line to startpoint, but moves last lineto */
void gp_path_closepath_current (GPPath * path);

/* Various methods */
ArtBpath * gp_path_bpath (const GPPath * path);
gint       gp_path_length (const GPPath * path);
gboolean   gp_path_is_empty (const GPPath * path);
gboolean   gp_path_has_currentpoint (const GPPath * path);
ArtPoint * gp_path_currentpoint (const GPPath * path, ArtPoint * p);
ArtBpath * gp_path_last_bpath (const GPPath * path);
ArtBpath * gp_path_first_bpath (const GPPath * path);
gboolean   gp_path_any_open (const GPPath * path);
gboolean   gp_path_all_open (const GPPath * path);
gboolean   gp_path_any_closed (const GPPath * path);
gboolean   gp_path_all_closed (const GPPath * path);

G_END_DECLS

#endif /* __GP_PATH_H__ */
