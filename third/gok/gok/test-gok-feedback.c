/*
* test-gok-feedback.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include "gok-feedback.h"
#include "gok-log.h"

GokFeedback *new_feedback1, *new_feedback2, *new_feedback3, *new_feedback4;

void setup ();
void teardown ();
void test_gok_feedback_get_key ();
void test_add ();
void test_find ();
void test_delete1 ();
void test_delete2a ();
void test_delete2b ();
void test_delete3 ();
void test_storage_and_retrieval ();
void test_unsetting_in_gconf ();

gint main (gint argc, gchar* argv[])
{
    gnome_init ("test-gok-feedback", "0.1", argc, argv);
    test_gok_feedback_get_key ();
	/* remove until bug 129391 is resolved
    test_add ();
    test_find ();
    test_delete1 ();
    test_delete2a ();
    test_delete2b ();
    test_delete3 ();
    test_storage_and_retrieval ();
    test_unsetting_in_gconf ();
	*/
    exit (0);
}

gboolean gok_main_safe_mode ()
{
	return FALSE;
}

void setup ()
{
    GokFeedback *feedback;

    gok_log_enter ();

    g_assert ( TRUE == gok_feedback_open () );

    /*
     * We are going to start by deleting all of the feedbacks. We will
     * do this by deleting the first feedback until it is NULL. This
     * assumes that gok_feedback_delete_feedback works correctly on the
     * first feedback.
     */

    gok_log ("Deleting all of the feedbacks");

    feedback = gok_feedback_get_first_feedback ();
    while (feedback != NULL)
    {
        gok_feedback_delete_feedback (feedback);
        feedback = gok_feedback_get_first_feedback ();
    }

    g_assert (NULL == gok_feedback_get_first_feedback ());

    gok_log ("Done deleting all of the feedbacks");

    /* We will now setup some feedbacks for use in testing */

    /* testfeedback1 will contain the default values */
    new_feedback1 = gok_feedback_new ();
    new_feedback1->pName = g_strdup ("testfeedback1");

    /* testfeedback2 has 2 flashes and no sound */
    new_feedback2 = gok_feedback_new ();
    new_feedback2->pName = g_strdup ("testfeedback2");
    new_feedback2->pDisplayName = g_strdup ("displayname for testfeedback2");
    new_feedback2->bFlashOn = TRUE;
    new_feedback2->NumberFlashes = 2;
    new_feedback2->bSoundOn = FALSE;

    /* testfeedback3 has no flashes but does have sound */
    new_feedback3 = gok_feedback_new ();
    new_feedback3->pName = g_strdup ("testfeedback3");
    new_feedback3->pDisplayName = g_strdup ("testfeedback3 displayname");
    new_feedback3->bFlashOn = FALSE;
    new_feedback3->bSoundOn = TRUE;
    new_feedback3->pNameSound = g_strdup ("/foo/bar/baz.wav");

    /* testfeedback4 has 1000 flashes and sound */
    new_feedback4 = gok_feedback_new ();
    new_feedback4->pName = g_strdup ("testfeedback4");
    new_feedback4->pDisplayName = g_strdup ("foo");
    new_feedback4->bFlashOn = TRUE;
    new_feedback4->NumberFlashes = 1000;
    new_feedback4->bSoundOn = TRUE;
    new_feedback4->pNameSound = g_strdup ("p/q/r/s");

    gok_log_leave ();
}

void teardown()
{
    GokFeedback *feedback;

    gok_log_enter ();

    /* Delete all of the feedbacks and close */
    
    feedback = gok_feedback_get_first_feedback ();
    while (feedback != NULL)
    {
        gok_feedback_delete_feedback (feedback);
        feedback = gok_feedback_get_first_feedback ();
    }

    g_assert (NULL == gok_feedback_get_first_feedback ());

    gok_feedback_close ();

    gok_log_leave ();
}

gchar* gok_feedback_get_key (const gchar* feedback_name,
                             const gchar* attribute);

void test_gok_feedback_get_key ()
{
    gchar* expected;
    gchar* got;

    gok_log_enter ();

    expected = "/apps/gok/feedbacks/foo/bar";
    got = gok_feedback_get_key ("foo", "bar");

    g_assert (strcmp (expected, got) == 0);

    gok_log_leave();
	
	g_free (got);
}

