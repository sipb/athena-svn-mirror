
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
 *                         gdkvideoio.cpp  -  description
 *                         ------------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Class to permit to display in GDK Drawing Area or
 *                          SDL.
 *
 */


#include "../config.h"

#include "gdkvideoio.h"
#include "gnomemeeting.h"
#include "misc.h"
#include "menu.h"

#include "gconf_widgets_extensions.h"

#include <ptlib/vconvert.h>

#ifdef HAS_SDL
#include <SDL.h>
#endif

#define new PNEW

/* Global Variables */

extern GtkWidget *gm;


/* The Methods */
GDKVideoOutputDevice::GDKVideoOutputDevice(int idno, GmWindow *w)
{
#ifdef HAS_SDL
  BOOL start_in_fullscreen = FALSE;
#endif
  
  gw = w;

  /* Used to distinguish between input and output device. */
  device_id = idno; 

#ifdef HAS_SDL
  screen = NULL;
  overlay = NULL;

  gnomemeeting_threads_enter ();
  
  /* Do we start in fullscreen or not */
  start_in_fullscreen = 
    gconf_get_bool (VIDEO_DISPLAY_KEY "start_in_fullscreen");
  
  /* Change the zoom value following we have to start in fullscreen
   * or not */
  if (!start_in_fullscreen) {
    
    if (gconf_get_float (VIDEO_DISPLAY_KEY "zoom_factor") == -1.0)
      gconf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", 1.00);
  } 
  else {

    gconf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", -1.0);
  }

  gnomemeeting_threads_leave ();
#endif
}


GDKVideoOutputDevice::~GDKVideoOutputDevice()
{
  PWaitAndSignal m(redraw_mutex);

#ifdef HAS_SDL
  /* If it is needed, delete the SDL window */
  if (screen) {
    
    SDL_FreeSurface (screen);
    if (overlay)
      SDL_FreeYUVOverlay (overlay);
    screen = NULL;
    overlay = NULL;
    SDL_Quit ();
  }
#endif

  /* Hide the 2 popup windows */
  gnomemeeting_threads_enter ();
  gnomemeeting_window_hide (GTK_WIDGET (gw->local_video_window));
  gnomemeeting_window_hide (GTK_WIDGET (gw->remote_video_window));
  gnomemeeting_threads_leave ();
}


