/* gok-scanner.h
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

#ifndef __GOKSCANNER_H__
#define __GOKSCANNER_H__

#include "gok-keyboard.h"
#include "gok-control.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* all possible calls used in all access methods */
/* Don't change the order of these unless you also update ArrayCallNames */
typedef enum {
CALL_CHUNKER_RESET,
CALL_CHUNKER_CHUNK_NONE,
CALL_CHUNKER_CHUNK_KEYS,
CALL_CHUNKER_CHUNK_ROWS,
CALL_CHUNKER_CHUNK_COLUMNS,
CALL_CHUNKER_NEXT_CHUNK,
CALL_CHUNKER_PREVIOUS_CHUNK,
CALL_CHUNKER_NEXT_KEY,
CALL_CHUNKER_PREVIOUS_KEY,
CALL_CHUNKER_KEY_UP,
CALL_CHUNKER_KEY_DOWN,
CALL_CHUNKER_KEY_LEFT,
CALL_CHUNKER_KEY_RIGHT,
CALL_CHUNKER_KEY_HIGHLIGHT,
CALL_CHUNKER_KEY_UNHIGHLIGHT,
CALL_CHUNKER_WRAP_TOFIRST_CHUNK,
CALL_CHUNKER_WRAP_TOLAST_CHUNK,
CALL_CHUNKER_WRAP_TOFIRST_KEY,
CALL_CHUNKER_WRAP_TOLAST_KEY,
CALL_CHUNKER_WRAP_TOBOTTOM,
CALL_CHUNKER_WRAP_TOTOP,
CALL_CHUNKER_WRAP_TOLEFT,
CALL_CHUNKER_WRAP_TORIGHT,
CALL_CHUNKER_MOVE_LEFTRIGHT,
CALL_CHUNKER_MOVE_TOPBOTTOM,
CALL_CHUNKER_IF_NEXT_CHUNK,
CALL_CHUNKER_IF_PREVIOUS_CHUNK,
CALL_CHUNKER_IF_NEXT_KEY,
CALL_CHUNKER_IF_PREVIOUS_KEY,
CALL_CHUNKER_IF_TOP,
CALL_CHUNKER_IF_BOTTOM,
CALL_CHUNKER_IF_LEFT,
CALL_CHUNKER_IF_RIGHT,
CALL_CHUNKER_IF_KEY_SELECTED,
CALL_CHUNKER_HIGHLIGHT_CENTER,
CALL_CHUNKER_HIGHLIGHT_FIRST_CHUNK,
CALL_CHUNKER_HIGHLIGHT_FIRST_KEY,
CALL_CHUNKER_SELECT_CHUNK,
CALL_CHUNKER_HIGHLIGHT_CHUNK,
CALL_CHUNKER_UNHIGHLIGHT_ALL,


CALL_SCANNER_REPEAT_ON,
/* CALL_SCANNER_REPEAT_OFF,  use CALL_STATE_RESTART */

CALL_TIMER1_SET,
CALL_TIMER1_STOP,
CALL_TIMER2_SET,
CALL_TIMER2_STOP,
CALL_TIMER3_SET,
CALL_TIMER3_STOP,
CALL_TIMER4_SET,
CALL_TIMER4_STOP,
CALL_TIMER5_SET,
CALL_TIMER5_STOP,

CALL_COUNTER_SET,
CALL_COUNTER_INCREMENT,
CALL_COUNTER_DECREMENT,
CALL_COUNTER_GET,

CALL_STATE_RESTART,
CALL_STATE_NEXT,
CALL_STATE_JUMP,

CALL_OUTPUT_SELECTEDKEY,
CALL_SET_SELECTEDKEY,

CALL_FEEDBACK,

CALL_GET_RATE
} CallIds;

typedef enum {
RATE_TYPE_UNDEFINED,
RATE_TYPE_EFFECT
} RateType;

