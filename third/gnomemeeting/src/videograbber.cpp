
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
 * GnomeMeeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         videograbber.cpp  -  description
 *                         --------------------------------
 *   begin                : Mon Feb 12 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Video4Linux compliant functions to manipulate the 
 *                          webcam device.
 *
 */


#include "../config.h"

#include "videograbber.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "tools.h"

#include "dialog.h"
#include "gconf_widgets_extensions.h"


/* Declarations */
extern GtkWidget *gm;


/* The functions */
GMVideoGrabber::GMVideoGrabber (BOOL start_grabbing,
				BOOL sync)
  :PThread (1000, NoAutoDeleteThread)
{
  /* Variables */
  height = 0;
  width = 0;

  whiteness = 0;
  brightness = 0;
  colour = 0;
  contrast = 0;

  gw = GnomeMeeting::Process ()->GetMainWindow ();

  
  /* Internal state */
  stop = FALSE;
  is_grabbing = start_grabbing;
  synchronous = sync;
  is_opened = FALSE;

  
  /* Initialisation */
  encoding_device = NULL;
  video_channel = NULL;
  grabber = NULL;

  if (synchronous)
    VGOpen ();

  
  /* Start the thread */
  this->Resume ();
  thread_sync_point.Wait ();
}


GMVideoGrabber::~GMVideoGrabber ()
{
  is_grabbing = FALSE;
  stop = TRUE;

  /* Wait for the device to be unlocked */
  PWaitAndSignal q(device_mutex);

  /* Wait for the Main () method to be terminated */
  PWaitAndSignal m(quit_mutex);
}


void
GMVideoGrabber::Main ()
{
  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();
 
  if (!synchronous)
    VGOpen ();

  while (!stop) {

    var_mutex.Wait ();
    if (is_grabbing == 1 && video_channel) {

      video_channel->Read (video_buffer, height * width * 3);
      video_channel->Write (video_buffer, height * width * 3);    
    }
    var_mutex.Signal ();

    Current()->Sleep (20);
  }

  VGClose ();
}


void
GMVideoGrabber::StartGrabbing (void)
{
  PWaitAndSignal m(var_mutex);
  
  is_grabbing = 1;
}


void
GMVideoGrabber::StopGrabbing (void)
{
  PWaitAndSignal m(var_mutex);

  is_grabbing = 0;
}


BOOL
GMVideoGrabber::IsGrabbing (void)
{
  PWaitAndSignal m(var_mutex);

  return is_grabbing;
}


GDKVideoOutputDevice *
GMVideoGrabber::GetEncodingDevice (void)
{
  PWaitAndSignal m(var_mutex);
  
  return encoding_device;
}


PVideoChannel *
GMVideoGrabber::GetVideoChannel (void)
{
  PWaitAndSignal m(var_mutex);
  
  return video_channel;
}


void
GMVideoGrabber::SetColour (int colour)
{
  PWaitAndSignal m(var_mutex);

  if (grabber)
    grabber->SetColour (colour);
}


void
GMVideoGrabber::SetBrightness (int brightness)
{
  PWaitAndSignal m(var_mutex);

  if (grabber)
    grabber->SetBrightness (brightness);
}


void
GMVideoGrabber::SetWhiteness (int whiteness)
{
  PWaitAndSignal m(var_mutex);

  if (grabber)
    grabber->SetWhiteness (whiteness);
}


void
GMVideoGrabber::SetContrast (int constrast)
{
  PWaitAndSignal m(var_mutex);
  
  if (grabber)
    grabber->SetContrast (constrast);
}


void
GMVideoGrabber::GetParameters (int *whiteness,
			       int *brightness, 
			       int *colour,
			       int *contrast)
{
  int hue = 0;
  
  PWaitAndSignal m(var_mutex);
  
  grabber->GetParameters (whiteness, brightness, colour, contrast, &hue);

  *whiteness = (int) *whiteness / 256;
  *brightness = (int) *brightness / 256;
  *colour = (int) *colour / 256;
  *contrast = (int) *contrast / 256;
}


BOOL
GMVideoGrabber::IsChannelOpen ()
{
  PWaitAndSignal m(var_mutex);

  if (video_channel && video_channel->IsOpen ()) 
    return TRUE;
  
  return FALSE;
}


void
GMVideoGrabber::Lock ()
{
  device_mutex.Wait ();
}


void
GMVideoGrabber::Unlock ()
{
  device_mutex.Signal ();
}


