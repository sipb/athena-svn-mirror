/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bsd-cdrom.c: BSD CD controlling functions.
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 *          Theo van Klaveren  <t.vanklaveren@student.utwente.nl>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgnome/gnome-i18n.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#ifdef HAVE_SYS_CDIO_H
# include <sys/cdio.h>
#endif
#include <errno.h>

#include "bsd-cdrom.h"

#define CD_FRAMES	75

static GnomeCDRomClass *parent_class = NULL;

typedef struct _BSDCDRomTrackInfo {
	char *name;
	unsigned char track;
	unsigned int audio_track:1;
	GnomeCDRomMSF address;
	GnomeCDRomMSF length;
} BSDCDRomTrackInfo;

struct _BSDCDRomPrivate {
	GnomeCDRomUpdate update;

	struct ioc_toc_header *tochdr;
	int number_tracks;
	unsigned char track0, track1;

	BSDCDRomTrackInfo *track_info;
};

static gboolean bsd_cdrom_eject (GnomeCDRom *cdrom,
				   GError **error);
static gboolean bsd_cdrom_next (GnomeCDRom *cdrom,
				  GError **error);
static gboolean bsd_cdrom_ffwd (GnomeCDRom *cdrom,
				  GError **error);
static gboolean bsd_cdrom_play (GnomeCDRom *cdrom,
				  int start_track,
				  GnomeCDRomMSF *start,
				  int finish_track,
				  GnomeCDRomMSF *finish,
				  GError **error);
static gboolean bsd_cdrom_pause (GnomeCDRom *cdrom,
				   GError **error);
static gboolean bsd_cdrom_stop (GnomeCDRom *cdrom,
				  GError **error);
static gboolean bsd_cdrom_rewind (GnomeCDRom *cdrom,
				    GError **error);
static gboolean bsd_cdrom_back (GnomeCDRom *cdrom,
				  GError **error);
static gboolean bsd_cdrom_get_status (GnomeCDRom *cdrom,
					GnomeCDRomStatus **status,
					GError **error);
static gboolean bsd_cdrom_close_tray (GnomeCDRom *cdrom,
					GError **error);

static GnomeCDRomMSF blank_msf = { 0, 0, 0};

