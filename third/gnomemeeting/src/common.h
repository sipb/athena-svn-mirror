
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         common.h  -  description
 *                         ------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains things common to the whole soft.
 *
 */


#ifndef GM_COMMON_H_
#define GM_COMMON_H_

#include <openh323buildopts.h>
#include <ptbuildopts.h>

#include <ptlib.h>
#include <h323.h>

#ifndef DISABLE_GNOME
#include <gnome.h>
#else
#include <gtk/gtk.h>
#endif

#ifndef DISABLE_GCONF
#include <gconf/gconf-client.h>
#else
#include "../lib/win32/gconf-simu.h"
#endif

#ifdef WIN32
#include <string.h>
#define strcasecmp strcmpi
#define vsnprintf _vsnprintf
#endif

#include "menu.h"


#define GENERAL_KEY         "/apps/gnomemeeting/general/"

#define USER_INTERFACE_KEY "/apps/gnomemeeting/general/user_interface/"
#define VIDEO_DISPLAY_KEY USER_INTERFACE_KEY "video_display/"
#define SOUND_EVENTS_KEY  "/apps/gnomemeeting/general/sound_events/"
#define AUDIO_DEVICES_KEY "/apps/gnomemeeting/devices/audio/"
#define VIDEO_DEVICES_KEY "/apps/gnomemeeting/devices/video/"
#define PERSONAL_DATA_KEY "/apps/gnomemeeting/general/personal_data/"
#define CALL_OPTIONS_KEY "/apps/gnomemeeting/general/call_options/"
#define NAT_KEY "/apps/gnomemeeting/general/nat/"
#define H323_ADVANCED_KEY "/apps/gnomemeeting/protocols/h323/advanced/"
#define H323_GATEKEEPER_KEY "/apps/gnomemeeting/protocols/h323/gatekeeper/"
#define H323_GATEWAY_KEY "/apps/gnomemeeting/protocols/h323/gateway/"
#define PORTS_KEY "/apps/gnomemeeting/protocols/h323/ports/"
#define CALL_FORWARDING_KEY "/apps/gnomemeeting/protocols/h323/call_forwarding/"
#define LDAP_KEY "/apps/gnomemeeting/protocols/ldap/"
#define AUDIO_CODECS_KEY "/apps/gnomemeeting/codecs/audio/"
#define VIDEO_CODECS_KEY  "/apps/gnomemeeting/codecs/video/"
#define CONTACTS_KEY        "/apps/gnomemeeting/contacts/"
#define CONTACTS_GROUPS_KEY "/apps/gnomemeeting/contacts/groups/"

#define GM_CIF_WIDTH   352
#define GM_CIF_HEIGHT  288
#define GM_QCIF_WIDTH  176
#define GM_QCIF_HEIGHT 144
#define GM_SIF_WIDTH   320
#define GM_SIF_HEIGHT  240
#define GM_QSIF_WIDTH  160
#define GM_QSIF_HEIGHT 120
#define GM_FRAME_SIZE  10

#define GM_MAIN_NOTEBOOK_HIDDEN 4

#define GNOMEMEETING_PAD_SMALL 1

#define GM_WINDOW(x) (GmWindow *)(x)

#ifndef _
#ifdef DISABLE_GNOME
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#endif
#endif


typedef struct _GmWindow GmWindow;
typedef struct _GmPrefWindow GmPrefWindow;
typedef struct _GmLdapWindow GmLdapWindow;
typedef struct _GmLdapWindowPage GmLdapWindowPage;
typedef struct _GmTextChat GmTextChat;
typedef struct _GmDruidWindow GmDruidWindow;
typedef struct _GmCallsHistoryWindow GmCallsHistoryWindow;
typedef struct _GmRtpData GmRtpData;


/* Type of section */
typedef enum {
  CONTACTS_SERVERS,
  CONTACTS_GROUPS
} SectionType;


/* Incoming Call Mode */
typedef enum {

  AVAILABLE,
  FREE_FOR_CHAT,
  BUSY,
  FORWARD,
  NUM_MODES
} IncomingCallMode;


/* Control Panel Section */
typedef enum {

  STATISTICS,
  DIALPAD,
  AUDIO_SETTINGS,
  VIDEO_SETTINGS,
  CLOSED,
  NUM_SECTIONS
} ControlPanelSection;


