/* libsrconf.h
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

#ifndef _LIBSRCONF_H
#define _LIBSRCONF_H

#include <glib.h>
#include <SREvent.h>
#include <gconf/gconf-value.h>

#ifdef INET6
    #define DEFAULT_BRLMON_IP		"::1"
#else
    #define DEFAULT_BRLMON_IP		"127.0.0.1"
#endif
#ifdef INET6
    #define DEFAULT_REMOTE_IP		"::1"
#else
    #define DEFAULT_REMOTE_IP		"127.0.0.1"
#endif

#define REMOTE_CONFIG_PATH 			"config/remote"
#define BRLMON_CONFIG_PATH 			"config/brlmon"


#define DEFAULT_BRLMON_PORT			7000
#define DEFAULT_REMOTE_PORT			7096
/*____________________________< BRAILLE_DEFAULTS>___________________________*/
#define MIN_SERIAL_PORT_NO			1
#define MAX_SERIAL_PORT_NO			4

#define MIN_TTY_CONSOL_NO			1
#define MAX_TTY_CONSOL_NO			10

#define MIN_BRAILLE_PORT 			1
#define MAX_BRAILLE_PORT 			10

#define DEFAULT_BRAILLE_ATTRIBUTES		8
#define DEFAULT_BRAILLE_STYLE 			"8"
#define DEFAULT_BRAILLE_CURSOR_STYLE		"underline"
#define DEFAULT_BRAILLE_TRANSLATION 		"de"
#define DEFAULT_BRAILLE_DEVICE 			"VARIO20"
#define DEFAULT_BRAILLE_DEVICE_INDEX 		0
#define DEFAULT_BRAILLE_DEVICE_COUNT		0
#define DEFAULT_BRAILLE_PORT_NO 		1
#define DEFAULT_BRAILLE_FILL_CHAR 		"##"
#define DEFAULT_BRAILLE_STATUS_CELL 		"CursorPos"
#define DEFAULT_BRAILLE_OPTICAL_SENSOR 		0
#define DEFAULT_BRAILLE_POSITION_SENSOR		2
/*____________________________< /BRAILLE_DEFAULTS>___________________________*/
/*____________________________< BRAILLE_MONITOR_DEFAULTS>___________________________*/
#define DEFAULT_BRAILLE_MONITOR_PANEL_POSITION   	"TOP"
#define DEFAULT_BRAILLE_MONITOR_MODE			"normal"
#define DEFAULT_BRAILLE_MONITOR_DOT78_COLOR		"#000000007D00"
#define DEFAULT_BRAILLE_MONITOR_DOT7_COLOR		"#7D0000000000"
#define DEFAULT_BRAILLE_MONITOR_DOT8_COLOR		"#00007D000000"
#define DEFAULT_BRAILLE_MONITOR_LINE			2
#define DEFAULT_BRAILLE_MONITOR_COLUMN			40
#define DEFAULT_BRAILLE_MONITOR_USE_THEME		FALSE
#define DEFAULT_BRAILLE_MONITOR_IS_PANEL		TRUE
#define DEFAULT_BRAILLE_MONITOR_FONT_SIZE		18
/*____________________________< /BRAILLE_MONITOR_DEFAULTS>___________________________*/
/*____________________________< SRCORE_DEFAULTS>_____________________________*/
#define DEFAULT_SRCORE_MAGNIF_ACTIVE		FALSE
#define DEFAULT_SRCORE_SPEECH_ACTIVE		TRUE
#define DEFAULT_SRCORE_BRAILLE_ACTIVE		FALSE
#define DEFAULT_SRCORE_BRAILLE_MONITOR_ACTIVE	FALSE

#define DEFAULT_SRCORE_BRAILLE_SENSITIVE 	TRUE
#define DEFAULT_SRCORE_BRAILLE_MONITOR_SENSITIVE TRUE
#define DEFAULT_SRCORE_SPEECH_SENSITIVE 	TRUE
#define DEFAULT_SRCORE_MAGNIFIER_SENSITIVE 	TRUE

#define DEFAULT_SRCORE_SCREEN_REVIEW		0x00000077
#define DEFAULT_SRCORE_LANGUAGE			"en"

