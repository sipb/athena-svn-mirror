/* gok-action.h
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

#ifndef __GOKACTION_H__
#define __GOKACTION_H__

#include <gnome.h>

/* different types of actions (keep these as bit values) */
#define ACTION_TYPE_SWITCH 1
#define ACTION_TYPE_MOUSEBUTTON 2
#define ACTION_TYPE_MOUSEPOINTER 4
#define ACTION_TYPE_DWELL 8
#define ACTION_TYPE_TIMER1 16
#define ACTION_TYPE_TIMER2 32
#define ACTION_TYPE_TIMER3 64
#define ACTION_TYPE_TIMER4 128
#define ACTION_TYPE_TIMER5 256
#define ACTION_TYPE_KEY    512
#define ACTION_TYPE_VALUECHANGE 1024

typedef enum {
ACTION_STATE_UNDEFINED,
ACTION_STATE_PRESS,
ACTION_STATE_RELEASE,
ACTION_STATE_CLICK,
ACTION_STATE_DOUBLECLICK,
ACTION_STATE_ENTER,
ACTION_STATE_LEAVE
} ActionState;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* If you add data members to this structure, initialize them in
 * gok_action_new
 */
typedef struct GokAction{
	gboolean bPermanent;
	gboolean bNewAction;
	gchar* pName;
	gchar* pNameBackup;
	gchar* pDisplayName;
	gchar* pDisplayNameBackup;
	gint Type;
	gint TypeBackup;
	gint State;
	gint StateBackup;
	gint Number;
	gint NumberBackup;
	gint Rate;
	gint RateBackup;
	gboolean bKeyAveraging;
	gboolean bKeyAveragingBackup;
	struct GokAction* pActionNext;
} GokAction;

GokAction* gok_action_new (void);
void gok_action_free (GokAction* action);

gboolean gok_action_open (void);
void gok_action_close (void);
GokAction* gok_action_get_first_action (void);
void gok_action_set_modified (gboolean bTrueFalse);
void gok_action_delete_action (GokAction* pActionDelete);
GokAction* gok_action_find_action (gchar* NameAction, gboolean bDisplayName);
void gok_action_add_action (GokAction* pActionNew);
void gok_action_backup (GokAction* pAction);
gboolean gok_action_revert (GokAction* pAction);
gchar* gok_action_get_name (gchar* pDisplayName);
gchar* gok_action_get_displayname (gchar* pName);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKACTION_H__ */
