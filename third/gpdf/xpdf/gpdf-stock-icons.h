/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Stock icons for GPdf
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef GPDF_STOCK_ICONS_H
#define GPDF_STOCK_ICONS_H

#include <aconf.h>
#include <glib/gmacros.h>

G_BEGIN_DECLS

/* Toolbar icons */
#define GPDF_STOCK_ZOOM_FIT_WIDTH 	"gpdf-zoom-fit-width"

/* Bookmarks icons */
#define GPDF_STOCK_BOOK_CLOSED	  	"gpdf-book-closed"
#define GPDF_STOCK_BOOK_OPENED	  	"gpdf-book-opened"
#define GPDF_STOCK_BOOK_CLOSED_MARK	"gpdf-book-closed-mark"
#define GPDF_STOCK_BOOK_OPENED_MARK	"gpdf-book-opened-mark"
#define GPDF_STOCK_BOOKMARKS		"gpdf-bookmarks"

#ifdef USE_ANNOTS_VIEW
/* Annotations icons */
#define GPDF_STOCK_ANNOT_CIRCLE		"gpdf-annot-circle"
#define GPDF_STOCK_ANNOT_FILEATTACHMENT	"gpdf-annot-fileattachment"
#define GPDF_STOCK_ANNOT_FREETEXT	"gpdf-annot-freetext"
#define GPDF_STOCK_ANNOT_HIGHLIGHT	"gpdf-annot-highlight"
#define GPDF_STOCK_ANNOT_INK		"gpdf-annot-ink"
#define GPDF_STOCK_ANNOT_LINE		"gpdf-annot-line"
#define GPDF_STOCK_ANNOT_LINK		"gpdf-annot-link"
#define GPDF_STOCK_ANNOT_MOVIE		"gpdf-annot-movie"
#define GPDF_STOCK_ANNOT_POPUP		"gpdf-annot-popup"
#define GPDF_STOCK_ANNOT_SOUND		"gpdf-annot-sound"
#define GPDF_STOCK_ANNOT_SQUARE		"gpdf-annot-square"
#define GPDF_STOCK_ANNOT_STAMP		"gpdf-annot-stamp"
#define GPDF_STOCK_ANNOT_STRIKEOUT	"gpdf-annot-strikeout"
#define GPDF_STOCK_ANNOT_TEXT		"gpdf-annot-text"
#define GPDF_STOCK_ANNOT_TRAPNET	"gpdf-annot-trapnet"
#define GPDF_STOCK_ANNOT_UNDERLINE	"gpdf-annot-underline"
#define GPDF_STOCK_ANNOT_WIDGET		"gpdf-annot-widget"
#define GPDF_STOCK_ANNOT_UNKNOWN	"gpdf-annot-unknown"
#endif

void gpdf_stock_icons_init (void);

G_END_DECLS

#endif /* GPDF_STOCK_ICONS_H */
