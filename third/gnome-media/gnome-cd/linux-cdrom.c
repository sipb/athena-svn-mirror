/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * linux-cdrom.c: Linux CD controlling functions.
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgnome/gnome-i18n.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "linux-cdrom.h"
#include "cddb.h"

#include <linux/cdrom.h>

static GnomeCDRomClass *parent_class = NULL;

typedef struct _LinuxCDRomTrackInfo {
	char *name;
	unsigned char track;
	unsigned int audio_track:1;
	GnomeCDRomMSF address;
	GnomeCDRomMSF length;
} LinuxCDRomTrackInfo;

struct _LinuxCDRomPrivate {
	GnomeCDRomUpdate update;

	struct cdrom_tochdr *tochdr;
	int number_tracks;
	unsigned char track0, track1;

	LinuxCDRomTrackInfo *track_info;
	
};

static gboolean linux_cdrom_eject (GnomeCDRom *cdrom,
				   GError **error);
static gboolean linux_cdrom_next (GnomeCDRom *cdrom,
				  GError **error);
static gboolean linux_cdrom_ffwd (GnomeCDRom *cdrom,
				  GError **error);
static gboolean linux_cdrom_play (GnomeCDRom *cdrom,
				  int start_track,
				  GnomeCDRomMSF *start,
				  int finish_track,
				  GnomeCDRomMSF *finish,
				  GError **error);
static gboolean linux_cdrom_pause (GnomeCDRom *cdrom,
				   GError **error);
static gboolean linux_cdrom_stop (GnomeCDRom *cdrom,
				  GError **error);
static gboolean linux_cdrom_rewind (GnomeCDRom *cdrom,
				    GError **error);
static gboolean linux_cdrom_back (GnomeCDRom *cdrom,
				  GError **error);
static gboolean linux_cdrom_get_status (GnomeCDRom *cdrom,
					GnomeCDRomStatus **status,
					GError **error);
static gboolean linux_cdrom_close_tray (GnomeCDRom *cdrom,
					GError **error);

static GnomeCDRomMSF blank_msf = { 0, 0, 0};