static void
bsd_cdrom_finalize (GObject *object)
{
	BSDCDRom *cdrom = (BSDCDRom *) object;

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
bsd_cdrom_open (BSDCDRom *lcd,
		    GError      **error)
{
	return gnome_cdrom_open_dev (GNOME_CDROM (lcd), error);
}

static void
bsd_cdrom_close (BSDCDRom *lcd)
{
	gnome_cdrom_close_dev (GNOME_CDROM (lcd), FALSE);
}

static void
bsd_cdrom_invalidate (BSDCDRom *lcd)
{
	if (lcd->priv->track_info == NULL) {
		g_free (lcd->priv->track_info);
		lcd->priv->track_info = NULL;
	}
}

static void
calculate_track_lengths (BSDCDRom *lcd)
{
	BSDCDRomPrivate *priv;
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
bsd_cdrom_update_cd (GnomeCDRom *cdrom)
{
	BSDCDRom *lcd = BSD_CDROM (cdrom);
	BSDCDRomPrivate *priv;
#ifdef __FreeBSD__
	struct ioc_read_toc_single_entry tocentry;
#else
	struct ioc_read_toc_entry tocentries;
	struct cd_toc_entry tocentry;
#endif
	int i, j;
	GError *error;

	priv = lcd->priv;

	if (bsd_cdrom_open (lcd, &error) == FALSE) {
		g_message ("Error opening CD");
		return;
	}

	if (ioctl (cdrom->fd, CDIOREADTOCHEADER, priv->tochdr) < 0) {
		g_message ("Error reading CD header");
		bsd_cdrom_close (lcd);

		return;
	}
	
	priv->track0 = priv->tochdr->starting_track;
	priv->track1 = priv->tochdr->ending_track;
	priv->number_tracks = priv->track1 - priv->track0 + 1;

	bsd_cdrom_invalidate (lcd);
	priv->track_info = g_malloc ((priv->number_tracks + 1) * sizeof (BSDCDRomTrackInfo));
#ifdef __FreeBSD__
	for (i = 0, j = priv->track0; i < priv->number_tracks; i++, j++) {
		tocentry.track = j;
		tocentry.address_format = CD_MSF_FORMAT;
#else
	tocentries.data_len = sizeof(tocentry);
	tocentries.data = &tocentry;
	for (i = 0, j = priv->track0; i < priv->number_tracks; i++, j++) {
		tocentries.starting_track = j;
		tocentries.address_format = CD_MSF_FORMAT;
#endif

#ifdef __FreeBSD__
		if (ioctl (cdrom->fd, CDIOREADTOCENTRY, &tocentry) < 0) {
#else
		if (ioctl (cdrom->fd, CDIOREADTOCENTRYS, &tocentries) < 0) {
#endif
			g_warning ("IOCtl failed");
			continue;
		}

		priv->track_info[i].track = j;
#ifdef __FreeBSD__
		priv->track_info[i].audio_track = tocentry.entry.control == 0 ? 1 : 0;
		ASSIGN_MSF (priv->track_info[i].address, tocentry.entry.addr.msf);
#else
		priv->track_info[i].audio_track = tocentry.control == 0 ? 1 : 0;
		ASSIGN_MSF (priv->track_info[i].address, tocentry.addr.msf);
#endif
	}

#ifdef __FreeBSD__
	/* On BSD, the leadout track is the track LAST_TRACK + 1. */
	tocentry.track = priv->number_tracks + 1;
	tocentry.address_format = CD_MSF_FORMAT;
	if (ioctl (cdrom->fd, CDIOREADTOCENTRY, &tocentry) < 0) {
#else
	/* On NetBSD, the leadout track is the track 0xAA. */
	tocentries.starting_track = 0xAA;
	tocentries.address_format = CD_MSF_FORMAT;
	if (ioctl (cdrom->fd, CDIOREADTOCENTRYS, &tocentries) < 0) {
#endif
		g_warning ("Error getting leadout");
		bsd_cdrom_invalidate (lcd);
		return;
	}
#ifdef __FreeBSD__
	ASSIGN_MSF (priv->track_info[priv->number_tracks].address, tocentry.entry.addr.msf);
#else
	ASSIGN_MSF (priv->track_info[priv->number_tracks].address, tocentry.addr.msf);
#endif
	calculate_track_lengths (lcd);

	bsd_cdrom_close (lcd);
	return;
}

static gboolean
bsd_cdrom_eject (GnomeCDRom *cdrom,
		     GError    **error)
{
	BSDCDRom *lcd;
	GnomeCDRomStatus *status;

	lcd = BSD_CDROM (cdrom);

	if (bsd_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

#ifdef __FreeBSD__
	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		bsd_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}
#else
	status = g_new0 (GnomeCDRomStatus, 1);
	status->volume = 0;
	status->cd = GNOME_CDROM_STATUS_OK;
	ioctl(cdrom->fd, CDIOCALLOW);
#endif

	if (status->cd != GNOME_CDROM_STATUS_TRAY_OPEN) {
		if (ioctl (cdrom->fd, CDIOCEJECT, 0) < 0) {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_SYSTEM_ERROR,
						      "(eject): ioctl failed: %s",
						      strerror (errno));
			}

			g_free (status);
			bsd_cdrom_close (lcd);
			return FALSE;
		}
	} else {
		/* Try to close the tray if it's open */
		if (gnome_cdrom_close_tray (cdrom, error) == FALSE) {
			
			g_free (status);
			bsd_cdrom_close (lcd);

			return FALSE;
		}
	}

	g_free (status);

	gnome_cdrom_close_dev (cdrom, TRUE);

	return TRUE;
}

static gboolean
bsd_cdrom_next (GnomeCDRom *cdrom,
		  GError **error)
{
	BSDCDRom *lcd;
	GnomeCDRomStatus *status;
	GnomeCDRomMSF msf, *endmsf;
	int track, end_track;

	lcd = BSD_CDROM (cdrom);
	if (bsd_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (bsd_cdrom_get_status (cdrom, &status, error) == FALSE) {
		bsd_cdrom_close (lcd);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		bsd_cdrom_close (lcd);
		g_free (status);
		
		return TRUE;
	}

	track = status->track + 1;
	g_free (status);
	if (track > lcd->priv->number_tracks) {
		/* Do nothing */
		bsd_cdrom_close (lcd);
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
	
	if (bsd_cdrom_play (cdrom, track, &msf, end_track, endmsf, error) == FALSE) {
		bsd_cdrom_close (lcd);
		return FALSE;
	}

	bsd_cdrom_close (lcd);
	return TRUE;
}

static gboolean
bsd_cdrom_ffwd (GnomeCDRom *cdrom,
		  GError **error)
{
	BSDCDRom *lcd;
	GnomeCDRomStatus *status;
	GnomeCDRomMSF *msf, *endmsf, end;
	int discend, frames, end_track;
	
	lcd = BSD_CDROM (cdrom);
	if (bsd_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (bsd_cdrom_get_status (cdrom, &status, error) == FALSE) {
		bsd_cdrom_close (lcd);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		bsd_cdrom_close (lcd);
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
		bsd_cdrom_close (lcd);
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
	
	if (bsd_cdrom_play (cdrom, -1, msf, end_track, endmsf, error) == FALSE) {
		g_free (status);
		bsd_cdrom_close (lcd);
		return FALSE;
	}

	g_free (status);
	bsd_cdrom_close (lcd);

	return TRUE;
}

static gboolean
bsd_cdrom_play (GnomeCDRom *cdrom,
		  int start_track,
		  GnomeCDRomMSF *start,
		  int finish_track,
		  GnomeCDRomMSF *finish,
		  GError **error)
{
	BSDCDRom *lcd;
	BSDCDRomPrivate *priv;
	GnomeCDRomStatus *status;
	struct ioc_play_msf msf;
	int minutes, seconds, frames;

	lcd = BSD_CDROM (cdrom);
	priv = lcd->priv;
	if (bsd_cdrom_open(lcd, error) == FALSE) {
		return FALSE;
	}

	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		bsd_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		if (status->cd == GNOME_CDROM_STATUS_TRAY_OPEN) {
			if (bsd_cdrom_close_tray (cdrom, error) == FALSE) {
				bsd_cdrom_close (lcd);
				g_free (status);
				return FALSE;
			}
		} else {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_NOT_READY,
						      "(bsd_cdrom_play): Drive not ready");
			}

			bsd_cdrom_close (lcd);
			g_free (status);
			return FALSE;
		}
	}

	g_free (status);
	/* Get the status again: It might have changed */
	if (gnome_cdrom_get_status (GNOME_CDROM (lcd), &status, error) == FALSE) {
		bsd_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}
	if (status->cd != GNOME_CDROM_STATUS_OK) {
		/* Stuff if :) */
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_NOT_READY,
					      "(bsd_cdrom_play): Drive still not ready");
		}

		bsd_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

	switch (status->audio) {
	case GNOME_CDROM_AUDIO_PAUSE:
		if (gnome_cdrom_pause (GNOME_CDROM (lcd), error) == FALSE) {
			g_free (status);
			bsd_cdrom_close (lcd);
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
			msf.start_m = status->absolute.minute;
			msf.start_s = status->absolute.second;
			msf.start_f = status->absolute.frame;
		} else {
			if (start_track >= 0) {
				GnomeCDRomMSF tmpmsf;
				
				add_msf (&priv->track_info[start_track - 1].address, start, &tmpmsf);
				msf.start_m = tmpmsf.minute;
				msf.start_s = tmpmsf.second;
				msf.start_f = tmpmsf.frame;
			} else {
				msf.start_m = start->minute;
				msf.start_s = start->second;
				msf.start_f = start->frame;
			}
		}

		if (finish == NULL) {
			msf.end_m = priv->track_info[priv->number_tracks].address.minute;
			msf.end_s = priv->track_info[priv->number_tracks].address.second;
			msf.end_f = priv->track_info[priv->number_tracks].address.frame;
		} else {
			if (finish_track >= 0) {
				GnomeCDRomMSF tmpmsf;

				add_msf (&priv->track_info[finish_track - 1].address, finish, &tmpmsf);
				msf.end_m = tmpmsf.minute;
				msf.end_s = tmpmsf.second;
				msf.end_f = tmpmsf.frame;
			} else {
				msf.end_m = finish->minute;
				msf.end_s = finish->second;
				msf.end_f = finish->frame;
			}
		}

		/* PLAY IT AGAIN */
		if (ioctl (cdrom->fd, CDIOCPLAYMSF, &msf) < 0) {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_SYSTEM_ERROR,
						      "(bsd_cdrom_play) ioctl failed %s",
						      strerror (errno));
			}

			bsd_cdrom_close (lcd);
			g_free (status);
			return FALSE;
		}
	}

	bsd_cdrom_close (lcd);
	g_free (status);
	return TRUE;
}

static gboolean
bsd_cdrom_pause (GnomeCDRom *cdrom,
		   GError **error)
{
	BSDCDRom *lcd;
	GnomeCDRomStatus *status;

	lcd = BSD_CDROM (cdrom);
	if (bsd_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		bsd_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_NOT_READY,
					      "(bsd_cdrom_pause): Drive not ready");
		}

		g_free (status);
		bsd_cdrom_close (lcd);
		return FALSE;
	}

	if (status->audio == GNOME_CDROM_AUDIO_PAUSE) {
		if (ioctl (cdrom->fd, CDIOCRESUME) < 0) {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_SYSTEM_ERROR,
						      "(bsd_cdrom_pause): Resume failed %s",
						      strerror (errno));
			}

			g_free (status);
			bsd_cdrom_close (lcd);
			return FALSE;
		}

		bsd_cdrom_close (lcd);
		g_free (status);
		return TRUE;
	}

	if (status->audio == GNOME_CDROM_AUDIO_PLAY) {
		if (ioctl (cdrom->fd, CDIOCPAUSE, 0) < 0) {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_SYSTEM_ERROR,
						      "(bsd_cdrom_pause): ioctl failed %s",
						      strerror (errno));
			}

			g_free (status);
			bsd_cdrom_close (lcd);
			return FALSE;
		}
	}

	g_free (status);
	bsd_cdrom_close (lcd);
	return TRUE;
}