/* maximum length of access method name */
#define MAX_ACCESS_METHOD_NAME 50

/* maximum length of the description of the access method */
#define MAX_DESCRIPTION_TEXT 200

/* maximum length of the rate name and display name */
#define MAX_RATE_NAME 20
#define MAX_DISPLAY_RATE_NAME 30

/* types of compare that can occur after each effect */
#define COMPARE_NO 0
#define COMPARE_EQUAL 1
#define COMPARE_LESSTHAN 2
#define COMPARE_GREATERTHAN 3
#define COMPARE_EQUALORLESSTHAN 4
#define COMPARE_EQUALORGREATERTHAN 5

/* an access method rate */
/* initialize members of this structure in gok_scanner_construct_rate */
typedef struct GokAccessMethodRate {
	gchar* Name;
	gint Type;
	gchar* StringValue;
	gint Value;
	gint ID;
	struct GokAccessMethodRate* pRateNext;
} GokAccessMethodRate;

/* an effect that occurs in the event handler */
typedef struct GokScannerEffect {
	gint CallId;
	gchar* CallParam1;
	gchar* CallParam2;
	gint CompareType;
	gint CompareValue;
	gint CallReturn;
	gchar* pName;
	struct GokScannerEffect* pEffectNext;
	struct GokScannerEffect* pEffectTrue;
	struct GokScannerEffect* pEffectFalse;
} GokScannerEffect;

/* an event handler */
typedef struct GokScannerHandler {
	gint TypeHandler;
	gboolean bPredefined;
	gchar* EffectName;
	gint EffectState;
	GokScannerEffect* pEffectFirst;
	struct GokScannerHandler* pHandlerNext;
} GokScannerHandler;

/* an event handler state */
/* if you add members to this structure, initialize them in gok_scanner_construct_state */
typedef struct GokScannerState {
	gchar* NameState;
	GokScannerEffect* pEffectInit;
	GokScannerHandler* pHandlerFirst;
	struct GokScannerState* pStateNext;
} GokScannerState;

/* an access method (e.g. Inverse Scanning) */
/* if you add members to this structure, initialize them in gok_scanner_create_access_method */
typedef struct GokAccessMethod {
	gchar Name [MAX_ACCESS_METHOD_NAME + 1];
	gchar DisplayName [MAX_ACCESS_METHOD_NAME + 1];
	gchar Description [MAX_DESCRIPTION_TEXT + 1];
	GokScannerState* pStateFirst;
	GokScannerEffect* pInitializationEffects;
	GokAccessMethodRate* pRateFirst;
	xmlDoc* pXmlDoc;
	GokControl* pControlOperation;
	GokControl* pControlFeedback;
	GokControl* pControlOptions;
	struct GokAccessMethod* pAccessMethodNext;
} GokAccessMethod;

/* functions */
gboolean gok_scanner_initialize (const gchar *directory, const gchar *accessmethod, const gchar *selectaction, const gchar *scanaction);
void gok_scanner_close (void);
void gok_scanner_stop (void);
void gok_scanner_start (void);
gboolean gok_scanner_change_method (gchar* NameAccessMethod);
GokAccessMethod* gok_scanner_create_access_method (gchar* Name);
void gok_scanner_reset_access_method (void);
void gok_scanner_change_state (GokScannerState* pState, gchar* NameAccessMethod);
void gok_scanner_next_state (void);
void gok_scanner_set_handlers_null (void);
void gok_scanner_delete_effect (GokScannerEffect* peffect);
gboolean gok_scanner_read_access_method (gchar* Filename);
GokScannerEffect* gok_scanner_read_effects (xmlNode* pNode, GokAccessMethod* pAccessMethod);
GokScannerEffect* gok_scanner_create_effect (gint CallId, gchar* Param1, gchar* Param2, gint CompareType, gint CompareValue, GokScannerEffect* pEffectTrue, GokScannerEffect* pEffectFalse, gchar* pName);
GokScannerState* gok_scanner_construct_state (void);
GokAccessMethodRate* gok_scanner_construct_rate (void);
GokScannerHandler* gok_scanner_construct_handler (gchar* pHandlerName);
gboolean gok_scanner_handler_uses_button(GokScannerHandler *h);
gboolean gok_scanner_handler_uses_core_mouse_button(char *accessname,
                                                    GokScannerHandler *h,
                                                    int button);