static void
linux_cdrom_finalize (GObject *object)
{
	LinuxCDRom *cdrom = (LinuxCDRom *) object;

	g_free (cdrom->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static int
msf_to_frames (GnomeCDRomMSF *msf)
{
	return (msf->minute * 60 * CD_FRAMES) + (msf->second * CD_FRAMES) + msf->frame;
}

static void
frames_to_msf (GnomeCDRomMSF *msf,
	       int frames)
{
	/* Now convert the difference in frame lengths back into MSF
	   format */
	msf->minute = frames / (60 * CD_FRAMES);
	frames -= (msf->minute * 60 * CD_FRAMES);
	msf->second = frames / CD_FRAMES;
	frames -= (msf->second * CD_FRAMES);
	msf->frame = frames;
}

static void
add_msf (GnomeCDRomMSF *msf1,
	 GnomeCDRomMSF *msf2,
	 GnomeCDRomMSF *dest)
{
	int frames1, frames2, total;

	frames1 = msf_to_frames (msf1);
	frames2 = msf_to_frames (msf2);

	total = frames1 + frames2;

	frames_to_msf (dest, total);
}

static gboolean
linux_cdrom_open (LinuxCDRom *lcd,
		  GError **error)
{
	return gnome_cdrom_open_dev (GNOME_CDROM (lcd), error);
}

static void
linux_cdrom_close (LinuxCDRom *lcd)
{
	gnome_cdrom_close_dev (GNOME_CDROM (lcd), FALSE);
}

static void
linux_cdrom_invalidate (LinuxCDRom *lcd)
{
	if (lcd->priv->track_info == NULL) {
		g_free (lcd->priv->track_info);
		lcd->priv->track_info = NULL;
	}
}

static void
calculate_track_lengths (LinuxCDRom *lcd)
{
	LinuxCDRomPrivate *priv;
	int i;

	priv = lcd->priv;
	for (i = 0; i < priv->number_tracks; i++) {
		GnomeCDRomMSF *msf1, *msf2;
		int f1, f2, df;

		msf1 = &priv->track_info[i].address;
		msf2 = &priv->track_info[i + 1].address;

		/* Convert all addresses to frames */
		f1 = msf_to_frames (msf1);
		f2 = msf_to_frames (msf2);

		df = f2 - f1;
		frames_to_msf (&priv->track_info[i].length, df);
	}
}

static void
linux_cdrom_update_cd (GnomeCDRom *cdrom)
{
	LinuxCDRom *lcd = LINUX_CDROM (cdrom);
	LinuxCDRomPrivate *priv;
	struct cdrom_tocentry tocentry;
	int i, j;
	GError *error;

	priv = lcd->priv;

	if (linux_cdrom_open (lcd, &error) == FALSE) {
		g_message ("Error opening CD");
		return;
	}

	if (ioctl (cdrom->fd, CDROMREADTOCHDR, priv->tochdr) < 0) {
		g_message ("Error reading CD header");
		linux_cdrom_close (lcd);

		return;
	}
	
	priv->track0 = priv->tochdr->cdth_trk0;
	priv->track1 = priv->tochdr->cdth_trk1;
	priv->number_tracks = priv->track1 - priv->track0 + 1;

	linux_cdrom_invalidate (lcd);
	priv->track_info = g_malloc ((priv->number_tracks + 1) * sizeof (LinuxCDRomTrackInfo));
	for (i = 0, j = priv->track0; i < priv->number_tracks; i++, j++) {
		tocentry.cdte_track = j;
		tocentry.cdte_format = CDROM_MSF;

		if (ioctl (cdrom->fd, CDROMREADTOCENTRY, &tocentry) < 0) {
			g_warning ("IOCtl failed");
			continue;
		}

		priv->track_info[i].track = j;
		priv->track_info[i].audio_track = tocentry.cdte_ctrl != CDROM_DATA_TRACK ? 1 : 0;
		ASSIGN_MSF (priv->track_info[i].address, tocentry.cdte_addr.msf);
	}

	tocentry.cdte_track = CDROM_LEADOUT;
	tocentry.cdte_format = CDROM_MSF;
	if (ioctl (cdrom->fd, CDROMREADTOCENTRY, &tocentry) < 0) {
		g_warning ("Error getting leadout");
		linux_cdrom_invalidate (lcd);
		g_free (priv->track_info);
		priv->track_info = NULL;
		return;
	}
	ASSIGN_MSF (priv->track_info[priv->number_tracks].address, tocentry.cdte_addr.msf);
	calculate_track_lengths (lcd);

	linux_cdrom_close (lcd);
	//g_free (priv->track_info);
	//priv->track_info = NULL;

	return;
}

static gboolean
linux_cdrom_eject (GnomeCDRom *cdrom,
		   GError **error)
{
	LinuxCDRom *lcd;
	GnomeCDRomStatus *status;

	lcd = LINUX_CDROM (cdrom);

	if (linux_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		linux_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_TRAY_OPEN) {
		if (ioctl (cdrom->fd, CDROMEJECT, 0) < 0) {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_SYSTEM_ERROR,
						      "(eject): ioctl failed: %s",
						      g_strerror (errno));
			}

			g_free (status);
			linux_cdrom_close (lcd);
			return FALSE;
		}
	} else {
		/* Try to close the tray if it's open */
		if (gnome_cdrom_close_tray (cdrom, error) == FALSE) {
			
			g_free (status);
			linux_cdrom_close (lcd);

			return FALSE;
		}
	}

	g_free (status);

	gnome_cdrom_close_dev (cdrom, TRUE);

	return TRUE;
}

