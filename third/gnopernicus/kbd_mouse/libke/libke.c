/* libke.c
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

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <cspi/spi.h>
#include "libke.h"
#include "libsrconf.h"
#include "SRMessages.h"

#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <X11/XKBlib.h>

#define  USE_ALL_WINDOWS

/* Solaris specific variable types. */
#ifdef __sun
#define u_int8_t   uint8_t
#define u_int16_t  uint16_t
#define u_int32_t  uint32_t
#define u_int64_t  uint64_t
#endif

#define KE_TIME_WAIT 			5000
#define COUNT_OF_ITEM_IN_LAYER_KEYSET 	17

#define MODIFIER(K,C) (K ? C : "")
#define LINE(X)       (X != 0 ? "-" : "")
#define STR(X)	      (X != NULL ? X : "")

#define Z_CHAR_POSITION 26

/**
 * Keyboard echo log flags.
 *
**/
enum
{
    KE_LOG_UNKNOWN	= 0,
    KE_LOG_AT_SPI 	= 1,
    KE_LOG_GNOPERNICUS  = 2  
};

static gint 			ke_log_flags;

typedef struct
{
    gint    flag;
    SREvent *evnt;
} IdleStruct;    
    
/**
 *
 * Key string translation table.
 *
**/
typedef struct 
{
    gchar *key;
    gchar *key_shifted;
} KeyTransTable;

KeyTransTable key_trans_table[]=
{
    {"asciitilde",	"grave"},	/* ~    */
    {"grave",		"asciitilde"},	/* `    */
    {"numbersign",	"3"},		/* #	*/
    {"exclam",		"1"},		/* !	*/
    {"asciicircum",	"6"},		/* ^	*/
    {"period",		"greater"},	/* .	*/
    {"quotedbl",	"apostrophe"},	/* \"	*/
    {"dollar",		"4"},		/* $	*/
    {"percent", 	"5"},		/* %	*/
    {"ampersand", 	"7"},		/* &	*/
    {"apostrophe",	"quotedbl"},	/* \'	*/
    {"parenleft", 	"9"},		/* (	*/
    {"parenright", 	"0"},		/* )	*/
    {"asterisk",	"8"},		/* *	*/
    {"plus",		"equal"},	/* +	*/
    {"comma",		"less"},	/* ,	*/
    {"minus",		"underscore"},	/* -	*/
    {"slash",		"question"},	/* /	*/
    {"colon",		"semicolon"},	/* :	*/
    {"semicolon",	"colon"},	/* ;	*/
    {"less",		"comma"},	/* <	*/
    {"equal",		"plus"},	/* =	*/
    {"greater",		"period"},	/* >	*/
    {"question",	"slash"},	/* ?	*/
    {"at",		"2"},		/* @	*/
    {"bracketleft",	"braceleft"},	/* [	*/
    {"backslash",	"bar"},		/* \\	*/
    {"bracketright",	"braceright"},	/* ]	*/
    {"underscore",	"minus"},	/* _	*/
    {"braceleft",	"bracketleft"},	/* {	*/
    {"bar",		"backslash"},	/* |	*/
    {"braceright",	"bracketright"},/* }	*/
    {"1",		"exclam"},	/* 1	*/
    {"2",		"at"},		/* 2	*/
    {"3",		"numbersign"},	/* 3	*/
    {"4",		"dollar"},	/* 4	*/
    {"5",		"percent"},	/* 5	*/
    {"6",		"asciicircum"},	/* 6	*/
    {"7",		"ampersand"},	/* 7	*/
    {"8",		"asterisk"},	/* 8	*/
    {"9",		"parenleft"},	/* 9	*/
    {"0",		"parenright"}	/* 0	*/
};    

/**
 *
 * Command layer state machine states.
 *
**/
typedef enum
{
    KE_LAYER_STATE_IDLE,
    KE_LAYER_STATE_LEVEL
} KeyboardLayerState;

/**
 *
 * Keyset item type.
 *
**/
typedef struct
{
    AccessibleKeySet *keyset;
    unsigned long    modifier;
    long	     keysym;
} KeySetItem;

/**
 *
 * Keyevent.
 *
**/ 
typedef struct
{
    glong keyID;
    gint  keycode;
    gchar *keystring;   
    guint modifiers;
}KEKeyEvent;

static gboolean	ke_user_key_list_unregister_events (void);

/**
 * User key.
**/
static AccessibleKeystrokeListener *ke_user_key_listener;
static GSList 			   *accessible_key_set_list   = NULL;
static GSList 			   *ke_user_key_list 	 = NULL;
static GSList 			   *reg_list = NULL;

/**
 * keyecho
**/
static AccessibleKeystrokeListener *ke_keyecho_listener;
static AccessibleKeySet 	   *ke_keyecho_keyset;

/**
 * layer
**/
static AccessibleKeystrokeListener *ke_layer_listener;
static AccessibleKeySet 	   *ke_layer_keyset  = NULL;
KeyboardLayerState 	 	   ke_layer_state    = KE_LAYER_STATE_IDLE;
static gint 		 	   ke_layer_level    = 0;

/**
 * keyboard_echo_cb:
 *
 * The callback function used for communication with caller, is set during
 * the init process.
 *
**/
static KeyboardEchoCB 		   ke_keyboard_event_sink_cb = NULL;

/**
 * KEStateEnum:
 *
 * Define the state of the keyboard echo library
 *    KE_IDLE    - library not initialized
 *    KE_RUNNING - initialized and running
**/
typedef enum 
{
    KE_IDLE,
    KE_RUNNING
} KEStateEnum;

/**
 * keyboard_echo_status
 *
 * The library status
 *
**/
static KEStateEnum ke_keyboard_status = KE_IDLE;

IdleStruct* ke_idle_struct_new ()
{
    IdleStruct* ke_idle;
    ke_idle = g_new0 (IdleStruct, 1);
    return ke_idle;
}

void ke_idle_struct_free (IdleStruct *ke_idle)
{
    sre_release_reference (ke_idle->evnt);
    g_free (ke_idle);
}

static gboolean
ke_repot_layer_cmd (gpointer data)
{
    IdleStruct *ke_idle = (IdleStruct*)data;

    if (ke_keyboard_event_sink_cb != NULL) 
	ke_keyboard_event_sink_cb (ke_idle->evnt, ke_idle->flag);
	
    ke_idle_struct_free (ke_idle);
    return FALSE;
}

