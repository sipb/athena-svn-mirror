/* gok-utf8-word-complete.c
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
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
*
*/

#include <string.h>
#include <gnome.h>
#include "gok-log.h"
#include "gok-sliding-window-word-complete.h"

/* implementations of GokWordCompleteClass virtual methods */

/* declarations of implementation methods */
static gchar** sw_wordcomplete_predict_string (GokWordComplete *complete, const gchar* pWord, gint num_predictions);
static const gchar* sw_wordcomplete_get_delimiter (GokWordComplete *complete);

/* 
 * This macro initializes GokUtf8WordComplete with the GType system 
 *   and defines ..._class_init and ..._instance_init functions 
 */
GNOME_CLASS_BOILERPLATE (GokSWWordComplete, gok_sw_wordcomplete,
			 GokUTF8WordComplete, GOK_TYPE_UTF8WORDCOMPLETE)

static void
gok_sw_wordcomplete_instance_init (GokSWWordComplete *complete)
{
    g_message ("created a sliding window word completion engine.\n");
}

static void
gok_sw_wordcomplete_class_init (GokSWWordCompleteClass *klass)
{
    GokWordCompleteClass *word_complete_class = (GokWordCompleteClass *) klass;
   
    parent_class = g_type_class_peek_parent (klass);

    word_complete_class->get_delimiter = sw_wordcomplete_get_delimiter;
    word_complete_class->predict_string = sw_wordcomplete_predict_string;
}

static gchar** sw_wordcomplete_predict_string (GokWordComplete *complete, const gchar* pWord, gint num_predictions)
{
    const gchar *cp;
    int len = g_utf8_strlen (pWord, -1);
    if (len > 1) 
	cp = g_utf8_offset_to_pointer (pWord, len - 1);
    else
	cp = pWord;
    return GOK_WORDCOMPLETE_CLASS (parent_class)->predict_string (complete, cp, num_predictions);
}

static const gchar* sw_wordcomplete_get_delimiter (GokWordComplete *complete)
{
    /* TODO: generalize for other locales */
    return "";
}
