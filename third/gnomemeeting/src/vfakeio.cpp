
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
 *                         vfakeio.cpp  -  description
 *                         ---------------------------
 *   begin                : Tue Jul 30 2003
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains a descendant of a Fake Input 
 *                          Device that display the GM logo when connected to
 *                          a remote party without using a camera.
 *
 */


#include "../config.h"

#include "vfakeio.h"
#include <ptlib/vconvert.h>

#include "misc.h"
#include "gconf_widgets_extensions.h"

#include "../pixmaps/text_logo.xpm"


GMH323FakeVideoInputDevice::GMH323FakeVideoInputDevice ()
{
  orig_pix = NULL;
  cached_pix = NULL;
	
  pos = 0;
  increment = 1;

  moving = false;
}


GMH323FakeVideoInputDevice::~GMH323FakeVideoInputDevice ()
{
  Close ();
}



BOOL
GMH323FakeVideoInputDevice::Open (const PString &name,
				  BOOL start_immediate)
{
  gchar *image_name = NULL;
    
  if (IsOpen ())
    return FALSE;
  
  if (name == "MovingLogo") {
  
    moving = true;
    orig_pix = gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);
    
    return TRUE;
  }

  /* from there on, we're in the static picture case! */
  moving = false;
  
  image_name = gconf_get_string (VIDEO_DEVICES_KEY "image");
  orig_pix =  gdk_pixbuf_new_from_file (image_name, NULL);
  g_free (image_name);

  if (orig_pix) 
    return TRUE;

  return FALSE;
}


BOOL
GMH323FakeVideoInputDevice::IsOpen ()
{
  if (orig_pix) 
    return TRUE;
  
  return FALSE;
}