#define DEFAULT_SRCORE_MINIMIZE			FALSE
#define DEFAULT_SRCORE_FIND_TEXT		""

/*____________________________< SRCORE_DEFAULTS>____________________________*/
/*____________________________< SPEECH_DEFAULTS>____________________________*/
#define SPEECH_RATE_STEP			15
#define SPEECH_VOLUME_STEP			5
#define SPEECH_PITCH_STEP			25

#define MAX_SPEECH_RATE				400
#define MIN_SPEECH_RATE				100
#define MAX_SPEECH_PITCH 			500
#define MIN_SPEECH_PITCH 			10
#define MAX_SPEECH_VOLUME 			100
#define MIN_SPEECH_VOLUME 			0

#define DEFAULT_SPEECH_PREEMPT 			TRUE
#define DEFAULT_SPEECH_RATE 			150
#define DEFAULT_SPEECH_PITCH 			100
#define DEFAULT_SPEECH_VOLUME 			100
#define DEFAULT_SPEECH_COUNT_TYPE		"NONE"
#define DEFAULT_SPEECH_VOICE_COUNT		0
#define DEFAULT_SPEECH_CAPITAL			CAPITAL_IGNORE
#define DEFAULT_SPEECH_PUNCTUATION		"IGNORE"
#define DEFAULT_SPEECH_TEXT_ECHO		"CHARACTER"
#define DEFAULT_SPEECH_MODIFIERS		"NONE"
#define DEFAULT_SPEECH_CURSORS			"NONE"
#define DEFAULT_SPEECH_SPACES			"ALL"
#define DEFAULT_SPEECH_ENGINE_VOICE		"V0 kevin - FreeTTS Gnome Speech Driver"
#define DEFAULT_SPEECH_ENGINE_DRIVER 		"FreeTTS_Gnome_Speech_Driver"
#define DEFAULT_SPEECH_DICTIONARY_ACTIVE	"NO"
#define DEFAULT_SPEECH_DICTIONARY_ENTRY 	"USA<>United States Of America" 
/*____________________________< /SPEECH_DEFAULTS>___________________________*/
/*____________________________< KEYBOARD_DEFAULTS>___________________________*/
#define DEFAULT_KEYBOARD_TAKE_MOUSE		FALSE
#define DEFAULT_KEYBOARD_SIMULATE_CLICK		FALSE
/*____________________________< /KEYBOARD_DEFAULTS>___________________________*/
/*____________________________< PRESENTATION_DEFAULTS>___________________________*/
#define DEFAULT_PRESENTATION_DEVICE 		"braille"
#define DEFAULT_PRESENTATION_ROLE		"generic"
#define DEFAULT_PRESENTATION_EVENT		"generic"

#define DEFAULT_PRESENTATION_ACTIVE_SETTING_NAME 	"default"
/*____________________________< /PRESENTATION_DEFAULTS>___________________________*/
/*____________________________< MAGNIFIER_DEFAULTS>___________________________*/
#define MAX_ZOOM_FACTOR_X  			15
#define MIN_ZOOM_FACTOR_X  			1
#define MAX_ZOOM_FACTOR_Y  			15
#define MIN_ZOOM_FACTOR_Y  			1

#define MAX_CURSOR_SIZE   			256
#define MIN_CURSOR_SIZE   			1

#define MAX_CROSSWIRE_SIZE   			256
#define MIN_CROSSWIRE_SIZE   			1

#define CURSOR_SIZE_INVALID  			0
#define CURSOR_MAG_INVALID   			1

#define DEFAULT_MAGNIFIER_DISPLAY_SIZE_X	640
#define DEFAULT_MAGNIFIER_DISPLAY_SIZE_Y	480

#define DEFAULT_MAGNIFIER_SCHEMA 		"schema1"

#define DEFAULT_MAGNIFIER_CURSOR		TRUE
#define DEFAULT_MAGNIFIER_CURSOR_NAME		"default"
#define DEFAULT_MAGNIFIER_CURSOR_SIZE		32
#define DEFAULT_MAGNIFIER_CURSOR_SCALE		FALSE
#define DEFAULT_MAGNIFIER_CURSOR_COLOR		0