/**
 * ke_log_at_spi
 *
 *
**/
static void
ke_log_at_spi (const AccessibleKeystroke *key)
{    
    if (!(ke_log_flags & KE_LOG_AT_SPI))
	return;
	
    fprintf (stderr, "\nAT:%xp--key event:sym %ld (%c)\tmods %x\tcode %d\ttime %ld\tkeystring \"%s\"\ttype %d (press = %d, release = %d)",
	      (unsigned int) key,
	      (glong) key->keyID, (gchar)key->keyID,
	      (guint) key->modifiers,
	      (gint) key->keycode, 
	      (long int) key->timestamp,
	      key->keystring,
	      (gint) key->type, (gint)SPI_KEY_PRESSED, (gint)SPI_KEY_RELEASED );   

}

/**
 * ke_log_gnopernicus
 *
 *
**/
static void
ke_log_gnopernicus (const SREvent *event)
{
    SRHotkeyData *srhotkey_data = NULL;
    
    if (!(ke_log_flags & KE_LOG_GNOPERNICUS))
	return;

    srhotkey_data = (SRHotkeyData *) event->data;
	
    fprintf (stderr, "\nGN:%xp--key event:type:%d, keyID:%ld, modifiers:%d, keystring:%s",
		     (unsigned int)event,
		     event->type,
		     srhotkey_data->keyID,
		     srhotkey_data->modifiers,
		     srhotkey_data->keystring);
}

/**
 * ke_get_log_flag:
 *
 * return:
**/
void
ke_get_log_flag (void)
{
    const  gchar *log  = NULL;
    
    log = g_getenv ("GNOPERNICUS_LOG");
    
    if (!log)
	log = "";
	
    ke_log_flags = KE_LOG_UNKNOWN;
    
    if (strstr (log, "at-spi"))
	ke_log_flags |= KE_LOG_AT_SPI;
    if (strstr (log, "gnopernicus"))
	ke_log_flags |= KE_LOG_GNOPERNICUS;

}

/**
 * ke_print_register_return_value
 *
 * Show registration status.
**/
static void
ke_print_register_return_value (SPIBoolean retval,
				const gchar *modifiers)
{
    sru_debug ("Key registry(%s): result %s", modifiers, retval ? "succeeded" : "failed");
}

/****************************************************************************/

/**
 * ke_srhotkey_data_destructor:
 *
 * @data:
 *
 * This function will be used to destroy data allocated in SREvent's data member.
 *
**/
static void 
ke_srhotkey_data_destructor(gpointer data)
{
    SRHotkeyData *srhotkey_data = (SRHotkeyData *) data;
    
    if (srhotkey_data != NULL)
    {
	g_free (srhotkey_data->keystring);
	g_free (srhotkey_data);
    }
}

/**
 * ke_return_keystring
 *
 * @keyID:
 *
 * Return keystring from keyID.
 *
 * return:keystring.
**/
static gchar*
ke_return_keystring (gint keyID)
{
    gchar *rv = NULL;
    
    switch (keyID)
    {
        case XK_Alt_L:	rv = g_strdup("Alt_L");break;
        case XK_Alt_R:	rv = g_strdup("Alt_R");break;
        case XK_Shift_L:rv = g_strdup("Shift_L");break;
        case XK_Shift_R:rv = g_strdup("Shift_R");break;
        case XK_Control_L:rv = g_strdup("Control_L");break;
        case XK_Control_R:rv = g_strdup("Control_R");break;
        case XK_Caps_Lock:rv = g_strdup("Caps_Lock");break;
        case XK_Num_Lock:rv = g_strdup("Num_Lock");break;
        case XK_Home:	rv = g_strdup("Home");break;
        case XK_End:	rv = g_strdup("End");break;
        case XK_Left:	rv = g_strdup("Left");break;
        case XK_Right:	rv = g_strdup("Right");break;
        case XK_Up:	rv = g_strdup("Up");break;
        case XK_Down:	rv = g_strdup("Down");break;
        case XK_Page_Up:rv = g_strdup("Page_Up");break;
        case XK_Page_Down:rv = g_strdup("Page_Down");break;
	case XK_BackSpace:rv = g_strdup ("BackSpace");break;
	case XK_Delete:rv = g_strdup ("Delete");break;
	case XK_Escape:rv = g_strdup ("Escape");break;
        default:
    	    rv = NULL;
    }
    sru_debug ("Return keystring:%s",rv);
    
    return rv;
}


/**
 * ke_call_srkey_cb
 *
 * @keyID:
 * @modifiers:
 * @keystring:
 *
 * This function is used to create the SREvent structure and to send it to
 * the callback when SR_KEY event occurs.
 *
 * return:
**/
static void 
ke_call_srkey_cb (gulong keyID, 
		  guint modifiers, 
		  const gchar *keystring)
{
    SREvent *evnt = NULL;
    SRHotkeyData *srhotkey_data = NULL;
    IdleStruct *ke_idle;
    evnt = sre_new ();
    sru_return_if_fail (evnt);
    srhotkey_data = (SRHotkeyData *) g_new0 (SRHotkeyData, 1);
	
    if (NULL != srhotkey_data )
    {
	srhotkey_data->keyID = keyID;
	srhotkey_data->modifiers = modifiers;
	
	if ( keyID > 255 ) /* 255 max printable ASCII code */
	{
	    if (!keystring || strlen (keystring) == 0)
		srhotkey_data->keystring = ke_return_keystring (keyID);
	    else
		srhotkey_data->keystring = g_strdup (keystring);
	}
	else
	    srhotkey_data->keystring = g_strdup_printf ("%c", (gchar)keyID );
	    
	evnt->data = srhotkey_data;
	evnt->type = SR_EVENT_KEY;
	evnt->data_destructor = ke_srhotkey_data_destructor;
	
	ke_log_gnopernicus (evnt);
	
	if (ke_keyboard_event_sink_cb != NULL) 
	{
	    sre_add_reference (evnt);
	    ke_idle = ke_idle_struct_new ();
	    ke_idle->flag = 0;
	    ke_idle->evnt = evnt;
	    g_idle_add (ke_repot_layer_cmd, ke_idle);
	}
    }	
    sre_release_reference (evnt);
}


