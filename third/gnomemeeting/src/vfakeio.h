
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
 *                         vfakeio.h  -  description
 *                         -------------------------
 *   begin                : Tue Jul 30 2003
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains a descendant of a Fake Input 
 *                          Device that display the GM logo when connected to
 *                          a remote party without using a camera.
 *
 */


#ifndef _VFAKEIO_H_
#define _VFAKEIO_H_

#define P_FORCE_STATIC_PLUGIN

#include "common.h"

 
class GMH323FakeVideoInputDevice : public PVideoInputDevice 
{
  PCLASSINFO(GMH323FakeVideoInputDevice, PVideoInputDevice);

  
 public:
  
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Creates the Fake Input Device.
   * PRE          :  /
   */
  GMH323FakeVideoInputDevice ();


  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMH323FakeVideoInputDevice ();

  
  BOOL Open (const PString &,
	     BOOL = TRUE);

  
  /**Determine of the device is currently open.
   */
  BOOL IsOpen() ;

  
  /**Close the device.
   */
  BOOL Close();

  
  /**Start the video device I/O.
   */
  BOOL Start();

  
  /**Stop the video device I/O capture.
   */
  BOOL Stop();


  /**Determine if the video device I/O capture is in progress.
   */
  BOOL IsCapturing();

  
  /**Get a list of all of the drivers available.
   */
  static PStringList GetInputDeviceNames();

  
  BOOL SetFrameSize (unsigned int,
		     unsigned int);
  
  
  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  BOOL GetFrameData (BYTE *, PINDEX * = NULL);


  /* DESCRIPTION  :  The destructor
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  BOOL GetFrameDataNoDelay (BYTE *, PINDEX * = NULL);

  
  BOOL GetFrame (PBYTEArray &);


  BOOL TestAllFormats ();

  
  /**Get the maximum frame size in bytes.
  */
  PINDEX GetMaxFrameBytes();

  
  /** Given a preset interval of n milliseconds, this function
      returns n msecs after the previous frame capture was initiated.
  */
  void WaitFinishPreviousFrame();

  
  /**Set the video format to be used.

  Default behaviour sets the value of the videoFormat variable and then
  returns the IsOpen() status.
  */
  BOOL SetVideoFormat (VideoFormat);

  
  /**Get the number of video channels available on the device.

  Default behaviour returns 1.
  */
  int GetNumChannels() ;

  
  /**Set the video channel to be used on the device.

  Default behaviour sets the value of the channelNumber variable and then
  returns the IsOpen() status.
  */
  BOOL SetChannel (int);
			

  /**Set the colour format to be used.

  Default behaviour sets the value of the colourFormat variable and then
  returns the IsOpen() status.
  */
  BOOL SetColourFormat (const PString &);

  
  /**Set the video frame rate to be used on the device.

  Default behaviour sets the value of the frameRate variable and then
  return the IsOpen() status.
  */
  BOOL SetFrameRate (unsigned);

  
  BOOL GetFrameSizeLimits (unsigned &,
			   unsigned &,
			   unsigned &,
			   unsigned &);
  
  
  PBYTEArray data;
  GdkPixbuf *orig_pix;
  GdkPixbuf *cached_pix;
  bool moving;
  int rgb_increment;
  int pos;
  int increment;
  PStringList GetDeviceNames() const
  { return GetInputDeviceNames(); }
};

PCREATE_VIDINPUT_PLUGIN (Picture, GMH323FakeVideoInputDevice);

#endif
