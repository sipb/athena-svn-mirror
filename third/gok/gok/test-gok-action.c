/*
* test-gok-action.c
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
#include "gok-action.h"
#include "gok-log.h"

GokAction *new_action1, *new_action2, *new_action3;

void setup ();
void teardown ();
void test_gok_action_get_key ();
void test_add ();
void test_find ();
void test_delete1 ();
void test_delete2a ();
void test_delete2b ();
void test_delete3 ();
void test_storage_and_retrieval_typical_action ();
void test_storage_and_retrieval_types ();
void test_storage_and_retrieval_states ();
void test_storage_and_retrieval_permanent ();
void test_storage_and_retrieval_key_averaging ();
void test_unsetting_in_gconf ();

gint main (gint argc, gchar* argv[])
{
    gnome_init ("test-gok-action", "0.1", argc, argv);

    test_gok_action_get_key ();
	/* remove until bug 129391 is resolved
    test_add ();
    test_find ();
    test_delete1 ();
    test_delete2a ();
    test_delete2b ();
    test_delete3 ();
    test_storage_and_retrieval_typical_action ();
    test_storage_and_retrieval_types ();
    test_storage_and_retrieval_states ();
    test_storage_and_retrieval_permanent ();
    test_storage_and_retrieval_key_averaging ();
	*/
    /* TODO: test storage and retrieval when pDisplayName = NULL */
    /* TODO: test storage and retrieval of values of number */
    /* TODO: test storage and retrieval of values of rate */
    /* TODO: test modifying a permanent action */
	/* remove until bug 129391 is resolved
    test_unsetting_in_gconf ();
	*/

    exit (0);
}

void setup ()
{
    GokAction *action;

    gok_log_enter ();

    g_assert ( TRUE == gok_action_open () );

    /*
     * We are going to start by deleting all of the actions. We will
     * do this by deleting the first action until it is NULL. This
     * assumes that gok_action_delete_action works correctly on the
     * first action.
     */

    gok_log ("Deleting all of the actions");

    action = gok_action_get_first_action ();
    while (action != NULL)
    {
        gok_action_delete_action (action);
        action = gok_action_get_first_action ();
    }

    g_assert (NULL == gok_action_get_first_action ());

    gok_log ("Done deleting all of the actions");

    /* We will now setup some actions for use in testing */

    new_action1 = gok_action_new ();
    new_action1->pName = g_strdup ("testaction1");
    new_action2 = gok_action_new ();
    new_action2->pName = g_strdup ("testaction2");
    new_action3 = gok_action_new ();
    new_action3->pName = g_strdup ("testaction3");

    gok_log_leave ();
}

void teardown()
{
    GokAction *action;

    gok_log_enter ();

    /* Delete all of the actions and close */
    
    action = gok_action_get_first_action ();
    while (action != NULL)
    {
        gok_action_delete_action (action);
        action = gok_action_get_first_action ();
    }

    g_assert (NULL == gok_action_get_first_action ());

    gok_action_close ();

    gok_log_leave ();
}

gchar* gok_action_get_key (const gchar* action_name,
                           const gchar* attribute);

void test_gok_action_get_key ()
{
    gchar* expected;
    gchar* got;

    expected = "/apps/gok/actions/foo/bar";
    got = gok_action_get_key ("foo", "bar");

    g_assert (strcmp (expected, got) == 0);

    g_free (got);
}

void test_add ()
{
    GokAction *action;

    gok_log_enter ();
    
    setup ();

    /*
     * We are now going to add an action and verify that it is the new
     * first action
     */

    gok_action_add_action (new_action1);
    g_assert (new_action1 == gok_action_get_first_action ());

    /*
     * We are now going to add a second action and verify that it is
     * the new second action. And that the first action is still
     * correct.
     */

    gok_action_add_action (new_action2);
    action = gok_action_get_first_action ();
    g_assert (new_action1 == action);
    g_assert ( new_action2 == (action->pActionNext) );

    /*
     * We are now going to add a third action and verify that it is
     * the new third action. And that the first and second actions are
     * still correct.
     */

    gok_action_add_action (new_action3);
    action = gok_action_get_first_action ();
    g_assert (new_action1 == action);
    action = action->pActionNext;
    g_assert (new_action2 == action);
    g_assert ( new_action3 == (action->pActionNext) );

    teardown ();

    gok_log_leave ();
}

void test_find ()
{
    setup ();

    /*
     * We are now going to add an action and verify that it is found
     */

    gok_action_add_action (new_action1);
    g_assert (new_action1 == gok_action_find_action("testaction1", FALSE));

    /*
     * We will now add a second action and verify that our two actions
     * can be found
     */

    gok_action_add_action (new_action2);
    g_assert (new_action1 == gok_action_find_action("testaction1", FALSE));
    g_assert (new_action2 == gok_action_find_action("testaction2", FALSE));

    /*
     * We will now add a third action and verify that our three
     * actions can be found
     */

    gok_action_add_action (new_action3);
    g_assert (new_action1 == gok_action_find_action("testaction1", FALSE));
    g_assert (new_action2 == gok_action_find_action("testaction2", FALSE));
    g_assert (new_action3 == gok_action_find_action("testaction3", FALSE));

    teardown ();
}