/**
 * ke_report_key_event:
 *
 * @key: AcceesibleKeystroke structure.
 * @user_data:
 *
 * The key listener function
 *
 * return: TRUE if consume a key event.
**/
static SPIBoolean
ke_report_keyecho_event (const AccessibleKeystroke *key, gpointer user_data)
{
    static gboolean key_busy = FALSE;    
    static GQueue  *key_queue = NULL;
    KEKeyEvent     *ev;

    sru_debug ("(Key)Received key event:\n\tsym %ld (%c)\n\tmods %x\n\tcode %d\n\ttime %ld\n\tkeystring \"%s\"\n",
	     (glong) key->keyID, (gchar)key->keyID,
	     (guint) key->modifiers,
	     (gint) key->keycode,
	     (long int) key->timestamp,
	     key->keystring);  
	     
    ke_log_at_spi (key);    
    
    if (key->type == SPI_KEY_RELEASED)
    {
	sru_debug ("keyrelease received");
	return FALSE;
    }
        
    ev = (KEKeyEvent *) g_new0 (KEKeyEvent, 1);
    
    sru_return_val_if_fail (ev, FALSE);

    if (!key_queue)
	key_queue = g_queue_new ();

    ev->keyID     = key->keyID;
    ev->modifiers = key->modifiers;
    ev->keycode	  = key->keycode;
    ev->keystring = g_strdup (key->keystring);
    
    g_queue_push_head (key_queue, ev);
    
    sru_debug ("key_busy:%d", key_busy);
    
    sru_return_val_if_fail (!key_busy, FALSE);

    key_busy = TRUE;
    
    while (!g_queue_is_empty (key_queue))
    {
	KEKeyEvent *key1;
	
	key1 = (KEKeyEvent *) g_queue_pop_tail (key_queue);
	
	ke_call_srkey_cb (key1->keyID, 0, key1->keystring);
	
	g_free (key1->keystring);		
	g_free (key1);
    }

    g_queue_free (key_queue);
    
    key_queue = NULL;

    key_busy = FALSE;
        
    return FALSE;
}



/**
 * ke_return_key
 *
 * @keyID: keyID for pressed numpad key.
 *
 * Return a layer code.
 *
 * return:layer code or -1 if not a numpad keys.
**/
static gint
ke_return_key (glong keyID)
{
    gint layer_code = -1;
    
    switch (keyID)
    {
	case XK_KP_0:		layer_code = 0; break;
	case XK_KP_1:		layer_code = 1; break;
	case XK_KP_2:		layer_code = 2; break;
	case XK_KP_3:		layer_code = 3; break;
	case XK_KP_4:		layer_code = 4; break;
	case XK_KP_5:		layer_code = 5; break;
	case XK_KP_6:		layer_code = 6; break;
	case XK_KP_7:		layer_code = 7; break;
	case XK_KP_8:		layer_code = 8; break;
	case XK_KP_9:		layer_code = 9; break;
	case XK_KP_Decimal:	layer_code = 10; break;
	case XK_KP_Separator:	layer_code = 10; break;
	case XK_KP_Enter:	layer_code = 11; break;
	case XK_KP_Add:		layer_code = 12; break;
	case XK_KP_Subtract:	layer_code = 13; break;
	case XK_KP_Multiply:	layer_code = 14; break;
	case XK_KP_Divide:	layer_code = 15; break;
#ifdef __sun
	/*
	 * The KeySyms returned by Solaris Xserver are incorrect.
	 * The values below are those returned on Solaris.
	 */
	case XK_KP_Insert:	layer_code = 0; break;
	case XK_F33:		layer_code = 1; break;
	case XK_Down:		layer_code = 2; break;
	case XK_F35:		layer_code = 3; break;
	case XK_Left:		layer_code = 4; break;
	case XK_F31:		layer_code = 5; break;
	case XK_Right:		layer_code = 6; break;
	case XK_F27:		layer_code = 7; break;
	case XK_Up:		layer_code = 8; break;
	case XK_F29:		layer_code = 9; break;
	case XK_KP_Delete:	layer_code = 10; break;
	case XK_F24:		layer_code = 13; break;
	case XK_F26:		layer_code = 14; break;
	case XK_F25:		layer_code = 15; break;
#endif
	default:
	break;
    }		
    
    return layer_code;
}



/**
 *
 * ke_call_keyboard_layer_cb
 *
 * @buf:string whit the SREvent will be called.
 * @flag: event flag (KE_LAYER_CHANGE , KE_LAYER_TIME_OUT or 0 flag).
 * @event_type: SREventType.
 * 
 * Emit command layer change event.
 *
 * return:
**/
static void 
ke_call_keyboard_layer_cb (const gchar *buf, 
			   gint 	flag,
			   SREventType  event_type)
{
    SREvent *evnt = NULL;
    IdleStruct *ke_idle;
    
    sru_return_if_fail (buf);
    sru_return_if_fail (strlen(buf) != 0);
    
    /* create SREvent structure */
    evnt = sre_new ();
    
    sru_return_if_fail (evnt);

    /* fill the event structure */
    evnt->data = g_strdup(buf);
    evnt->type = event_type;
    evnt->data_destructor = g_free;
	
    sru_debug ("Command event:%d <%s>", event_type, buf);
    
    if (evnt->data)
    {
    	ke_log_gnopernicus (evnt);

	/* call event sink from srcore */
        if (ke_keyboard_event_sink_cb != NULL) 
	{
	    sre_add_reference (evnt);
	    ke_idle = ke_idle_struct_new ();
	    ke_idle->flag = flag;
	    ke_idle->evnt = evnt;
	    g_idle_add (ke_repot_layer_cmd, ke_idle);
	}
    }	
    sre_release_reference (evnt);
}


/**
 *
 * ke_press_wait_function 
 *
 * @user_data:
 *
 * Layer change timer callback.
 *
 * return: FALSE when a timer is destroyed.
 *
**/
static gboolean
ke_press_wait_function (gpointer user_data)
{
    /* check if other key is pressed then 0 */
    if (ke_layer_state != KE_LAYER_STATE_IDLE)
    {
	gchar *cmd;
	
	ke_layer_state = KE_LAYER_STATE_IDLE;
				
	cmd = g_strdup_printf("L%02d", ke_layer_level);				

	/* send the time out event */	
	ke_call_keyboard_layer_cb (cmd, 
				    KE_LAYER_TIME_OUT, 
				    SR_EVENT_COMMAND_LAYER_CHANGED);
	
	sru_debug ("Command code %s TIMEOUT", cmd);				

	g_free (cmd);
    }
    
    return FALSE;
}