gboolean gok_scanner_read_rates (xmlNode* pNode, GokAccessMethod* pAccessMethod);
gboolean gok_scanner_read_description (xmlDoc* pDoc, xmlNode* pNode, GokAccessMethod* pAccessMethod);
gboolean gok_scanner_read_operation (xmlDoc* pDoc, xmlNode* pNode, GokAccessMethod* pAccessMethod);
void gok_scanner_read_ui_loop (GokControl* pControl, xmlNode* pNode);
gboolean gok_scanner_read_feedback (xmlDoc* pDoc, xmlNode* pNode, GokAccessMethod* pAccessMethod);
gboolean gok_scanner_read_options (xmlDoc* pDoc, xmlNode* pNode, GokAccessMethod* pAccessMethod);
void gok_scanner_update_rates (void);
GokAccessMethod* gok_scanner_get_current_access_method (void);
GokAccessMethod* gok_scanner_get_first_access_method (void);
GokScannerState* gok_scanner_get_current_state (void);
gboolean gok_scanner_current_state_uses_core_mouse_button(int button);
gboolean gok_scanner_current_state_uses_corepointer (void);
void gok_scanner_get_pointer_location (gint* pX, gint* pY);
gint gok_scanner_timer_set (gint Rate, gint ID);
gint gok_scanner_timer_stop (gint TimerId);
gint gok_scanner_make_type_from_string (gchar* pString);
void gok_scanner_timer_set_dwell_rate (gint rate);
void gok_scanner_timer_start_dwell (void);
void gok_scanner_timer_stop_dwell (void);
gboolean gok_scanner_timer_on_dwell (gpointer data);
gboolean gok_scanner_get_multiple_rates (gchar* NameAccessMethod, gchar* NameSetting, gint* Value);

/* handler functions */
void gok_scanner_left_button_down (void);
void gok_scanner_left_button_up (void);
void gok_scanner_right_button_down (void);
void gok_scanner_right_button_up (void);
void gok_scanner_middle_button_down (void);
void gok_scanner_middle_button_up (void);
void gok_scanner_on_button4_down (void);
void gok_scanner_on_button4_up (void);
void gok_scanner_on_button5_down (void);
void gok_scanner_on_button5_up (void);
void gok_scanner_mouse_movement (gint x, gint y);
void gok_scanner_input_motion (gint *motion_data, gint n_axes);
gboolean gok_scanner_on_timer1 (gpointer data);
gboolean gok_scanner_on_timer2 (gpointer data);
gboolean gok_scanner_on_timer3 (gpointer data);
gboolean gok_scanner_on_timer4 (gpointer data);
gboolean gok_scanner_on_timer5 (gpointer data);
void gok_scanner_on_key_enter (GokKey* pKey);
void gok_scanner_on_key_leave (GokKey* pKey);
void gok_scanner_perform_effects (GokScannerEffect* pEffect);
void gok_scanner_on_switch1_down (void);
void gok_scanner_on_switch1_up (void);
void gok_scanner_on_switch2_down (void);
void gok_scanner_on_switch2_up (void);
void gok_scanner_on_switch3_down (void);
void gok_scanner_on_switch3_up (void);
void gok_scanner_on_switch4_down (void);
void gok_scanner_on_switch4_up (void);
void gok_scanner_on_switch5_down (void);
void gok_scanner_on_switch5_up (void);
void gok_scanner_repeat_on(void);
void gok_scanner_drop_refs (GokKey *pKey);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKSCANNER_H__ */
