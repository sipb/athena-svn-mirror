/* srspc.h
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
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

#ifndef _SRSPC_H_
#define _SRSPC_H_

#include <glib.h>
#include "srmain.h"
#include "libsrconf.h"

typedef enum
{
    SRC_SPEECH_SPELL_NORMAL,
    SRC_SPEECH_SPELL_CHAR_BY_CHAR,
    SRC_SPEECH_SPELL_MILITARY,
    SRC_SPEECH_SPELL_AUTO
}SRCSpeechSpellingMode;

typedef enum
{
    SRC_SPEECH_COUNT_NONE,
    SRC_SPEECH_COUNT_ALL,
    SRC_SPEECH_COUNT_AUTO
}SRCSpeechCountMode;

typedef enum
{
    SRC_SPEECH_PUNCTUATION_IGNORE,
    SRC_SPEECH_PUNCTUATION_SOME,
    SRC_SPEECH_PUNCTUATION_MOST,
    SRC_SPEECH_PUNCTUATION_ALL
}SRCSpeechPunctuationType;

typedef enum
{
    SRC_SPEECH_TEXT_ECHO_CHAR,
    SRC_SPEECH_TEXT_ECHO_WORD,
    SRC_SPEECH_TEXT_ECHO_NONE
}SRCSpeechTextEchoType;

typedef enum
{
    SRC_SPEECH_MODIFIERS_ECHO_NONE,
    SRC_SPEECH_MODIFIERS_ECHO_ALL
}SRCSpeechModifiersEchoType;

typedef enum
{
    SRC_SPEECH_CURSORS_ECHO_NONE,
    SRC_SPEECH_CURSORS_ECHO_ALL
}SRCSpeechCursorsEchoType;

typedef enum
{
    SRC_SPEECH_SPACES_ECHO_NONE,
    SRC_SPEECH_SPACES_ECHO_ALL
}SRCSpeechSpacesEchoType;

typedef enum
{
    SRC_SPEECH_DICTIONARY_NO,
    SRC_SPEECH_DICTIONARY_YES
}SRCSpeechDictionaryType;

typedef enum
{
    SRC_MODIF_INCREASE,
    SRC_MODIF_DECREASE,
    SRC_MODIF_DEFAULT
}SRCModifyMode;

typedef enum
{
    SRC_SPEECH_PRIORITY_IDLE,
    SRC_SPEECH_PRIORITY_MESSAGE,
    SRC_SPEECH_PRIORITY_SYSTEM,
    SRC_SPEECH_PRIORITY_WARNING,
    SRC_SPEECH_PRIORITY_ERROR,
}SRCSpeechPriority;

gboolean src_speech_init	  ();
void     src_speech_terminate	  ();
gboolean src_speech_send_chunk 	  (gchar *chunk, SRCSpeechPriority priority, gboolean preempt);
void     src_speech_shutup 	  ();
void     src_speech_pause 	  ();
void     src_speech_resume 	  ();
gboolean src_speech_say_message	  (gchar *message);
gboolean src_speech_init_voice 	  (gchar *voice);
gboolean src_speech_test_voice 	  (gchar *voice);
gboolean src_speech_modify_pitch  (SRCModifyMode mode);
gboolean src_speech_modify_rate   (SRCModifyMode mode);
gboolean src_speech_modify_volume (SRCModifyMode mode);

gboolean src_speech_set_spelling_mode	(SRCSpeechSpellingMode mode);
SRCSpeechSpellingMode src_speech_get_spelling_mode ();
gchar*   src_speech_process_string 	(const gchar *str, 
						    SRCSpeechCountMode cmode,
						    SRCSpeechSpellingMode smode);
gchar*   src_speech_get_text_voice_attributes 	(gchar *voice,
						    SRCSpeechSpellingMode smode);
gboolean src_speech_process_config_changed 	(SRConfigStructure *config);
gboolean src_speech_process_voice_config_changed (SRConfigStructure *config);

/*FIXME: next functions must be removed*/
gchar*   src_speech_get_voice 	(gchar *voice);
gboolean src_speech_say		(gchar *message, gboolean shutup);
gchar*   src_process_string 	(const gchar *str, SRCSpeechCountMode cmode,
						    SRCSpeechSpellingMode smode);
						    
SRCSpeechTextEchoType 		src_speech_get_text_echo_type 	   ();
SRCSpeechSpacesEchoType		src_speech_get_spaces_echo_type    ();
SRCSpeechModifiersEchoType	src_speech_get_modifiers_echo_type ();
SRCSpeechCursorsEchoType	src_speech_get_cursors_echo_type   ();

/*FIXME end */
gboolean src_speech_set_idle (gchar *ido);
#ifdef SRS_NO_MARKERS_SUPPORTED
gboolean src_speech_add_created_voice (gchar *name, gboolean has_callback);
#else
gboolean src_speech_add_created_voice (gchar *name);
#endif

typedef struct
{
    gchar *name;
    gchar **voices;
}SRSDriver;
SRSDriver*	srs_driver_new ();
gboolean src_speech_process_drivers (GPtrArray *drivers);

#endif /* _SRSPC_H_ */
