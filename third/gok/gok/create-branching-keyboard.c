/* 
* Copyright 2004 Sun Microsystems, Inc.,
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <glib.h>

typedef struct
{
    GIOChannel *io;
    gint row;
    gint col;
    gint maxcol;
} KbdFile;

gint
read_branch_count (GIOChannel *infile)
{
    gchar *line;
    guint len;
    GError *error = NULL;
    gint count = 0;
    GIOStatus status;
    do
    {
	status = g_io_channel_read_line (infile, &line, &len, NULL, &error);
	if (status != G_IO_STATUS_NORMAL || len < 2 || 
	    g_str_has_prefix (line, "#")) 
	{
	    continue;
	}
	if (g_utf8_strchr (line, 8, ':')) 
	{
	    ++count;
	}
    } while ((status == G_IO_STATUS_NORMAL) || (status == G_IO_STATUS_AGAIN));
    return count;
}

static KbdFile*
kbd_file_create (gchar *name, gint maxcols)
{
    KbdFile *kbd;
    gchar *buf;
    gsize bytz, stored;
    gchar *fname, *utf8name;

    kbd = g_new0 (KbdFile, 1);
    kbd->row = kbd->col = 0;
    kbd->maxcol = maxcols;
    utf8name = g_strconcat (name, ".kbd", NULL);
    fname = g_filename_from_utf8 (utf8name, -1, &stored, &bytz, NULL);
    g_message ("[%d] creating %s", stored, fname);
    kbd->io = g_io_channel_new_file (fname, "w", NULL);
    buf = g_strconcat ("<?xml version=\"1.0\"?>\n",
		       "<GOK:GokFile xmlns:GOK=\"http://www.gnome.org/GOK\">\n\n",
		       "<GOK:keyboard name=\"", name, "\" wordcompletion=\"yes\">\n", NULL);
    g_io_channel_write_chars (kbd->io, buf, -1, &bytz, NULL);
    g_free (buf);
    buf = g_strconcat ("\t<GOK:key ");
    g_io_channel_write_chars (kbd->io, "\t<GOK:key left = \"0\" right = \"1\" top = \"0\" bottom = \"1\" type=\"branchBack\">\n", -1, &bytz, NULL);
    g_io_channel_write_chars (kbd->io, "\t\t<GOK:label type = \"branchBack\">\345\220\216\351\200\200</GOK:label>\n", -1, &bytz, NULL);
    g_io_channel_write_chars (kbd->io, "\t</GOK:key>\n", -1, &bytz, NULL);
    kbd->col = 1;
    g_free (fname);
    g_free (utf8name);
    return kbd;
}

static void
kbd_file_close (KbdFile *kbd)
{
    gchar *buf;
    gsize bytz;
    buf = g_strconcat ("</GOK:keyboard>\n</GOK:GokFile>\n", NULL);
    g_io_channel_write_chars (kbd->io, buf, -1, &bytz, NULL);
    g_io_channel_close (kbd->io);
}

static void
kbd_file_add_key (KbdFile *kbd, gunichar unichar, gchar *label, gchar *mod_label, gchar *branch_tag)
{
    gsize bytz;
    gchar *outputbuf, *tbuf;
    gchar buf[120];
    g_assert (kbd);
    g_assert (kbd->io);
    snprintf (buf, 120, "\" left=\"%d\" right=\"%d\" top=\"%d\" bottom=\"%d\">\n", kbd->col, kbd->col+1, kbd->row, kbd->row+1);
    kbd->col++;
    if (kbd->col > kbd->maxcol) 
    {
	++kbd->row;
	kbd->col = 0;
    }
    if (branch_tag)
    {
	g_io_channel_write_chars (kbd->io, "\t<GOK:key type=\"branch\" target=\"", -1, &bytz, NULL);
	g_io_channel_write_chars (kbd->io, branch_tag, -1, &bytz, NULL);
	g_io_channel_write_chars (kbd->io, buf, -1, &bytz, NULL);
    }
    else
    {
	g_io_channel_write_chars (kbd->io, "\t<GOK:key type=\"normal", -1, &bytz, NULL);
	g_io_channel_write_chars (kbd->io, buf, -1, &bytz, NULL);
    }
    tbuf = g_strconcat ("\t\t<GOK:label>", label, "</GOK:label>\n", NULL);
    g_io_channel_write_chars (kbd->io, tbuf, -1, &bytz, NULL);
    if (mod_label) 
    {
	tbuf = g_strconcat ("\t\t<GOK:label level=\"1\">", mod_label, "</GOK:label>\n", NULL);	
	g_io_channel_write_chars (kbd->io, tbuf, -1, &bytz, NULL);
    }
    if (!branch_tag)
    {
	gchar charbuf[10];
	snprintf (charbuf, 9, "U+%x", unichar);
	tbuf = g_strconcat ("\t\t<GOK:output type=\"keysym\">", charbuf, "</GOK:output>\n", NULL);	
	g_io_channel_write_chars (kbd->io, tbuf, -1, &bytz, NULL);
    }
    g_io_channel_write_chars (kbd->io, "\t</GOK:key>\n", -1, &bytz, NULL);
    g_free (tbuf);
}

static void
create_kbd_from_file (GIOChannel *infile, gchar *kbd_name, gint columns)
{
    gchar *line;
    guint len;
    GError *error = NULL;
    GIOStatus status;
    KbdFile *branch_kbd = kbd_file_create (kbd_name, columns);
    do 
    {
	char label_english[64];
	status = g_io_channel_read_line (infile, &line, &len, NULL, &error);
	if (status != G_IO_STATUS_NORMAL || len < 2 || 
	    g_str_has_prefix (line, "#")) 
	{
	    if (error != NULL) 
	    {
		g_message (error->message);
	    }
	    continue;
	}
	/* data line */
	if (g_utf8_strchr (line, 8, ':')) 
	{
	    gchar *chars = NULL, **tokens, cbuf[6];
	    tokens = g_strsplit (line, ":", 2); 
	    if (tokens && tokens[0]) chars = tokens[1];
	    {
		KbdFile *kbd;
		gchar dbuf[12];
		snprintf (dbuf, 11, "zh-U%x", g_utf8_get_char (tokens[0]));
		g_message ("English label: %s, line=%s", label_english, tokens[1]);
		kbd = kbd_file_create (dbuf, columns);
		kbd_file_add_key (branch_kbd, g_utf8_get_char (tokens[0]), tokens[0], label_english, dbuf);
		while (chars) 
		{
		    gunichar unichar;
		    unichar = g_utf8_get_char (chars);
		    if (!unichar || (*chars == '\n')) break;
		    printf ("%c [U+%4x]\t", unichar, unichar);
		    cbuf [g_unichar_to_utf8 (unichar, cbuf)] = '\0';
		    kbd_file_add_key (kbd, unichar, cbuf, NULL, NULL);
		    chars = g_utf8_next_char (chars);
		}
		kbd_file_close (kbd);
	    }
	    g_strfreev (tokens);
	}
	else 
	{
	    sscanf (line, "%63s", label_english);
	}
    } while ((status == G_IO_STATUS_NORMAL) || (status == G_IO_STATUS_AGAIN));
    kbd_file_close (branch_kbd);
}

int main (int argc, gchar **argv)
{
    GError *error;
    GIOChannel *infile;
    gint rows = 4;
    if (argc > 2)
    {
	rows = atoi (argv[2]);
    }
    else if (argc < 2 || !strcmp (argv[1], "--usage"))
    {
	printf ("usage: create-branching-keyboard <infile.data> [<n_desired_rows>]\n");
    }
    if (infile = g_io_channel_new_file (argv[1], "r", NULL))
    {
	gint cols;
	g_io_channel_set_line_term (infile, NULL, -1);
	cols = ceil ((read_branch_count (infile) + 1) / rows);
	g_message ("using %d columns", cols);
	g_io_channel_seek_position (infile, 0, G_SEEK_SET, NULL);
	create_kbd_from_file (infile, argv[1], cols);
    }
    else
	g_error ("can't open file %s for reading", argv[1]);
}