#define DEFAULT_MAGNIFIER_CROSSWIRE		TRUE
#define DEFAULT_MAGNIFIER_CROSSWIRE_CLIP	FALSE	
#define DEFAULT_MAGNIFIER_CROSSWIRE_SIZE	10
#define DEFAULT_MAGNIFIER_CROSSWIRE_COLOR	0

#define DEFAULT_MAGNIFIER_ZP_LEFT		320
#define DEFAULT_MAGNIFIER_ZP_TOP		0
#define DEFAULT_MAGNIFIER_ZP_WIDTH		G_MAXINT
#define DEFAULT_MAGNIFIER_ZP_HEIGHT		G_MAXINT
#define DEFAULT_MAGNIFIER_BORDER_WIDTH		2
#define DEFAULT_MAGNIFIER_BORDER_COLOR		0

#define DEFAULT_MAGNIFIER_ID			"generic_zoomer"
#define DEFAULT_MAGNIFIER_SOURCE		":0.0"
#define DEFAULT_MAGNIFIER_TARGET		":0.0"

#define DEFAULT_MAGNIFIER_ZOOM_FACTOR_XY	2
#define DEFAULT_MAGNIFIER_ZOOM_FACTOR_LOCK	TRUE

#define DEFAULT_MAGNIFIER_INVERT		FALSE
#define DEFAULT_MAGNIFIER_SMOOTHING		"none"

#define DEFAULT_MAGNIFIER_TRACKING		"focus"
#define DEFAULT_MAGNIFIER_MOUSE_TRACKING	"mouse-push"

#define DEFAULT_MAGNIFIER_ALIGNMENT_X		"auto"
#define DEFAULT_MAGNIFIER_ALIGNMENT_Y		"auto"


#define DEFAULT_MAGNIFIER_VISIBLE		TRUE

#define DEFAULT_MAGNIFIER_PANNING		FALSE	

#define DEFAULT_SCREEN_MIN_SIZE			0
#define DEFAULT_SCREEN_MAX_SIZE			1023

/*____________________________</MAGNIFIER_DEFAULTS>___________________________*/
/*___________________________________________________________________________*/
#define CMD_MIN_LAYER 				0
#define CMD_MAX_LAYER 				15
/*___________________________________________________________________________*/

/*____________________________<BRAILLE>_______________________________________*/
#define BRAILLE_ATTRIBUTES			"attribute"
#define BRAILLE_STYLE 				"braille_style"
#define BRAILLE_CURSOR_STYLE			"cursor_style"
#define BRAILLE_TRANSLATION 			"translation_table"
#define BRAILLE_DEVICE 				"device"
#define BRAILLE_PORT_NO 			"port_no"
#define BRAILLE_FILL_CHAR 			"fill_char"
#define BRAILLE_STATUS_CELL 			"status"
#define BRAILLE_OPTICAL_SENSOR 			"optical"
#define BRAILLE_POSITION_SENSOR			"position"
#define BRAILLE_DEVICE_COUNT			"brldev_count"
#define BRAILLE_DEFAULT_DEVICE			"brldev_default"
/*____________________________</BRAILLE>______________________________________*/
/*____________________________<SRCORE>________________________________________*/
#define SRCORE_EXIT_KEY				"exit"
#define SRCORE_EXIT_ACK_KEY			"exit_ack"

#define SRCORE_INITIALIZATION_END		"init_end"

#define SRCORE_UI_COMMAND			"ui_command"

#define SRCORE_FIND_TYPE			"find_type"
#define SRCORE_FIND_TEXT			"find_text"

#define SRCORE_MAGNIF_ACTIVE			"mag_active"
#define SRCORE_SPEECH_ACTIVE			"sp_active"
#define SRCORE_BRAILLE_ACTIVE			"br_active"
#define SRCORE_BRAILLE_MONITOR_ACTIVE		"bm_active"

#define SRCORE_LANGUAGE				"language"
#define SRCORE_SCREEN_REVIEW			"screen_review"