void test_delete1 ()
{
    setup ();

    gok_action_add_action (new_action1);
    g_assert (new_action1 == gok_action_get_first_action());
    gok_action_delete_action (new_action1);
    g_assert (NULL == gok_action_get_first_action());

    teardown ();
}

void test_delete2a ()
{
    setup ();
    
    gok_action_add_action (new_action1);
    gok_action_add_action (new_action2);
    gok_action_delete_action (new_action1);
    g_assert (new_action2 == gok_action_get_first_action ());
    g_assert (NULL == new_action2->pActionNext);

    teardown ();
}

void test_delete2b ()
{
    setup ();
    
    gok_action_add_action (new_action1);
    gok_action_add_action (new_action2);
    gok_action_delete_action (new_action2);
    g_assert (new_action1 == gok_action_get_first_action ());
    g_assert (NULL == new_action1->pActionNext);

    teardown ();
}

void test_delete3 ()
{
    setup ();
    
    gok_action_add_action (new_action1);
    gok_action_add_action (new_action2);
    gok_action_add_action (new_action3);
    gok_action_delete_action (new_action2);
    g_assert (new_action1 == gok_action_get_first_action ());
    g_assert (new_action3 == new_action1->pActionNext);

    teardown ();
}

void test_storage_and_retrieval_typical_action ()
{
    GokAction *got_action;
    GokAction *new_action;

    setup ();

    /* Build a new action */

    new_action = gok_action_new ();
    new_action->pName = g_strdup ("testaction");
    new_action->pNameBackup = NULL;
    new_action->pDisplayName = g_strdup ("displayname for testaction");
    new_action->pDisplayNameBackup = NULL;
    new_action->Type = ACTION_TYPE_SWITCH;
    new_action->TypeBackup = new_action->Type;
    new_action->State = ACTION_STATE_PRESS;
    new_action->StateBackup = new_action->State;
    new_action->Number = 1;
    new_action->NumberBackup = new_action->Number;
    new_action->Rate = 1;
    new_action->RateBackup = new_action->Rate;
    new_action->bPermanent = FALSE;
    new_action->bNewAction = TRUE;
    new_action->bKeyAveraging = FALSE;
    new_action->pActionNext = NULL;

    /*
     * Add the new action and gok_action_close so that the action is
     * written to GConf
     */

    gok_action_add_action (new_action);
    gok_action_close ();

    /*
     * gok_action_open so that the action is read from GConf and test
     * that it contains what we want it to
     */

    got_action = NULL;
    g_assert ( TRUE == gok_action_open () );
    got_action = gok_action_find_action ("testaction", FALSE);

    g_assert (got_action != NULL);
    g_assert (strcmp ("testaction", got_action->pName) == 0);
    g_assert (strcmp ("displayname for testaction",
                      got_action->pDisplayName) == 0);
    g_assert (got_action->Type == ACTION_TYPE_SWITCH);
    g_assert (got_action->State == ACTION_STATE_PRESS);
    g_assert (got_action->Number == 1);
    g_assert (got_action->Rate == 1);
    g_assert (got_action->bPermanent == FALSE);
    g_assert (got_action->bKeyAveraging ==  FALSE);

    teardown ();
}

void test_storage_and_retrieval_types ()
{
    GokAction *got_action;
    GokAction *new_action;
    gint types[]  = { ACTION_TYPE_SWITCH,        ACTION_TYPE_MOUSEBUTTON,
                      ACTION_TYPE_MOUSEPOINTER,  ACTION_TYPE_DWELL };
    gint type_counter;

    for (type_counter = 0; type_counter < 4; type_counter++)
    {
        setup ();

        gok_log ("type_counter = %d", type_counter);

        /* Build a new action */

        new_action = gok_action_new ();
        new_action->pName = g_strdup ("testaction");
        new_action->pNameBackup = NULL;
        new_action->Type = types[type_counter];
        new_action->TypeBackup = new_action->Type;
                    
        /*
         * Add the new action and gok_action_close so that the action
         * is written to GConf
         */

        gok_action_add_action (new_action);
        gok_action_close ();

        /*
         * gok_action_open so that the action is read from GConf and
         * test that it contains what we want it to
         */

        got_action = NULL;
        g_assert ( TRUE == gok_action_open () );
        got_action = gok_action_find_action ("testaction", FALSE);

        g_assert (got_action != NULL);
        g_assert (strcmp ("testaction", got_action->pName) == 0);
                    
        g_assert (got_action->Type == types[type_counter]);

        teardown ();
    }
}

