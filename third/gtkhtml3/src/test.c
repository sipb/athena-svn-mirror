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
#include <unistd.h>
#include <string.h>

#include <gtk/gtkwindow.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtksignal.h>

#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-ui-init.h>

#include "gtkhtml.h"
#include "gtkhtmldebug.h"
#include "gtkhtml-stream.h"

#include "htmlengine.h"

#define BUTTON_INDEX 6

GtkWidget *html;

gchar *html_files [] = {"test1.html", "test2.html", "test9.html", "test11.html", "test6.html"};

static gchar *welcome =
"Czech (&#268;e&#353;tina) &#268;au, Ahoj, Dobr&#253; den<BR>"
"French (Français) Bonjour, Salut<BR>"
"Korean (한글)   안녕하세요, 안녕하십니까<BR>"
"Russian (Русский) Здравствуйте!<BR>"
"Chinese (Simplified) <span lang=\"zh-cn\">元气	开发</span><BR>"
"Chinese (Traditional) <span lang=\"zh-tw\">元氣	開發</span><BR>"
"Japanese <span lang=\"ja\">元気	開発<BR></FONT>";

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
	GtkHTMLStream *stream;

	stream = gtk_html_begin (GTK_HTML (html));
	url_requested (GTK_HTML (html), html_files [k], stream, NULL);
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

static gchar *
encode_html (gchar *txt)
{
	GString *str;
	gchar *rv;

	str = g_string_new (NULL);

	do {
		gunichar uc;

		uc = g_utf8_get_char (txt);
		if (uc > 160) {
			g_string_append_printf (str, "&#%u;", uc);
		} else {
			g_string_append_c (str, uc);
		}
	} while ((txt = g_utf8_next_char (txt)) && *txt);

	rv = str->str;
	g_string_free (str, FALSE);

	return rv;
}

static void
dump_cb (GtkWidget *widget, gpointer data)
{
	g_print ("Object Tree\n");
	g_print ("-----------\n");

	gtk_html_debug_dump_tree (GTK_HTML (html)->engine->clue, 0);
}

static void
dump_simple_cb (GtkWidget *widget, gpointer data)
{
	g_print ("Simple Object Tree\n");
	g_print ("-----------\n");

	gtk_html_debug_dump_tree_simple (GTK_HTML (html)->engine->clue, 0);
}

int
main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *button [BUTTON_INDEX];
	GtkWidget *swindow;
	GtkWidget *debug;
	gchar *str []= {"Example 1", "Example 2", "Example 3", "Example 4", "Example 5", "Quit"};
	int i = 0;
	 
	gnome_program_init ("libgtkhtml test", "0.0", LIBGNOMEUI_MODULE, argc, argv, NULL);
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	vbox = gtk_vbox_new (FALSE, 0);
	hbox = gtk_hbox_new (FALSE, 0);
	swindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	html = gtk_html_new_from_string (encode_html (welcome), -1);
	g_signal_connect (html, "url_requested", G_CALLBACK (url_requested), NULL);

	for (; i < BUTTON_INDEX; i++)
		button [i] = gtk_button_new_with_label (str [i]);
		
	gtk_container_add (GTK_CONTAINER (window), vbox);
	gtk_container_add (GTK_CONTAINER (swindow), html);
	gtk_box_pack_start (GTK_BOX (vbox), swindow, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
	for (i = 0; i < BUTTON_INDEX; i++)
		gtk_box_pack_start (GTK_BOX (hbox), button [i], FALSE, FALSE, 0);

	debug = gtk_button_new_with_label ("Dump");
	gtk_box_pack_end (GTK_BOX (hbox), debug, FALSE, FALSE, 0);
	g_signal_connect (debug, "clicked", G_CALLBACK (dump_cb), NULL);
	debug = gtk_button_new_with_label ("Dump simple");
	gtk_box_pack_end (GTK_BOX (hbox), debug, FALSE, FALSE, 0);
	g_signal_connect (debug, "clicked", G_CALLBACK (dump_simple_cb), NULL);

	gtk_window_set_title (GTK_WINDOW (window), _("GtkHTML Test"));
	gtk_window_set_default_size (GTK_WINDOW (window), 500, 500);

	for (i = 0; i < BUTTON_INDEX -1; i++)
		g_signal_connect (button [i], "clicked", G_CALLBACK (button_cb), GINT_TO_POINTER (i));
	g_signal_connect (button [BUTTON_INDEX - 1], "clicked", G_CALLBACK (quit_cb), NULL);

	gtk_widget_show_all (window);
		
	gtk_main ();

	return 0;

}