static gboolean
linux_cdrom_next (GnomeCDRom *cdrom,
		  GError **error)
{
	LinuxCDRom *lcd;
	GnomeCDRomStatus *status;
	GnomeCDRomMSF msf, *endmsf;
	int track, end_track;

	lcd = LINUX_CDROM (cdrom);
	if (linux_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (linux_cdrom_get_status (cdrom, &status, error) == FALSE) {
		linux_cdrom_close (lcd);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		linux_cdrom_close (lcd);
		g_free (status);
		
		return TRUE;
	}

	track = status->track + 1;
	g_free (status);
	if (track > lcd->priv->number_tracks) {
		/* Do nothing */
		linux_cdrom_close (lcd);
		return TRUE;
	}

	msf.minute = 0;
	msf.second = 0;
	msf.frame = 0;

	if (cdrom->playmode == GNOME_CDROM_WHOLE_CD) {
		end_track = -1;
		endmsf = NULL;
	} else {
		end_track = track + 1;
		endmsf = &msf;
	}
	
	if (linux_cdrom_play (cdrom, track, &msf, end_track, endmsf, error) == FALSE) {
		linux_cdrom_close (lcd);
		return FALSE;
	}

	linux_cdrom_close (lcd);
	return TRUE;
}

static gboolean
linux_cdrom_ffwd (GnomeCDRom *cdrom,
		  GError **error)
{
	LinuxCDRom *lcd;
	GnomeCDRomStatus *status;
	GnomeCDRomMSF *msf, *endmsf, end;
	int discend, frames, end_track;
	
	lcd = LINUX_CDROM (cdrom);
	if (linux_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (linux_cdrom_get_status (cdrom, &status, error) == FALSE) {
		linux_cdrom_close (lcd);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		linux_cdrom_close (lcd);
		g_free (status);
		
		return TRUE;
	}

	msf = &status->absolute;
	/* Convert MSF to frames to do calculations on it */
	frames = msf_to_frames (msf);
	/* Add a second */
	frames += CD_FRAMES;

	/* Check if we've skipped past the end */
	discend = msf_to_frames (&lcd->priv->track_info[lcd->priv->number_tracks].address);
	if (frames >= discend) {
		/* Do nothing */
		g_free (status);
		linux_cdrom_close (lcd);
		return TRUE;
	}
	
	/* Convert back to MSF */
	frames_to_msf (msf, frames);
	/* Zero the frames */
	msf->frame = 0;

	if (cdrom->playmode == GNOME_CDROM_WHOLE_CD) {
		endmsf = NULL;
		end_track = -1;
	} else {
		end.minute = 0;
		end.second = 0;
		end.frame = 0;
		endmsf = &end;
		end_track = status->track + 1;
	}
	
	if (linux_cdrom_play (cdrom, -1, msf, end_track, endmsf, error) == FALSE) {
		g_free (status);
		linux_cdrom_close (lcd);
		return FALSE;
	}

	g_free (status);
	linux_cdrom_close (lcd);

	return TRUE;
}

static gboolean
linux_cdrom_play (GnomeCDRom *cdrom,
		  int start_track,
		  GnomeCDRomMSF *start,
		  int finish_track,
		  GnomeCDRomMSF *finish,
		  GError **error)
{
	LinuxCDRom *lcd;
	LinuxCDRomPrivate *priv;
	GnomeCDRomStatus *status;
	struct cdrom_msf msf;

	lcd = LINUX_CDROM (cdrom);
	priv = lcd->priv;
	if (linux_cdrom_open(lcd, error) == FALSE) {
		return FALSE;
	}

	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		linux_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		if (status->cd == GNOME_CDROM_STATUS_TRAY_OPEN) {
			if (linux_cdrom_close_tray (cdrom, error) == FALSE) {
				linux_cdrom_close (lcd);
				g_free (status);
				return FALSE;
			}
		} else {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_NOT_READY,
						      "(linux_cdrom_play): Drive not ready");
			}

			linux_cdrom_close (lcd);
			g_free (status);
			return FALSE;
		}
	}

	g_free (status);
	
	/* Get the status again: It might have changed */
	if (gnome_cdrom_get_status (GNOME_CDROM (lcd), &status, error) == FALSE) {
		linux_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}
	if (status->cd != GNOME_CDROM_STATUS_OK) {
		/* Stuff if :) */
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_NOT_READY,
					      "(linux_cdrom_play): Drive still not ready");
		}

		linux_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

	switch (status->audio) {
	case GNOME_CDROM_AUDIO_PAUSE:
		if (gnome_cdrom_pause (GNOME_CDROM (lcd), error) == FALSE) {
			g_free (status);
			linux_cdrom_close (lcd);
			return FALSE;
		}
		break;

	case GNOME_CDROM_AUDIO_NOTHING:
	case GNOME_CDROM_AUDIO_COMPLETE:
	case GNOME_CDROM_AUDIO_STOP:
	case GNOME_CDROM_AUDIO_ERROR:
	default:
		/* Start playing */
		if (start == NULL) {
			msf.cdmsf_min0 = status->absolute.minute;
			msf.cdmsf_sec0 = status->absolute.second;
			msf.cdmsf_frame0 = status->absolute.frame;
		} else {
			if (start_track > 0 &&
			    priv && priv->track_info &&
			    start_track <= priv->number_tracks ) {
				GnomeCDRomMSF tmpmsf;
				
				add_msf (&priv->track_info[start_track - 1].address, start, &tmpmsf);
				msf.cdmsf_min0 = tmpmsf.minute;
				msf.cdmsf_sec0 = tmpmsf.second;
				msf.cdmsf_frame0 = tmpmsf.frame;
			} else {
				msf.cdmsf_min0 = start->minute;
				msf.cdmsf_sec0 = start->second;
				msf.cdmsf_frame0 = start->frame;
			}
		}

		if (finish == NULL) {
			if ( priv && priv->track_info && 
			    priv->number_tracks >0 ) {
				msf.cdmsf_min1 = priv->track_info[priv->number_tracks].address.minute;
				msf.cdmsf_sec1 = priv->track_info[priv->number_tracks].address.second;
				msf.cdmsf_frame1 = priv->track_info[priv->number_tracks].address.frame;
			} else {
				msf.cdmsf_min1 = 0;
				msf.cdmsf_sec1 = 0;
				msf.cdmsf_frame1 = 0;
			}
		} else {
			if (finish_track > 0 &&
			    priv && priv->track_info &&
			    finish_track <= priv->number_tracks ) {
				GnomeCDRomMSF tmpmsf;

				add_msf (&priv->track_info[finish_track - 1].address, finish, &tmpmsf);
				msf.cdmsf_min1 = tmpmsf.minute;
				msf.cdmsf_sec1 = tmpmsf.second;
				msf.cdmsf_frame1 = tmpmsf.frame;
			} else {
				msf.cdmsf_min1 = finish->minute;
				msf.cdmsf_sec1 = finish->second;
				msf.cdmsf_frame1 = finish->frame;
			}
		}

		/* PLAY IT AGAIN */
		if (ioctl (cdrom->fd, CDROMPLAYMSF, &msf) < 0) {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_SYSTEM_ERROR,
						      "(linux_cdrom_play) ioctl failed %s",
						      g_strerror (errno));
			}

			linux_cdrom_close (lcd);
			g_free (status);
			return FALSE;
		}
	}

	linux_cdrom_close (lcd);
	g_free (status);
	return TRUE;
}

