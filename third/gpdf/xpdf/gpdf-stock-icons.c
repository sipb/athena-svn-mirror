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

#include <gtk/gtkiconfactory.h>
#include <gtk/gtkstock.h>
#include <gdk/gdkpixbuf.h>
#include "gpdf-stock-icons.h"

/* Toolbar icons files */
#define STOCK_ZOOM_FIT_WIDTH_FILE 	"fitwidth.png"

/* Bookmarks icons files */
#define STOCK_BOOK_CLOSED_FILE	  	"stock_book-closed.png"
#define STOCK_BOOK_OPENED_FILE	  	"stock_book-opened.png"
#define STOCK_BOOK_CLOSED_MARK_FILE	"stock_book-closed-mark.png"
#define STOCK_BOOK_OPENED_MARK_FILE	"stock_book-opened-mark.png"
#define STOCK_BOOKMARKS_FILE		"stock_bookmarks.png"

#ifdef USE_ANNOTS_VIEW
/* Annotations icons files */
#define STOCK_ANNOT_CIRCLE_FILE		"stock-annot-circle.png"
#define STOCK_ANNOT_FILEATTACHMENT_FILE "stock-annot-fileattachment.png"
#define STOCK_ANNOT_FREETEXT_FILE 	"stock-annot-freetext.png"
#define STOCK_ANNOT_HIGHLIGHT_FILE 	"stock-annot-highlight.png"
#define STOCK_ANNOT_INK_FILE 		"stock-annot-ink.png"
#define STOCK_ANNOT_LINE_FILE 		"stock-annot-line.png"
#define STOCK_ANNOT_LINK_FILE 		"stock-annot-link.png"
#define STOCK_ANNOT_MOVIE_FILE 		"stock-annot-movie.png"
#define STOCK_ANNOT_POPUP_FILE 		"stock-annot-popup.png"
#define STOCK_ANNOT_SOUND_FILE 		"stock-annot-sound.png"
#define STOCK_ANNOT_SQUARE_FILE 	"stock-annot-square.png"
#define STOCK_ANNOT_STAMP_FILE 		"stock-annot-stamp.png"
#define STOCK_ANNOT_STRIKEOUT_FILE 	"stock-annot-strikeout.png"
#define STOCK_ANNOT_TEXT_FILE 		"stock-annot-text.png"
#define STOCK_ANNOT_TRAPNET_FILE 	"stock-annot-trapnet.png"
#define STOCK_ANNOT_UNDERLINE_FILE 	"stock-annot-underline.png"
#define STOCK_ANNOT_WIDGET_FILE 	"stock-annot-widget.png"
#define STOCK_ANNOT_UNKNOWN_FILE 	"stock-annot-unknown.png"
#endif

#define GPDF_ADD_STOCK_ICON(id, file, def_id)				       \
{				  					       \
	GdkPixbuf *pixbuf;						       \
	GtkIconSet *icon_set = NULL;					       \
        pixbuf = gdk_pixbuf_new_from_file (GNOMEICONDIR "/gpdf/" file, NULL);  \
        if (pixbuf) {							       \
        	icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);	       \
	} else if (def_id) {						       \
		icon_set = gtk_icon_factory_lookup_default (def_id);	       \
		gtk_icon_set_ref (icon_set);				       \
	}								       \
        gtk_icon_factory_add (factory, id, icon_set);   		       \
        gtk_icon_set_unref (icon_set);					       \
}


void
gpdf_stock_icons_init (void)
{
        GtkIconFactory *factory;

        factory = gtk_icon_factory_new ();
        gtk_icon_factory_add_default (factory);

	/*
	 * Toolbar icons
	 */
	
	/* fitwidth gpdf stock icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ZOOM_FIT_WIDTH, STOCK_ZOOM_FIT_WIDTH_FILE, GTK_STOCK_ZOOM_FIT);
	
	/*
	 * Bookmarks view icons
	 */

	/* book closed gpdf stock icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_BOOK_CLOSED, STOCK_BOOK_CLOSED_FILE, 0); 

	/* book opened gpdf stock icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_BOOK_OPENED, STOCK_BOOK_OPENED_FILE, 0); 

	/* book closed w/ mark gpdf stock icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_BOOK_CLOSED_MARK, STOCK_BOOK_CLOSED_MARK_FILE, 0); 

	/* book opened w/ mark gpdf stock icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_BOOK_OPENED_MARK, STOCK_BOOK_OPENED_MARK_FILE, 0); 

	/* bookmarks gpdf stock icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_BOOKMARKS, STOCK_BOOKMARKS_FILE, 0);

#ifdef USE_ANNOTS_VIEW
	/*
	 * Annotations view icons
	 */

	/* Circle annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_CIRCLE, STOCK_ANNOT_CIRCLE_FILE, 0);

	/* Fileattachment annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_FILEATTACHMENT, STOCK_ANNOT_FILEATTACHMENT_FILE, 0);

	/* Freetext annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_FREETEXT, STOCK_ANNOT_FREETEXT_FILE, 0);

	/* Highlight annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_HIGHLIGHT, STOCK_ANNOT_HIGHLIGHT_FILE, 0);

	/* Ink annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_INK, STOCK_ANNOT_INK_FILE, 0);

	/* Line annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_LINE, STOCK_ANNOT_LINE_FILE, 0);

	/* Link annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_LINK, STOCK_ANNOT_LINK_FILE, 0);

	/* Movie annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_MOVIE, STOCK_ANNOT_MOVIE_FILE, 0);

	/* Popup annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_POPUP, STOCK_ANNOT_POPUP_FILE, 0);

	/* Sound annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_SOUND, STOCK_ANNOT_SOUND_FILE, 0);

	/* Square annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_SQUARE, STOCK_ANNOT_SQUARE_FILE, 0);

	/* Stamp annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_STAMP, STOCK_ANNOT_STAMP_FILE, 0);

	/* Strikeout annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_STRIKEOUT, STOCK_ANNOT_STRIKEOUT_FILE, 0);

	/* Text annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_TEXT, STOCK_ANNOT_TEXT_FILE, 0);

	/* Trapnet annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_TRAPNET, STOCK_ANNOT_TRAPNET_FILE, 0);

	/* Underline annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_UNDERLINE, STOCK_ANNOT_UNDERLINE_FILE, 0);

	/* Widget annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_WIDGET, STOCK_ANNOT_WIDGET_FILE, 0);

	/* Unknown annotation icon */
	GPDF_ADD_STOCK_ICON (GPDF_STOCK_ANNOT_UNKNOWN, STOCK_ANNOT_UNKNOWN_FILE, 0);
#endif

	g_object_unref (G_OBJECT (factory));
}
