/*  stock-icons.c
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2002 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Started: Mon 17 June 2002
 *
 *  Authors: Jorn Baayen <jorn@nl.linux.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 */

#include <gtk/gtk.h>
#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "../pixmaps/inlines.h"
#include "stock-icons.h"

/**
 * gnomemeeting_stock_icons_init:
 *
 * Initializes the GnomeMeeting stock icons 
 *
 **/
void
gnomemeeting_stock_icons_init (void)
{
	GtkIconFactory *factory;
	int i;

        typedef struct 
        {
                char *id;
                const guint8 *data;
        } GmStockIcon;                

	static GmStockIcon items[] =
	{
	        { GM_STOCK_ADDRESSBOOK_24, gm_addressbook_24_stock_data },
	        { GM_STOCK_ADDRESSBOOK_16, gm_addressbook_16_stock_data },
		{ GM_STOCK_TEXT_CHAT,     gm_text_chat_stock_data },
		{ GM_STOCK_CONTROL_PANEL, gm_control_panel_stock_data },
		{ GM_STOCK_CONNECT,       gm_connect_stock_data },
		{ GM_STOCK_DISCONNECT,    gm_disconnect_stock_data },
		{ GM_STOCK_VIDEO_PREVIEW, gm_video_preview_stock_data },
		{ GM_STOCK_AUDIO_MUTE,    gm_audio_mute_stock_data },
		{ GM_STOCK_VOLUME,        gm_volume_stock_data },
		{ GM_STOCK_MICROPHONE,    gm_microphone_stock_data },
                { GM_STOCK_SPEAKER_PHONE, gm_speaker_phone_stock_data },
		{ GM_STOCK_VIDEO_MUTE,    gm_video_mute_stock_data },
		{ GM_STOCK_STATUS_AVAILABLE, gm_status_available_stock_data },
		{ GM_STOCK_STATUS_RINGING,   gm_status_ringing_stock_data},
		{ GM_STOCK_STATUS_BUSY, gm_status_busy_stock_data},
		{ GM_STOCK_STATUS_FORWARD, gm_status_forward_stock_data },
		{ GM_STOCK_STATUS_FREE_FOR_CHAT, gm_status_free_for_chat_stock_data },
		{ GM_STOCK_STATUS_IN_A_CALL, gm_status_in_a_call_stock_data },
	
		{ GM_STOCK_DRUID_AUDIO, gm_druid_audio_stock_data},
		{ GM_STOCK_DRUID_VIDEO, gm_druid_video_stock_data},
		{ GM_STOCK_DRUID_IXJ, gm_druid_ixj_stock_data},
		{ GM_STOCK_DRUID_PERSONAL, gm_druid_personal_stock_data},
		{ GM_STOCK_DRUID_CONNECTION, gm_druid_connection_stock_data},
		{ GM_STOCK_REMOTE_CONTACT, gm_remote_contact_stock_data},
		{ GM_STOCK_LOCAL_CONTACT, gm_local_contact_stock_data},
		{ GM_STOCK_WHITENESS, gm_whiteness_stock_data},
		{ GM_STOCK_BRIGHTNESS, gm_brightness_stock_data},
		{ GM_STOCK_COLOURNESS, gm_colourness_stock_data},
		{ GM_STOCK_CONTRAST, gm_contrast_stock_data},
		{ GM_STOCK_CONNECT_16, gm_connect_16_stock_data},
		{ GM_STOCK_DISCONNECT_16, gm_disconnect_16_stock_data},
		{ GM_STOCK_CALLS_HISTORY, gm_calls_history_stock_data},
	};

	factory = gtk_icon_factory_new ();
	gtk_icon_factory_add_default (factory);

	for (i = 0; i < (int) G_N_ELEMENTS (items); i++)
	{
		GtkIconSet *icon_set;
		GdkPixbuf *pixbuf;

                pixbuf = gdk_pixbuf_new_from_inline (-1, items[i].data, FALSE, NULL);

		icon_set = gtk_icon_set_new_from_pixbuf (pixbuf);

		gtk_icon_factory_add (factory, items[i].id, icon_set);
		gtk_icon_set_unref (icon_set);
		
		g_object_unref (G_OBJECT (pixbuf));
	}

	g_object_unref (G_OBJECT (factory));
}
