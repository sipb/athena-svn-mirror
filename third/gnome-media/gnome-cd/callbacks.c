/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * callbacks.c
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gerror.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkrange.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmessagedialog.h> 
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-about.h>
#include <libgnome/gnome-help.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-widget.h>

#include <bonobo-activation/bonobo-activation.h>

#include "cddb.h"
#include "cdrom.h"
#include "display.h"
#include "gnome-cd.h"

#include "GNOME_Media_CDDBSlave2.h"

static GNOME_Media_CDDBTrackEditor track_editor = CORBA_OBJECT_NIL;

static void
maybe_close_tray (GnomeCD *gcd)
{
	GnomeCDRomStatus *status;
	GError *error;

	if (gnome_cdrom_get_status (gcd->cdrom, &status, &error) == FALSE) {
		gcd_warning ("%s", error);
		g_error_free (error);
		g_free (status);
		return;
	}

	if (status->cd == GNOME_CDROM_STATUS_TRAY_OPEN) {
		if (gnome_cdrom_close_tray (gcd->cdrom, &error) == FALSE) {
			gcd_warning ("%s", error);
			g_error_free (error);
		}
	}

	g_free (status);
	return;
}

void
eject_cb (GtkButton *button,
	  GnomeCD *gcd)
{
	GError *error;

	if (gnome_cdrom_eject (gcd->cdrom, &error) == FALSE) {
		gcd_warning ("%s", error);
		g_error_free (error);
	}

	if (gcd->current_image == gcd->pause_image) {
		AtkObject *aob;

		aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
		atk_object_set_name (aob, _("Play"));

		gtk_container_remove (GTK_CONTAINER (gcd->play_b), gcd->current_image);
		gtk_container_add (GTK_CONTAINER (gcd->play_b), gcd->play_image);
		gcd->current_image = gcd->play_image;
	}
}