void
GMVideoGrabber::VGOpen (void)
{
  GMH323EndPoint *ep = NULL;
  
  PString input_device;
  PString plugin;

  gchar *dialog_title = NULL;
  gchar *dialog_msg = NULL;
  gchar *tmp_msg = NULL;
  gchar *gconf_value = NULL;
  
  int error_code = 0;
  int channel = 0;
  int size = 0;

  BOOL no_device_found = FALSE;
  
  PVideoDevice::VideoFormat format = PVideoDevice::PAL;

  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if (!is_opened) {
    
    /* Get the video device options from the GConf database */
    gnomemeeting_threads_enter ();

    gconf_value = gconf_get_string (VIDEO_DEVICES_KEY "input_device");
    input_device = gconf_value;
    g_free (gconf_value);
    
    gconf_value = gconf_get_string (VIDEO_DEVICES_KEY "plugin");
    plugin = gconf_value;
    g_free (gconf_value);
    
    channel = gconf_get_int (VIDEO_DEVICES_KEY "channel");

    size = gconf_get_int (VIDEO_DEVICES_KEY "size");

    format =
      (PVideoDevice::VideoFormat) gconf_get_int (VIDEO_DEVICES_KEY "format");

    height = (size == 0) ? GM_QCIF_HEIGHT : GM_CIF_HEIGHT; 
    width = (size == 0) ? GM_QCIF_WIDTH : GM_CIF_WIDTH;

    no_device_found = (input_device == _("No device found"));
    gnomemeeting_threads_leave ();


    /* If there is no device, directly open the fake device */
    if (!no_device_found) {
 
      gnomemeeting_threads_enter ();
      gnomemeeting_log_insert (_("Opening video device %s with plugin %s"), (const char *) input_device, (const char *) plugin);
      gnomemeeting_threads_leave ();

      var_mutex.Wait ();
      grabber = 
	PVideoInputDevice::CreateOpenedDevice (plugin, input_device, FALSE);
      if (!grabber)
	error_code = 1;
      else if (!grabber->SetVideoFormat (format))
	error_code = 2;
      else if (!grabber->SetChannel (channel))
	error_code = 3;
      else if (!grabber->SetColourFormatConverter ("YUV420P"))
	error_code = 4;
      else if (!grabber->SetFrameRate (30))
	error_code = 5;
      else if (!grabber->SetFrameSizeConverter (width, height, FALSE))
	error_code = 6;
      var_mutex.Signal ();
    

      /* If no error */
      if (!error_code) {

	gnomemeeting_threads_enter ();
	gnomemeeting_log_insert (_("Successfully opened video device %s, channel %d"), (const char *) input_device, channel);
	gnomemeeting_threads_leave ();
      }
      else {
	
	/* If we want to open the fake device for a real error, and not because
	   the user chose the Picture device */
	gnomemeeting_threads_enter ();
	dialog_title =
	  g_strdup_printf (_("Error while opening video device %s"),
			   (const char *) input_device);

	/* Translators: Do not translate MovingLogo and Picture */
	tmp_msg = g_strdup (_("A moving GnomeMeeting logo will be transmitted during calls. Notice that you can always transmit a given image or the moving GnomeMeeting logo by choosing \"Picture\" as video plugin and \"MovingLogo\" or \"StaticPicture\" as device."));
	gnomemeeting_log_insert (_("Couldn't open the video device"));
	switch (error_code) {
	  
	case 1:
	  dialog_msg = g_strconcat (tmp_msg, "\n\n", _("There was an error while opening the device. Please check your permissions and make sure that the appropriate driver is loaded."), NULL);
	  break;
	  
	case 2:
	  dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Your video driver doesn't support the requested video format."), NULL);
	  break;

	case 3:
	  dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Could not open the chosen channel."), NULL);
	  break;
      
	case 4:
	  dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Your driver doesn't seem to support any of the colour formats supported by GnomeMeeting.\n Please check your kernel driver documentation in order to determine which Palette is supported."), NULL);
	  break;
	  
	case 5:
	  dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Error while setting the frame rate."), NULL);
	  break;

	case 6:
	  dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Error while setting the frame size."), NULL);
	  break;
	}

	gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (gm),
					       VIDEO_DEVICES_KEY "enable_preview",
					       dialog_title,
					       dialog_msg);
	g_free (dialog_msg);
	g_free (dialog_title);
	g_free (tmp_msg);

	gnomemeeting_threads_leave ();
      }
    }
      
    if (error_code || no_device_found) {
	
      /* delete the failed grabber and open the fake grabber */
      var_mutex.Wait ();
      if (grabber) {
	
	delete grabber;
	grabber = NULL;
      }

      grabber =
	PVideoInputDevice::CreateOpenedDevice ("Picture",
					       "MovingLogo",
					       FALSE);
      if (grabber) {
	
	grabber->SetColourFormatConverter ("YUV420P");
	grabber->SetVideoFormat (PVideoDevice::PAL);
	grabber->SetChannel (1);    
	grabber->SetFrameRate (6);
	grabber->SetFrameSizeConverter (width, height, FALSE);
	
	gnomemeeting_threads_enter ();
	gnomemeeting_log_insert (_("Opened the video device using the \"Picture\" video plugin"));
	gnomemeeting_threads_leave ();
      }
      var_mutex.Signal ();
    }
   

    if (grabber)
      grabber->Start ();

    var_mutex.Wait ();
    video_channel = new PVideoChannel ();
    encoding_device = new GDKVideoOutputDevice (1, gw);
    encoding_device->SetColourFormatConverter ("YUV420P");

    if (grabber)
      video_channel->AttachVideoReader (grabber);
    video_channel->AttachVideoPlayer (encoding_device);

    is_opened = TRUE;
    var_mutex.Signal ();
  
    encoding_device->SetFrameSize (width, height);  

      
    /* Setup the video settings */
    GetParameters (&whiteness, &brightness, &colour, &contrast);
    gnomemeeting_threads_enter ();
    GTK_ADJUSTMENT (gw->adj_brightness)->value = brightness;
    GTK_ADJUSTMENT (gw->adj_whiteness)->value = whiteness;
    GTK_ADJUSTMENT (gw->adj_colour)->value = colour;
    GTK_ADJUSTMENT (gw->adj_contrast)->value = contrast;
    gtk_widget_queue_draw (GTK_WIDGET (gw->video_settings_frame));
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame),
			      TRUE);
    gnomemeeting_threads_leave ();

      
    /* Update the GUI sensitivity if not in a call */
    if (ep->GetCallingState () == GMH323EndPoint::Standby) {

      gnomemeeting_threads_enter ();      
      gnomemeeting_menu_update_sensitivity (TRUE, FALSE, TRUE);
      gnomemeeting_threads_leave ();
    }
  }
}
  