BOOL
GMH323FakeVideoInputDevice::Close ()
{
  gnomemeeting_threads_enter ();
  if (orig_pix != NULL)
    g_object_unref (G_OBJECT (orig_pix));
  if (cached_pix != NULL)
    g_object_unref (G_OBJECT (cached_pix));
  gnomemeeting_threads_leave ();
  
  orig_pix = NULL;
  cached_pix = NULL;
  
  return TRUE;
}

  
BOOL
GMH323FakeVideoInputDevice::Start ()
{
  return TRUE;
}

  
BOOL
GMH323FakeVideoInputDevice::Stop ()
{
  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::IsCapturing ()
{
  return IsCapturing ();
}


PStringList
GMH323FakeVideoInputDevice::GetInputDeviceNames ()
{
  PStringList l;

  l.AppendString ("MovingLogo");
  l.AppendString ("StaticPicture");

  return l;
}


BOOL
GMH323FakeVideoInputDevice::SetFrameSize (unsigned int width,
					  unsigned int height)
{
  if (!PVideoDevice::SetFrameSize (width, height))
    return FALSE;

  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::GetFrame (PBYTEArray &a)
{
  PINDEX returned;

  if (!GetFrameData (a.GetPointer (), &returned))
    return FALSE;

  a.SetSize (returned);
  
  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::GetFrameData (BYTE *a, PINDEX *i)
{
  WaitFinishPreviousFrame ();

  GetFrameDataNoDelay (a, i);

  *i = CalculateFrameBytes (frameWidth, frameHeight, colourFormat);

  return TRUE;
}


BOOL GMH323FakeVideoInputDevice::GetFrameDataNoDelay (BYTE *frame, PINDEX *i)
{
  guchar *data = NULL;

  unsigned width = 0;
  unsigned height = 0;

  int orig_width = 0;
  int orig_height = 0;
  
  GetFrameSize (width, height);

  gnomemeeting_threads_enter ();
  
  if (!cached_pix) {
    
    cached_pix = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
                                 width, height);
    gdk_pixbuf_fill (cached_pix, 0x000000FF); /* Opaque black */

    if (!moving) { // create the ever-displayed picture
 
      orig_width = gdk_pixbuf_get_width (orig_pix);
      orig_height = gdk_pixbuf_get_height (orig_pix);
      
      if ((unsigned)orig_width <= width && (unsigned)orig_height <= height)
	/* the picture fits in the  target space: center it */
        gdk_pixbuf_copy_area (orig_pix, 0, 0, orig_width, orig_height,
			      cached_pix, 
                              (width - orig_width) / 2, 
                              (height - orig_height) / 2);
      else { /* the picture doesn't fit: scale 1:1, and center */
	double scale_w, scale_h, scale;
	
	scale_w = (double)width / orig_width;
	scale_h = (double)height / orig_height;
	
	if (scale_w < scale_h) // one of them is known to be < 1
	  scale = scale_w;
	else
	  scale = scale_h;
	
	GdkPixbuf *scaled_pix = gdk_pixbuf_scale_simple (orig_pix, (int)(scale*orig_width),(int)(scale*orig_height), GDK_INTERP_BILINEAR);
	gdk_pixbuf_copy_area (scaled_pix, 0, 0, (int)(scale*orig_width), (int)(scale*orig_height), cached_pix,
			      (width - (int)(scale*orig_width)) / 2, (height - (int)(scale*orig_height)) / 2);
	g_object_unref (G_OBJECT (scaled_pix));
      }
    }
  }
   
  if (moving) {  // recompute the cache pix
    
    orig_width = gdk_pixbuf_get_width (orig_pix);
    orig_height = gdk_pixbuf_get_height (orig_pix);

    gdk_pixbuf_fill (cached_pix, 0x000000FF); /* Opaque black */
    gdk_pixbuf_copy_area (orig_pix, 0, 0, orig_width, orig_height, 
			  cached_pix, (width - orig_width) / 2, pos);

    pos = pos + increment;

    if ((int) pos > (int) height - orig_height - 10) increment = -1;
    if (pos < 10) increment = +1;
  }

  data = gdk_pixbuf_get_pixels (cached_pix);
  rgb_increment = gdk_pixbuf_get_n_channels (cached_pix);

  if (converter)
    converter->Convert (data, frame);

  gnomemeeting_threads_leave ();


  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::TestAllFormats ()
{
  return TRUE;
}


PINDEX
GMH323FakeVideoInputDevice::GetMaxFrameBytes ()
{
  return CalculateFrameBytes (frameWidth, frameHeight, colourFormat);
}


void
GMH323FakeVideoInputDevice::WaitFinishPreviousFrame ()
{
  frameTimeError += msBetweenFrames;

  PTime now;
  PTimeInterval delay = now - previousFrameTime;
  frameTimeError -= (int)delay.GetMilliSeconds();
  frameTimeError += 1000 / frameRate;
  previousFrameTime = now;

  if (frameTimeError > 0) {
    PTRACE(6, "FakeVideo\t Sleep for " << frameTimeError << " milli seconds");
#ifdef P_LINUX
    usleep(frameTimeError * 1000);
#else
    PThread::Current()->Sleep(frameTimeError);
#endif
  }
}


BOOL
GMH323FakeVideoInputDevice::SetVideoFormat (VideoFormat newFormat)
{
  return PVideoDevice::SetVideoFormat (newFormat);
}


int
GMH323FakeVideoInputDevice::GetNumChannels()
{
  return 1;
}


BOOL
GMH323FakeVideoInputDevice::SetChannel (int newChannel)
{
  return PVideoDevice::SetChannel (newChannel);
}


BOOL
GMH323FakeVideoInputDevice::SetColourFormat (const PString &newFormat)
{
  if (newFormat == "BGR32") 
    return PVideoDevice::SetColourFormat (newFormat);

  return FALSE;  
}


BOOL
GMH323FakeVideoInputDevice::SetFrameRate (unsigned rate)
{
  PVideoDevice::SetFrameRate (12);
 
  return TRUE;
}


BOOL
GMH323FakeVideoInputDevice::GetFrameSizeLimits (unsigned & minWidth,
						unsigned & minHeight,
						unsigned & maxWidth,
						unsigned & maxHeight)
{
  minWidth  = 10;
  minHeight = 10;
  maxWidth  = 1000;
  maxHeight =  800;

  return TRUE;
}