#define SRCORE_KEY_PATH				"srcore/"
#define SRCORE_MINIMIZE_PATH 			"config/show_gnopernicus_min"
#define SRCORE_BRLMON_IP			"config/brlmon/ip"

#define SRCORE_MAGNIF_SENSITIVE			"mag_sensitive"
#define SRCORE_SPEECH_SENSITIVE			"sp_sensitive"
#define SRCORE_BRAILLE_SENSITIVE		"br_sensitive"
#define SRCORE_BRAILLE_MONITOR_SENSITIVE	"bm_sensitive"
/*____________________________</SRCORE>_______________________________________*/
/*____________________________<KEYBOARD>______________________________________*/
#define KEYBOARD_TAKE_MOUSE			"take_mouse"
#define KEYBOARD_SIMULATE_CLICK 		"simulate_click"
/*____________________________</KEYBOARD>_____________________________________*/
/*____________________________< MAGNIFIER>____________________________________*/
#define MAGNIFIER_DISPLAY_SIZE_X		"display_size_x"
#define MAGNIFIER_DISPLAY_SIZE_Y		"display_size_y"

#define MAGNIFIER_CURSOR			"cursor"
#define MAGNIFIER_CURSOR_NAME 			"cursor_name"
#define MAGNIFIER_CURSOR_SIZE 			"cursor_size"
#define MAGNIFIER_CURSOR_SCALE    		"cursor_scale"
#define MAGNIFIER_CURSOR_COLOR			"cursor_color"

#define MAGNIFIER_CROSSWIRE			"crosswire"
#define MAGNIFIER_CROSSWIRE_CLIP		"crosswire_clip"
#define MAGNIFIER_CROSSWIRE_SIZE 		"crosswire_size"
#define MAGNIFIER_CROSSWIRE_COLOR 		"crosswire_color"

#define MAGNIFIER_ZP_LEFT			"zp_left"
#define MAGNIFIER_ZP_TOP 			"zp_top"
#define MAGNIFIER_ZP_WIDTH			"zp_width"
#define MAGNIFIER_ZP_HEIGHT			"zp_height"
#define MAGNIFIER_BORDER_WIDTH 			"border_width"
#define MAGNIFIER_BORDER_COLOR 			"border_color"

#define MAGNIFIER_ID				"id"
#define MAGNIFIER_SOURCE			"source"
#define MAGNIFIER_TARGET			"target"

#define MAGNIFIER_ZOOM_FACTOR_X			"zoom_factor_x"
#define MAGNIFIER_ZOOM_FACTOR_Y			"zoom_factor_y"
#define MAGNIFIER_ZOOM_FACTOR_LOCK		"zoom_factor_lock"

#define MAGNIFIER_INVERT			"invert"
#define MAGNIFIER_SMOOTHING			"smoothing"

#define MAGNIFIER_TRACKING			"tracking"
#define MAGNIFIER_MOUSE_TRACKING		"mouse_tracking"

#define MAGNIFIER_ALIGNMENT_X			"alignment_x"
#define MAGNIFIER_ALIGNMENT_Y			"alignment_y"

#define MAGNIFIER_VISIBLE 			"visible"
#define MAGNIFIER_PANNING	  		"panning"
/*____________________________</MAGNIFIER>____________________________________*/
/*____________________________<SPEECH>________________________________________*/

#define SPEECH_COUNT				"count"
#define SPEECH_CAPITAL	 			"capital"
#define SPEECH_PUNCTUATION 			"punctuation"
#define SPEECH_TEXT_ECHO			"text_echo"
#define SPEECH_MODIFIERS			"modifiers"
#define SPEECH_CURSORS				"cursors"
#define SPEECH_SPACES				"spaces"
#define SPEECH_ENGINE_DRIVERS			"engine_drivers" 
#define SPEECH_VOICE_COUNT 			"voice_count"
#define SPEECH_VOICE_TEST			"voice_test"
#define SPEECH_GNOPERNICUS_SPEAKERS 		"gnopernicus_speakers"
#define SPEECH_VOICE_CHANGES	 		"voice_changes"
#define SPEECH_VOICE_REMOVED	 		"voice_removed"
#define SPEECH_DICTIONARY_ACTIVE		"dictionary_active"
#define SPEECH_DICTIONARY_LIST			"dictionary_list"
#define SPEECH_DICTIONARY_CHANGES		"dictionary_changes"
/*____________________________</SPEECH>_______________________________________*/
/*____________________________<COMMAND MAP>___________________________________*/
#define SRC_KEY_SECTION 			"command_map/keyboard"

