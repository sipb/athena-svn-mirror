
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
 *                         sound_handling.h  -  description
 *                         --------------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains sound handling functions.
 *
 */


#ifndef __GM_SOUND_HANDLING_H
#define __GM_SOUND_HANDLING_H

#include "common.h"
#include "endpoint.h"

#ifndef WIN32
#include <esd.h>
#endif


#define GM_AUDIO_TESTER(x) (GMAudioTester *)(x)

enum { SOURCE_AUDIO, SOURCE_MIC };


/* The functions */

/* DESCRIPTION   :  /
 * BEHAVIOR      : Puts ESD (and Artsd if support compiled in) into standby 
 *                 mode. An error message is displayed in the gnomemeeting
 *                 log if it failed. No message is displayed if it is
 *                 succesful.
 * PRE           : /
 */
void gnomemeeting_sound_daemons_suspend ();


/* DESCRIPTION   :  /
 * BEHAVIOR      : Puts ESD (and Artsd if support compiled in) into normal
 *                 mode. An error message is displayed in the gnomemeeting
 *                 log if it failed. No message is displayed if it is
 *                 succesful.
 * PRE           : /
 */
void gnomemeeting_sound_daemons_resume ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Selects the Mic as recording input. OSS only.
 * PRE          :  /
 */
void gnomemeeting_mixers_mic_select (void);
     

/* GnomeMeeting Sound Event class */
class GMSoundEvent
{

 public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Builds the PSound corresponding to the event name
   *                 and plays it if it is valid.
   * PRE          :  The sound event name.
   */
  GMSoundEvent (PString);

 private:
  void Main ();

  PString event;
};


/* Audio Tester class */
class GMAudioTester : public PThread
{
  PCLASSINFO(GMAudioTester, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  GMAudioTester (gchar *,
		 gchar *,
		 gchar *);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMAudioTester ();


  void Main ();

protected:

  BOOL stop;

  PMutex quit_mutex;
  PSyncPoint thread_sync_point;

  PSoundChannel *player;
  PSoundChannel *recorder;

  char *buffer_ring;

  PString audio_manager;
  PString audio_player;
  PString audio_recorder;
  
  PMutex buffer_ring_access_mutex;

  GmWindow *gw;
  
  GtkWidget *test_label;
  GtkWidget *test_dialog;
  GtkWidget *level_meter;
  
  friend class GMAudioRP;
};
#endif


class GMAudioRP : public PThread
{
  PCLASSINFO(GMAudioRP, PThread);

 public:

  GMAudioRP (GMAudioTester *, PString, PString, BOOL);
  ~GMAudioRP ();

  void Main ();

 private:
  gfloat GetAverageSignalLevel (const short *, int);
  
  BOOL is_encoding;
  GMAudioTester *tester;
  PString driver_name;
  PString device_name;
  PMutex quit_mutex;
  PSyncPoint thread_sync_point;
  
  BOOL stop;
};