static gboolean
bsd_cdrom_stop (GnomeCDRom *cdrom,
		  GError **error)
{
	BSDCDRom *lcd;
	GnomeCDRomStatus *status;

	lcd = BSD_CDROM (cdrom);
	if (bsd_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		bsd_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

#if 0
	if (status->audio == GNOME_CDROM_AUDIO_PAUSE) {
		if (bsd_cdrom_pause (cdrom, error) == FALSE) {
			bsd_cdrom_close (lcd);
			g_free (status);
			return FALSE;
		}
	}
#endif

	if (ioctl (cdrom->fd, CDIOCSTOP, 0) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(bsd_cdrom_stop) ioctl failed %s",
					      strerror (errno));
		}

		bsd_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

	bsd_cdrom_close (lcd);
	g_free (status);
	return TRUE;
}

static gboolean
bsd_cdrom_rewind (GnomeCDRom *cdrom,
		    GError **error)
{
	BSDCDRom *lcd;
	GnomeCDRomMSF *msf, tmpmsf, end, *endmsf;
	GnomeCDRomStatus *status;
	int discstart, frames, end_track;

	lcd = BSD_CDROM (cdrom);
	if (bsd_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (bsd_cdrom_get_status (cdrom, &status, error) == FALSE) {
		bsd_cdrom_close (lcd);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		bsd_cdrom_close (lcd);
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
		bsd_cdrom_close (lcd);
		return TRUE;
	}

	frames_to_msf (&tmpmsf, frames);
	tmpmsf.frame = 0; /* Zero the frames */

	if (cdrom->playmode = GNOME_CDROM_WHOLE_CD) {
		end_track = -1;
		endmsf = NULL;
	} else {
		end_track = status->track + 1;
		end.minute = 0;
		end.second = 0;
		end.frame = 0;
		endmsf = &end;
	}
	
	if (bsd_cdrom_play (cdrom, -1, &tmpmsf, end_track, endmsf, error) == FALSE) {
		g_free (status);
		
		bsd_cdrom_close (lcd);
		return FALSE;
	}

	bsd_cdrom_close (lcd);
	g_free (status);

	return TRUE;
}

static gboolean
bsd_cdrom_back (GnomeCDRom *cdrom,
		  GError **error)
{
	BSDCDRom *lcd;
	GnomeCDRomStatus *status;
	GnomeCDRomMSF msf, *endmsf;
	int track, end_track;

	lcd = BSD_CDROM (cdrom);
	if (bsd_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (bsd_cdrom_get_status (cdrom, &status, error) == FALSE) {
		bsd_cdrom_close (lcd);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		bsd_cdrom_close (lcd);
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
		bsd_cdrom_close (lcd);
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
	if (bsd_cdrom_play (cdrom, track, &msf, end_track, endmsf, error) == FALSE) {
		g_free (status);
		bsd_cdrom_close (lcd);
		return FALSE;
	}

	g_free (status);
	bsd_cdrom_close (lcd);
	return TRUE;
}

/* There should probably be 2 get_status functions. A public one and the private one.
   The private one would get called by the update handler every second, and the
   public one would just return a copy of the status */
static gboolean
bsd_cdrom_get_status (GnomeCDRom *cdrom,
			GnomeCDRomStatus **status,
			GError **error)
{
	BSDCDRom *lcd;
	BSDCDRomPrivate *priv;
	GnomeCDRomStatus *realstatus;
	struct ioc_read_subchannel subchnl;
	struct cd_sub_channel_info subchnl_info;
	struct ioc_vol vol;
	int cd_status;

	g_return_val_if_fail (status != NULL, TRUE);
	
	lcd = BSD_CDROM (cdrom);
	priv = lcd->priv;

	*status = g_new0 (GnomeCDRomStatus, 1);
	realstatus = *status;
	realstatus->volume = 0;

	if (bsd_cdrom_open (lcd, error) == FALSE) {
		bsd_cdrom_close (lcd);
		g_free (realstatus);
		*status = NULL;
		return FALSE;
	}

#if 0
	cd_status = ioctl (cdrom->fd, CDROM_DRIVE_STATUS, CDSL_CURRENT);
	if (cd_status != -1) {
		switch (cd_status) {
		case CDS_NO_INFO:
			realstatus->cd = GNOME_CDROM_STATUS_NO_DISC;
			realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
			realstatus->track = -1;

			bsd_cdrom_close (lcd);
			return TRUE;

		case CDS_NO_DISC:
			realstatus->cd = GNOME_CDROM_STATUS_NO_DISC;
			realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
			realstatus->track = -1;

			bsd_cdrom_close (lcd);
			return TRUE;
			
		case CDS_TRAY_OPEN:
			realstatus->cd = GNOME_CDROM_STATUS_TRAY_OPEN;
			realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
			realstatus->track = -1;

			bsd_cdrom_close (lcd);
			return TRUE;

		case CDS_DRIVE_NOT_READY:
			realstatus->cd = GNOME_CDROM_STATUS_DRIVE_NOT_READY;
			realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
			realstatus->track = -1;
			
			bsd_cdrom_close (lcd);
			return TRUE;

		default:
			realstatus->cd = GNOME_CDROM_STATUS_OK;
			break;
		}
	} else {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(bsd_cdrom_get_status): ioctl error %s",
					      strerror (errno));
		}

		bsd_cdrom_close (lcd);
		g_free (realstatus);
		*status = NULL;
		return FALSE;
	}
#endif
#ifdef __FreeBSD__
	/* FIXME: Figure out how to do this on BSD */
#else
	if (ioctl (cdrom->fd, CDIOREADTOCHEADER, priv->tochdr) < 0) {
		realstatus->cd = GNOME_CDROM_STATUS_NO_DISC;
		realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
		realstatus->track = -1;

		bsd_cdrom_close (lcd);
		return TRUE;
	}
	subchnl.track = 0;
#endif
	realstatus->cd = GNOME_CDROM_STATUS_OK;

	subchnl.address_format = CD_MSF_FORMAT;
	subchnl.data_format = CD_CURRENT_POSITION;
	subchnl.data = &subchnl_info;
	subchnl.data_len = sizeof(struct cd_sub_channel_info);
	if (ioctl (cdrom->fd, CDIOCREADSUBCHANNEL, &subchnl) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(bsd_cdrom_get_status): CDIOCREADSUBCHANNEL ioctl failed %s",
					      strerror (errno));
		}

		bsd_cdrom_close (lcd);
		g_free (realstatus);
		*status = NULL;
		return FALSE;
	}

	/* Get the volume */
	if (ioctl (cdrom->fd, CDIOCGETVOL, &vol) < 0) {
		g_warning ("(bsd_cdrom_get_status): CDIOCGETVOL ioctl failed %s",
			   strerror (errno));
		realstatus->volume = -1; /* -1 means no volume command */
	} else {
		realstatus->volume = vol.vol[0];
	}

	bsd_cdrom_close (lcd);

	ASSIGN_MSF (realstatus->relative, blank_msf);
	ASSIGN_MSF (realstatus->absolute, blank_msf);
	ASSIGN_MSF (realstatus->length, blank_msf);

	realstatus->track = 1;
	switch (subchnl.data->header.audio_status) {
	case CD_AS_PLAY_IN_PROGRESS:
		realstatus->audio = GNOME_CDROM_AUDIO_PLAY;
		ASSIGN_MSF (realstatus->relative, subchnl.data->what.position.reladdr.msf);
		ASSIGN_MSF (realstatus->absolute, subchnl.data->what.position.absaddr.msf);
		realstatus->track = subchnl.data->what.position.track_number;
		if(priv && realstatus->track>0 &&
		   realstatus->track<=priv->number_tracks){
			/* track_info may not be initialized */
			ASSIGN_MSF (realstatus->length, priv->track_info[realstatus->track-1].length);
		}
		break;

	case CD_AS_PLAY_PAUSED:
		realstatus->audio = GNOME_CDROM_AUDIO_PAUSE;
		ASSIGN_MSF (realstatus->relative, subchnl.data->what.position.reladdr.msf);
		ASSIGN_MSF (realstatus->absolute, subchnl.data->what.position.absaddr.msf);
		realstatus->track = subchnl.data->what.position.track_number;
		if(priv && realstatus->track>0 &&
		   realstatus->track<=priv->number_tracks){
			/* track_info may not be initialized */
			ASSIGN_MSF (realstatus->length, priv->track_info[realstatus->track-1].length);
		}
		break;

	case CD_AS_PLAY_COMPLETED:
		realstatus->audio = GNOME_CDROM_AUDIO_COMPLETE;
		ASSIGN_MSF (realstatus->relative, subchnl.data->what.position.reladdr.msf);
		ASSIGN_MSF (realstatus->absolute, subchnl.data->what.position.absaddr.msf);
		realstatus->track = subchnl.data->what.position.track_number;
		if(priv && realstatus->track>0 &&
		   realstatus->track<=priv->number_tracks){
			/* track_info may not be initialized */
			ASSIGN_MSF (realstatus->length, priv->track_info[realstatus->track-1].length);
		}
		break;

	case CD_AS_AUDIO_INVALID:
	case CD_AS_NO_STATUS:
		realstatus->audio = GNOME_CDROM_AUDIO_STOP;
		break;

	case CD_AS_PLAY_ERROR:
	default:
		realstatus->audio = GNOME_CDROM_AUDIO_ERROR;
		break;
	}

	return TRUE;
}

