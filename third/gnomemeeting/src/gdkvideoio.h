
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
 *                         gdkvideoio.h  -  description
 *                         ----------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Class to permit to display in GDK Drawing Area or
 *                          SDL.
 *
 */


#ifndef _GDKVIDEOIO_H_
#define _GDKVIDEOIO_H_

#include "common.h"

#ifdef HAS_SDL
#include <SDL.h>
#endif




class GDKVideoOutputDevice : public PVideoOutputDevice
{
  PCLASSINFO(GDKVideoOutputDevice, PVideoOutputDevice);


  public:

    
  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Setups the parameters, 
   *                 int = 0 if we do not transmit,
   *                 1 otherwise, if we do not transmit, 
   *                 default display = local
   *                 else default display = remote.
   * PRE          :  GmWindow is a valid pointer to a valid
   *                 GmWindow structure.
   */
  GDKVideoOutputDevice (int, GmWindow *);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GDKVideoOutputDevice ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Open the device given the device name.
   * PRE          :  Device name to open, immediately start device.
   */
  BOOL Open (const PString &, BOOL) { return TRUE; }

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return a list of all of the drivers available.
   * PRE          :  /
   */
  PStringList GetDeviceNames() const;


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Get the maximum frame size in bytes.
   * PRE          :  /
   */
  PINDEX GetMaxFrameBytes() { return 352 * 288 * 3 * 2; }

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if the output device is open.
   * PRE          :  /
   */
  BOOL IsOpen ();


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  If data for the end frame is received, then we convert
   *                 it from to RGB32 and we display it.
   * PRE          :  x and y positions in the picture (>=0),
   *                 width and height (>=0),
   *                 the data, and a boolean indicating if it is the end
   *                 frame or not.
   */
  BOOL SetFrameData (unsigned, unsigned, unsigned, unsigned,
		     const BYTE *, BOOL);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if the colour format is supported (ie RGB24).
   * PRE          :  /
   */
  BOOL SetColourFormat (const PString &);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Displays the current frame.
   * PRE          :  /
   */
  BOOL EndFrame();


  BOOL Start () {return TRUE;};
  
  BOOL Stop () {return TRUE;};
    
 protected:


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Redraw the frame given as parameter.
   * PRE          :  /
   */
  BOOL Redraw ();


  int device_id;      /* The current device : encoding or not */
  int display_config; /* Current display : local or remote or both */

  PMutex redraw_mutex;
  PBYTEArray frameStore;
  
#ifdef HAS_SDL
  SDL_Surface *screen;
  SDL_Overlay *overlay;

  SDL_Rect dest;
  PMutex sdl_mutex;  /* Mutex to ensure that only one thread access to the SDL
			stuff at the same time */
#endif

  enum {REMOTE, LOCAL};

  GmWindow *gw;
};
#endif