static gboolean
linux_cdrom_pause (GnomeCDRom *cdrom,
		   GError **error)
{
	LinuxCDRom *lcd;
	GnomeCDRomStatus *status;

	lcd = LINUX_CDROM (cdrom);
	if (linux_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		linux_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_NOT_READY,
					      "(linux_cdrom_pause): Drive not ready");
		}

		g_free (status);
		linux_cdrom_close (lcd);
		return FALSE;
	}

	if (status->audio == GNOME_CDROM_AUDIO_PAUSE) {
		if (ioctl (cdrom->fd, CDROMRESUME) < 0) {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_SYSTEM_ERROR,
						      "(linux_cdrom_pause): Resume failed %s",
						      g_strerror (errno));
			}

			g_free (status);
			linux_cdrom_close (lcd);
			return FALSE;
		}

		linux_cdrom_close (lcd);
		g_free (status);
		return TRUE;
	}

	if (status->audio == GNOME_CDROM_AUDIO_PLAY) {
		if (ioctl (cdrom->fd, CDROMPAUSE, 0) < 0) {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_SYSTEM_ERROR,
						      "(linux_cdrom_pause): ioctl failed %s",
						      g_strerror (errno));
			}

			g_free (status);
			linux_cdrom_close (lcd);
			return FALSE;
		}
	}

	g_free (status);
	linux_cdrom_close (lcd);
	return TRUE;
}

