/* -*- mode: c; c-basic-offset: 8 -*- */

/*
    This file is part of the GuileRepl library

    Copyright 2001 Ariel Rios <ariel@linuxppc.org>

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <gnome.h>
#include "gtkhtml.h"
#include "gtkhtml-stream.h"

#define BUTTON_INDEX 6

GtkWidget *html;

gchar *html_files [] = {"test1.html", "test2.html", "test9.html", "test11.html", "test6.html"};

static void
url_requested (GtkHTML *html, const char *url, GtkHTMLStream *stream, gpointer data)
{
	int fd;
	gchar *filename;

	filename = g_strconcat ("tests/", url, NULL);
	fd = open (filename, O_RDONLY);

	if (fd != -1) {
#define MY_BUF_SIZE 32768
		gchar *buf;
		size_t size;

		buf = alloca (MY_BUF_SIZE);
		while ((size = read (fd, buf, MY_BUF_SIZE)) > 0) {
			gtk_html_stream_write (stream, buf, size);
		}
		gtk_html_stream_close (stream, size == -1 ? GTK_HTML_STREAM_ERROR : GTK_HTML_STREAM_OK);
		close (fd);
	} else
		gtk_html_stream_close (stream, GTK_HTML_STREAM_ERROR);
	g_free (filename);
}

static void
read_html (GtkWidget *html, gint k)
{
	int fd, n;
	gchar *html_file = g_strconcat ("tests/", html_files [k], NULL);
	char ostr [BUFSIZ];
	fd = open (html_file, O_RDONLY);

	if (fd != -1) {
		n = read (fd, ostr, BUFSIZ);
		gtk_html_load_from_string (GTK_HTML (html), ostr, n);

		close (fd);
	}
	g_free (html_file);
}

static void
button_cb (GtkWidget *button, gpointer data)
{
	read_html (html, GPOINTER_TO_INT (data));
}

static void
quit_cb (GtkWidget *button)
{
	gtk_main_quit ();
}

int
main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *button [BUTTON_INDEX];
	GtkWidget *swindow;
	gchar *str []= {"Example 1", "Example 2", "Example 3", "Example 4", "Example 5", "Quit"};
	int i = 0;
	 
	gtk_init (&argc, &argv);
	gdk_rgb_init ();
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	vbox = gtk_vbox_new (FALSE, 0);
	hbox = gtk_hbox_new (FALSE, 0);
	swindow = gtk_scrolled_window_new (NULL, NULL);
	html = gtk_html_new ();
	gtk_signal_connect (GTK_OBJECT (html), "url_requested", GTK_SIGNAL_FUNC (url_requested), NULL);

	for (; i < BUTTON_INDEX; i++)
		button [i] = gtk_button_new_with_label (str [i]);
		
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_container_add (GTK_CONTAINER (swindow), html);
	gtk_box_pack_start (GTK_BOX (vbox), swindow, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	for (i = 0; i < BUTTON_INDEX; i++)
		gtk_box_pack_start (GTK_BOX (hbox), button [i], FALSE, FALSE, 0);

	gtk_window_set_title (GTK_WINDOW (window), "GtkHTML Test");
	gtk_widget_set_usize (GTK_WIDGET (swindow), 500, 500);

	for (i = 0; i < BUTTON_INDEX -1; i++)
		gtk_signal_connect (GTK_OBJECT (button [i]), "clicked", button_cb, GINT_TO_POINTER (i));
	gtk_signal_connect (GTK_OBJECT (button [BUTTON_INDEX - 1]), "clicked", quit_cb, NULL);
	
	gtk_widget_show_all (window);
		
	gtk_main ();

	return 0;

}

	