static gboolean
bsd_cdrom_close_tray (GnomeCDRom *cdrom,
			GError **error)
{
	BSDCDRom *lcd;

	lcd = BSD_CDROM (cdrom);
	if (bsd_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (ioctl (cdrom->fd, CDIOCCLOSE) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(bsd_cdrom_close_tray): ioctl failed %s",
					      strerror (errno));
		}

		bsd_cdrom_close (lcd);
		return FALSE;
	}

	bsd_cdrom_close (lcd);
	return TRUE;
}

static gboolean
bsd_cdrom_set_volume (GnomeCDRom *cdrom,
			int volume,
			GError **error)
{
	BSDCDRom *lcd;
	BSDCDRomPrivate *priv;
	struct ioc_vol vol;

	lcd = BSD_CDROM (cdrom);
	priv = lcd->priv;

	if (bsd_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}
	
	vol.vol[0] = (u_char)volume;
	vol.vol[1] = vol.vol[2] = vol.vol[3] = vol.vol[0];
	
	if (ioctl (cdrom->fd, CDIOCSETVOL, &vol) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(bsd_cdrom_set_volume:1): ioctl failed %s",
					      strerror (errno));
		}

		bsd_cdrom_close (lcd);
		return FALSE;
	}

	bsd_cdrom_close (lcd);
	return TRUE;
}