/**
*
* ke_report_layer_key  
*
* @key: AcceesibleKeystroke structure.
* @user_data:
*
* Layer command key event callback.
*
* return: TRUE if consume a key event.
**/
static SPIBoolean 
ke_report_layer_key_event (const AccessibleKeystroke *key, gpointer user_data)
{
    static gboolean layer_busy = FALSE;    
    static GQueue  *layer_queue = NULL;
    static gint	    layer_timeout_id = 0;
    KEKeyEvent     *ev;
    
    sru_debug ("(Layer)Received key event:\n\tsym %ld (%c)\n\tmods %x\n\tcode %d\n\ttime %ld\n\tkeystring \"%s\"\n\ttype %d (press = %d, release = %d)",
	      (glong) key->keyID, (gchar)key->keyID,
	      (guint) key->modifiers,
	      (gint) key->keycode, 
	      (long int) key->timestamp,
	      key->keystring,
	      (gint) key->type, (gint)SPI_KEY_PRESSED, (gint)SPI_KEY_RELEASED );   

    ke_log_at_spi (key);	          
    
    /* filter the release events */
    if (key->type == SPI_KEY_RELEASED)
    {
	sru_debug ("keyrelease received");
	return TRUE;
    }

    /* create event structure */
    ev = (KEKeyEvent *) g_new0 (KEKeyEvent, 1);
    
    sru_return_val_if_fail (ev, FALSE);
    
    /* create the event queue */
    if (!layer_queue)
	layer_queue = g_queue_new ();

    /* fill the event structure */
    ev->keyID     = key->keyID;
    ev->modifiers = key->modifiers;
    ev->keycode	  = key->keycode;
    ev->keystring = NULL;
    
    /* push event in queue */
    g_queue_push_head (layer_queue, ev);

    sru_debug ("layer_busy:%d", layer_busy);
    
    /* check for reentrancy */
    sru_return_val_if_fail (!layer_busy, TRUE);
    
    layer_busy = TRUE;
        
    while (!g_queue_is_empty (layer_queue))
    {
	KEKeyEvent *key1;
	gint	   level = -1;
	
	/* pop event from queue */
	key1 = (KEKeyEvent *) g_queue_pop_tail (layer_queue);
		
	/* check if it is a valid numpad key and return the level */
	if ((level = ke_return_key (key1->keyID)) != -1)
	{
	    switch (ke_layer_state)
	    {
		case KE_LAYER_STATE_IDLE:
		{
		    if (level == 0)
		    {			    
			sru_debug ("Layer change key pressed");
			    
			ke_layer_state = KE_LAYER_STATE_LEVEL;
			    
			/* wait to press a numpad key that to change the layer */
			layer_timeout_id = g_timeout_add (KE_TIME_WAIT , 
							  ke_press_wait_function , 
							  NULL);
		    }
		    else
		    {
			gchar *cmd = NULL;
			    
			ke_layer_state = KE_LAYER_STATE_IDLE;

			/* create a command */
			cmd = g_strdup_printf ("L%02dK%02d", 
						ke_layer_level, 
						level);				
						    
			ke_call_keyboard_layer_cb (cmd, 0, SR_EVENT_COMMAND_LAYER);
				    
			sru_debug ("Command code %s",cmd);				
			
			g_free (cmd);
		    }
		    break;
		}
		case KE_LAYER_STATE_LEVEL:
		{
		    gchar *cmd = NULL;
		    
		    if (layer_timeout_id)
			g_source_remove (layer_timeout_id);
		    
		    layer_timeout_id = 0;
		    
		    ke_layer_level   = level;
			    
		    /* create a layer change command */
		    cmd = g_strdup_printf ("L%02d", ke_layer_level);
					
		    ke_call_keyboard_layer_cb ( cmd, 
						KE_LAYER_CHANGED, 
						SR_EVENT_COMMAND_LAYER_CHANGED);
			     
		    sru_debug ("Command:%s",cmd);
			    
		    g_free (cmd);
			    
		    ke_layer_state = KE_LAYER_STATE_IDLE;
			
		    break;
		}
		default:
		    ke_layer_state = KE_LAYER_STATE_IDLE;
		break;
	    }
	}
	else
	    ke_layer_state = KE_LAYER_STATE_IDLE;

	g_free (key1);
    }

    g_queue_free (layer_queue);
    layer_queue = NULL;
    layer_busy = FALSE;
  
    return TRUE;
}


/**
 * ke_key_set_item_new
 *
 * Create keyset item.
 *
 * return: new keyset item.
**/
static KeySetItem*
ke_key_set_item_new (void)
{
    KeySetItem *item = NULL;
    
    item = (KeySetItem *) g_new0 (KeySetItem, 1);
    
    return item;    
}

/**
 * ke_key_set_item_free
 *
 * @item:
 *
 * Free keyset item.
**/
static void
ke_key_set_item_free (KeySetItem *item)
{
    sru_return_if_fail (item);
    
    if (item->keyset)
        SPI_freeAccessibleKeySet (item->keyset);

    g_free (item);
}

/**
 * ke_translate_key_revers
 *
 * @keystring:
 *
 * Translate from keystring to shfited key.
 *
 * return: key.
**/
static gchar*
ke_translate_key_revers (const gchar *keystring,
			gboolean shift_modifier)
{
    gint i;

    sru_return_val_if_fail (keystring, NULL);
    
    if (strlen (keystring) == 1 && g_ascii_isalpha (*keystring))
    {
	return g_ascii_strup (keystring, -1);
    }
    	
    for (i = 0 ; i < G_N_ELEMENTS (key_trans_table) ; i++)
    {
	if (!strcmp (keystring, key_trans_table[i].key))
	{
	    if (!shift_modifier)
		return g_strdup (key_trans_table[i].key);
		
	    return g_strdup (key_trans_table[i].key_shifted);
	}
    }
    
    return g_strdup (keystring);
}

/**
 * ke_translate_key
 *
 * Translate from keystring to key.
 *
**/
gchar*
ke_translate_key (const gchar *key)
{
    gint i;
    
    for (i = 0 ; i < G_N_ELEMENTS (key_trans_table) ; i++)
    {
	if (!strcmp (key, key_trans_table[i].key))
	{
	    return g_strdup (key_trans_table[i].key_shifted);
	}
    }
    return g_strdup (key);
}

/**
 * ke_string_to_keycode
 *
 * @string:
 *
 * Covert string to keycode.
 *
 * return: keycode.
**/
static gchar*
ke_string_to_keysym (const gchar *string, gboolean modifier)
{
    gchar   *keysym  = NULL;
    gchar   *tmp_str = NULL;
    long     lkeysym;

    modifier = modifier & SPI_KEYMASK_SHIFT;
    
    if (g_utf8_strlen (string, -1) == 1)
    {
        gunichar uchar = g_utf8_get_char (string);
	
	/* check if shift modifier is active */
        if (modifier)
	{
	    if (g_unichar_isdigit (uchar))
		tmp_str = ke_translate_key (string);
	    else
		tmp_str = g_ascii_strup (string, -1);
	}
	else
	{
	    tmp_str = g_ascii_strdown (string, -1);
	}
    }
    else
    {
        if (modifier)
    	    tmp_str = ke_translate_key (string);
	else
	    tmp_str = g_strdup (string);
    }
    
    lkeysym  = XStringToKeysym (tmp_str);
    
    g_free (tmp_str);
     
    if (g_unichar_validate ((guint32)lkeysym))
    {
        keysym = (gchar *) g_new0 (gchar *, 8);
	
	if (g_unichar_to_utf8 ((guint32)lkeysym, keysym) < 1)
	    sru_warning ("Invalid character.");
    }
    
    return  keysym;
}