void
GMVideoGrabber::VGClose ()
{
  GMH323EndPoint *ep = NULL;

  ep = GnomeMeeting::Process ()->Endpoint ();
  
  if (is_opened) {

    var_mutex.Wait ();
    is_grabbing = 0;
    if (video_channel) 
      delete (video_channel);
    var_mutex.Signal ();
    
    
    /* Update menu sensitivity if we are not in a call */
    gnomemeeting_threads_enter ();
     if (ep->GetCallingState () == GMH323EndPoint::Standby
	&& !gconf_get_bool (VIDEO_DEVICES_KEY "enable_preview")) {
      
      gnomemeeting_menu_update_sensitivity (TRUE, FALSE, FALSE);
      gnomemeeting_init_main_window_logo (gw->main_video_image);
    }
    gtk_widget_set_sensitive (GTK_WIDGET (gw->video_settings_frame), FALSE);
    gnomemeeting_threads_leave ();
    
    
    /* Initialisation */
    var_mutex.Wait ();
    video_channel = NULL;
    is_opened = FALSE;
    encoding_device = NULL;
    grabber = NULL;
    var_mutex.Signal ();
  }

  
  /* Quick Hack for buggy drivers that return from the ioctl before the device
     is really closed */
  PThread::Current ()->Sleep (1000);
}


/* The video tester class */
GMVideoTester::GMVideoTester (gchar *m,
			      gchar *r)
  :PThread (1000, AutoDeleteThread)
{
#ifndef DISABLE_GNOME
  if (m)
    video_manager = PString (m);
  if (r)
    video_recorder = PString (r);

  test_dialog = NULL;
  test_label = NULL;
  
  this->Resume ();
  thread_sync_point.Wait ();
#endif
}


GMVideoTester::~GMVideoTester ()
{
#ifndef DISABLE_GNOME
 PWaitAndSignal m(quit_mutex);
#endif
}