static gboolean
linux_cdrom_stop (GnomeCDRom *cdrom,
		  GError **error)
{
	LinuxCDRom *lcd;
	GnomeCDRomStatus *status;

	lcd = LINUX_CDROM (cdrom);
	if (linux_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		linux_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

#if 0
	if (status->audio == GNOME_CDROM_AUDIO_PAUSE) {
		if (linux_cdrom_pause (cdrom, error) == FALSE) {
			linux_cdrom_close (lcd);
			g_free (status);
			return FALSE;
		}
	}
#endif

	if (ioctl (cdrom->fd, CDROMSTOP, 0) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(linux_cdrom_stop) ioctl failed %s",
					      g_strerror (errno));
		}

		linux_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

	linux_cdrom_close (lcd);
	return TRUE;
}

static gboolean
linux_cdrom_rewind (GnomeCDRom *cdrom,
		    GError **error)
{
	LinuxCDRom *lcd;
	GnomeCDRomMSF *msf, tmpmsf, end, *endmsf;
	GnomeCDRomStatus *status;
	int discstart, frames, end_track;

	lcd = LINUX_CDROM (cdrom);
	if (linux_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (linux_cdrom_get_status (cdrom, &status, error) == FALSE) {
		linux_cdrom_close (lcd);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		linux_cdrom_close (lcd);
		g_free (status);
		
		return TRUE;
	}
	
	msf = &status->absolute;

	frames = msf_to_frames (msf);
	frames -= CD_FRAMES; /* Back one second */

	/* Check we've not run back past the start */
	discstart = msf_to_frames (&lcd->priv->track_info[0].address);
	if (frames < discstart) {
		g_free (status);
		linux_cdrom_close (lcd);
		return TRUE;
	}

	frames_to_msf (&tmpmsf, frames);
	tmpmsf.frame = 0; /* Zero the frames */

	if (cdrom->playmode == GNOME_CDROM_WHOLE_CD) {
		end_track = -1;
		endmsf = NULL;
	} else {
		end_track = status->track + 1;
		end.minute = 0;
		end.second = 0;
		end.frame = 0;
		endmsf = &end;
	}
	
	if (linux_cdrom_play (cdrom, -1, &tmpmsf, end_track, endmsf, error) == FALSE) {
		g_free (status);
		
		linux_cdrom_close (lcd);
		return FALSE;
	}

	linux_cdrom_close (lcd);
	g_free (status);

	return TRUE;
}

static gboolean
linux_cdrom_back (GnomeCDRom *cdrom,
		  GError **error)
{
	LinuxCDRom *lcd;
	GnomeCDRomStatus *status;
	GnomeCDRomMSF msf, *endmsf;
	int track, end_track;

	lcd = LINUX_CDROM (cdrom);
	if (linux_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (linux_cdrom_get_status (cdrom, &status, error) == FALSE) {
		linux_cdrom_close (lcd);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		linux_cdrom_close (lcd);
		g_free (status);
		
		return TRUE;
	}

	/* If we're > 0:00 on the track go back to the start of it, 
	   otherwise go to the previous track */
	if (status->relative.minute != 0 || status->relative.second != 0) {
		track = status->track;
	} else {
		track = status->track - 1;
	}

	if (track <= 0) {
		/* nothing */
		g_free (status);
		linux_cdrom_close (lcd);
		return TRUE;
	}

	msf.minute = 0;
	msf.second = 0;
	msf.frame = 0;

	if (cdrom->playmode == GNOME_CDROM_WHOLE_CD) {
		end_track = -1;
		endmsf = NULL;
	} else {
		end_track = track + 1;
		endmsf = &msf;
	}
	if (linux_cdrom_play (cdrom, track, &msf, end_track, endmsf, error) == FALSE) {
		g_free (status);
		linux_cdrom_close (lcd);
		return FALSE;
	}

	g_free (status);
	linux_cdrom_close (lcd);
	return TRUE;
}

/* There should probably be 2 get_status functions.
   A public one and the private one.
   The private one would get called by the update handler every second, and the
   public one would just return a copy of the status */
static gboolean
linux_cdrom_get_status (GnomeCDRom *cdrom,
			GnomeCDRomStatus **status,
			GError **error)
{
	LinuxCDRom *lcd;
	LinuxCDRomPrivate *priv;
	GnomeCDRomStatus *realstatus;
	struct cdrom_subchnl subchnl;
	struct cdrom_tocentry tocentry;
	struct cdrom_volctrl vol;
	int cd_status;
	int i,j;
	/* bug 117695: set this to FALSE if this ioctl is not supported */
	static gboolean CDROMVOLREAD_supported = TRUE;
	
	g_return_val_if_fail (status != NULL, TRUE);
	
	lcd = LINUX_CDROM (cdrom);
	priv = lcd->priv;

	*status = g_new0 (GnomeCDRomStatus, 1);
	realstatus = *status;
	realstatus->volume = 0;
	
	if (linux_cdrom_open (lcd, error) == FALSE) {
		linux_cdrom_close (lcd);
		g_free (realstatus);
		*status = NULL;
		return FALSE;
	}

	cd_status = ioctl (cdrom->fd, CDROM_DRIVE_STATUS, CDSL_CURRENT);
	if (cd_status != -1) {
		switch (cd_status) {
		case CDS_NO_INFO:
			realstatus->cd = GNOME_CDROM_STATUS_NO_DISC;
			realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
			realstatus->track = -1;

			linux_cdrom_close (lcd);
			return TRUE;

		case CDS_NO_DISC:
			realstatus->cd = GNOME_CDROM_STATUS_NO_DISC;
			realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
			realstatus->track = -1;

			linux_cdrom_close (lcd);
			return TRUE;
			
		case CDS_TRAY_OPEN:
			realstatus->cd = GNOME_CDROM_STATUS_TRAY_OPEN;
			realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
			realstatus->track = -1;

			linux_cdrom_close (lcd);
			return TRUE;

		case CDS_DRIVE_NOT_READY:
			realstatus->cd = GNOME_CDROM_STATUS_DRIVE_NOT_READY;
			realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
			realstatus->track = -1;
			
			linux_cdrom_close (lcd);
			return TRUE;

		default:
			if (ioctl (cdrom->fd, CDROMREADTOCHDR, priv->tochdr) < 0) {
				g_print ("Error reading CD header");
				linux_cdrom_close (lcd);

				return;
			}
			realstatus->cd = GNOME_CDROM_STATUS_DATA_CD;

			for (i = 0, j = priv->tochdr->cdth_trk0; i < (priv->tochdr->cdth_trk1 - priv->tochdr->cdth_trk0 + 1); i++, j++) {
				tocentry.cdte_track = j;
				tocentry.cdte_format = CDROM_MSF;

				if (ioctl (cdrom->fd, CDROMREADTOCENTRY, &tocentry) < 0) {
					g_print ("IOCtl failed");
					continue;
				}

				if (tocentry.cdte_ctrl != CDROM_DATA_TRACK) {
					realstatus->cd = GNOME_CDROM_STATUS_OK;
					break; 
				}
			}
			break;
		}
	} else {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(linux_cdrom_get_status): ioctl error %s",
					      g_strerror (errno));
		}

		linux_cdrom_close (lcd);
		g_free (realstatus);
		*status = NULL;
		return FALSE;
	}

	subchnl.cdsc_format = CDROM_MSF;
	if (ioctl (cdrom->fd, CDROMSUBCHNL, &subchnl) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(linux_cdrom_get_status): CDROMSUBCHNL ioctl failed %s",
					      g_strerror (errno));
		}

		linux_cdrom_close (lcd);
		g_free (realstatus);
		*status = NULL;
		return FALSE;
	}

	/* Get the volume */
	if (CDROMVOLREAD_supported) {
		if (ioctl (cdrom->fd, CDROMVOLREAD, &vol) < 0) {
			/* if ENOTSUP, flag so we don't repeatedly call */
			if (errno == ENOTSUP) {
				g_warning ("(linux_cdrom_get_status): CDROMVOLREAD ioctl not supported.");
				CDROMVOLREAD_supported = FALSE;
			} else {
				g_warning ("(linux_cdrom_get_status): CDROMVOLREAD ioctl failed %s",
					g_strerror (errno));
				realstatus->volume = -1; /* -1 means no volume command */
			}
		} else {
			realstatus->volume = vol.channel0;
		}
	}

	linux_cdrom_close (lcd);

	ASSIGN_MSF (realstatus->relative, blank_msf);
	ASSIGN_MSF (realstatus->absolute, blank_msf);
	ASSIGN_MSF (realstatus->length, blank_msf);
	
	realstatus->track = 1;
	switch (subchnl.cdsc_audiostatus) {
	case CDROM_AUDIO_PLAY:
		realstatus->audio = GNOME_CDROM_AUDIO_PLAY;
		ASSIGN_MSF (realstatus->relative, subchnl.cdsc_reladdr.msf);
		ASSIGN_MSF (realstatus->absolute, subchnl.cdsc_absaddr.msf);
		realstatus->track = subchnl.cdsc_trk;
		if(priv && realstatus->track>0 && 
		   realstatus->track<=priv->number_tracks){
			// track_info may not be initialized
			ASSIGN_MSF (realstatus->length, priv->track_info[realstatus->track-1].length);
		}
		break;

	case CDROM_AUDIO_PAUSED:
		realstatus->audio = GNOME_CDROM_AUDIO_PAUSE;
		ASSIGN_MSF (realstatus->relative, subchnl.cdsc_reladdr.msf);
		ASSIGN_MSF (realstatus->absolute, subchnl.cdsc_absaddr.msf);
		realstatus->track = subchnl.cdsc_trk;
		if(priv && realstatus->track>0 && 
		   realstatus->track<=priv->number_tracks){
			// track_info may not be initialized
			ASSIGN_MSF (realstatus->length, priv->track_info[realstatus->track-1].length);
		}
		break;

	case CDROM_AUDIO_COMPLETED:
		realstatus->audio = GNOME_CDROM_AUDIO_COMPLETE;
		ASSIGN_MSF (realstatus->relative, subchnl.cdsc_reladdr.msf);
		ASSIGN_MSF (realstatus->absolute, subchnl.cdsc_absaddr.msf);
		realstatus->track = subchnl.cdsc_trk;
		if(priv && realstatus->track>0 && 
		   realstatus->track<=priv->number_tracks){
			// track_info may not be initialized
			ASSIGN_MSF (realstatus->length, priv->track_info[realstatus->track-1].length);
		}
		break;
		
	case CDROM_AUDIO_INVALID:
	case CDROM_AUDIO_NO_STATUS:
		realstatus->audio = GNOME_CDROM_AUDIO_STOP;
		break;
		
	case CDROM_AUDIO_ERROR:
	default:
		realstatus->audio = GNOME_CDROM_AUDIO_ERROR;
		break;
	}

	return TRUE;
}