/**
 * ke_keysym_to_string:
 *
 * @keysym:
 * @shift_modifier: TRUE if shift modified.
 *
 * Convert keysym to string.
 *
 * return: string.
**/
static gchar*
ke_keysym_to_string (glong keysym, gint modifier)
{
    gchar *keystring = NULL;
    gchar *tmp = NULL;

    /* if special character code [0-32] */
    if (keysym <= Z_CHAR_POSITION)
	return g_strdup_printf ("%c", ('A' + (gint)keysym - 1));
	    	
    /* return a keystring from keysym */
    tmp = XKeysymToString (keysym);

    /* translate a key */
    keystring = ke_translate_key_revers (tmp, (modifier & SPI_KEYMASK_SHIFT));

/*    g_free (tmp);*/ /*FIXME: tmp could be free or not? */
    return keystring;
}

/**
*
* ke_report_user_key  
*
* @key: AcceesibleKeystroke structure.
* @user_data:
*
* User defined key event callback.
*
* return: TRUE if consume a key event.
**/
static SPIBoolean 
ke_report_user_key_event (const AccessibleKeystroke *key, gpointer user_data)
{
    static gboolean user_busy = FALSE;    
    static GQueue  *user_queue = NULL;
    KEKeyEvent     *ev;
    sru_debug ("(User)Received key event:\n\tsym %ld (%c)\n\tmods %x\n\tcode %d\n\ttime %ld\n\tkeystring \"%s\"\n\ttype %d (press = %d, release = %d)",
	      (glong) key->keyID, (gchar)key->keyID,
	      (guint) key->modifiers,
	      (gint) key->keycode, 
	      (long int) key->timestamp,
	      key->keystring,
	      (gint) key->type, (gint)SPI_KEY_PRESSED, (gint)SPI_KEY_RELEASED );   

    ke_log_at_spi (key); 

    /* Reject release event */     
    if (key->type == SPI_KEY_RELEASED)
    {
	sru_debug ("keyrelease received");
	return TRUE;
    }
    
    /* create event struct */    
    ev = (KEKeyEvent *) g_new0 (KEKeyEvent, 1);
    
    sru_return_val_if_fail (ev, FALSE);

    /* create queue if not exist yet */
    if (!user_queue)
	user_queue = g_queue_new ();

    /* fill event struct */
    ev->keyID     = key->keyID;
    ev->modifiers = key->modifiers;
    ev->keycode	  = key->keycode;
    ev->keystring = NULL;

    /* push event in queue */
    g_queue_push_head (user_queue, ev);

    sru_debug ("user_busy:%d", user_busy);
    /* check for reentrancy */
    sru_return_val_if_fail (!user_busy, TRUE);

    user_busy = TRUE;
        
    while (!g_queue_is_empty (user_queue))
    {
	KEKeyEvent *key1;
	gboolean modif = FALSE;
	gchar *cmd = NULL;
	gchar *keystr = NULL;
    
	/* pop from queue the event */
	key1 = (KEKeyEvent *) g_queue_pop_tail (user_queue);
	/* get the status of modifiers */
	if (key->modifiers & (~(SPI_KEYMASK_ALT | SPI_KEYMASK_CONTROL | SPI_KEYMASK_SHIFT)))
	    sru_warning ("Callback invoked with an unexpected modifier.");
	modif  = key1->modifiers & (SPI_KEYMASK_ALT | SPI_KEYMASK_CONTROL | SPI_KEYMASK_SHIFT);
	
	/* get the keystring */
	keystr = ke_keysym_to_string (key1->keyID, modif);
        /* create a command */
	cmd = g_strconcat ( MODIFIER(modif & SPI_KEYMASK_ALT,"A"),
		    	    MODIFIER(modif & SPI_KEYMASK_CONTROL,"C"),
		    	    MODIFIER(modif & SPI_KEYMASK_SHIFT,"S"),
		    	    LINE(modif),
		    	    STR(keystr), NULL);
	g_free (key1);
	g_free (keystr);
	
	sru_debug ("(User)keyevent:%s", cmd);	  

	/* Verify if this event is registered. */
	/* This needed until does not fix the BUG: */
	ke_call_keyboard_layer_cb (cmd, 0, SR_EVENT_COMMAND_LAYER);

	g_free (cmd);
    }

    g_queue_free (user_queue);
    user_queue = NULL;
    user_busy = FALSE;
        
    return TRUE;
}

/**
 * ke_return_modifier_mask
 *
 * @data:
 * @modifiers:
 * @key:
 *
 * Return modifier and key from data.
 *
 * return: FALSE at failed and invalid string.
**/
static gboolean
ke_return_modifier_and_key (const gchar *data,
			    unsigned long *modifiers,
			    gchar  **key)
{
    gchar *delimit;
    gchar *iter;
    unsigned long rv = 0;
    
    *modifiers = SPI_KEYMASK_UNMODIFIED;
    *key = NULL;
    
    sru_return_val_if_fail (data, FALSE);
    
    delimit = g_strrstr (data, "-");

    if (!delimit)
    {
	*key = g_strdup (data);
	sru_debug ("No delimiter in key combination.");
	return TRUE;
    }
    
    iter = (gchar*)data;
    
    /* get the modifier */
    while (*iter != '-')
    {
	switch (*iter)
	{
	    case 'A':
		rv = rv | SPI_KEYMASK_ALT;
	    break;
	    case 'C':
		rv = rv | SPI_KEYMASK_CONTROL;
	    break;
	    case 'S':
		rv = rv | SPI_KEYMASK_SHIFT;
	    break;
	    default:
		return FALSE;
	    break;
	}
	iter++;
    }
    
    *modifiers = rv;
    *key       = g_strdup (delimit + 1);
    
    return TRUE;
}