static gboolean
bsd_cdrom_is_cdrom_device (GnomeCDRom *cdrom,
			       const char *device,
			       GError **error)
{
	struct ioc_vol vol;
	int fd;

	if (device == NULL || *device == 0) {
		return FALSE;
	}
	
	fd = open (device, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		return FALSE;
	}

	/* Fire a harmless ioctl at the device. */
	if (ioctl (fd, CDIOCGETVOL, &vol) < 0) {
		/* Failed, it's not a CDROM drive */
		close (fd);
		
		return FALSE;
	}
	
	close (fd);

	return TRUE;
}

static gboolean
bsd_cdrom_get_cddb_data (GnomeCDRom *cdrom,
			   GnomeCDRomCDDBData **data,
			   GError **error)
{
	BSDCDRom *lcd;
	BSDCDRomPrivate *priv;
	int i, t = 0, n = 0;

	lcd = BSD_CDROM (cdrom);
	priv = lcd->priv;

	if (bsd_cdrom_open (lcd, error) == FALSE) {
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

	bsd_cdrom_close (lcd);
	return TRUE;
}

static void
class_init (BSDCDRomClass *klass)
{
	GObjectClass *object_class;
	GnomeCDRomClass *cdrom_class;

	object_class = G_OBJECT_CLASS (klass);
	cdrom_class = GNOME_CDROM_CLASS (klass);

	object_class->finalize = bsd_cdrom_finalize;

	cdrom_class->eject = bsd_cdrom_eject;
	cdrom_class->next = bsd_cdrom_next;
	cdrom_class->ffwd = bsd_cdrom_ffwd;
	cdrom_class->play = bsd_cdrom_play;
	cdrom_class->pause = bsd_cdrom_pause;
	cdrom_class->stop = bsd_cdrom_stop;
	cdrom_class->rewind = bsd_cdrom_rewind;
	cdrom_class->back = bsd_cdrom_back;
	cdrom_class->get_status = bsd_cdrom_get_status;
	cdrom_class->close_tray = bsd_cdrom_close_tray;
	cdrom_class->set_volume = bsd_cdrom_set_volume;
	cdrom_class->is_cdrom_device = bsd_cdrom_is_cdrom_device;
	cdrom_class->update_cd = bsd_cdrom_update_cd;
	
	/* For CDDB */
  	cdrom_class->get_cddb_data = bsd_cdrom_get_cddb_data;
	
	parent_class = g_type_class_peek_parent (klass);
}

static void
init (BSDCDRom *cdrom)
{
	cdrom->priv = g_new0 (BSDCDRomPrivate, 1);
	cdrom->priv->tochdr = g_new (struct ioc_toc_header, 1);
	cdrom->priv->track_info = NULL;
}
	
/* API */
GType
bsd_cdrom_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		GTypeInfo info = {
			sizeof (BSDCDRomClass),
			NULL, NULL, (GClassInitFunc) class_init, NULL, NULL,
			sizeof (BSDCDRom), 0, (GInstanceInitFunc) init,
		};

		type = g_type_register_static (GNOME_CDROM_TYPE, "BSDCDRom", &info, 0);
	}

	return type;
}

GnomeCDRom *
gnome_cdrom_new (const char *cdrom_device,
		 GnomeCDRomUpdate update,
		 GError **error)
{
	return gnome_cdrom_construct (
		g_object_new (bsd_cdrom_get_type (), NULL),
		cdrom_device, update, GNOME_CDROM_DEVICE_STATIC, error);
}