static gboolean
linux_cdrom_close_tray (GnomeCDRom *cdrom,
			GError **error)
{
	LinuxCDRom *lcd;

	lcd = LINUX_CDROM (cdrom);
	if (linux_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (ioctl (cdrom->fd, CDROMCLOSETRAY) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(linux_cdrom_close_tray): ioctl failed %s",
					      g_strerror (errno));
		}

		linux_cdrom_close (lcd);
		return FALSE;
	}

	linux_cdrom_close (lcd);
	return TRUE;
}

static gboolean
linux_cdrom_set_volume (GnomeCDRom *cdrom,
			int volume,
			GError **error)
{
	LinuxCDRom *lcd;
	LinuxCDRomPrivate *priv;
	struct cdrom_volctrl vol;

	lcd = LINUX_CDROM (cdrom);
	priv = lcd->priv;

	if (linux_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}
	
	vol.channel0 = volume;
	vol.channel1 = vol.channel2 = vol.channel3 = volume;
	
	if (ioctl (cdrom->fd, CDROMVOLCTRL, &vol) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(linux_cdrom_set_volume:1): ioctl failed %s",
					      g_strerror (errno));
		}

		linux_cdrom_close (lcd);
		return FALSE;
	}

	linux_cdrom_close (lcd);
	return TRUE;
}