#define SRC_KEY_PAD_SECTION 			"command_map/key_pad"
#define SRC_BRAILLE_KEY_SECTION 		"command_map/braille_key"
#define SRC_USER_DEF_SECTION 			"command_map/user_key"

#define SRC_KEY_PAD_LIST  			"key_pad_list"
#define SRC_BRAILLE_KEY_LIST			"braille_key_list"
#define SRC_USER_DEF_LIST			"user_key_list"

#define CMDMAP_KEY_PAD_LIST_PATH 		"command_map/key_pad/key_pad_list"
#define CMDMAP_BRAILLE_KEYS_LIST_PATH 		"command_map/braille_key/braille_key_list"
#define CMDMAP_USER_DEF_LIST_PATH 		"command_map/user_key/user_key_list"

#define CMDMAP_CHANGES_END			"/changes_end"
/*____________________________</COMMAND MAP>___________________________________*/
/*____________________________<PRESENTATION>___________________________________*/
#define PRESENTATION_CHANGES_END 		"/changes_end"
#define PRESENTATION_SETTING_LIST		"presentation_list"
#define PRESENTATION_ACTIVE_SETTING		"active_setting"
/*____________________________</PRESENTATION>__________________________________*/
/*____________________________</BRAILLE MONITOR>__________________________________*/
#define BRAILLE_MONITOR_PANEL_POSITION_KEY 	"brlmon_position"
#define BRAILLE_MONITOR_COLUMN_KEY	 	"cell_column"
#define BRAILLE_MONITOR_LINE_KEY		"cell_line"
#define BRAILLE_MONITOR_MODE_KEY		"display_mode"
#define BRAILLE_MONITOR_DOT7_KEY 	 	"dot7"
#define BRAILLE_MONITOR_DOT8_KEY 	 	"dot8"
#define BRAILLE_MONITOR_DOT78_KEY 	 	"dot78"
#define BRAILLE_MONITOR_USE_THEME_KEY		"use_theme_color"
#define BRAILLE_MONITOR_IS_PANEL_KEY		"is_brlmon_panel"
#define BRAILLE_MONITOR_FONT_SIZE_KEY		"font_size"
/*____________________________</BRAILLE MONITOR>__________________________________*/
/*____________________________<SECTIONS>__________________________________*/

#define CONFIG_PATH 				"/apps/gnopernicus/"

#define BRAILLE_MONITOR_PATH 			"brlmon"
#define BRAILLE_MONITOR_KEY_PATH 		"brlmon/"

#define BRAILLE_PATH 				"braille"
#define BRAILLE_KEY_PATH			"braille/"

#define MAGNIFIER_PATH 				"magnifier"
#define MAGNIFIER_CONFIG_PATH			"magnifier/schema1/generic_zoomer"
#define MAGNIFIER_ACTIVE_SCHEMA 		"schema1/generic_zoomer/"

#define SRCORE_PATH 				"srcore"	
#define KEYBOARD_PATH 				"kbd_mouse"
#define PRESENTATION_PATH			"presentation"
#define COMMAND_MAP_PATH			"command_map"

#define SPEECH_PATH  				"speech"
#define SPEECH_SETTING_KEY_PATH  		"speech/settings/"
#define SPEECH_DRIVERS_PATH			"speech/drivers/"
#define SPEECH_VOICE_PARAM_KEY_PATH  		"speech/voices_parameters/"
#define SPEECH_VOICE_KEY_PATH			"speech/voice/"
#define SPEECH_DICTIONARY_PATH			"speech/dictionary/"