void
play_cb (GtkButton *button,
	 GnomeCD *gcd)
{
	GError *error;
	GnomeCDRomStatus *status;
	GnomeCDRomMSF msf;
	AtkObject *aob;
	int end_track;
	GnomeCDRomMSF *endmsf;

	if (gnome_cdrom_get_status (gcd->cdrom, &status, &error) == FALSE) {
		gcd_warning ("%s", error);
		g_error_free (error);
		g_free (status);
		return;

	}

	switch (status->cd) {
	case GNOME_CDROM_STATUS_TRAY_OPEN:
		if (gnome_cdrom_close_tray (gcd->cdrom, &error) == FALSE) {
			gcd_warning ("Cannot close tray: %s", error);
			g_error_free (error);

			g_free (status);
			return;
		}

		/* Tray is closed, now play */
		msf.minute = 0;
		msf.second = 0;
		msf.frame = 0;
		if (gcd->cdrom->playmode == GNOME_CDROM_WHOLE_CD) {
			end_track = -1;
			endmsf = NULL;
		} else {
			end_track = 2;
			endmsf = &msf;
		}
		
		if (gnome_cdrom_play (gcd->cdrom, 1, &msf, end_track, endmsf, &error) == FALSE) {
			g_warning ("%s: %s", G_GNUC_FUNCTION, error->message);
			g_error_free (error);

			g_free (status);
			return;
		}
		break;

	case GNOME_CDROM_STATUS_DRIVE_NOT_READY:
		gcd_warning ("Drive not ready: %s", NULL);

		g_free (status);
		return;

	case GNOME_CDROM_STATUS_OK:
	default:
		break;
	}

	switch (status->audio) {
	case GNOME_CDROM_AUDIO_PLAY:
		if (gcd->current_image == gcd->play_image) {
			aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
			atk_object_set_name (aob, _("Pause"));
			gtk_container_remove (GTK_CONTAINER (gcd->play_b), gcd->current_image);
			gtk_container_add (GTK_CONTAINER (gcd->play_b), gcd->pause_image);
			gcd->current_image = gcd->pause_image;
		} else { 
			if (gnome_cdrom_pause (gcd->cdrom, &error) == FALSE) {
				gcd_warning ("%s", error);
				g_error_free (error);

				g_free (status);
				return;
			}

			if (gcd->current_image == gcd->pause_image) {
				aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
				atk_object_set_name (aob, _("Play"));
				gtk_container_remove (GTK_CONTAINER (gcd->play_b), gcd->current_image);
				gtk_container_add (GTK_CONTAINER (gcd->play_b),
						   gcd->play_image);
				gcd->current_image = gcd->play_image;
			}
		}
		break;

	case GNOME_CDROM_AUDIO_PAUSE:
		if (gcd->cdrom->playmode == GNOME_CDROM_WHOLE_CD) {
			end_track = -1;
			endmsf = NULL;
		} else {
			end_track = status->track + 1;
			msf.minute = 0;
			msf.second = 0;
			msf.frame = 0;
			endmsf = &msf;
		}
		if (gnome_cdrom_play (gcd->cdrom, status->track,
				      &status->relative, end_track, endmsf, &error) == FALSE) {
			gcd_warning ("%s", error);
			g_error_free (error);
			
			g_free (status);
			return;
		}

		if (gcd->current_image == gcd->play_image) {
			aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
			atk_object_set_name (aob, _("Pause"));
			gtk_container_remove (GTK_CONTAINER (gcd->play_b), gcd->current_image);
			gtk_container_add (GTK_CONTAINER (gcd->play_b), gcd->pause_image);
			gcd->current_image = gcd->pause_image;
		}
		break;

	case GNOME_CDROM_AUDIO_COMPLETE:
	case GNOME_CDROM_AUDIO_STOP:
		msf.minute = 0;
		msf.second = 0;
		msf.frame = 0;

		if (gcd->cdrom->playmode == GNOME_CDROM_WHOLE_CD) {
			end_track = -1;
			endmsf = NULL;
		} else {
			end_track = 2;
			endmsf = &msf;
		}
		if (gnome_cdrom_play (gcd->cdrom, 1, &msf, end_track, endmsf, &error) == FALSE) {
			gcd_warning ("%s", error);
			g_error_free (error);

			g_free (status);
			return;
		}

		if (gcd->current_image == gcd->play_image) {
			aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
			atk_object_set_name (aob, _("Pause"));
			gtk_container_remove (GTK_CONTAINER (gcd->play_b), gcd->current_image);
			gtk_container_add (GTK_CONTAINER (gcd->play_b), gcd->pause_image);
			gcd->current_image = gcd->pause_image;
		}
		break;

	case GNOME_CDROM_AUDIO_ERROR:
		gcd_warning ("Error playing CD: %s", NULL);
		g_free (status);
		return;

	case GNOME_CDROM_AUDIO_NOTHING:
	default:
		break;
	}

	g_free (status);
}

void
stop_cb (GtkButton *button,
	 GnomeCD *gcd)
{
	GError *error;

	/* Close the tray if needed */
	maybe_close_tray (gcd);

	if (gnome_cdrom_stop (gcd->cdrom, &error) == FALSE) {
		gcd_warning ("%s", error);
		g_error_free (error);
	}

	if (gcd->current_image == gcd->pause_image) {
		AtkObject *aob;

		aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
		atk_object_set_name (aob, _("Play"));

		gtk_container_remove (GTK_CONTAINER (gcd->play_b), gcd->current_image);
		gtk_container_add (GTK_CONTAINER (gcd->play_b), gcd->play_image);
		gcd->current_image = gcd->play_image;
	}
}

static gboolean
ffwd_timeout_cb (gpointer data)
{
	GError *error;
	GnomeCD *gcd = data;

	if (gnome_cdrom_fast_forward (gcd->cdrom, &error) == FALSE) {
		gcd_warning ("%s", error);
		g_error_free (error);
		return FALSE;
	}

	return TRUE;
}

int
ffwd_press_cb (GtkButton *button,
	       GdkEvent *ev,
	       GnomeCD *gcd)
{
	if (gcd->timeout > 0) {
		return FALSE;
	}
	
	maybe_close_tray (gcd);

	ffwd_timeout_cb (gcd);
	
	gcd->timeout = gtk_timeout_add (140, ffwd_timeout_cb, gcd);

	if (gcd->current_image == gcd->play_image) {
		AtkObject *aob;

		aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
		atk_object_set_name (aob, _("Pause"));

		gtk_container_remove (GTK_CONTAINER (gcd->play_b), gcd->current_image);
		gtk_container_add (GTK_CONTAINER (gcd->play_b), gcd->pause_image);
		gcd->current_image = gcd->pause_image;
	}

	return FALSE;
}