static gboolean
linux_cdrom_is_cdrom_device (GnomeCDRom *cdrom,
			     const char *device,
			     GError **error)
{
	int fd;

	if (device == NULL || *device == 0) {
		return FALSE;
	}
	
	fd = open (device, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		return FALSE;
	}

	/* Fire a harmless ioctl at the device. */
	if (ioctl (fd, CDROM_GET_CAPABILITY, 0) < 0) {
		/* Failed, it's not a CDROM drive */
		close (fd);
		
		return FALSE;
	}
	
	close (fd);
	
	return TRUE;
}

static gboolean
linux_cdrom_get_cddb_data (GnomeCDRom *cdrom,
			   GnomeCDRomCDDBData **data,
			   GError **error)
{
	LinuxCDRom *lcd;
	LinuxCDRomPrivate *priv;
	int i, t = 0, n = 0;

	lcd = LINUX_CDROM (cdrom);
	priv = lcd->priv;

	if (linux_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (priv->track_info == NULL) {
		*data = NULL;
		return TRUE;
	}
	
	*data = g_new0 (GnomeCDRomCDDBData, 1);

	for (i = 0; i < priv->number_tracks; i++) {
		n += cddb_sum ((priv->track_info[i].address.minute * 60) + 
			       priv->track_info[i].address.second);
		t += ((priv->track_info[i + 1].address.minute * 60) +
		      priv->track_info[i + 1].address.second) - 
			((priv->track_info[i].address.minute * 60) + 
			 priv->track_info[i].address.second);
	}

	(*data)->discid = ((n % 0xff) << 24 | t << 8 | (priv->track1));
	(*data)->ntrks = priv->track1;
	(*data)->nsecs = (priv->track_info[priv->track1].address.minute * 60) + priv->track_info[priv->track1].address.second;
	(*data)->offsets = g_new0 (unsigned int, priv->track1 + 1);

	for (i = priv->track0 - 1; i < priv->track1; i++) {
		(*data)->offsets[i] = msf_to_frames (&priv->track_info[i].address);
	}

	linux_cdrom_close (lcd);
	return TRUE;
}

static void
linux_cdrom_class_init (LinuxCDRomClass *klass)
{
	GObjectClass *object_class;
	GnomeCDRomClass *cdrom_class;
	
	object_class = G_OBJECT_CLASS (klass);
	cdrom_class = GNOME_CDROM_CLASS (klass);

	object_class->finalize = linux_cdrom_finalize;

	cdrom_class->eject = linux_cdrom_eject;
	cdrom_class->next = linux_cdrom_next;
	cdrom_class->ffwd = linux_cdrom_ffwd;
	cdrom_class->play = linux_cdrom_play;
	cdrom_class->pause = linux_cdrom_pause;
	cdrom_class->stop = linux_cdrom_stop;
	cdrom_class->rewind = linux_cdrom_rewind;
	cdrom_class->back = linux_cdrom_back;
	cdrom_class->get_status = linux_cdrom_get_status;
	cdrom_class->close_tray = linux_cdrom_close_tray;
	cdrom_class->set_volume = linux_cdrom_set_volume;
	cdrom_class->is_cdrom_device = linux_cdrom_is_cdrom_device;
	cdrom_class->update_cd = linux_cdrom_update_cd;
	
	/* For CDDB */
  	cdrom_class->get_cddb_data = linux_cdrom_get_cddb_data;

	parent_class = g_type_class_peek_parent (klass);
}

static void
linux_cdrom_init (LinuxCDRom *cdrom)
{
	cdrom->priv = g_new0 (LinuxCDRomPrivate, 1);
	cdrom->priv->tochdr = g_new0 (struct cdrom_tochdr, 1);
	cdrom->priv->track_info = NULL;
}
	
/* API */
GType
linux_cdrom_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		GTypeInfo info = {
			sizeof (LinuxCDRomClass),
			NULL, NULL, (GClassInitFunc) linux_cdrom_class_init, NULL, NULL,
			sizeof (LinuxCDRom), 0, (GInstanceInitFunc) linux_cdrom_init,
		};

		type = g_type_register_static (GNOME_CDROM_TYPE, "LinuxCDRom", &info, 0);
	}

	return type;
}

GnomeCDRom *
gnome_cdrom_new (const char      *cdrom_device,
		 GnomeCDRomUpdate update,
		 GError         **error)
{
	return gnome_cdrom_construct (
		g_object_new (linux_cdrom_get_type (), NULL),
		cdrom_device, update, GNOME_CDROM_DEVICE_STATIC, error);
}
