
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */

/*
 *                         gtk-text-buffer-addon.c  -  description 
 *                         ------------------------------------
 *   begin                : Sat Nov 29 2003, but based on older code
 *   copyright            : (C) 2000-2004 by Julien Puydt
 *                                           Miguel Rodríguez,
 *                                           Stéphane Wirtel
 *                                           Kenneth Christiansen
 *   description          : Add-on functions for regex-based context menus
 *
 */


#include "gtk-text-buffer-addon.h"

#include <regex.h>

struct regex_match
{
  const gchar *buffer;
  regoff_t start;
  regoff_t end;
  GtkTextTag *tag;
};

static void
find_match (GtkTextTag *tag, gpointer data)
{
  gint match;
  regmatch_t regmatch;
  struct regex_match *smatch = data;
  regex_t *regex = g_object_get_data (G_OBJECT(tag), "regex");

  if(regex != NULL) { // we are concerned only by the tags that use the regex addon!
    match = regexec (regex, smatch->buffer, 1, &regmatch, 0);
    if (!match) { // there's an uri that matches
      if (smatch->tag == NULL
	  || (smatch->tag != NULL
	      && regmatch.rm_so < smatch->start)) {
	smatch->start = regmatch.rm_so;
	smatch->end = regmatch.rm_eo;
	smatch->tag = tag;
      }
    }
  }
}

/**
 * gtk_text_buffer_insert_with_regex:
 * @buf: A pointer to a GtkTextBuffer
 * @bufiter: An iterator for the buffer
 * @text: A text string
 *
 * Inserts @text into the @buf, but with all matching regex shown
 * in a beautified way
 **/
void
gtk_text_buffer_insert_with_regex (GtkTextBuffer *buf,
				   GtkTextIter *bufiter,
				   const gchar *text)
{
  gchar *string = NULL;
  struct regex_match match;

  match.buffer = text;
  match.tag = NULL;
  gtk_text_tag_table_foreach (gtk_text_buffer_get_tag_table (buf),
			      find_match, &match);
  while (match.tag != NULL) { /* as long as there is an url to treat */
    RegexDisplayFunc func;

    /* if the match isn't at the beginning, we treat that beginning as simple text */
    if (match.start) {
      g_assert (match.start <= strlen (text));
      gtk_text_buffer_insert (buf, bufiter, text, match.start);
    }/*  */

    /* treat the uri we found */
    string = g_strndup (text + match.start,
			match.end - match.start);

    func = g_object_get_data (G_OBJECT(match.tag),
					       "regex-display");
    if (func == NULL) { /* this doesn't need any fancy stuff to be displayed */
      func = gtk_text_buffer_insert_plain;
    }
    (*func)(buf, bufiter, match.tag, string);
    g_free (string);
    /* the rest will be our new buffer on next loop */
    text += match.end;
    match.buffer = text;
    match.tag = NULL;
    gtk_text_tag_table_foreach (gtk_text_buffer_get_tag_table (buf),
				find_match, &match);
  }

  /* we treat what's left after we found all uris */
  gtk_text_buffer_insert (buf, bufiter, text, -1);
}

/**
 * gtk_text_buffer_insert_plain:
 * @buf: A pointer to a GtkTextBuffer
 * @iter: An iterator for the buffer
 * @tag: A pointer to a GtkTextTag
 * @text: A text string
 *
 * Inserts @text into the @buf, with the given tag, as simple text
 **/
void
gtk_text_buffer_insert_plain (GtkTextBuffer *buf,
			      GtkTextIter *iter,
			      GtkTextTag *tag,
			      const gchar *text)
{
  gtk_text_buffer_insert_with_tags (buf, iter, text, -1, tag, NULL);
}


/* here is the code used to display an emoticon/smiley, through the use
 * of a specialized function
 */

/* the smileys as pictures */
#include "../pixmaps/inline_emoticons.h"

/* needed to store the association of a string with an image */
struct _smiley
{
  const char *smile;
  const guint8 *picture;
};

/* the table that associates each smiley-as-a-text with a smiley-as-a-picture */
static const struct _smiley table_smiley [] =
  {
    {":)", gm_emoticon_face1},
    {"8)", gm_emoticon_face2},
    {"8-)", gm_emoticon_face2},
    {";)", gm_emoticon_face3},
    {";-)", gm_emoticon_face3},
    {":(", gm_emoticon_face4},
    {":-(", gm_emoticon_face4},
    {":O", gm_emoticon_face5},
    {":o", gm_emoticon_face5},
    {":-O", gm_emoticon_face5},
    {":-o", gm_emoticon_face5},
    {":-D", gm_emoticon_face6},
    {":D", gm_emoticon_face6},
    {":-)", gm_emoticon_face7},
    {":|", gm_emoticon_face8},
    {":-|", gm_emoticon_face8},
    {":-/", gm_emoticon_face9},
    {":/", gm_emoticon_face9},
    {":-P", gm_emoticon_face10},
    {":-p", gm_emoticon_face10},
    {":P", gm_emoticon_face10},
    {":p", gm_emoticon_face10},
    {":'(", gm_emoticon_face11},
    {":[", gm_emoticon_face12},
    {":-*", gm_emoticon_face13},
    {":-x", gm_emoticon_face14},
    {"B-)", gm_emoticon_face15},
    {"B)", gm_emoticon_face15},
    {"x*O", gm_emoticon_face16},
    {"(.)", gm_emoticon_face17},
    {"(|)", gm_emoticon_face18},
    {":-.", gm_emoticon_face19},
    {"X)", gm_emoticon_dead_happy},
    {"X|", gm_emoticon_dead},
    {"X(", gm_emoticon_dead_sad},
    {"}:)", gm_emoticon_happy_devil},
    {"|)", gm_emoticon_nose_glasses},
    {"}:(", gm_emoticon_sad_devil},
    {"|(", gm_emoticon_sad_glasses},
    {"|-(", gm_emoticon_sad_nose_glasses},
    {"|-)", gm_emoticon_happy_nose_glasses},
    {NULL}
  };

/**
 * gtk_text_buffer_insert_smiley:
 * @buf: A pointer to a GtkTextBuffer
 * @iter: An iterator for the buffer
 * @tag: A pointer to a GtkTextTag
 * @smile: A text string
 *
 * Inserts @smile into the @buf, with the given tag, as an image looked up in a table
 **/
void
gtk_text_buffer_insert_smiley (GtkTextBuffer *buf,
			       GtkTextIter *iter,
			       GtkTextTag *tag,
			       const gchar *smile)
{
  GdkPixbuf *pixbuf = NULL;
  const struct _smiley *tmp;
  for (tmp = table_smiley;
       tmp->smile != NULL && tmp->picture != NULL;
       tmp++) {
    if (strcmp (smile, tmp->smile) == 0) {
      pixbuf = gdk_pixbuf_new_from_inline (-1, tmp->picture, 
					   FALSE, NULL);
      break;
    }
  }
  if (pixbuf != NULL)
    gtk_text_buffer_insert_pixbuf (buf, iter, pixbuf);
  else { // the regex mis-matched!
      gtk_text_buffer_insert (buf, iter, smile, -1); 
      g_warning ("This: %s was erroneously seen as a smiley!", smile);
    }
}