int
ffwd_release_cb (GtkButton *button,
		 GdkEvent *ev,
		 GnomeCD *gcd)
{
	if (gcd->timeout > 0) {
		gtk_timeout_remove (gcd->timeout);
		gcd->timeout = 0;
	}

	return FALSE;
}

void
next_cb (GtkButton *button,
	 GnomeCD *gcd)
{
	GError *error;

	maybe_close_tray (gcd);

	if (gnome_cdrom_next (gcd->cdrom, &error) == FALSE) {
		gcd_warning ("%s", error);
		g_error_free (error);
	}

	if (gcd->current_image == gcd->play_image) {
		AtkObject *aob;

		aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
		atk_object_set_name (aob, _("Pause"));
		gtk_container_remove (GTK_CONTAINER (gcd->play_b), gcd->current_image);
		gtk_container_add (GTK_CONTAINER (gcd->play_b), gcd->pause_image);
		gcd->current_image = gcd->pause_image;
	}
}

void
back_cb (GtkButton *button,
	 GnomeCD *gcd)
{
	GError *error;

	maybe_close_tray (gcd);

	if (gnome_cdrom_back (gcd->cdrom, &error) == FALSE) {
		gcd_warning ("%s", error);
		g_error_free (error);
	}

	if (gcd->current_image == gcd->play_image) {
		AtkObject *aob;

		aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
		atk_object_set_name (aob, _("Pause"));
		gtk_container_remove (GTK_CONTAINER (gcd->play_b), gcd->current_image);
		gtk_container_add (GTK_CONTAINER (gcd->play_b), gcd->pause_image);
		gcd->current_image = gcd->pause_image;
	}
}

static gboolean
rewind_timeout_cb (gpointer data)
{
	GError *error;
	GnomeCD *gcd = data;
	
	if (gnome_cdrom_rewind (gcd->cdrom, &error) == FALSE) {
		gcd_warning ("%s", error);
		g_error_free (error);
		return FALSE;
	}
	
	return TRUE;
}

int
rewind_press_cb (GtkButton *button,
		 GdkEvent *ev,
		 GnomeCD *gcd)
{
	if (gcd->timeout > 0) {
		return FALSE;
	}
	
	maybe_close_tray (gcd);

	/* Call it so a click will activate it */
	rewind_timeout_cb (gcd);
	gcd->timeout = gtk_timeout_add (140, rewind_timeout_cb, gcd);

	if (gcd->current_image == gcd->play_image) {
		AtkObject *aob;

		aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
		atk_object_set_name (aob, _("Pause"));
		gtk_container_remove (GTK_CONTAINER (gcd->play_b), gcd->current_image);
		gtk_container_add (GTK_CONTAINER (gcd->play_b), gcd->pause_image);
		gcd->current_image = gcd->pause_image;
	}

	return FALSE;
}

int
rewind_release_cb (GtkButton *button,
		   GdkEvent *ev,
		   GnomeCD *gcd)
{
	if (gcd->timeout > 0) {
		gtk_timeout_remove (gcd->timeout);
		gcd->timeout = 0;
	}
	
	return FALSE;
}

static void
set_track_option_menu (GtkOptionMenu *menu,
		       int track)
{
	g_signal_handlers_block_matched (G_OBJECT (menu),
					 G_SIGNAL_MATCH_FUNC,
					 0, 0, NULL,
					 G_CALLBACK (skip_to_track), NULL);
	gtk_option_menu_set_history (menu, track - 1);
	g_signal_handlers_unblock_matched (G_OBJECT (menu),
					   G_SIGNAL_MATCH_FUNC,
					   0, 0, NULL,
					   G_CALLBACK (skip_to_track), NULL);
}