static gboolean
ke_register_keysetitem (KeySetItem *ksi)
{
    SPIBoolean retval = FALSE;

    retval = SPI_registerAccessibleKeystrokeListener (ke_user_key_listener,
			    				(AccessibleKeySet *) ksi->keyset,
				    			ksi->modifier,
				    			(gulong) ( SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
				    			SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
    ke_print_register_return_value (retval, "");

    return TRUE;
}


/**
 * ke_user_key_list_register_events:
 *
 * @list: user keylist.
 *
 * Register a user keylist events.
 *
**/
static void
ke_user_key_list_register_events (GSList *list)
{
    GSList *elem = NULL;
    elem = list;
    
    while (elem)
    {
        unsigned long modifiers;
	gchar *key;

	/* return the modifiers	and the key from data */
	if (ke_return_modifier_and_key ((gchar*)elem->data, &modifiers, &key))
	{
	    KeySetItem 		*ksi = NULL;
	    AccessibleKeySet 	*keyset = NULL;
	    gchar		*keysym = NULL;

	    modifiers |= SPI_KEYMASK_NUMLOCK;
	    /* if no key go to next step */
	    if (!key) 
	    {
	    	elem = elem->next;
		continue;
	    }
	    
	    /* create key set item */
	    ksi = ke_key_set_item_new ();
	    
	    if (!ksi) 
	    {
		g_free (key);
		elem = elem->next;
		continue;
	    }
	    
	    sru_debug ("Modifier %s combination:%ld-%s",(gchar*)elem->data,modifiers,key);
	    
	    /* return a keysym values from key */
	    keysym   = ke_string_to_keysym (key, modifiers & SPI_KEYMASK_SHIFT);
	    
	    /* create an AccessibleKeySet with a keysym */
	    keyset   = SPI_createAccessibleKeySet (1, (const char*)keysym, NULL, NULL);
	    
	    g_free (keysym);
	    
	    /* fill the key set item */
	    ksi->keyset   = keyset;
	    ksi->modifier = modifiers;
	    
	    /* append keyset item in list */
	    accessible_key_set_list = g_slist_append (accessible_key_set_list, ksi);
	
	    /* register key events */
	    if (ksi->keyset)
	    {	
		ke_register_keysetitem (ksi);
	    }
	}
	
	g_free (key);
	
	elem = elem->next;
    }
}


static gboolean
ke_layer_register_events (AccessibleKeystrokeListener *ke_layer_listener,
			  AccessibleKeySet 	      *ke_layer_keyset)
{
    gboolean retval = TRUE;
    /*
	Layer key listener. Listen key event from numpad. Listen only if a numlock 
	is active.
    */
    retval = SPI_registerAccessibleKeystrokeListener (ke_layer_listener,
				      (AccessibleKeySet *) ke_layer_keyset,
				      SPI_KEYMASK_NUMLOCK,
				      (gulong) ( SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
#ifdef USE_ALL_WINDOWS
				      SPI_KEYLISTENER_ALL_WINDOWS);
#else
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
#endif

    ke_print_register_return_value (retval, "(NUMLOCK)");
    retval = SPI_registerAccessibleKeystrokeListener (ke_layer_listener,
				      (AccessibleKeySet *) ke_layer_keyset,
				      SPI_KEYMASK_SHIFTLOCK|SPI_KEYMASK_NUMLOCK,
				      (gulong) ( SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
#ifdef USE_ALL_WINDOWS
				      SPI_KEYLISTENER_ALL_WINDOWS);
#else
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
#endif
    ke_print_register_return_value (retval, "(SL|NUMLOCK)");
    retval = SPI_registerAccessibleKeystrokeListener (ke_layer_listener,
				      (AccessibleKeySet *) ke_layer_keyset,
				      SPI_KEYMASK_SHIFT|SPI_KEYMASK_NUMLOCK,
				      (gulong) ( SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
#ifdef USE_ALL_WINDOWS
				      SPI_KEYLISTENER_ALL_WINDOWS);
#else
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
#endif
    ke_print_register_return_value (retval, "(S|NUMLOCK)");
    retval = SPI_registerAccessibleKeystrokeListener (ke_layer_listener,
				      (AccessibleKeySet *) ke_layer_keyset,
				      SPI_KEYMASK_ALT|SPI_KEYMASK_NUMLOCK,
				      (gulong) ( SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
#ifdef USE_ALL_WINDOWS				
				      SPI_KEYLISTENER_ALL_WINDOWS);
#else
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
#endif
    ke_print_register_return_value (retval, "(S|NUMLOCK)");
    retval = SPI_registerAccessibleKeystrokeListener (ke_layer_listener,
				      (AccessibleKeySet *) ke_layer_keyset,
				      SPI_KEYMASK_CONTROL|SPI_KEYMASK_NUMLOCK,
				      (gulong) ( SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
#ifdef USE_ALL_WINDOWS
				      SPI_KEYLISTENER_ALL_WINDOWS);
#else
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
#endif				      
    ke_print_register_return_value (retval, "(C|NUMLOCK)");
    retval = SPI_registerAccessibleKeystrokeListener (ke_layer_listener,
				      (AccessibleKeySet *) ke_layer_keyset,
				      SPI_KEYMASK_ALT|SPI_KEYMASK_SHIFT|SPI_KEYMASK_NUMLOCK,
				      (gulong) ( SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
#ifdef USE_ALL_WINDOWS
				      SPI_KEYLISTENER_ALL_WINDOWS);
#else
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
#endif				      
    ke_print_register_return_value (retval, "(A|S|NUMLOCK)");
    retval = SPI_registerAccessibleKeystrokeListener (ke_layer_listener,
				      (AccessibleKeySet *) ke_layer_keyset,
				      SPI_KEYMASK_CONTROL|SPI_KEYMASK_SHIFT|SPI_KEYMASK_NUMLOCK,
				      (gulong) ( SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
#ifdef USE_ALL_WINDOWS
				      SPI_KEYLISTENER_ALL_WINDOWS);
#else
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
#endif
    ke_print_register_return_value (retval, "(C|S|NUMLOCK)");
    retval = SPI_registerAccessibleKeystrokeListener (ke_layer_listener,
				      (AccessibleKeySet *) ke_layer_keyset,
				      SPI_KEYMASK_CONTROL|SPI_KEYMASK_ALT|SPI_KEYMASK_NUMLOCK,
				      (gulong) ( SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
#ifdef USE_ALL_WINDOWS
				      SPI_KEYLISTENER_ALL_WINDOWS);
#else
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
#endif
    ke_print_register_return_value (retval, "(C|A|NUMLOCK)");
    
    return retval;
}

static gboolean
ke_keyecho_register_events (AccessibleKeystrokeListener *ke_key_listener,
			    AccessibleKeySet 	        *ke_key_keyset)
{
    gboolean retval = TRUE;
    /* will listen only to unshifted key events, only press */
    /*
	Key echo listener: Listen the following keys: ALT_L, ALT_R, 
	SHIFT_L, SHIFT_R, Control_L, Control_R, Up, Down, Left, Right, 
	PageUp, PageDown, Home, End, CapsLock, NumLock,
    */
    retval = SPI_registerAccessibleKeystrokeListener (ke_key_listener,
				      (AccessibleKeySet *) ke_key_keyset,
				      SPI_KEYMASK_UNMODIFIED,
				      (gulong) (SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
    ke_print_register_return_value (retval, "(U)");
    retval = SPI_registerAccessibleKeystrokeListener (ke_key_listener,
				      (AccessibleKeySet *) ke_key_keyset,
				      SPI_KEYMASK_NUMLOCK,
				      (gulong) (SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
    ke_print_register_return_value (retval, "(U|NUMLOCK)");
    retval = SPI_registerAccessibleKeystrokeListener (ke_key_listener,
				      (AccessibleKeySet *) ke_key_keyset,
				      SPI_KEYMASK_SHIFTLOCK,
				      (gulong) (SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
    ke_print_register_return_value (retval, "(SL)");
    retval = SPI_registerAccessibleKeystrokeListener (ke_key_listener,
				      (AccessibleKeySet *) ke_key_keyset,
				      SPI_KEYMASK_SHIFTLOCK | SPI_KEYMASK_NUMLOCK,
				      (gulong) (SPI_KEY_PRESSED | SPI_KEY_RELEASED ),
				      SPI_KEYLISTENER_NOSYNC | SPI_KEYLISTENER_CANCONSUME);
    ke_print_register_return_value (retval, "(SL|NUMLOCK)");
    
    return retval;
}

/**
 *
 * ke_get_keypad_keycodes
 *
 * Return a keycode array for numpad keys.
 *
 *
 * return: keycode array.
**/
static short*
ke_get_keypad_keycodes (void)
{
    Display *display;
    short *keycodes = (short *) g_new0 (short *, COUNT_OF_ITEM_IN_LAYER_KEYSET);	

    display = GDK_DISPLAY ();
    
    keycodes [0] = XKeysymToKeycode (display, XK_KP_0);
    keycodes [1] = XKeysymToKeycode (display, XK_KP_1);
    keycodes [2] = XKeysymToKeycode (display, XK_KP_2);
    keycodes [3] = XKeysymToKeycode (display, XK_KP_3);
    keycodes [4] = XKeysymToKeycode (display, XK_KP_4);
    keycodes [5] = XKeysymToKeycode (display, XK_KP_5);
    keycodes [6] = XKeysymToKeycode (display, XK_KP_6);
    keycodes [7] = XKeysymToKeycode (display, XK_KP_7);
    keycodes [8] = XKeysymToKeycode (display, XK_KP_8);
    keycodes [9] = XKeysymToKeycode (display, XK_KP_9);
    keycodes [10] = XKeysymToKeycode (display, XK_KP_Decimal);
    keycodes [11] = XKeysymToKeycode (display, XK_KP_Enter);
    keycodes [12] = XKeysymToKeycode (display, XK_KP_Add);
    keycodes [13] = XKeysymToKeycode (display, XK_KP_Subtract);
    keycodes [14] = XKeysymToKeycode (display, XK_KP_Multiply);
    keycodes [15] = XKeysymToKeycode (display, XK_KP_Divide);
    /* Fix for bug #148774 - some kbds report XK_KP_Separator in numpad */
    keycodes [16] = XKeysymToKeycode (display, XK_KP_Separator);
    	
    return keycodes;
}

/**
 *
 * ke_get_key_keysyms
 *
 * Return a keysym array for a specific sticky and other function keys. 
 *
 * return: keysym array
 *
**/
long keyecho_keysym[] = { 
    XK_Alt_L, 
    XK_Alt_R,
    XK_Shift_L,
    XK_Shift_R,
    XK_Control_L,
    XK_Control_R,
    XK_Caps_Lock,
    XK_Num_Lock,
    XK_Home,
    XK_End,
    XK_Left,
    XK_Right,
    XK_Up,
    XK_Down,
    XK_Page_Up,
    XK_Page_Down,
    XK_BackSpace,
    XK_Delete,
    XK_Escape
};

gchar*
ke_get_keyecho_keysyms (void)
{
    gchar *rv = NULL;
    gint i;
    Display *disp;
    disp = GDK_DISPLAY ();

    for (i = 0 ; i < G_N_ELEMENTS(keyecho_keysym) ; i++)
    {
	short key;
	key = XKeysymToKeycode (disp, keyecho_keysym[i]);
	if (g_unichar_validate ((guint32)key)) 
        {
	    gchar *keysym = NULL;
    	    gchar *tmp = NULL;
	
    	    keysym = (gchar *) g_new0 (gchar *, 8);
	
	    g_unichar_to_utf8 ((guint32)keyecho_keysym[i], keysym);
	
	    if (rv)
		tmp = g_strconcat (rv, keysym, NULL);
	    else
		tmp = g_strdup (keysym);
		
	    g_free (keysym);
	    g_free (rv);
	    rv = tmp;
        }
    }
    
    return rv;
}


/**
 * ke_get_config_settings:
 *
 * @list: Return user defined keys list.
 *
 * Used to get working parameters,
 *
 * return: FALSE at fail.
**/
static gboolean
ke_get_config_settings (GSList **list)

{
    GSList *list_tmp = NULL;
    
    *list = NULL;            
    
    if (!srconf_get_data_with_default (SRC_USER_DEF_LIST , CFGT_LIST, 
		&list_tmp, (gpointer)NULL ,SRC_USER_DEF_SECTION)) 
	return FALSE;
    
    if (!list_tmp) 
	return FALSE;
    
    *list = list_tmp;

    return TRUE;
}

/**
 * ke_user_key_list_free:
 *
 * @list: user_key_list
 *
 * Free a user key list.
 *
**/
static void
ke_user_key_list_free (GSList *list)
{
    GSList *elem = NULL;    

    if (!list)
	return;

    for (elem = list ; elem ; elem = elem->next)
	g_free (elem->data);
    
    g_slist_free (list);
    
    list = NULL;
}


/**
 * ke_config_changed:
 *
 * Using this function the KE can be notified for configuration changes
**/
void 
ke_config_changed (void)
{
    sru_debug ("ke_config_changed invoked.");
    
    ke_user_key_list_unregister_events ();  
    
    ke_user_key_list_free (ke_user_key_list);
    ke_user_key_list_free (reg_list);
    ke_user_key_list = NULL;
    
    if (ke_get_config_settings (&ke_user_key_list))
    {
	ke_user_key_list_register_events (ke_user_key_list);
    }
}


/**
 * ke_init:
 *
 * @kecb: the callback function used to send data to caller
 *
 * This function initialize the keyboard echo library, set the working
 * parameters, create and register key listener(s)
 *
 * return: TRUE at success.
**/
gboolean
ke_init (KeyboardEchoCB kecb)
{
    short  *ke_layer_keycodes = NULL;
    gchar  *ke_keyecho_keysyms = NULL;
  
    sru_return_val_if_fail (ke_keyboard_status == KE_IDLE, FALSE);
    sru_return_val_if_fail (kecb, FALSE);

    /*
	Get log flag from enviroment
    */    
    ke_get_log_flag ();
    
    sru_debug ("ke_init...");
    
    /*
	Attribute keyboard event sink function from srcore.
    */
    ke_keyboard_event_sink_cb = kecb;

    /*
	User keys get from gconf.
    */
    ke_user_key_list = NULL;
    ke_get_config_settings (&ke_user_key_list);
      
    /* 
	Prepare the keyboard snoopers 
    */
    ke_layer_listener 	 = SPI_createAccessibleKeystrokeListener (ke_report_layer_key_event, NULL);
    ke_user_key_listener = SPI_createAccessibleKeystrokeListener (ke_report_user_key_event, NULL);
    ke_keyecho_listener  = SPI_createAccessibleKeystrokeListener (ke_report_keyecho_event, NULL);
    
    /*
	Layer commands.
    */
    ke_layer_keycodes = ke_get_keypad_keycodes ();
    ke_layer_keyset   = SPI_createAccessibleKeySet (COUNT_OF_ITEM_IN_LAYER_KEYSET, NULL, ke_layer_keycodes, NULL);
    g_free (ke_layer_keycodes);

    /*
	Keyboard echo.
    */
    ke_keyecho_keysyms = ke_get_keyecho_keysyms ();
    ke_keyecho_keyset  = SPI_createAccessibleKeySet (G_N_ELEMENTS (keyecho_keysym),ke_keyecho_keysyms, NULL, NULL);
    g_free (ke_keyecho_keysyms);

    /*
	User key registering.
    */  
    if (ke_user_key_list)
	ke_user_key_list_register_events (ke_user_key_list);

    /*
	Layer key registering.
    */    
    ke_layer_register_events (ke_layer_listener,
			     (AccessibleKeySet *) ke_layer_keyset);
    /*
	Keyboard echo key registering.
    */
    ke_keyecho_register_events  (ke_keyecho_listener,
				(AccessibleKeySet *) ke_keyecho_keyset);

    ke_keyboard_status = KE_RUNNING;

    sru_debug ("done.status = KE_RUNNING");

    return TRUE;
}

/**
 * ke_user_key_list_unregister_events:
 *
 * Unregister user keys.
 *
 * return: TRUE if success.
**/
static gboolean
ke_user_key_list_unregister_events (void)
{
    GSList *elem = NULL;

    if (!accessible_key_set_list)
	return FALSE;
    	
    elem = accessible_key_set_list;
    
    while (elem)
    {
	KeySetItem *ksi = elem->data;
	
	SPI_deregisterAccessibleKeystrokeListener (ke_user_key_listener, 
						   ksi->modifier);

	ke_key_set_item_free (ksi);
	
	elem->data = NULL;
	
	elem = elem->next;
    }
    
    g_slist_free (accessible_key_set_list);
    
    accessible_key_set_list = NULL;
    
    return TRUE;
}

static void 
ke_layer_unregister_events (void)
{
    SPI_deregisterAccessibleKeystrokeListener (ke_layer_listener, 
					    SPI_KEYMASK_NUMLOCK);
    SPI_deregisterAccessibleKeystrokeListener (ke_layer_listener, 
					    SPI_KEYMASK_NUMLOCK | 
					    SPI_KEYMASK_SHIFTLOCK);
    SPI_deregisterAccessibleKeystrokeListener (ke_layer_listener, 
					    SPI_KEYMASK_NUMLOCK | 
					    SPI_KEYMASK_SHIFT);
    SPI_deregisterAccessibleKeystrokeListener (ke_layer_listener, 
					    SPI_KEYMASK_NUMLOCK | 
					    SPI_KEYMASK_ALT);
    SPI_deregisterAccessibleKeystrokeListener (ke_layer_listener, 
					    SPI_KEYMASK_NUMLOCK | 
					    SPI_KEYMASK_CONTROL);
    SPI_deregisterAccessibleKeystrokeListener (ke_layer_listener, 
					    SPI_KEYMASK_NUMLOCK  | 
					    SPI_KEYMASK_SHIFT |
					    SPI_KEYMASK_CONTROL);
    SPI_deregisterAccessibleKeystrokeListener (ke_layer_listener, 
					    SPI_KEYMASK_NUMLOCK  | 
					    SPI_KEYMASK_ALT   |
					    SPI_KEYMASK_CONTROL);
    SPI_deregisterAccessibleKeystrokeListener (ke_layer_listener, 
					    SPI_KEYMASK_NUMLOCK  | 
					    SPI_KEYMASK_SHIFT |
					    SPI_KEYMASK_ALT);
}

static void
ke_keyecho_unregister_events (void)
{
    /* deregister keylisteners */
    SPI_deregisterAccessibleKeystrokeListener (ke_keyecho_listener, 
					    SPI_KEYMASK_UNMODIFIED );
    SPI_deregisterAccessibleKeystrokeListener (ke_keyecho_listener, 
					    SPI_KEYMASK_NUMLOCK );
    SPI_deregisterAccessibleKeystrokeListener (ke_keyecho_listener, 
					    SPI_KEYMASK_SHIFTLOCK );
    SPI_deregisterAccessibleKeystrokeListener (ke_keyecho_listener, 
					    SPI_KEYMASK_SHIFTLOCK |
					    SPI_KEYMASK_NUMLOCK );
}

/**
 * ke_terminate
 *
 * Deregister key listener(s)
 *
**/
void
ke_terminate (void)
{
    sru_return_if_fail (ke_keyboard_status != KE_IDLE);
    
    sru_debug ("ke_terminate...");

    /* unregister keyecho events */
    ke_keyecho_unregister_events ();
    
    /* unregister layer events */
    ke_layer_unregister_events ();
    
    /* unregister user_defined events */
    ke_user_key_list_unregister_events ();  
    
    ke_user_key_list_free (ke_user_key_list);
    ke_user_key_list_free (reg_list);
    
    /* unref key listeners */
    AccessibleKeystrokeListener_unref (ke_layer_listener);
    AccessibleKeystrokeListener_unref (ke_keyecho_listener);
    AccessibleKeystrokeListener_unref (ke_user_key_listener);  
  
    SPI_freeAccessibleKeySet (ke_layer_keyset);
    SPI_freeAccessibleKeySet (ke_keyecho_keyset);    
	
    ke_keyboard_status = KE_IDLE;
    
    sru_debug ("done.");
}
