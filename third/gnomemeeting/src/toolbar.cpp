
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
 *                        toolbar.cpp  -  description
 *                        ---------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the toolbar.
 *
 */


#include "../config.h"


#include "toolbar.h"
#include "gnomemeeting.h"
#include "callbacks.h"
#include "config.h"
#include "misc.h" 

#include "stock-icons.h"
#include "history-combo.h"
#include "gconf_widgets_extensions.h"

/* Declarations */
extern GtkWidget *gm;


static void url_combo_changed            (GtkEditable  *, gpointer);

static void connect_button_clicked       (GtkToggleButton *, gpointer);
static void toolbar_button_changed       (GtkWidget *, gpointer);
static void toolbar_cp_button_changed    (GtkWidget *, gpointer);


/* Static functions */
static void url_combo_changed (GtkEditable  *e, gpointer data)
{
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

  gchar *tip_text = (gchar *)
    gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (data)->entry));

  gtk_tooltips_set_tip (gw->tips, GTK_WIDGET (GTK_COMBO (data)->entry), 
			tip_text, NULL);
}


/* DESCRIPTION  :  This callback is called when the user toggles the 
 *                 connect button.
 * BEHAVIOR     :  Connect or disconnect.
 * PRE          :  /
 */
static void connect_button_clicked (GtkToggleButton *w, gpointer data)
{
  if (gtk_toggle_button_get_active (w))  
    connect_cb (NULL, NULL);  
  else
    disconnect_cb (NULL, NULL);
}


/* DESCRIPTION  :  This callback is called when the user presses the control 
 *                 panel button in the toolbar. 
 *                 (See menu_toggle_changed)
 * BEHAVIOR     :  Updates the gconf cache : 0 or 3 (off) for the cp section.
 * PRE          :  data is the key.
 */
static void toolbar_cp_button_changed (GtkWidget *w, gpointer data)
{
  if (gconf_get_int ((gchar *) data) 
      == GM_MAIN_NOTEBOOK_HIDDEN) { 
    
    gconf_set_int ((gchar *) data, 0);
  } 
  else {   
    gconf_set_int ((gchar *) data, GM_MAIN_NOTEBOOK_HIDDEN);
  }
}


/* DESCRIPTION  :  This callback is called when the user presses a
 *                 button in the toolbar. 
 *                 (See menu_toggle_changed)
 * BEHAVIOR     :  Updates the gconf cache.
 * PRE          :  data is the key.
 */
static void toolbar_button_changed (GtkWidget *widget, gpointer data)
{
  bool shown = gconf_get_bool ((gchar *) data);

  gconf_set_bool ((gchar *) data, !shown);
}



