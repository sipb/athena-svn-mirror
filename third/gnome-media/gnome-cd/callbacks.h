/*
 * callbacks.h
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#include "gnome-cd.h"

void eject_cb (GtkButton *button, 
	       GnomeCD *cdrom);
void play_cb (GtkButton *button,
	      GnomeCD *cdrom);
void stop_cb (GtkButton *button,
	      GnomeCD *cdrom);
int ffwd_press_cb (GtkButton *button,
		   GdkEvent *ev,
		   GnomeCD *gcd);
int ffwd_release_cb (GtkButton *button,
		     GdkEvent *ev,
		     GnomeCD *gcd);
void next_cb (GtkButton *button,
	      GnomeCD *cdrom);
void back_cb (GtkButton *button,
	      GnomeCD *cdrom);
int rewind_press_cb (GtkButton *button,
		     GdkEvent *ev,
		     GnomeCD *gcd);
int rewind_release_cb (GtkButton *button,
		       GdkEvent *ev,
		       GnomeCD *gcd);
void mixer_cb (GtkButton *button,
	       GnomeCD *gcd);

void cd_status_changed_cb (GnomeCDRom *cdrom,
			   GnomeCDRomStatus *status,
			   GnomeCD *gcd);
void about_cb (GtkWidget *widget,
	       gpointer data);
void help_cb (GtkWidget *widget,
	      gpointer data);
		
void loopmode_changed_cb (GtkWidget *widget,
			  GnomeCDRomMode mode,
			  GnomeCD *gcd);
void playmode_changed_cb (GtkWidget *widget,
			  GnomeCDRomMode mode,
			  GnomeCD *gcd);
void open_preferences (GtkWidget *widget,
		       GnomeCD *gcd);
void open_track_editor (GtkWidget *widget,
			GnomeCD *gcd);
void destroy_track_editor (void);
void volume_changed (GtkRange *range,
		     GnomeCD *gcd);
#endif