static void
set_window_track_title (GnomeCD *gcd,
			GnomeCDRomStatus *status)
{
	int idx = status->track - 1;
	const char *artist = "";
	const char *track_name = "";

	if (gcd->disc_info) {
		if (idx >= 0 && idx < gcd->disc_info->ntracks &&
		    gcd->disc_info->track_info)
			track_name = gcd->disc_info->track_info [idx]->name;

		artist = gcd->disc_info->artist ? gcd->disc_info->artist : "";
	}

	gnome_cd_set_window_title (gcd, artist, track_name);
}

/* Do all the stuff for when the status is ok */
static void
status_ok (GnomeCD *gcd,
	   GnomeCDRomStatus *status)
{
	AtkObject *aob;
	int track;
	char *text;
	
	/* Allow the track editor to work */
	gtk_widget_set_sensitive (gcd->trackeditor_b, TRUE);

	/* Set the track menu on. */
	gtk_widget_set_sensitive (gcd->tracks, TRUE);
	
	/* All buttons can be used when the tray is open,
	   they should shut the tray. */
	gtk_widget_set_sensitive (gcd->back_b, TRUE);
	gtk_widget_set_sensitive (gcd->rewind_b, TRUE);
	gtk_widget_set_sensitive (gcd->play_b, TRUE);
	gtk_widget_set_sensitive (gcd->stop_b, TRUE);
	gtk_widget_set_sensitive (gcd->ffwd_b, TRUE);
	gtk_widget_set_sensitive (gcd->next_b, TRUE);
	gtk_widget_set_sensitive (gcd->eject_b, TRUE);

	switch (status->audio) {
	case GNOME_CDROM_AUDIO_NOTHING:
		break;

	case GNOME_CDROM_AUDIO_PLAY:
		/* Change the play button to pause */
		aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
		atk_object_set_name (aob, _("Pause"));

		if (gcd->current_image != gcd->pause_image) {
			gtk_container_remove (GTK_CONTAINER (gcd->play_b),
					      gcd->play_image);
			gtk_container_add (GTK_CONTAINER (gcd->play_b),
					   gcd->pause_image);
			gcd->current_image = gcd->pause_image;
		}
		/* Find out if the track has changed */
		track = gtk_option_menu_get_history (GTK_OPTION_MENU (gcd->tracks));
		if (track + 1 != status->track) {
			set_track_option_menu (GTK_OPTION_MENU (gcd->tracks),
					       status->track);
		}
		
		
/*  		cd_display_clear (CD_DISPLAY (gcd->display)); */
		if (status->relative.second >= 10) {
			text = g_strdup_printf ("%d:%d", status->relative.minute, status->relative.second);
		} else {
			text = g_strdup_printf ("%d:0%d", status->relative.minute, status->relative.second);
		}
		
		cd_display_set_line (CD_DISPLAY (gcd->display),
				     CD_DISPLAY_LINE_TIME, text);
		g_free (text);
		set_window_track_title (gcd, status);
			
		/* Update the tray icon tooltip */
		if (gcd->disc_info != NULL) {
			text = g_strdup_printf (_("Playing %s - %s"),
						gcd->disc_info->artist ? gcd->disc_info->artist : _("Unknown Artist"),
						gcd->disc_info->title ? gcd->disc_info->title : _("Unknown Album"));
		} else {
			text = g_strdup (_("Playing"));
		}
		
		gtk_tooltips_set_tip (gcd->tray_tips, gcd->tray, text, NULL);
		g_free (text);
		
		break;

	case GNOME_CDROM_AUDIO_PAUSE:
		/* Change the play button to pause */
		aob = gtk_widget_get_accessible (GTK_WIDGET (gcd->play_b));
		atk_object_set_name (aob, _("Play"));
		if (gcd->current_image != gcd->play_image) {
			gtk_container_remove (GTK_CONTAINER (gcd->play_b),
					      gcd->pause_image);
			gtk_container_add (GTK_CONTAINER (gcd->play_b),
					   gcd->play_image);
			gcd->current_image = gcd->play_image;
		}

		set_window_track_title (gcd, status);

		/* Update the tray icon tooltip */
		gtk_tooltips_set_tip (gcd->tray_tips, gcd->tray, _("Paused"), NULL);
		break;
		
	case GNOME_CDROM_AUDIO_COMPLETE:
		if (gcd->cdrom->loopmode == GNOME_CDROM_LOOP) {
			if (gcd->cdrom->playmode == GNOME_CDROM_WHOLE_CD) {
				/* Around we go */
				play_cb (NULL, gcd);
			} else {
				GnomeCDRomMSF msf;
				int start_track, end_track;
				GError *error;
				
				/* CD has gone to the start of the next track.
				   Track we want to loop is track - 1 */
				start_track = status->track - 1;
				end_track = status->track;
				
				msf.minute = 0;
				msf.second = 0;
				msf.frame = 0;
				
				if (gnome_cdrom_play (gcd->cdrom, start_track, &msf,
						      end_track, &msf, &error) == FALSE) {
					gcd_warning ("%s", error);
					g_error_free (error);
					
					g_free (status);
					return;
				}
			}
		} else {
			/* We've stopped playing anything
			   reset play button, and track option menu */
			if (gcd->current_image != gcd->play_image) {
				gtk_container_remove (GTK_CONTAINER (gcd->play_b),
						      gcd->pause_image);
				gtk_container_add (GTK_CONTAINER (gcd->play_b),
						   gcd->play_image);
				gcd->current_image = gcd->play_image;
			}

			set_track_option_menu (GTK_OPTION_MENU (gcd->tracks), 1);

			/* Update tray icon tooltip */
			gtk_tooltips_set_tip (gcd->tray_tips, gcd->tray, _("CD Player"), NULL);
		}		
		break;
		
	case GNOME_CDROM_AUDIO_STOP:
		cd_display_set_line (CD_DISPLAY (gcd->display),
				     CD_DISPLAY_LINE_TIME, "");
		if (gcd->disc_info != NULL) {
			gnome_cd_set_window_title (gcd,
						   gcd->disc_info->artist ?
						   gcd->disc_info->artist : "",
						   gcd->disc_info->title ?
						   gcd->disc_info->title : "");
		} else {
			gnome_cd_set_window_title (gcd, NULL, NULL);
		}

		/* Update the tray icon tooltip */
		gtk_tooltips_set_tip (gcd->tray_tips, gcd->tray, _("Stopped"), NULL);
		break;
		
	case GNOME_CDROM_AUDIO_ERROR:
		cd_display_clear (CD_DISPLAY (gcd->display));
		cd_display_set_line (CD_DISPLAY (gcd->display),
				     CD_DISPLAY_LINE_TIME, _("Disc error"));
		if (gcd->disc_info != NULL) {
			gnome_cd_set_window_title (gcd, gcd->disc_info->artist ?
						   gcd->disc_info->artist : "",
						   gcd->disc_info->title ?
						   gcd->disc_info->title : "");
		} else {
			gnome_cd_set_window_title (gcd, NULL, NULL);
		}

		/* Update the tray icon tooltip */
		gtk_tooltips_set_tip (gcd->tray_tips, gcd->tray, _("No disc"), NULL);
		break;
		
	default:
		if (gcd->disc_info != NULL) {
			gnome_cd_set_window_title (gcd,
						   gcd->disc_info->artist ?
						   gcd->disc_info->artist : "",
						   gcd->disc_info->title ?
						   gcd->disc_info->title : "");
		} else {
			gnome_cd_set_window_title (gcd, NULL, NULL);
		}
		
		break;
	}
	
	if (gcd->last_status == NULL ||
	    gcd->last_status->cd != GNOME_CDROM_STATUS_OK) {
		cddb_get_query (gcd);
	}
		
	if (gcd->disc_info == NULL) {
		cd_display_set_line (CD_DISPLAY (gcd->display),
				     CD_DISPLAY_LINE_ARTIST,
				     _("Unknown Artist"));
		cd_display_set_line (CD_DISPLAY (gcd->display),
				     CD_DISPLAY_LINE_ALBUM,
				     _("Unknown Album"));
		gnome_cd_set_window_title (gcd, NULL, NULL);
	} else {
		GnomeCDDiscInfo *info = gcd->disc_info;
		
		cd_display_set_line (CD_DISPLAY (gcd->display),
				     CD_DISPLAY_LINE_ARTIST,
				     info->artist ? info->artist :
				     _("Unknown Artist"));
		cd_display_set_line (CD_DISPLAY (gcd->display),
				     CD_DISPLAY_LINE_ALBUM,
				     info->title ? info->title :
				     _("Unknown Album"));
	}
}