void test_add ()
{
    GokFeedback *feedback;

    gok_log_enter ();
    
    setup ();

    /*
     * We are now going to add a feedback and verify that it is the new
     * first feedback
     */

    gok_feedback_add_feedback (new_feedback1);
    g_assert (new_feedback1 == gok_feedback_get_first_feedback ());

    /*
     * We are now going to add a second feedback and verify that it is
     * the new second feedback. And that the first feedback is still
     * correct.
     */

    gok_feedback_add_feedback (new_feedback2);
    feedback = gok_feedback_get_first_feedback ();
    g_assert (new_feedback1 == feedback);
    g_assert ( new_feedback2 == (feedback->pFeedbackNext) );

    /*
     * We are now going to add a third feedback and verify that it is
     * the new third feedback. And that the first and second feedbacks are
     * still correct.
     */

    gok_feedback_add_feedback (new_feedback3);
    feedback = gok_feedback_get_first_feedback ();
    g_assert (new_feedback1 == feedback);
    feedback = feedback->pFeedbackNext;
    g_assert (new_feedback2 == feedback);
    g_assert ( new_feedback3 == (feedback->pFeedbackNext) );

    teardown ();

    gok_log_leave ();
}

void test_find ()
{
    gok_log_enter ();
    setup ();

    /*
     * We are now going to add a feedback and verify that it is found
     */

    gok_feedback_add_feedback (new_feedback1);
    g_assert (new_feedback1 == gok_feedback_find_feedback("testfeedback1", FALSE));

    /*
     * We will now add a second feedback and verify that our two feedbacks
     * can be found
     */

    gok_feedback_add_feedback (new_feedback2);
    g_assert (new_feedback1 == gok_feedback_find_feedback("testfeedback1", FALSE));
    g_assert (new_feedback2 == gok_feedback_find_feedback("testfeedback2", FALSE));

    /*
     * We will now add a third feedback and verify that our three
     * feedbacks can be found
     */

    gok_feedback_add_feedback (new_feedback3);
    g_assert (new_feedback1 == gok_feedback_find_feedback("testfeedback1", FALSE));
    g_assert (new_feedback2 == gok_feedback_find_feedback("testfeedback2", FALSE));
    g_assert (new_feedback3 == gok_feedback_find_feedback("testfeedback3", FALSE));

    teardown ();
	gok_log_leave();
}

void test_delete1 ()
{
    gok_log_enter ();
    setup ();

    gok_feedback_add_feedback (new_feedback1);
    g_assert (new_feedback1 == gok_feedback_get_first_feedback());
    gok_feedback_delete_feedback (new_feedback1);
    g_assert (NULL == gok_feedback_get_first_feedback());

    teardown ();
    gok_log_leave ();
}

void test_delete2a ()
{
    gok_log_enter ();
    setup ();
    
    gok_feedback_add_feedback (new_feedback1);
    gok_feedback_add_feedback (new_feedback2);
    gok_feedback_delete_feedback (new_feedback1);
    g_assert (new_feedback2 == gok_feedback_get_first_feedback ());
    g_assert (NULL == new_feedback2->pFeedbackNext);

    teardown ();
    gok_log_leave ();
}

void test_delete2b ()
{
    gok_log_enter ();
    setup ();
    
    gok_feedback_add_feedback (new_feedback1);
    gok_feedback_add_feedback (new_feedback2);
    gok_feedback_delete_feedback (new_feedback2);
    g_assert (new_feedback1 == gok_feedback_get_first_feedback ());
    g_assert (NULL == new_feedback1->pFeedbackNext);

    teardown ();
    gok_log_leave ();
}

void test_delete3 ()
{
    gok_log_enter ();
    setup ();
    
    gok_feedback_add_feedback (new_feedback1);
    gok_feedback_add_feedback (new_feedback2);
    gok_feedback_add_feedback (new_feedback3);
    gok_feedback_delete_feedback (new_feedback2);
    g_assert (new_feedback1 == gok_feedback_get_first_feedback ());
    g_assert (new_feedback3 == new_feedback1->pFeedbackNext);

    teardown ();
    gok_log_leave ();
}

