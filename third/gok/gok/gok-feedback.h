/* gok-feedback.h
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
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

#ifndef __GOKFEEDBACK_H__
#define __GOKFEEDBACK_H__

#include "gok-key.h"
#include <gnome.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
If you add data members to this structure, initialize them in 
gok_feedback_new
*/
typedef struct GokFeedback{
	gboolean bPermanent;
	gboolean bNewFeedback;
	
	gchar* pName;
	gchar* pNameBackup;
	gchar* pDisplayName;
	gchar* pDisplayNameBackup;
	gboolean bFlashOn;
	gboolean bFlashOnBackup;
	gint NumberFlashes;
	gint NumberFlashesBackup;
	gboolean bSoundOn;
	gboolean bSoundOnBackup;
	gchar* pNameSound;
	gchar* pNameSoundBackup;
	gboolean bSpeechOn;
	gboolean bSpeechOnBackup;
	
	struct GokFeedback* pFeedbackNext;
} GokFeedback;

GokFeedback* gok_feedback_new (void);
void gok_feedback_free (GokFeedback* pFeedback);
gboolean gok_feedback_open (void);
void gok_feedback_close (void);
GokFeedback* gok_feedback_get_first_feedback (void);
GokFeedback* gok_feedback_find_feedback (gchar* NameFeedback, gboolean bDisplayName);
void gok_feedback_add_feedback (GokFeedback* pFeedbackNew);
void gok_feedback_delete_feedback (GokFeedback* pFeedbackDelete);
void gok_feedback_backup (GokFeedback* pFeedback);
gboolean gok_feedback_revert (GokFeedback* pFeedback);
gint gok_feedback_perform_effect (gchar* pNameFeedback);
gchar* gok_feedback_get_name (gchar* pDisplayName);
gchar* gok_feedback_get_displayname (gchar* pName);
void gok_feedback_timer_start_key_flash (gint NumberFlashes);
void gok_feedback_timer_stop_key_flash (void);
gboolean gok_feedback_timer_on_key_flash (gpointer data);
void gok_feedback_unhighlight (GokKey* pGokKey, gboolean bFlash);
void gok_feedback_highlight (GokKey* pGokKey, gboolean bFlash);
GokKey* gok_feedback_get_selected_key(void);
GokKey* gok_feedback_get_highlighted_key(void);
void gok_feedback_set_selected_key (GokKey* pKey);
void gok_feedback_set_highlighted_key (GokKey* pKey);
void gok_feedback_drop_refs (GokKey *pKey);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKFEEDBACK_H__ */