void
cd_status_changed_cb (GnomeCDRom *cdrom,
		      GnomeCDRomStatus *status,
		      GnomeCD *gcd)
{
	if (gcd->not_ready == TRUE) {
		return;
	}
	
	switch (status->cd) {
	case GNOME_CDROM_STATUS_OK:
		status_ok (gcd, status);
		break;
		
	case GNOME_CDROM_STATUS_NO_DISC:
		if (gcd->disc_info != NULL) {
			cddb_free_disc_info (gcd->disc_info);
			gcd->disc_info = NULL;
		}
		
		gtk_widget_set_sensitive (gcd->trackeditor_b, FALSE);

		/* Set the other widgets off */
		gtk_widget_set_sensitive (gcd->tracks, FALSE);
		gtk_widget_set_sensitive (gcd->back_b, FALSE);
		gtk_widget_set_sensitive (gcd->rewind_b, FALSE);
		gtk_widget_set_sensitive (gcd->play_b, FALSE);
		gtk_widget_set_sensitive (gcd->stop_b, FALSE);
		gtk_widget_set_sensitive (gcd->ffwd_b, FALSE);
		gtk_widget_set_sensitive (gcd->next_b, FALSE);

		/* We can still use the eject button when there's no disc. */
		gtk_widget_set_sensitive (gcd->eject_b, TRUE);

		cd_display_clear (CD_DISPLAY (gcd->display));
		cd_display_set_line (CD_DISPLAY (gcd->display), CD_DISPLAY_LINE_TIME, _("No disc"));
		gnome_cd_set_window_title (gcd, NULL, NULL);
		break;

	case GNOME_CDROM_STATUS_TRAY_OPEN:
		if (gcd->disc_info != NULL) {
			cddb_free_disc_info (gcd->disc_info);
			gcd->disc_info = NULL;
		}
		
		gtk_widget_set_sensitive (gcd->trackeditor_b, FALSE);

		/* Set the track menu off. */
		gtk_widget_set_sensitive (gcd->tracks, FALSE);

		/* All buttons can be used when the tray is open,
		   they should shut the tray. */
		gtk_widget_set_sensitive (gcd->back_b, TRUE);
		gtk_widget_set_sensitive (gcd->rewind_b, TRUE);
		gtk_widget_set_sensitive (gcd->play_b, TRUE);
		gtk_widget_set_sensitive (gcd->stop_b, TRUE);
		gtk_widget_set_sensitive (gcd->ffwd_b, TRUE);
		gtk_widget_set_sensitive (gcd->next_b, TRUE);
		gtk_widget_set_sensitive (gcd->eject_b, TRUE);

		cd_display_clear (CD_DISPLAY (gcd->display));
		cd_display_set_line (CD_DISPLAY (gcd->display), CD_DISPLAY_LINE_TIME, _("Drive open"));
		gnome_cd_set_window_title (gcd, NULL, NULL);
		break;

	case GNOME_CDROM_STATUS_DATA_CD:
		if (gcd->disc_info != NULL) {
				cddb_free_disc_info (gcd->disc_info);
				gcd->disc_info = NULL;
		}	
		gtk_widget_set_sensitive (gcd->trackeditor_b, FALSE);

                /* Set the other widgets off */
                gtk_widget_set_sensitive (gcd->tracks, FALSE);
                gtk_widget_set_sensitive (gcd->back_b, FALSE);
                gtk_widget_set_sensitive (gcd->rewind_b, FALSE);
                gtk_widget_set_sensitive (gcd->play_b, FALSE);
                gtk_widget_set_sensitive (gcd->stop_b, FALSE);
                gtk_widget_set_sensitive (gcd->ffwd_b, FALSE);
                gtk_widget_set_sensitive (gcd->next_b, FALSE);

		/* Eject button can be used even to eject a data cd */
		gtk_widget_set_sensitive (gcd->eject_b, TRUE);
	
		cd_display_clear (CD_DISPLAY (gcd->display));
                cd_display_set_line (CD_DISPLAY (gcd->display), CD_DISPLAY_LINE_TIME, _("Data CD"));
                gnome_cd_set_window_title (gcd, NULL, NULL);
                break;

	case GNOME_CDROM_STATUS_NO_CDROM:
		if (gcd->disc_info != NULL) {
				cddb_free_disc_info (gcd->disc_info);
				gcd->disc_info = NULL;
		}	
		gtk_widget_set_sensitive (gcd->trackeditor_b, FALSE);

                /* Set the other widgets off */
                gtk_widget_set_sensitive (gcd->tracks, FALSE);
                gtk_widget_set_sensitive (gcd->back_b, FALSE);
                gtk_widget_set_sensitive (gcd->rewind_b, FALSE);
                gtk_widget_set_sensitive (gcd->play_b, FALSE);
                gtk_widget_set_sensitive (gcd->stop_b, FALSE);
                gtk_widget_set_sensitive (gcd->ffwd_b, FALSE);
                gtk_widget_set_sensitive (gcd->next_b, FALSE);
		gtk_widget_set_sensitive (gcd->eject_b, TRUE);
	
		cd_display_clear (CD_DISPLAY (gcd->display));
                cd_display_set_line (CD_DISPLAY (gcd->display), CD_DISPLAY_LINE_TIME, _("No Cdrom"));
                gnome_cd_set_window_title (gcd, NULL, NULL);

		/* Updated the tray icon tooltip */
		gtk_tooltips_set_tip (gcd->tray_tips, gcd->tray, _("No Cdrom"), NULL);
                break;

	default:
		if (gcd->disc_info != NULL) {
			cddb_free_disc_info (gcd->disc_info);
			gcd->disc_info = NULL;
		}

		gtk_widget_set_sensitive (gcd->trackeditor_b, FALSE);
		cd_display_clear (CD_DISPLAY (gcd->display));
		cd_display_set_line (CD_DISPLAY (gcd->display), CD_DISPLAY_LINE_TIME, _("Drive Error"));
		gnome_cd_set_window_title (gcd, NULL, NULL);
		break;
	}

	if (gcd->last_status != NULL) {
		g_free (gcd->last_status);
	}

	gcd->last_status = gnome_cdrom_copy_status (status);
}