/* The functions */
GtkWidget *
gnomemeeting_init_main_toolbar ()
{
  GtkWidget *hbox = NULL;
  GtkWidget *image = NULL;

  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();
  

  /* The main horizontal toolbar */
  GtkWidget *toolbar = gtk_toolbar_new ();


  /* Combo */
  gw->combo =
    gm_history_combo_new (USER_INTERFACE_KEY "main_window/urls_history");

  gtk_combo_set_use_arrows_always (GTK_COMBO(gw->combo), TRUE);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (gw->combo)->entry), "h323:");

  gtk_tooltips_set_tip (gw->tips, GTK_WIDGET (GTK_COMBO (gw->combo)->entry), 
			"h323:", NULL);

  gtk_combo_disable_activate (GTK_COMBO (gw->combo));
  g_signal_connect (G_OBJECT (GTK_COMBO (gw->combo)->entry), "activate",
  		    G_CALLBACK (connect_cb), NULL);

  hbox = gtk_hbox_new (FALSE, 2);

  gtk_box_pack_start (GTK_BOX (hbox), gw->combo, TRUE, TRUE, 1);
  gtk_box_pack_start (GTK_BOX (hbox), toolbar, FALSE, FALSE, 1);
 
  gtk_container_set_border_width (GTK_CONTAINER (toolbar), 2);

  g_signal_connect (G_OBJECT (GTK_WIDGET (GTK_COMBO(gw->combo)->entry)),
		    "changed", G_CALLBACK (url_combo_changed), 
		    (gpointer)  (gw->combo));


  /* The connect button */
  gw->connect_button = gtk_toggle_button_new ();
  gtk_tooltips_set_tip (gw->tips, GTK_WIDGET (gw->connect_button), 
			_("Enter an URL to call on the left, and click on this button to connect to the given URL"), NULL);
  
  image = gtk_image_new_from_stock (GM_STOCK_DISCONNECT, 
                                    GTK_ICON_SIZE_LARGE_TOOLBAR);

  gtk_container_add (GTK_CONTAINER (gw->connect_button), GTK_WIDGET (image));
  g_object_set_data (G_OBJECT (gw->connect_button), "image", image);

  gtk_widget_set_size_request (GTK_WIDGET (gw->connect_button), 28, 28);

  gtk_toolbar_append_widget (GTK_TOOLBAR (toolbar), gw->connect_button,
			     NULL, NULL);

  g_signal_connect (G_OBJECT (gw->connect_button), "clicked",
                    G_CALLBACK (connect_button_clicked), 
		    gw->connect_button);

  gtk_widget_show (GTK_WIDGET (gw->combo));
  gtk_widget_show_all (GTK_WIDGET (gw->connect_button));
  gtk_widget_show_all (GTK_WIDGET (hbox));
  gtk_widget_show_all (GTK_WIDGET (toolbar));

  return hbox;
}


