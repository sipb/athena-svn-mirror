/*
* test-gok-wordcomplete.c
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto
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

#include <gnome.h>
#include "gok-word-complete.h"

#define PREDICTIONS_PER_INVOCATION 5
#define NUMBER_OF_PREDICTIONS 1500

gint main (gint argc, gchar* argv[])
{
    GokWordComplete *complete;
    gint i, j, k, n = 0;
    gchar *string, *s;
    GError *error = NULL;
    GTimer *timer = g_timer_new ();
    gdouble sec;
    gulong usec;

    gnome_init ("test-gok-wordcomplete", "0.1", argc, argv);

    complete = gok_wordcomplete_get_default ();
    gok_wordcomplete_set_aux_dictionaries (complete, "/usr/share/dict/words");

    g_timer_start (timer);
    g_assert (gok_wordcomplete_open (complete, "."));
    sec = g_timer_elapsed (timer, &usec);
    fprintf (stderr, "Word Completor opened in %f/sec\n", sec);

    g_file_get_contents ("../dictionary.txt", &s, NULL, &error);

    g_assert (error == NULL);

    /* strtok is evil, but probably OK for a test harness :-) */
    string = strtok (s, " \t\n\b.,!?"); /* "WPDictFile" */

    string = strtok (NULL, " \t\n\b.,!?"); /* first word */

    g_timer_start (timer);

    for (i = 0; (i < NUMBER_OF_PREDICTIONS) && string; ++i) 
    {
	    gchar **word_predictions;
	    k = 0;

	    do 
	    {
		    gchar strbuff[6];
		    strbuff [k+1] = '\0';
		    strbuff [k] = string [k];
		    ++n;

		    word_predictions = gok_wordcomplete_predict_string (complete, strbuff, 
									PREDICTIONS_PER_INVOCATION);
		    if (word_predictions)
		    {
			    for (j = 0; word_predictions[j]; ++j) 
			    {
				    g_free (word_predictions [j]); 
			    }
		    }
		    ++k;
	    } while (word_predictions && string[k] && k < 5);

	    string = strtok (NULL, " \n\t,.!?\b"); /* an int */
	    string = strtok (NULL, " \n\t,.!?\b"); /* another int */
	    string = strtok (NULL, " \n\t,.!?\b");
    }

    sec = g_timer_elapsed (timer, &usec);

    fprintf (stderr, "Made %d predictions (using %d prefixes) in %lf sec; %f/sec\n", n, i, sec, n/sec);

    return 0;
}