void
about_cb (GtkWidget *widget,
	  gpointer data)
{
	static GtkWidget *about = NULL;
	const char *authors[2] = {"Iain Holmes <iain@prettypeople.org>", NULL};
	
	if (about == NULL) {
		about = gnome_about_new (_("CD Player"), VERSION,
					 "Copyright \xc2\xa9 2001-2002 Iain Holmes",
					 _("A CD player for GNOME"),
					 authors, NULL, NULL, NULL);
		g_signal_connect (G_OBJECT (about), "destroy",
				  G_CALLBACK (gtk_widget_destroyed), &about);
		gtk_widget_show (about);
	}
}

void
help_cb (GtkWidget *widget,
	 gpointer data)
{
	GError *error = NULL;

	gnome_help_display ("gnome-cd", NULL, &error); 
	if (error) {
		GtkWidget *msgbox;
		msgbox = gtk_message_dialog_new (NULL,
						 GTK_DIALOG_MODAL,
						 GTK_MESSAGE_ERROR,
						 GTK_BUTTONS_CLOSE,
						 ("There was an error displaying help: \n%s"),
						 error->message);
		g_signal_connect (G_OBJECT(msgbox), "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);
		gtk_widget_show (msgbox);
		g_error_free (error);
	} 
}
 
void
loopmode_changed_cb (GtkWidget *display,
		     GnomeCDRomMode mode,
		     GnomeCD *gcd)
{
	gcd->cdrom->loopmode = mode;
}

void
playmode_changed_cb (GtkWidget *display,
		     GnomeCDRomMode mode,
		     GnomeCD *gcd)
{
	gcd->cdrom->playmode = mode;
}

gboolean
tray_icon_clicked (GtkWidget *widget, GdkEventButton *event, GnomeCD *gcd)
{
	if (event->button != 3) {
		if (GTK_WIDGET_VISIBLE (gcd->window)) {
			gtk_widget_hide (gcd->window);
		} else {
			gtk_widget_show (gcd->window);
		}

		return TRUE;
	} else {
		return FALSE;
	}
}

void
open_preferences (GtkWidget *widget,
		  GnomeCD *gcd)
{
	static GtkWidget *dialog = NULL;

	if (dialog == NULL) {
		dialog = preferences_dialog_show (gcd, FALSE);
		g_signal_connect (G_OBJECT (dialog), "destroy",
				  G_CALLBACK (gtk_widget_destroyed), &dialog);
	} else {
		gdk_window_show (dialog->window);
		gdk_window_raise (dialog->window);
	}
}

#define CDDBSLAVE_TRACK_EDITOR_IID "OAFIID:GNOME_Media_CDDBSlave2_TrackEditor"

static void
restart_track_editor (gpointer data)
{
	CORBA_Environment ev;
	
	CORBA_exception_init (&ev);
	track_editor = bonobo_activation_activate_from_id (CDDBSLAVE_TRACK_EDITOR_IID, 0, NULL, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Could not reactivate track editor.\n%s", CORBA_exception_id (&ev));
		track_editor = CORBA_OBJECT_NIL;
		CORBA_exception_free (&ev);

		return;
	}

	ORBit_small_listen_for_broken (track_editor, G_CALLBACK (restart_track_editor), &ev);
	CORBA_exception_free (&ev);
}

void
destroy_track_editor (void)
{
	if (track_editor != CORBA_OBJECT_NIL) {
		ORBit_small_unlisten_for_broken (track_editor, G_CALLBACK (restart_track_editor));
		bonobo_object_release_unref (track_editor, NULL);
	}
}

void
open_track_editor (GtkWidget *widget,
		   GnomeCD *gcd)
{
	GError *error;
	GnomeCDRomCDDBData *data;
	CORBA_Environment ev;
	char *discid;
	
	if (gnome_cdrom_get_cddb_data (gcd->cdrom, &data, &error) == FALSE) {
		gcd_warning ("gnome_cdrom_get_cddb_data returned FALSE: %s", error);
		g_error_free (error);
		return;
	}
	
	CORBA_exception_init (&ev);
	if (track_editor == CORBA_OBJECT_NIL) {
		track_editor = bonobo_activation_activate_from_id (CDDBSLAVE_TRACK_EDITOR_IID, 0, NULL, &ev);
		if (BONOBO_EX (&ev)) {
			/* FIXME: Should be an error dialog */
			g_warning ("Could not activate track editor.\n%s",
				   CORBA_exception_id (&ev));
			CORBA_exception_free (&ev);
			
			return;
		}
		
		if (track_editor == CORBA_OBJECT_NIL) {
			/* FIXME: Should be an error dialog */
			g_warning ("Could not start track editor.");
			
			return;
		}

		/* Listen for the trackeditor dying on us,
		   then restart it */
		ORBit_small_listen_for_broken (track_editor, G_CALLBACK (restart_track_editor), &ev);
	}

	if (data != NULL) {
		discid = g_strdup_printf ("%08lx", (gulong) data->discid);
		GNOME_Media_CDDBTrackEditor_setDiscID (track_editor, discid, &ev);
		g_free (discid);
	}
	
	GNOME_Media_CDDBTrackEditor_showWindow (track_editor, &ev);
	if (BONOBO_EX (&ev)) {
		/* FIXME: Should be an error dialog */
		CORBA_exception_free (&ev);
		bonobo_object_release_unref (track_editor, NULL);
		return;
	}
	CORBA_exception_free (&ev);
}

void
volume_changed (GtkRange *range,
		GnomeCD *gcd)
{
	double volume;
	GError *error;

	volume = gtk_range_get_value (range);

	if (gnome_cdrom_set_volume (gcd->cdrom, (int) -volume, &error) == FALSE) {
		gcd_warning ("Error setting volume: %s", error);
		g_error_free (error);
	}
}