struct _GmTextChat
{
  GtkWidget     *text_view;
  GtkTextBuffer *text_buffer;
  gboolean	something_typed;
  gchar		*begin_msg;
};


struct _GmRtpData
{
  int   tr_audio_bytes;
  float tr_audio_speed [100];
  int   tr_audio_pos;
  float tr_video_speed [100];
  int   tr_video_pos;
  int   tr_video_bytes;
  int   re_audio_bytes;
  float re_audio_speed [100];
  int   re_audio_pos;
  int   re_video_bytes;
  float re_video_speed [100];
  int   re_video_pos;
};


struct _GmWindow
{
  GtkTooltips *tips;
  GtkWidget *main_menu;
  GtkWidget *tray_popup_menu;
  GtkWidget *video_popup_menu;
  GtkWidget *audio_transmission_popup;
  GtkWidget *audio_reception_popup;
  GtkObject *adj_play;
  GtkObject *adj_rec;
  GtkObject *adj_whiteness;
  GtkObject *adj_brightness;
  GtkObject *adj_colour;
  GtkObject *adj_contrast;
  GtkWidget *docklet;
  GtkWidget *video_settings_frame;
  GtkWidget *audio_settings_frame;
  GtkWidget *statusbar;
  GtkWidget *remote_name;
  GtkWidget *splash_win;
  GtkWidget *combo;
  GtkWidget *log_window;
  GtkWidget *log_text_view;
  GtkWidget *main_notebook;
  GtkWidget *main_video_image;
  GtkWidget *local_video_image;
  GtkWidget *local_video_window;
  GtkWidget *remote_video_image;
  GtkWidget *remote_video_window;
  GtkWidget *video_frame;
  GtkWidget *pref_window;
  GtkWidget *ldap_window;
  GtkWidget *chat_window;
  GtkWidget *calls_history_window;
  GtkWidget *pc_to_phone_window;
  GtkWidget *preview_button;
  GtkWidget *connect_button;
  GtkWidget *video_chan_button;
  GtkWidget *audio_chan_button;
  GtkWidget *incoming_call_popup;
  GtkWidget *transfer_call_popup;
  GtkWidget *stats_label;
  GtkWidget *stats_drawing_area;

#ifndef DISABLE_GNOME
  GtkWidget *druid_window;
#endif

  GdkColor colors [6];


  PStringArray video_devices;
  PStringArray audio_recorder_devices;
  PStringArray audio_player_devices;
  PStringArray audio_managers;
  PStringArray video_managers;
};


struct _GmLdapWindow
{
  GtkWidget *main_menu;
  GtkWidget *notebook;
  GtkWidget *tree_view;
  GtkWidget *option_menu;
};


struct _GmLdapWindowPage
{
  GtkWidget *section_name;
  GtkWidget *tree_view;
  GtkWidget *statusbar;
  GtkWidget *option_menu;
  GtkWidget *search_entry;

  PThread *ils_browser;
  PMutex search_quit_mutex;
  
  gchar *contact_section_name;
  gint page_type;
};


struct _GmDruidWindow
{
#ifndef DISABLE_GNOME
  GnomeDruid *druid;
#endif
  GtkWidget *ils_register;
  GtkWidget *audio_test_button;
  GtkWidget *video_test_button;
  GtkWidget *enable_microtelco;
  GtkWidget *kind_of_net;
  GtkWidget *progress;
  GtkWidget *audio_manager;
  GtkWidget *video_manager;
  GtkWidget *audio_player;
  GtkWidget *audio_recorder;
  GtkWidget *video_device;
  GtkWidget *gk_alias;
  GtkWidget *gk_password;
  GtkWidget *name;
  GtkWidget *use_callto;
  GtkWidget *mail;
#ifndef DISABLE_GNOME
  GnomeDruidPageEdge *page_edge;
#endif
};


struct _GmCallsHistoryWindow
{
  GtkListStore *given_calls_list_store;
  GtkListStore *received_calls_list_store;
  GtkListStore *missed_calls_list_store;
  GtkWidget *search_entry;
};


struct _GmPrefWindow
{
  GtkListStore *codecs_list_store;
  GtkWidget *sound_events_list;
  GtkWidget *audio_player;
  GtkWidget *audio_recorder;
  GtkWidget *video_device;
};
#endif /* GM_COMMON_H */