void GMVideoTester::Main ()
{
#ifndef DISABLE_GNOME
  GmWindow *gw = NULL;
  GmDruidWindow *dw = NULL;

  PVideoInputDevice *grabber = NULL;
  
  int height = GM_QCIF_HEIGHT; 
  int width = GM_QCIF_WIDTH; 
  int error_code = -1;
  int cpt = 0;

  gchar *dialog_msg = NULL;
  gchar *tmp = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  dw = GnomeMeeting::Process ()->GetDruidWindow ();

  PWaitAndSignal m(quit_mutex);
  thread_sync_point.Signal ();

  if (video_recorder.IsEmpty ()
      || video_manager.IsEmpty ()
      || video_recorder == PString (_("No device found"))
      || video_recorder == PString (_("Picture")))
    return;
  
  gdk_threads_enter ();
  test_dialog =
    gtk_dialog_new_with_buttons ("Video test running",
				 GTK_WINDOW (gw->druid_window),
				 (GtkDialogFlags) (GTK_DIALOG_MODAL),
				 GTK_STOCK_OK,
				 GTK_RESPONSE_ACCEPT,
				 NULL);
  dialog_msg = 
    g_strdup_printf (_("GnomeMeeting is now testing the %s video device. If you experience machine crashes, then report a bug to the video driver author."), (const char *) video_recorder);
  test_label = gtk_label_new (dialog_msg);
  gtk_label_set_line_wrap (GTK_LABEL (test_label), true);
  g_free (dialog_msg);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (test_dialog)->vbox), test_label,
		      FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (test_dialog)->vbox), 
		      gtk_hseparator_new (), FALSE, FALSE, 2);

  test_label = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (test_dialog)->vbox), 
		      test_label, FALSE, FALSE, 2);

  g_signal_connect (G_OBJECT (test_dialog), "delete-event",
		    G_CALLBACK (gtk_widget_hide_on_delete), NULL);

  gtk_window_set_transient_for (GTK_WINDOW (test_dialog),
				GTK_WINDOW (gw->druid_window));
  gtk_widget_show_all (test_dialog);
  gdk_threads_leave ();

  
  while (cpt < 6 && error_code == -1) {

    if (!video_recorder.IsEmpty ()
	&& !video_manager.IsEmpty ()) {

      error_code = -1;
      
      grabber = 
	PVideoInputDevice::CreateOpenedDevice (video_manager,
					       video_recorder,
					       FALSE);

      if (!grabber)
	error_code = 0;
      else
	if (!grabber->SetVideoFormat (PVideoDevice::Auto))
	  error_code = 2;
      else
        if (!grabber->SetChannel (0))
          error_code = 2;
      else
	if (!grabber->SetColourFormatConverter ("YUV420P"))
	  error_code = 3;
      else
	if (!grabber->SetFrameRate (30))
	  error_code = 4;
      else
	if (!grabber->SetFrameSizeConverter (width, height, FALSE))
	  error_code = 5;
      else
	grabber->Close ();

      if (grabber)
	delete (grabber);
    }


    if (error_code == -1) 
      dialog_msg = g_strdup_printf (_("Test %d done"), cpt);
    else
      dialog_msg = g_strdup_printf (_("Test %d failed"), cpt);

    tmp = g_strdup_printf ("<b>%s</b>", dialog_msg);
    gdk_threads_enter ();
    gtk_label_set_markup (GTK_LABEL (test_label), tmp);
    gdk_threads_leave ();
    g_free (dialog_msg);
    g_free (tmp);

    cpt++;
    PThread::Current () ->Sleep (100);
  }

  
  if (error_code != - 1) {
    
    switch (error_code)	{
	  
    case 0:
      dialog_msg = g_strdup_printf (_("Error while opening %s."),
			     (const char *) video_recorder);
      break;
      
    case 1:
      dialog_msg = g_strdup_printf (_("Your video driver doesn't support the requested video format."));
      break;
      
    case 2:
      dialog_msg = g_strdup_printf (_("Could not open the chosen channel with the chosen video format."));
      break;
      
    case 3:
      dialog_msg = g_strdup_printf (_("Your driver doesn't support any of the color formats tried by GnomeMeeting"));
      break;
      
    case 4:
      dialog_msg = g_strdup_printf ( _("Error with the frame rate."));
      break;
      
    case 5:
      dialog_msg = g_strdup_printf (_("Error with the frame size."));
      break;
    }

    gdk_threads_enter ();
    gnomemeeting_error_dialog (GTK_WINDOW (gw->druid_window),
			       _("Failed to open the device"),
			       dialog_msg);
    gdk_threads_leave ();
    
    g_free (dialog_msg);
  }

  gdk_threads_enter ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dw->video_test_button),
				FALSE);
  if (test_dialog)
    gtk_widget_destroy (test_dialog);
  gdk_threads_leave ();
#endif
}