BOOL GDKVideoOutputDevice::Redraw ()
{
  GMH323EndPoint *ep = NULL;
  
  GtkWidget *image = NULL;
  GtkWidget *window = NULL;
  GdkPixbuf *src_pic = NULL;
  GdkPixbuf *zoomed_pic = NULL;
  GdkPixbuf *tmp = NULL;
  int image_width = 0, image_height = 0;
  PMutex both_mutex;

  gboolean unref = true;

  GdkInterpType interp_type = GDK_INTERP_NEAREST;
  int zoomed_width = GM_QCIF_WIDTH; 
  int zoomed_height = GM_QCIF_HEIGHT;

  int display = LOCAL_VIDEO;

  /* Common to the 2 instances of gdkvideoio */
  static GdkPixbuf *local_pic = NULL;
  static gboolean logo_displayed = false;
  static double zoom = 1.0;

#ifdef HAS_SDL
  static gboolean has_to_fs = false;
  static gboolean has_to_switch_fs = false;
  static gboolean old_fullscreen = false;
  static int fs_device = 0;
  Uint32 rmask, gmask, bmask, amask;
  SDL_Surface *surface = NULL;
  SDL_Surface *blit_conf = NULL;
#endif

  ep = GnomeMeeting::Process ()->Endpoint ();
  
  gnomemeeting_threads_enter ();

  
  /* Take the mutexes before the redraw */
  redraw_mutex.Wait ();


  /* Updates the zoom value */
  zoom = gconf_get_float (VIDEO_DISPLAY_KEY "zoom_factor");
  
  if (zoom != 0.5 && zoom != 1 && zoom != 2 && zoom != -1.00)
    zoom = 1.0;


  /* If we are not in a call, then display the local video. If we
   * are in a call, then display what GConf tells us, except if
   * it requests to display both video streams and that there is only
   * one available 
   */
  display = gconf_get_int (VIDEO_DISPLAY_KEY "video_view");

  if (!ep->CanAutoStartTransmitVideo () 
      || !ep->CanAutoStartReceiveVideo ()) {

    if (device_id == REMOTE)
      display = REMOTE_VIDEO;
    else if (device_id == LOCAL)
      display = LOCAL_VIDEO;
  }

  if (ep->GetCallingState () != GMH323EndPoint::Connected) 
    display = LOCAL_VIDEO;

  
  /* Update the display selection in the main and in the video popup menus */
  gtk_radio_menu_select_with_id (gw->main_menu, "local_video", display);
  gtk_radio_menu_select_with_id (gw->video_popup_menu, "local_video", display);

  
  /* Show or hide the different windows if needed */
  if (display == BOTH) { /* display == BOTH */

    /* Display the GnomeMeeting logo in the main window */
    if (!logo_displayed) {

      gnomemeeting_init_main_window_logo (gw->main_video_image);
      logo_displayed = true;
    }

    if (device_id == LOCAL && !GTK_WIDGET_VISIBLE (gw->local_video_window))
      gnomemeeting_window_show (GTK_WIDGET (gw->local_video_window));
    
    if (device_id == REMOTE && !GTK_WIDGET_VISIBLE (gw->remote_video_window))
      gnomemeeting_window_show (GTK_WIDGET (gw->remote_video_window));

    if (device_id == REMOTE) {

      image = gw->remote_video_image;
      window = gw->remote_video_window;
    }
    else {

      image = gw->local_video_image;
      window = gw->local_video_window;
    }

    gtk_window_get_size (GTK_WINDOW (window), 
			 &zoomed_width,
			 &zoomed_height);
  }
  else {

    logo_displayed = false;
    zoomed_width = (int) (frameWidth * zoom);
    zoomed_height = (int) (frameHeight * zoom);

    image = gw->main_video_image;

    if (display == BOTH_LOCAL) { /* display != BOTH && display == BOTH_LOCAL */

      /* Overwrite the zoom values and the GtkImage in which we will 
	 display if display == BOTH_LOCAL and that we receive
	 the data for the LOCAL window */
      if (device_id == LOCAL) {

	gtk_window_get_size (GTK_WINDOW (gw->local_video_window), 
			     &zoomed_width,
			     &zoomed_height);
      }

      if (device_id == LOCAL && !GTK_WIDGET_VISIBLE (gw->local_video_window))
	gnomemeeting_window_show (GTK_WIDGET (gw->local_video_window));
      
      if (GTK_WIDGET_VISIBLE (gw->remote_video_window))
	gnomemeeting_window_hide (GTK_WIDGET (gw->remote_video_window));
    }
    else { /* display != BOTH && display != BOTH_LOCAL */
      
      if (GTK_WIDGET_VISIBLE (gw->local_video_window))
	gnomemeeting_window_hide (GTK_WIDGET (gw->local_video_window));
      
      if (GTK_WIDGET_VISIBLE (gw->remote_video_window))
	gnomemeeting_window_hide (GTK_WIDGET (gw->remote_video_window));
    }
  }


#ifdef HAS_SDL
  /* If it is needed, delete the SDL window */
  sdl_mutex.Wait ();

  /* If we are in fullscreen, but that the option is not to be in
     fullscreen, delete the fullscreen SDL window */
  if (screen && zoom != -1.00) {
    
    SDL_FreeSurface (screen);
    screen = NULL;
    
    SDL_Quit ();
  }
  else
    /* If fullscreen is selected but that we are not in fullscreen, 
       call SDL init */
    if (zoom == -1.0 && !screen)
      SDL_Init (SDL_INIT_VIDEO);

  sdl_mutex.Signal ();



  if (zoom == -1.0) {
      
    zoomed_width = gconf_get_int (VIDEO_DISPLAY_KEY "fullscreen_width");
    zoomed_height = gconf_get_int (VIDEO_DISPLAY_KEY "fullscreen_height");
  }
#endif

  
  /* The real size picture */
  tmp =  
    gdk_pixbuf_new_from_data ((const guchar *) frameStore,
			      GDK_COLORSPACE_RGB, FALSE, 8, frameWidth,
			      frameHeight, frameWidth * 3, NULL, NULL);
  src_pic = gdk_pixbuf_copy (tmp);
  g_object_unref (tmp);


  if (gconf_get_bool (VIDEO_DISPLAY_KEY "enable_bilinear_filtering")) 
    interp_type = GDK_INTERP_BILINEAR;


  /* The zoomed picture */
  if ((zoomed_width != (int)frameWidth)||
      (zoomed_height != (int)frameHeight) ||
      (interp_type != GDK_INTERP_NEAREST)) {

    zoomed_pic = 
      gdk_pixbuf_scale_simple (src_pic, zoomed_width, 
			       zoomed_height, interp_type);
  }
  else {

    zoomed_pic = src_pic;
    unref = false;
  }

  /* Need to redefine screen size in main window (popups are already
     of the good size) ? */
  gtk_widget_get_size_request (GTK_WIDGET (gw->video_frame), 
			       &image_width, &image_height);

  if ( zoom != -1.0
       &&
      ((image_width != zoomed_width) || (image_height != zoomed_height)) 
       &&
      ( ((device_id == LOCAL) && (display == LOCAL_VIDEO)) ||
	((device_id == REMOTE) && (display == REMOTE_VIDEO)) ||
	((device_id == REMOTE) && (display == BOTH_LOCAL)) ||
	((display == BOTH_INCRUSTED) && (device_id == REMOTE)) ) ){

    gtk_widget_set_size_request (GTK_WIDGET (gw->video_frame),
				 zoomed_width, 
				 zoomed_height);
  }
  gnomemeeting_threads_leave ();


#ifdef HAS_SDL  
  /* Need to go full screen or to close the SDL window ? */
  gnomemeeting_threads_enter ();
  sdl_mutex.Wait ();
  has_to_fs = (zoom == -1.0);

  /* Need to toggle fullscreen */
  if (old_fullscreen == !has_to_fs) {

    has_to_switch_fs = true;
    old_fullscreen = has_to_fs;
  }

  SDL_Event event;
  
  /* If we are in full screen, check that "Esc" is not pressed */
  if (has_to_fs)
    while (SDL_PollEvent (&event)) {
  
      if (event.type == SDL_KEYDOWN) {

	/* Exit Full Screen */
	if ((event.key.keysym.sym == SDLK_ESCAPE) ||
	    (event.key.keysym.sym == SDLK_f)) {

	  has_to_fs = !has_to_fs;
	  gconf_set_float (VIDEO_DISPLAY_KEY "zoom_factor", 1.0);
          has_to_switch_fs = true;
	}
      }

    }
  

  if (has_to_switch_fs) {

    if (display == 0)
      fs_device = 1;
    else
      fs_device = 0;
  }
    
  sdl_mutex.Signal ();
  gnomemeeting_threads_leave ();
#endif


  /* We display what we transmit, or what we receive */
  if (zoom != -1.0) {

    if ( (((device_id == REMOTE && display == REMOTE_VIDEO) ||
           (device_id == LOCAL && display == LOCAL_VIDEO)) && (display != BOTH_INCRUSTED)) 
         || ((display == BOTH_LOCAL) && (device_id == REMOTE)) ) {

      gnomemeeting_threads_enter ();
      gtk_image_set_from_pixbuf (GTK_IMAGE (gw->main_video_image), 
                                 GDK_PIXBUF (zoomed_pic));
      gnomemeeting_threads_leave ();
    }


    if ( (display == BOTH) || 
         ((display == BOTH_LOCAL) && (device_id == LOCAL)) ) {

      gnomemeeting_threads_enter ();
      if (device_id == LOCAL)
        gtk_image_set_from_pixbuf (GTK_IMAGE (gw->local_video_image), 
                                   GDK_PIXBUF (zoomed_pic));
      else
        gtk_image_set_from_pixbuf (GTK_IMAGE (gw->remote_video_image), 
                                   GDK_PIXBUF (zoomed_pic));

      gnomemeeting_threads_leave ();
    }
  }

#ifdef HAS_SDL
  /* Display local in a SDL window, or the selected device in fullscreen */
  if ( (has_to_fs) && (device_id == fs_device) ) {
  
    gnomemeeting_threads_enter ();
    sdl_mutex.Wait ();
   
    /* Go fullscreen */
    if (has_to_switch_fs || (has_to_fs && !screen)) {
      
	screen = SDL_SetVideoMode (640, 480, 0, 
				   SDL_SWSURFACE | SDL_HWSURFACE | 
				   SDL_ANYFORMAT);
      
      SDL_WM_ToggleFullScreen (screen);
      SDL_ShowCursor (SDL_DISABLE);
      has_to_switch_fs = false;
    }


    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    surface =
      SDL_CreateRGBSurfaceFrom ((void *) gdk_pixbuf_get_pixels (zoomed_pic),
				zoomed_width, zoomed_height, 24,
				zoomed_width * 3, rmask, gmask, bmask, amask);


    blit_conf = SDL_DisplayFormat (surface);

    dest.x = (int) (screen->w - zoomed_width) / 2;
    dest.y = (int) (screen->h - zoomed_height) / 2;
    dest.w = zoomed_width;
    dest.h = zoomed_height;

    SDL_BlitSurface (blit_conf, NULL, screen, &dest);
    SDL_UpdateRect(screen, 0, 0, 640, 480);

    SDL_FreeSurface (surface);
    SDL_FreeSurface (blit_conf);
    sdl_mutex.Signal ();
    gnomemeeting_threads_leave ();
  }
#endif


  /* we display both of them */
  if (display == BOTH_INCRUSTED) {

    /* What we transmit, in small */
    if (device_id == LOCAL) {

      both_mutex.Wait ();
      gnomemeeting_threads_enter ();
  
      if (local_pic) {

	g_object_unref (local_pic);
	local_pic = NULL;
      }

      local_pic = 
 	gdk_pixbuf_scale_simple (zoomed_pic, GM_QCIF_WIDTH / 3, 
 				 GM_QCIF_HEIGHT / 3, 
 				 GDK_INTERP_NEAREST);
      gnomemeeting_threads_leave ();
      both_mutex.Signal ();
    }

    /* What we receive, in big */
    if ((device_id == REMOTE) && (local_pic))  {

      both_mutex.Wait ();
      gnomemeeting_threads_enter ();

      if (local_pic) {
	gdk_pixbuf_copy_area  (local_pic, 
			       0 , 0,
			       GM_QCIF_WIDTH / 3, GM_QCIF_HEIGHT / 3, 
			       zoomed_pic,
			       zoomed_width - GM_QCIF_WIDTH / 3, 
			       zoomed_height - GM_QCIF_HEIGHT / 3);
	
	gtk_image_set_from_pixbuf (GTK_IMAGE (gw->main_video_image), 
				   GDK_PIXBUF (zoomed_pic));

	g_object_unref (local_pic);
	local_pic = NULL;
      }
    
      gnomemeeting_threads_leave ();
      both_mutex.Signal ();
    }

  }


  gnomemeeting_threads_enter ();
  g_object_unref (G_OBJECT (src_pic));
  if (unref) 
    g_object_unref (G_OBJECT (zoomed_pic));
  gnomemeeting_threads_leave ();

  redraw_mutex.Signal ();
	
  return TRUE;
}


PStringList GDKVideoOutputDevice::GetDeviceNames() const
{
  PStringList  devlist;
  devlist.AppendString(GetDeviceName());

  return devlist;
}


BOOL GDKVideoOutputDevice::IsOpen ()
{
  return TRUE;
}


BOOL GDKVideoOutputDevice::SetFrameData(
					unsigned x,
					unsigned y,
					unsigned width,
					unsigned height,
					const BYTE * data,
					BOOL endFrame)
{
  if (x+width > frameWidth || y+height > frameHeight)
    return FALSE;

  if (!endFrame)
    return FALSE;

  frameStore.SetSize (width * height * 3);
  
  if (converter)
    converter->Convert (data, frameStore.GetPointer ());
  
  EndFrame ();
  
  return TRUE;
}


BOOL GDKVideoOutputDevice::EndFrame()
{
  Redraw ();
  
  return TRUE;
}


BOOL GDKVideoOutputDevice::SetColourFormat (const PString & colour_format)
{
  if (colour_format == "BGR24")
    return PVideoOutputDevice::SetColourFormat (colour_format);

  return FALSE;  
}
