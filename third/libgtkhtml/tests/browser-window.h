/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
   Copyright (C) 2000 CodeFactory AB
   Copyright (C) 2000 Jonas Borgström <jonas@codefactory.se>
   Copyright (C) 2000 Anders Carlsson <andersca@codefactory.se>
   
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
*/

#ifndef __BROWSER_WINDOW_H__
#define __BROWSER_WINDOW_H__

#include <gtk/gtk.h>

#include <document/htmldocument.h>
#include <view/htmlview.h>

#define BROWSER_TYPE_WINDOW			(browser_window_get_type ())
#define BROWSER_WINDOW(obj)			(GTK_CHECK_CAST ((obj), BROWSER_TYPE_WINDOW, BrowserWindow))
#define BROWSER_WINDOW_CLASS(klass)		(GTK_CHECK_CLASS_CAST ((klass), BROWSER_TYPE_WINDOW, BrowserWindowClass))
#define BROWSER_IS_WINDOW(obj)			(GTK_CHECK_TYPE ((obj), BROWSER_TYPE_WINDOW))
#define BROWSER_IS_WINDOW_CLASS(klass)		(GTK_CHECK_CLASS_TYPE ((obj), BROWSER_TYPE_WINDOW))


typedef struct _BrowserWindow       BrowserWindow;
typedef struct _BrowserWindowClass  BrowserWindowClass;

struct _BrowserWindow
{
	GtkWindow parent;

	GtkWidget *entry;
	GtkItemFactory *item_factory;
	GtkWidget *vbox;

	GtkWidget *status_bar;

	HtmlDocument *doc;
	HtmlView *view;
};

struct _BrowserWindowClass
{
	GtkWindowClass parent_class;
};


GtkType    browser_window_get_type (void);
GtkWidget *browser_window_new      (HtmlDocument *doc);

#endif /* __BROWSER_WINDOW_H__ */