#define SPEECH_SETTING_SECTION   		"speech/setting"
#define SPEECH_DRIVERS_SECTION			"speech/drivers"
#define SPEECH_PARAMETER_SECTION 		"speech/voices_parameters"
#define SPEECH_VOICE_SECTION     		"speech/voice"
#define SPEECH_DICTIONARY_SECTION		"speech/dictionary"

/*____________________________</SECTIONS>__________________________________*/
/**
 * SRConfigurablesEnum:
 *
 * The configurable modules of Gnopernicus,
**/
typedef enum {
	CFGM_BRAILLE = 1,
	CFGM_GNOPI,
	CFGM_KBD_MOUSE,
	CFGM_MAGNIFIER,
	CFGM_SRCORE,
	CFGM_SPEECH,
	CFGM_SPEECH_VOICE_PARAM,
	CFGM_SPEECH_VOICE,
	CFGM_KEY_PAD,
	CFGM_PRESENTATION
} SRConfigurablesEnum;

/**
 * SRConfigTypesEnum:
 *
 * types used for configuration
**/
typedef enum {
    CFGT_BOOL	= GCONF_VALUE_BOOL,
    CFGT_INT	= GCONF_VALUE_INT,
    CFGT_FLOAT	= GCONF_VALUE_FLOAT,
    CFGT_STRING = GCONF_VALUE_STRING,
    CFGT_LIST   = GCONF_VALUE_LIST,
    CFGT_UNSET	= GCONF_VALUE_INVALID
} SRConfigTypesEnum;

typedef struct {
    gchar *key;
    SRConfigTypesEnum type;
    void  *settings;
} DefaultSettings;

/**
 * SRConfigStructure;:
 *
 * Structure for send configuration changes
**/
typedef struct {
    SRConfigurablesEnum module;
    gchar 		*key;
    SRConfigTypesEnum   type;
    gpointer 		newvalue;
} SRConfigStructure;

/**
 * SRConfCB:
 *
 * prototype of the function used for communication.
 * The SR_EVENT's Source member indicates which module's configuration
 * changed by a SRConfigurablesEnum. The flags is not used yet.
 *
**/
#define SRConfCB SROnEventProc



gboolean
srconf_free_slist (GSList *list);

/**
 * srconf_init:
 *
 * This function initialize Gconf and register the notifications
 *	srconfcb - the callback function used to send data to caller
 *	gconf_root_dir_path - the root dir of GConf
**/
gboolean 
srconf_init(SRConfCB srconfcb, const gchar *_gconf_root_dir_path, const gchar *config_source);

/**
 * srconf_terminate:
 *
 * Deregister notifications and destroy additional objects
 *
**/
void 
srconf_terminate();

/**
 *
 * Convert SRConfigTypesEnum to GConfValueType
 *
**/
GConfValueType 
srconf_convert_SRConfigTypesEnum_to_GConfValueType(SRConfigTypesEnum type);

/**
 * srconf_get_config_data_with_data
 *
 * function used to get configuration information
 * 	key - <in>, the name of the configuration information to be get
 * 	conftype - <in>, the type of config data
 *	data - <out>, a pointer to a memory location where srconf_get_config_data
 *		will put the config data. The CFGT_STRINGS must be freed by user.
 *		At CFGT_LIST type if used default value the returned pointer is same 
 *		with default_data
 *	default_data - returned value if the key not exist
 *	confmodule - <in>, which module's configuration directory to be used
 *	return  - return TRUE if use default value.
**/

gboolean 
srconf_get_config_data_with_default(const gchar *key, SRConfigTypesEnum conftype, gpointer data,gpointer default_data,SRConfigurablesEnum confmodule);

#define GET_BRAILLE_CONFIG_DATA_WITH_DEFAULT(key,ct,p,def)	\
	(srconf_get_config_data_with_default((key),(ct),(p),(def),CFGM_BRAILLE))
#define GET_GNOPI_CONFIG_DATA_WITH_DEFAULT(key,ct,p,def)	\
	(srconf_get_config_data_with_default((key),(ct),(p),(def),CFGM_GNOPI))
#define GET_KBD_MOUSE_CONFIG_DATA_WITH_DEFAULT(key,ct,p,def)	\
	(srconf_get_config_data_with_default((key),(ct),(p),(def),CFGM_KBD_MOUSE))