void test_storage_and_retrieval_states ()
{
    GokAction *got_action;
    GokAction *new_action;
    gint states[] = { ACTION_STATE_UNDEFINED,   ACTION_STATE_PRESS,
                      ACTION_STATE_RELEASE,     ACTION_STATE_CLICK,
                      ACTION_STATE_DOUBLECLICK, ACTION_STATE_ENTER,
                      ACTION_STATE_LEAVE };
    gint state_counter;

    for (state_counter = 0; state_counter < 7; state_counter++)
    {
        setup ();

        gok_log ("state_counter = %d", state_counter);

        /* Build a new action */

        new_action = gok_action_new ();
        new_action->pName = g_strdup ("testaction");
        new_action->pNameBackup = NULL;
        new_action->State = states[state_counter];
        new_action->StateBackup = new_action->State;
                    
        /*
         * Add the new action and gok_action_close so that the action
         * is written to GConf
         */

        gok_action_add_action (new_action);
        gok_action_close ();

        /*
         * gok_action_open so that the action is read from GConf and
         * test that it contains what we want it to
         */

        got_action = NULL;
        g_assert ( TRUE == gok_action_open () );
        got_action = gok_action_find_action ("testaction", FALSE);

        g_assert (got_action != NULL);
        g_assert (strcmp ("testaction", got_action->pName) == 0);
        g_assert (got_action->State == states[state_counter]);

        teardown ();
    }
}

void test_storage_and_retrieval_permanent ()
{
    GokAction *got_action;
    GokAction *new_action;
    gint permanent_counter;
    gboolean booleans[] = { FALSE, TRUE };

    for (permanent_counter = 0; permanent_counter < 2;
         permanent_counter++)
    {
    
        setup ();

        gok_log ("permanent_counter = %d", permanent_counter);

        /* Build a new action */

        new_action = gok_action_new ();
        new_action->pName = g_strdup ("testaction");
        new_action->pNameBackup = NULL;
        new_action->bPermanent = booleans[permanent_counter];
                    
        /*
         * Add the new action and gok_action_close so that the action
         * is written to GConf
         */

        gok_action_add_action (new_action);
        gok_action_close ();

        /*
         * gok_action_open so that the action is read from GConf and
         * test that it contains what we want it to
         */

        got_action = NULL;
        g_assert ( TRUE == gok_action_open () );
        got_action = gok_action_find_action ("testaction", FALSE);

        g_assert (got_action != NULL);
        g_assert (strcmp ("testaction", got_action->pName) == 0);
        g_assert (got_action->bPermanent == booleans[permanent_counter]);

        teardown ();
    }
}

void test_storage_and_retrieval_key_averaging ()
{
    GokAction *got_action;
    GokAction *new_action;
    gint key_averaging_counter;
    gboolean booleans[] = { FALSE, TRUE };

    for (key_averaging_counter = 0; key_averaging_counter < 2;
         key_averaging_counter++)
    {
        setup ();

        gok_log ("key_averaging_counter = %d", key_averaging_counter);

        /* Build a new action */

        new_action = gok_action_new ();
        new_action->pName = g_strdup ("testaction");
        new_action->pNameBackup = NULL;
        new_action->bKeyAveraging = booleans[key_averaging_counter];
                    
        /*
         * Add the new action and gok_action_close so that the action
         * is written to GConf
         */

        gok_action_add_action (new_action);
        gok_action_close ();

        /*
         * gok_action_open so that the action is read from GConf and
         * test that it contains what we want it to
         */

        got_action = NULL;
        g_assert ( TRUE == gok_action_open () );
        got_action = gok_action_find_action ("testaction", FALSE);

        g_assert (got_action != NULL);
        g_assert (strcmp ("testaction", got_action->pName) == 0);
        g_assert (got_action->bKeyAveraging ==  booleans[key_averaging_counter]);
        teardown ();
    }
}

void test_unsetting_in_gconf ()
{
    GokAction* action;

    gok_log_enter ();
    setup ();

    /* Add new_action1 and verify that it is stored in memory */
    gok_action_add_action (new_action1);
    g_assert (new_action1 == gok_action_find_action ("testaction1", FALSE));

    /* Use gok_action_close to make sure that new_action1 is written
     * to GConf
     */
    gok_action_close ();

    /* Use gok_action_open to read in from GConf and verify that
     * testaction1 is there
     */
    g_assert ( TRUE == gok_action_open () );
    action = gok_action_find_action ("testaction1", FALSE);
    g_assert (action != NULL);

    gok_action_delete_action (action);

    /* Use gok_action_close to update GConf */
    gok_action_close ();

    /* Use gok_action_open to read in from GConf and verify that
     * testaction1 is not there
     */
    g_assert ( TRUE == gok_action_open () );
    g_assert (gok_action_find_action ("testaction1", FALSE) == NULL);

    teardown ();
    gok_log_leave ();
}
