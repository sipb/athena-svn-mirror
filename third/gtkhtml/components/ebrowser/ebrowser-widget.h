#ifndef _EBROWSER_WIDGET_H_
#define _EBROWSER_WIDGET_H_

/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Author: Lauris Kaplinski  <lauris@helixcode.com>
*/

#include <libgnome/gnome-defs.h>
#include <gtkhtml.h>
#include <ebrowser/ebrowser-history.h>

BEGIN_GNOME_DECLS

typedef enum {
	EBROWSER_PROTOCOL_UNKNOWN,
	EBROWSER_PROTOCOL_INTERNAL,
	EBROWSER_PROTOCOL_RELATIVE,
	EBROWSER_PROTOCOL_HTTP,
	EBROWSER_PROTOCOL_FILE
} EBrowserProtocol;

#define EBROWSER_TYPE (ebrowser_get_type ())
#define EBROWSER(o) (GTK_CHECK_CAST ((o), EBROWSER_TYPE, EBrowser))
#define IS_EBROWSER(o) (GTK_CHECK_TYPE ((o), EBROWSER_TYPE))

typedef struct _EBrowser EBrowser;
typedef struct _EBrowserClass EBrowserClass;

struct _EBrowser {
	GtkHTML html;
	gchar * http_proxy;
	gchar * url;
	gchar * baseroot;
	gchar * basedir;
	EBrowserProtocol baseprotocol;
	guint followlinks : 1;
	guint followredirect : 1;
	guint allowsubmit : 1;
	guint defaultbgcolor;
	gchar * defaultfont;

	GSList * loaders;

	EBrowserHistory *history;
	short            history_size;
};

struct _EBrowserClass {
	GtkHTMLClass parent_class;
	void (* url_set)    (EBrowser * ebrower, const gchar * url);
	void (* status_set) (EBrowser * ebrower, const gchar * status);
	void (* request)    (EBrowser * ebrower, const gchar * uri);
	void (* done)       (EBrowser * ebrower);
};

GtkType    ebrowser_get_type (void);
GtkWidget *ebrowser_new      (void);
void       ebrowser_stop     (EBrowser * ebrowser);

/*
 * fixme: Implement via loaders?
 */

gpointer ebrowser_base_stream (EBrowser * ebr);

END_GNOME_DECLS

#endif