GtkWidget *gnomemeeting_init_left_toolbar (void)
{
  GtkWidget *image = NULL;
  GtkWidget *left_toolbar = NULL;

  PString dev;
  
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();


  left_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_orientation (GTK_TOOLBAR (left_toolbar), 
			       GTK_ORIENTATION_VERTICAL);

  image =
    gtk_image_new_from_stock (GM_STOCK_TEXT_CHAT, 
			      GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (image);
  gtk_toolbar_append_item (GTK_TOOLBAR (left_toolbar),
			   NULL,
			   _("Open text chat"), 
			   NULL,
			   image,
			   GTK_SIGNAL_FUNC (toolbar_button_changed),
			   (gpointer) USER_INTERFACE_KEY "main_window/show_chat_window");
  
  image = gtk_image_new_from_stock (GM_STOCK_CONTROL_PANEL, 
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (image);
  gtk_toolbar_append_item (GTK_TOOLBAR (left_toolbar),
			   NULL,
			   _("Open control panel"),
			   NULL,
			   image,
			   GTK_SIGNAL_FUNC (toolbar_cp_button_changed),
			   (gpointer) USER_INTERFACE_KEY "main_window/control_panel_section");

  
  image = gtk_image_new_from_stock (GM_STOCK_ADDRESSBOOK_24,
				    GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_show (image);
  gtk_toolbar_append_item (GTK_TOOLBAR (left_toolbar),
			   NULL,
			   _("Open address book"),
			   NULL,
			   image,
			   GTK_SIGNAL_FUNC (show_window_cb),
			   (gpointer) gw->ldap_window); 

  gtk_toolbar_set_style (GTK_TOOLBAR (left_toolbar), GTK_TOOLBAR_ICONS);

  
  /* Video Preview Button */
  gw->preview_button = gtk_toggle_button_new ();

  image = gtk_image_new_from_stock (GM_STOCK_VIDEO_PREVIEW, 
                                    GTK_ICON_SIZE_MENU);

  gtk_container_add (GTK_CONTAINER (gw->preview_button), GTK_WIDGET (image));
  GTK_TOGGLE_BUTTON (gw->preview_button)->active =
    gconf_get_bool (VIDEO_DEVICES_KEY "enable_preview");

  g_signal_connect (G_OBJECT (gw->preview_button), "toggled",
		    G_CALLBACK (toggle_changed),
		    (gpointer) VIDEO_DEVICES_KEY "enable_preview");

  gtk_tooltips_set_tip (gw->tips, gw->preview_button,
			_("Display images from your camera device"),
			NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (left_toolbar), 
			     gw->preview_button, NULL, NULL);


  /* Audio Channel Button */
  gw->audio_chan_button = gtk_toggle_button_new ();
 
  image = gtk_image_new_from_stock (GM_STOCK_AUDIO_MUTE, 
                                    GTK_ICON_SIZE_MENU);

  gtk_container_add (GTK_CONTAINER (gw->audio_chan_button), 
		     GTK_WIDGET (image));

  gtk_widget_set_sensitive (GTK_WIDGET (gw->audio_chan_button), FALSE);

  g_signal_connect (G_OBJECT (gw->audio_chan_button), "clicked",
		    G_CALLBACK (pause_channel_callback), GINT_TO_POINTER (0));

  gtk_tooltips_set_tip (gw->tips, gw->audio_chan_button,
			_("Audio transmission status. During a call, click here to suspend or resume the audio transmission."), NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (left_toolbar), 
			     gw->audio_chan_button, NULL, NULL);


  /* Video Channel Button */
  gw->video_chan_button = gtk_toggle_button_new ();

  image = gtk_image_new_from_stock (GM_STOCK_VIDEO_MUTE,
				    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (gw->video_chan_button), 
		     GTK_WIDGET (image));

  gtk_widget_set_sensitive (GTK_WIDGET (gw->video_chan_button), FALSE);

  g_signal_connect (G_OBJECT (gw->video_chan_button), "clicked",
		    G_CALLBACK (pause_channel_callback), GINT_TO_POINTER (1));

  gtk_tooltips_set_tip (gw->tips, gw->video_chan_button,
			_("Video transmission status. During a call, click here to suspend or resume the video transmission."), NULL);

  gtk_toolbar_append_widget (GTK_TOOLBAR (left_toolbar), 
			     gw->video_chan_button, NULL, NULL);

  gtk_widget_show_all (GTK_WIDGET (gw->preview_button));
  gtk_widget_show_all (GTK_WIDGET (gw->audio_chan_button));
  gtk_widget_show_all (GTK_WIDGET (gw->video_chan_button));


  return left_toolbar;
}


void connect_button_update_pixmap (GtkToggleButton *button, int pressed)
{
  GtkWidget *image = NULL;
  
  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  image = (GtkWidget *) g_object_get_data (G_OBJECT (button), "image");
  
  if (image != NULL)	{
    
    if (pressed == 1) 
      {
        gtk_image_set_from_stock (GTK_IMAGE (image),
                                  GM_STOCK_CONNECT, 
                                  GTK_ICON_SIZE_LARGE_TOOLBAR);
        
        /* Block the signal */
        g_signal_handlers_block_by_func (G_OBJECT (button), 
                                         (void *) connect_button_clicked, 
                                         (gpointer) gw->connect_button);
        gtk_toggle_button_set_active (button, TRUE);
        g_signal_handlers_unblock_by_func (G_OBJECT (button), 
                                           (void *) connect_button_clicked, 
                                           (gpointer) gw->connect_button);
      } else {
        
        gtk_image_set_from_stock (GTK_IMAGE (image),
                                  GM_STOCK_DISCONNECT, 
                                  GTK_ICON_SIZE_LARGE_TOOLBAR);
        
        g_signal_handlers_block_by_func (G_OBJECT (button), 
                                         (void *) connect_button_clicked, 
                                         (gpointer) gw->connect_button);
        gtk_toggle_button_set_active (button, FALSE);
        g_signal_handlers_unblock_by_func (G_OBJECT (button), 
                                           (void *) connect_button_clicked, 
                                           (gpointer) gw->connect_button);
      }   
    
    gtk_widget_queue_draw (GTK_WIDGET (image));
    gtk_widget_queue_draw (GTK_WIDGET (button));
  }
}