void test_storage_and_retrieval()
{
    GokFeedback *got_feedback;

    gok_log_enter ();
    setup ();

    /*
     * Add the test feedbacks and gok_feedback_close so that they are
     * written to GConf
     */

    gok_feedback_add_feedback (new_feedback1);
    gok_feedback_add_feedback (new_feedback2);
    gok_feedback_add_feedback (new_feedback3);
    gok_feedback_add_feedback (new_feedback4);
    gok_feedback_close ();

    /*
     * gok_feedback_open so that the feedbacks are read from GConf and test
     * that they have the attributes that we expect
     */

    got_feedback = NULL;
    g_assert ( TRUE == gok_feedback_open () );

    got_feedback = gok_feedback_find_feedback ("testfeedback1", FALSE);
    g_assert (got_feedback != NULL);
    g_assert (got_feedback->bPermanent == FALSE);
    g_assert (strcmp ("testfeedback1", got_feedback->pName) == 0);
    g_assert (strcmp ("testfeedback1", got_feedback->pDisplayName) == 0);
    g_assert (got_feedback->bFlashOn == TRUE);
    g_assert (got_feedback->NumberFlashes == 4);
    g_assert (got_feedback->bSoundOn == FALSE);
    g_assert (got_feedback->pNameSound == NULL);
	gok_feedback_delete_feedback (got_feedback);

    got_feedback = gok_feedback_find_feedback ("testfeedback2", FALSE);
    g_assert (got_feedback != NULL);
    g_assert (got_feedback->bPermanent == FALSE);
    g_assert (strcmp ("testfeedback2", got_feedback->pName) == 0);
    g_assert (strcmp ("displayname for testfeedback2",
                      got_feedback->pDisplayName) == 0);
    g_assert (got_feedback->bFlashOn == TRUE);
    g_assert (got_feedback->NumberFlashes == 2);
    g_assert (got_feedback->bSoundOn == FALSE);
    g_assert (got_feedback->pNameSound == NULL);
	gok_feedback_delete_feedback (got_feedback);

    got_feedback = gok_feedback_find_feedback ("testfeedback3", FALSE);
    g_assert (got_feedback != NULL);
    g_assert (got_feedback->bPermanent == FALSE);
    g_assert (strcmp ("testfeedback3", got_feedback->pName) == 0);
    g_assert (strcmp ("testfeedback3 displayname",
                      got_feedback->pDisplayName) == 0);
    g_assert (got_feedback->bFlashOn == FALSE);
    g_assert (got_feedback->NumberFlashes == 4);
    g_assert (got_feedback->bSoundOn == TRUE);
    g_assert (strcmp ("/foo/bar/baz.wav", got_feedback->pNameSound) == 0);
	gok_feedback_delete_feedback (got_feedback);

    got_feedback = gok_feedback_find_feedback ("testfeedback4", FALSE);
    g_assert (got_feedback != NULL);
    g_assert (got_feedback->bPermanent == FALSE);
    g_assert (strcmp ("testfeedback4", got_feedback->pName) == 0);
    g_assert (strcmp ("foo", got_feedback->pDisplayName) == 0);
    g_assert (got_feedback->bFlashOn == TRUE);
    g_assert (got_feedback->NumberFlashes == 1000);
    g_assert (got_feedback->bSoundOn == TRUE);
    g_assert (strcmp ("p/q/r/s", got_feedback->pNameSound) == 0);
	gok_feedback_delete_feedback (got_feedback);

	teardown ();
    gok_log_leave ();
}

void test_unsetting_in_gconf ()
{
    GokFeedback* feedback;

    gok_log_enter ();
    setup ();

    /* Add new_feedback1 and verify that it is stored in memory */
    gok_feedback_add_feedback (new_feedback1);
    g_assert (new_feedback1 == gok_feedback_find_feedback ("testfeedback1", FALSE));

    /* Use gok_feedback_close to make sure that new_feedback1 is written
     * to GConf
     */
    gok_feedback_close ();

    /* Use gok_feedback_open to read in from GConf and verify that
     * testfeedback1 is there
     */
    g_assert ( TRUE == gok_feedback_open () );
    feedback = gok_feedback_find_feedback ("testfeedback1", FALSE);
    g_assert (feedback != NULL);

    gok_feedback_delete_feedback (feedback);

    /* Use gok_feedback_close to update GConf */
    gok_feedback_close ();

    /* Use gok_feedback_open to read in from GConf and verify that
     * testfeedback1 is not there
     */
    g_assert ( TRUE == gok_feedback_open () );
    g_assert (gok_feedback_find_feedback ("testfeedback1", FALSE) == NULL);

    teardown ();
    gok_log_leave ();
}
