
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
 *                         videograbber.h  -  description
 *                         ------------------------------
 *   begin                : Mon Feb 12 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Video4Linux compliant functions to manipulate the 
 *                          webcam device.
 *
 */


#ifndef _VIDEO_GRABBER_H_
#define _VIDEO_GRABBER_H_

#include "common.h"
#include "gdkvideoio.h"


class GMVideoGrabber : public PThread
{
  PCLASSINFO(GMVideoGrabber, PThread);

 public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialises the VideoGrabber, the VideoGrabber is opened
   *                 asynchronously given the GConf parameters. If the opening
   *                 fails, an error popup is displayed.
   * PRE          :  First parameter is TRUE if the VideoGrabber must grab
   *                 once opened. The second one is TRUE if the VideoGrabber
   *                 must be opened synchronously.
   */
  GMVideoGrabber (BOOL = FALSE, BOOL = FALSE);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMVideoGrabber (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Start to grab, i.e. read from the specified device 
   *                 and display images in the main interface.
   * PRE          :  /
   */
  void StartGrabbing (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Stop to grab, i.e. read from the specified device 
   *                 and display images in the main interface.
   * PRE          :  /
   */
  void StopGrabbing (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if we are grabbing.
   * PRE          :  /
   */
  BOOL IsGrabbing (void);

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the GDKVideoOutputDevice used to display
   *                 the camera images.
   * PRE          :  /
   */
  GDKVideoOutputDevice *GetEncodingDevice (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns the PVideoChannel associated with the device.
   * PRE          :  /
   */
  PVideoChannel *GetVideoChannel (void);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the colour for the specified device.
   * PRE          :  0 <= int <= 65535
   */
  void SetColour (int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the brightness for the specified device.
   * PRE          :  0 <= int <= 65535
   */
  void SetBrightness (int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the whiteness for the specified device.
   * PRE          :  0 <= int <= 65535
   */
  void SetWhiteness (int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Sets the contrast for the specified device.
   * PRE          :  0 <= int <= 65535
   */
  void SetContrast (int);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns respectively the whiteness, brightness, 
   *                 colour, contrast for the specified device.
   * PRE          :  Allocated pointers to int. Grabber must be opened.
   */
  void GetParameters (int *, int *, int *, int *);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if the associated PVideoChannel is Open,
   *                 FALSE otherwise.
   * PRE          :  /
   */
  BOOL IsChannelOpen ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Lock the device, preventing it to be Closed and deleted.
   * PRE          :  /
   */
  void Lock ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Unlock the device.
   * PRE          :  /
   */
  void Unlock ();

  
 protected:
  void Main (void);
  void VGOpen (void);
  void VGClose (void);

  GmWindow *gw;
  
  int height;
  int width;
  int whiteness;
  int brightness;
  int colour;
  int contrast;

  char video_buffer [3 * GM_CIF_WIDTH * GM_CIF_HEIGHT];

  PVideoChannel *video_channel;
  PVideoInputDevice *grabber;
  GDKVideoOutputDevice *encoding_device;

  BOOL stop;
  BOOL is_grabbing;
  BOOL synchronous;
  BOOL is_opened;

  PMutex var_mutex;      /* To protect variables that are read and written
			    from various threads */
  PMutex quit_mutex;     /* To exit */
  PMutex device_mutex;   /* To Lock and Unlock and not exit until
			    it is unlocked */
  PSyncPoint thread_sync_point;
};


class GMVideoTester : public PThread
{
  PCLASSINFO(GMVideoTester, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  
   * PRE          :  /
   */
  GMVideoTester (gchar *,
		 gchar *);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMVideoTester ();


  void Main ();


protected:

  PString video_manager;
  PString video_recorder;

  GtkWidget *test_label;
  GtkWidget *test_dialog;

  PMutex quit_mutex;
  PSyncPoint thread_sync_point;
};
#endif