#define GET_MAGNIFIER_CONFIG_DATA_WITH_DEFAULT(key,ct,p,def)	\
	(srconf_get_config_data_with_default((key),(ct),(p),(def),CFGM_MAGNIFIER))
#define GET_SPEECH_CONFIG_DATA_WITH_DEFAULT(key,ct,p,def)	\
	(srconf_get_config_data_with_default((key),(ct),(p),(def),CFGM_SPEECH))
#define GET_SRCORE_CONFIG_DATA_WITH_DEFAULT(key,ct,p,def)	\
	(srconf_get_config_data_with_default((key),(ct),(p),(def),CFGM_SRCORE))
#define GET_SRLOW_CONFIG_DATA_WITH_DEFAULT(key,ct,p,def)	\
	(srconf_get_config_data_with_default((key),(ct),(p),(def),CFGM_SRLOW))

/**
 * srconf_get_data_with_default:
 *
 * function used to get configuration information
 * 	key - <in>, the name of the configuration information to be get
 * 	conftype - <in>, the type of config data
 *	data - <out>, a pointer to a memory location where srconf_get_config_data
 *		will put the config data. The CFGT_STRINGs must be freed by user
 *		At CFGT_LIST type if used default value the returned pointer is same 
 *		with default_data
 *	default_data - returned value if the key is not exist
 *	section - <in>, which section want to write (format Section/Section1)
  *	return  - return TRUE if use default value.
**/

gboolean 
srconf_get_data_with_default(const gchar *key, SRConfigTypesEnum conftype, gpointer data,gpointer default_data , const gchar *section);

/**
 * srconf_set_config_data:
 *
 * function used to set configuration information
 * 	key - <in>, the name of the configuration information to be set
 * 	conftype - <in>, the type of config data
 *	data - <in>, a pointer to a memory location where srconf_set_config_data
 *		can find the config data. This function do not free anything.
 *	confmodule - <in>, which module's configuration directory to be used
**/
gboolean 
srconf_set_config_data(const gchar *key, SRConfigTypesEnum conftype, const gpointer data,SRConfigurablesEnum confmodule);

#define SET_BRAILLE_CONFIG_DATA(key,ct,p)	\
	(srconf_set_config_data((key),(ct),(p),CFGM_BRAILLE))
#define SET_GNOPI_CONFIG_DATA(key,ct,p)		\
	(srconf_set_config_data((key),(ct),(p),CFGM_GNOPI))
#define SET_KBD_MOUSE_CONFIG_DATA(key,ct,p)	\
	(srconf_set_config_data((key),(ct),(p),CFGM_KBD_MOUSE))
#define SET_MAGNIFIER_CONFIG_DATA(key,ct,p)	\
	(srconf_set_config_data((key),(ct),(p),CFGM_MAGNIFIER))
#define SET_SPEECH_CONFIG_DATA(key,ct,p)	\
	(srconf_set_config_data((key),(ct),(p),CFGM_SPEECH))
#define SET_SRCORE_CONFIG_DATA(key,ct,p)	\
	(srconf_set_config_data((key),(ct),(p),CFGM_SRCORE))
#define SET_SRLOW_CONFIG_DATA(key,ct,p)		\
	(srconf_set_config_data((key),(ct),(p),CFGM_SRLOW))

/**
 * srconf_set_data:
 *
 * function used to set configuration information
 * 	key - <in>, the name of the configuration information to be set
 * 	conftype - <in>, the type of config data
 *	data - <in>, a pointer to a memory location where srconf_set_config_data
 *		can find the config data. This function do not free anything.
 *	section - <in>, which section want to write (format Section/Section1)
**/
gboolean 
srconf_set_data(const gchar *key, SRConfigTypesEnum conftype, const gpointer data,const gchar *section);


/**
 *
 * Unset a specified key.
 * @key - key what it unset
 *
**/
gboolean
srconf_unset_key (const gchar *key,const gchar *section);

gchar*
srconf_presentationi_get_chunk (const gchar *device_role_event);
#endif /*_LIBSRCONF_H */
