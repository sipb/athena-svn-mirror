/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * solaris-cdrom.c: Solaris CD controlling functions.
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes  <iain@ximian.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/cdio.h>
#include <sys/audioio.h>	
#include <errno.h>
#include <libdevinfo.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gnome-cd.h"
#include "solaris-cdrom.h"

static GnomeCDRomClass *parent_class = NULL;
gboolean cdrom_drive_present = FALSE;

typedef struct _SolarisCDRomTrackInfo {
	char *name;
	unsigned char track;
	unsigned int audio_track:1;
	GnomeCDRomMSF address;
	GnomeCDRomMSF length;
} SolarisCDRomTrackInfo;

struct _SolarisCDRomPrivate {
	GnomeCDRomUpdate update;

	struct cdrom_tochdr *tochdr;
	int number_tracks;
	unsigned char track0, track1;

	SolarisCDRomTrackInfo *track_info;
};

/* specify which disk links to use in the /dev directory */
#define DEVLINK_REGEX   "rdsk/.*"

/* The list of names of possible cdrom types used by libdevinfo. */
static char *disktypes[] = {
        DDI_NT_CD_CHAN,
        DDI_NT_CD,
        NULL
};

static di_devlink_handle_t      handle;
static char                     prev_name[MAXPATHLEN];

static int              findcdroms(di_node_t node, di_minor_t minor, void *arg);static void             findevs();
static int              find_devpath(di_devlink_t devlink, void *arg);
static int              is_cdrom(di_node_t node, di_minor_t minor);


static gboolean solaris_cdrom_eject (GnomeCDRom *cdrom,
				   GError **error);
static gboolean solaris_cdrom_next (GnomeCDRom *cdrom,
				  GError **error);
static gboolean solaris_cdrom_ffwd (GnomeCDRom *cdrom,
				  GError **error);
static gboolean solaris_cdrom_play (GnomeCDRom *cdrom,
				  int start_track,
				  GnomeCDRomMSF *start,
				  int finish_track,
				  GnomeCDRomMSF *finish,
				  GError **error);
static gboolean solaris_cdrom_pause (GnomeCDRom *cdrom,
				   GError **error);
static gboolean solaris_cdrom_stop (GnomeCDRom *cdrom,
				  GError **error);
static gboolean solaris_cdrom_rewind (GnomeCDRom *cdrom,
				    GError **error);
static gboolean solaris_cdrom_back (GnomeCDRom *cdrom,
				  GError **error);
static gboolean solaris_cdrom_get_status (GnomeCDRom *cdrom,
					GnomeCDRomStatus **status,
					GError **error);
static gboolean solaris_cdrom_close_tray (GnomeCDRom *cdrom,
					GError **error);

static GnomeCDRomMSF blank_msf = { 0, 0, 0};

/* To determine whether the machine has a cdrom drive or not */
void
find_cdrom()
{
        handle = di_devlink_init(NULL, 0);
        findevs();
        di_devlink_fini(&handle);
}

static void
findevs()
{
        di_node_t               di_root;

        di_root = di_init("/", DINFOCPYALL );
        di_walk_minor(di_root, NULL, 0, NULL, findcdroms);
        di_fini(di_root);
}

static int
findcdroms(di_node_t node, di_minor_t minor, void *arg)
{
        if (di_minor_spectype(minor) == S_IFCHR && is_cdrom(node, minor)) {
            char        *devpath;

            devpath = di_devfs_path(node);

            /* avoid doing every minor node for the cdrom */
            if (strcmp(devpath, prev_name) != 0) {
                char    dev_name[MAXPATHLEN];

                strlcpy(prev_name, devpath, sizeof(prev_name));

                (void) snprintf(dev_name, sizeof (dev_name), "%s:%s", devpath,
                        di_minor_name(minor));

                /* Walk the /dev tree to get the devlinks. */
                di_devlink_walk(handle, DEVLINK_REGEX, dev_name,
                    DI_PRIMARY_LINK, NULL, find_devpath);
            }

            di_devfs_path_free((void *) devpath);
        }

        return (DI_WALK_CONTINUE);
}

static int
find_devpath(di_devlink_t devlink, void *arg)
{
        char    *devlink_path;

        devlink_path = (char *)di_devlink_path(devlink);
        if (devlink_path != NULL) {
	    cdrom_drive_present = TRUE;
        }
        return (DI_WALK_CONTINUE);
}

static int
is_cdrom(di_node_t node, di_minor_t minor)
{
        char    *type;
        int     type_index;

        if ((type = di_minor_nodetype(minor)) == NULL) {
            return (0);
        }

        for (type_index = 0; disktypes[type_index]; type_index++) {
            if (strcmp(type, disktypes[type_index]) == 0) {
                return (1);
            }
        }

        return (0);
}




static void
solaris_cdrom_finalize (GObject *object)
{
	SolarisCDRom *cdrom = (SolarisCDRom *) object;

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
solaris_cdrom_open (SolarisCDRom *lcd,
		    GError      **error)
{
	return gnome_cdrom_open_dev (GNOME_CDROM (lcd), error);
}

static void
solaris_cdrom_close (SolarisCDRom *lcd)
{
	gnome_cdrom_close_dev (GNOME_CDROM (lcd), FALSE);
}

static void
solaris_cdrom_invalidate (SolarisCDRom *lcd)
{
	if (lcd->priv->track_info == NULL) {
		g_free (lcd->priv->track_info);
		lcd->priv->track_info = NULL;
	}
}

static void
calculate_track_lengths (SolarisCDRom *lcd)
{
	SolarisCDRomPrivate *priv;
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
solaris_cdrom_update_cd (GnomeCDRom *cdrom)
{
	SolarisCDRom *lcd = SOLARIS_CDROM (cdrom);
	SolarisCDRomPrivate *priv;
	struct cdrom_tocentry tocentry;
	int i, j;
	GError *error;

	priv = lcd->priv;

	if (solaris_cdrom_open (lcd, &error) == FALSE) {
		g_warning ("Error opening CD");
		return;
	}

	if (ioctl (cdrom->fd, CDROMREADTOCHDR, priv->tochdr) < 0) {
		g_warning ("Error reading CD header");
		solaris_cdrom_close (lcd);

		return;
	}
	
	priv->track0 = priv->tochdr->cdth_trk0;
	priv->track1 = priv->tochdr->cdth_trk1;
	priv->number_tracks = priv->track1 - priv->track0 + 1;

	solaris_cdrom_invalidate (lcd);
	priv->track_info = g_malloc ((priv->number_tracks + 1) * sizeof (SolarisCDRomTrackInfo));
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
		solaris_cdrom_invalidate (lcd);
		return;
	}
	ASSIGN_MSF (priv->track_info[priv->number_tracks].address, tocentry.cdte_addr.msf);
	calculate_track_lengths (lcd);

	solaris_cdrom_close (lcd);
	return;
}

static gboolean
solaris_cdrom_eject (GnomeCDRom *cdrom,
		   GError **error)
{
	SolarisCDRom *lcd;
	GnomeCDRomStatus *status;

	lcd = SOLARIS_CDROM (cdrom);

	if (solaris_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		solaris_cdrom_close (lcd);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_TRAY_OPEN) {
		if (ioctl (cdrom->fd, CDROMEJECT, 0) < 0) {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_SYSTEM_ERROR,
						      "(eject): ioctl failed: %s",
						      strerror (errno));
			}

			g_free (status);
			solaris_cdrom_close (lcd);
			return FALSE;
		}
	} else {
		/* Try to close the tray if it's open */
		if (gnome_cdrom_close_tray (cdrom, error) == FALSE) {
			
			g_free (status);
			solaris_cdrom_close (lcd);

			return FALSE;
		}
	}

	g_free (status);

	gnome_cdrom_close_dev (cdrom, TRUE);

	return TRUE;
}

static gboolean
solaris_cdrom_next (GnomeCDRom *cdrom,
		  GError **error)
{
	SolarisCDRom *lcd;
	GnomeCDRomStatus *status;
	GnomeCDRomMSF msf, *endmsf;
	int track, end_track;

	lcd = SOLARIS_CDROM (cdrom);
	if (solaris_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (solaris_cdrom_get_status (cdrom, &status, error) == FALSE) {
		solaris_cdrom_close (lcd);
		return FALSE;
	}

	track = status->track + 1;
	g_free (status);
	if (track > lcd->priv->number_tracks) {
		/* Do nothing */
		solaris_cdrom_close (lcd);
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
	
	if (solaris_cdrom_play (cdrom, track, &msf, end_track, endmsf, error) == FALSE) {
		solaris_cdrom_close (lcd);
		return FALSE;
	}

	solaris_cdrom_close (lcd);
	return TRUE;
}

static gboolean
solaris_cdrom_ffwd (GnomeCDRom *cdrom,
		  GError **error)
{
	SolarisCDRom *lcd;
	GnomeCDRomStatus *status;
	GnomeCDRomMSF *msf, *endmsf, end;
	int discend, frames, end_track;
	
	lcd = SOLARIS_CDROM (cdrom);
	if (solaris_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (solaris_cdrom_get_status (cdrom, &status, error) == FALSE) {
		solaris_cdrom_close (lcd);
		return FALSE;
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
		solaris_cdrom_close (lcd);
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
	
	if (solaris_cdrom_play (cdrom, -1, msf, end_track, endmsf, error) == FALSE) {
		g_free (status);
		solaris_cdrom_close (lcd);
		return FALSE;
	}

	g_free (status);
	solaris_cdrom_close (lcd);

	return TRUE;
}

static gboolean
solaris_cdrom_play (GnomeCDRom *cdrom,
		  int start_track,
		  GnomeCDRomMSF *start,
		  int finish_track,
		  GnomeCDRomMSF *finish,
		  GError **error)
{
	SolarisCDRom *lcd;
	SolarisCDRomPrivate *priv;
	GnomeCDRomStatus *status;
	struct cdrom_msf msf;
	int minutes, seconds, frames;

	lcd = SOLARIS_CDROM (cdrom);
	priv = lcd->priv;
	if (solaris_cdrom_open(lcd, error) == FALSE) {
		return FALSE;
	}

	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		solaris_cdrom_close (lcd);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		if (status->cd == GNOME_CDROM_STATUS_TRAY_OPEN) {
			if (solaris_cdrom_close_tray (cdrom, error) == FALSE) {
				solaris_cdrom_close (lcd);
				g_free (status);
				return FALSE;
			}
		} else {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_NOT_READY,
						      "(solaris_cdrom_play): Drive not ready");
			}

			solaris_cdrom_close (lcd);
			g_free (status);
			return FALSE;
		}
	}

	g_free (status);
	/* Get the status again: It might have changed */
	if (gnome_cdrom_get_status (GNOME_CDROM (lcd), &status, error) == FALSE) {
		solaris_cdrom_close (lcd);
		return FALSE;
	}
	if (status->cd != GNOME_CDROM_STATUS_OK) {
		/* Stuff if :) */
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_NOT_READY,
					      "(solaris_cdrom_play): Drive still not ready");
		}

		solaris_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

	switch (status->audio) {
	case GNOME_CDROM_AUDIO_PAUSE:
		if (gnome_cdrom_pause (GNOME_CDROM (lcd), error) == FALSE) {
			g_free (status);
			solaris_cdrom_close (lcd);
			return FALSE;
		}

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
			if (start_track >= 0) {
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
			msf.cdmsf_min1 = priv->track_info[priv->number_tracks].address.minute;
			msf.cdmsf_sec1 = priv->track_info[priv->number_tracks].address.second;
			msf.cdmsf_frame1 = priv->track_info[priv->number_tracks].address.frame;
		} else {
			if (finish_track >= 0) {
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
						      "(solaris_cdrom_play) ioctl failed %s",
						      strerror (errno));
			}

			solaris_cdrom_close (lcd);
			g_free (status);
			return FALSE;
		}
	}

	solaris_cdrom_close (lcd);
	g_free (status);
	return TRUE;
}

static gboolean
solaris_cdrom_pause (GnomeCDRom *cdrom,
		   GError **error)
{
	SolarisCDRom *lcd;
	GnomeCDRomStatus *status;

	lcd = SOLARIS_CDROM (cdrom);
	if (solaris_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		solaris_cdrom_close (lcd);
		return FALSE;
	}

	if (status->cd != GNOME_CDROM_STATUS_OK) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_NOT_READY,
					      "(solaris_cdrom_pause): Drive not ready");
		}

		g_free (status);
		solaris_cdrom_close (lcd);
		return FALSE;
	}

	if (status->audio == GNOME_CDROM_AUDIO_PAUSE) {
		if (ioctl (cdrom->fd, CDROMRESUME) < 0) {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_SYSTEM_ERROR,
						      "(solaris_cdrom_pause): Resume failed %s",
						      strerror (errno));
			}

			g_free (status);
			solaris_cdrom_close (lcd);
			return FALSE;
		}

		solaris_cdrom_close (lcd);
		g_free (status);
		return TRUE;
	}

	if (status->audio == GNOME_CDROM_AUDIO_PLAY) {
		if (ioctl (cdrom->fd, CDROMPAUSE, 0) < 0) {
			if (error) {
				*error = g_error_new (GNOME_CDROM_ERROR,
						      GNOME_CDROM_ERROR_SYSTEM_ERROR,
						      "(solaris_cdrom_pause): ioctl failed %s",
						      strerror (errno));
			}

			g_free (status);
			solaris_cdrom_close (lcd);
			return FALSE;
		}
	}

	g_free (status);
	solaris_cdrom_close (lcd);
	return TRUE;
}

static gboolean
solaris_cdrom_stop (GnomeCDRom *cdrom,
		  GError **error)
{
	SolarisCDRom *lcd;
	GnomeCDRomStatus *status;

	lcd = SOLARIS_CDROM (cdrom);
	if (solaris_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (gnome_cdrom_get_status (cdrom, &status, error) == FALSE) {
		solaris_cdrom_close (lcd);
		return FALSE;
	}

#if 0
	if (status->audio == GNOME_CDROM_AUDIO_PAUSE) {
		if (solaris_cdrom_pause (cdrom, error) == FALSE) {
			solaris_cdrom_close (lcd);
			g_free (status);
			return FALSE;
		}
	}
#endif

	if (ioctl (cdrom->fd, CDROMSTOP, 0) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(solaris_cdrom_stop) ioctl failed %s",
					      strerror (errno));
		}

		solaris_cdrom_close (lcd);
		g_free (status);
		return FALSE;
	}

	solaris_cdrom_close (lcd);
	g_free (status);
	return TRUE;
}

static gboolean
solaris_cdrom_rewind (GnomeCDRom *cdrom,
		    GError **error)
{
	SolarisCDRom *lcd;
	GnomeCDRomMSF *msf, tmpmsf, end, *endmsf;
	GnomeCDRomStatus *status;
	int discstart, frames, end_track;

	lcd = SOLARIS_CDROM (cdrom);
	if (solaris_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (solaris_cdrom_get_status (cdrom, &status, error) == FALSE) {
		solaris_cdrom_close (lcd);
		return FALSE;
	}

	msf = &status->absolute;

	frames = msf_to_frames (msf);
	frames -= CD_FRAMES; /* Back one second */

	/* Check we've not run back past the start */
	discstart = msf_to_frames (&lcd->priv->track_info[0].address);
	if (frames < discstart) {
		g_free (status);
		solaris_cdrom_close (lcd);
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
	
	if (solaris_cdrom_play (cdrom, -1, &tmpmsf, end_track, endmsf, error) == FALSE) {
		g_free (status);
		
		solaris_cdrom_close (lcd);
		return FALSE;
	}

	solaris_cdrom_close (lcd);
	g_free (status);

	return TRUE;
}

static gboolean
solaris_cdrom_back (GnomeCDRom *cdrom,
		  GError **error)
{
	SolarisCDRom *lcd;
	GnomeCDRomStatus *status;
	GnomeCDRomMSF msf, *endmsf;
	int track, end_track;

	lcd = SOLARIS_CDROM (cdrom);
	if (solaris_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	if (solaris_cdrom_get_status (cdrom, &status, error) == FALSE) {
		solaris_cdrom_close (lcd);
		return FALSE;
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
		solaris_cdrom_close (lcd);
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
	if (solaris_cdrom_play (cdrom, track, &msf, end_track, endmsf, error) == FALSE) {
		g_free (status);
		solaris_cdrom_close (lcd);
		return FALSE;
	}

	g_free (status);
	solaris_cdrom_close (lcd);
	return TRUE;
}

/* There should probably be 2 get_status functions. A public one and the private one.
   The private one would get called by the update handler every second, and the
   public one would just return a copy of the status */
static gboolean
solaris_cdrom_get_status (GnomeCDRom *cdrom,
			GnomeCDRomStatus **status,
			GError **error)
{
	SolarisCDRom *lcd;
	SolarisCDRomPrivate *priv;
	GnomeCDRomStatus *realstatus;
	struct cdrom_subchnl subchnl;
	struct cdrom_tocentry tocentry;
	struct audio_info audioinfo;
	int vol_fd;
	int cd_status;

	g_return_val_if_fail (status != NULL, TRUE);
	
	lcd = SOLARIS_CDROM (cdrom);
	priv = lcd->priv;

	*status = g_new0 (GnomeCDRomStatus, 1);
	realstatus = *status;
	realstatus->volume = 0;

	if (solaris_cdrom_open (lcd, error) == FALSE) {
		static gboolean function_called = FALSE;
		g_free (realstatus);
		solaris_cdrom_close (lcd);
		if (!function_called)
		find_cdrom (); 
		function_called = TRUE;
                if (cdrom_drive_present) 
                        realstatus->cd = GNOME_CDROM_STATUS_NO_DISC;
                else
                        realstatus->cd = GNOME_CDROM_STATUS_NO_CDROM;
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

			solaris_cdrom_close (lcd);
			return TRUE;

		case CDS_NO_DISC:
			realstatus->cd = GNOME_CDROM_STATUS_NO_DISC;
			realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
			realstatus->track = -1;

			solaris_cdrom_close (lcd);
			return TRUE;
			
		case CDS_TRAY_OPEN:
			realstatus->cd = GNOME_CDROM_STATUS_TRAY_OPEN;
			realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
			realstatus->track = -1;

			solaris_cdrom_close (lcd);
			return TRUE;

		case CDS_DRIVE_NOT_READY:
			realstatus->cd = GNOME_CDROM_STATUS_DRIVE_NOT_READY;
			realstatus->audio = GNOME_CDROM_AUDIO_NOTHING;
			realstatus->track = -1;
			
			solaris_cdrom_close (lcd);
			return TRUE;

		default:
			realstatus->cd = GNOME_CDROM_STATUS_OK;
			break;
		}
	} else {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(solaris_cdrom_get_status): ioctl error %s",
					      strerror (errno));
		}

		solaris_cdrom_close (lcd);
		g_free (realstatus);
		return FALSE;
	}
#else
	tocentry.cdte_track = CDROM_LEADOUT;
	tocentry.cdte_format = CDROM_MSF;
	if (ioctl (cdrom->fd, CDROMREADTOCENTRY, &tocentry) < 0) {
		g_warning ("Error getting leadout");
		solaris_cdrom_invalidate (lcd);
		return FALSE;
	}
	if (tocentry.cdte_ctrl == CDROM_DATA_TRACK)
		realstatus->cd = GNOME_CDROM_STATUS_DATA_CD;
	else
		realstatus->cd = GNOME_CDROM_STATUS_OK;
#endif

	subchnl.cdsc_format = CDROM_MSF;
	if (ioctl (cdrom->fd, CDROMSUBCHNL, &subchnl) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(solaris_cdrom_get_status): CDROMSUBCHNL ioctl failed %s",
					      strerror (errno));
		}

		solaris_cdrom_close (lcd);
		g_free (realstatus);
		return FALSE;
	}
	/* get initial volume */
	vol_fd = open ( "/dev/audioctl", O_RDWR);
	if (ioctl (vol_fd, AUDIO_GETINFO, &audioinfo) < 0) {
			g_warning ("(solaris_cdrom_get_status): AUDIO_GETINFO ioctl failed %s",
				   strerror (errno));
			realstatus->volume = -1;
	} else {
		realstatus->volume = audioinfo.play.gain;
	}

	close (vol_fd);

	solaris_cdrom_close (lcd);

	ASSIGN_MSF (realstatus->relative, blank_msf);
	ASSIGN_MSF (realstatus->absolute, blank_msf);
	realstatus->track = 1;
	switch (subchnl.cdsc_audiostatus) {
	case CDROM_AUDIO_PLAY:
		realstatus->audio = GNOME_CDROM_AUDIO_PLAY;
		ASSIGN_MSF (realstatus->relative, subchnl.cdsc_reladdr.msf);
		ASSIGN_MSF (realstatus->absolute, subchnl.cdsc_absaddr.msf);
		realstatus->track = subchnl.cdsc_trk;

		break;

	case CDROM_AUDIO_PAUSED:
		realstatus->audio = GNOME_CDROM_AUDIO_PAUSE;
		ASSIGN_MSF (realstatus->relative, subchnl.cdsc_reladdr.msf);
		ASSIGN_MSF (realstatus->absolute, subchnl.cdsc_absaddr.msf);
		realstatus->track = subchnl.cdsc_trk;

		break;

	case CDROM_AUDIO_COMPLETED:
		realstatus->audio = GNOME_CDROM_AUDIO_COMPLETE;
		ASSIGN_MSF (realstatus->relative, subchnl.cdsc_reladdr.msf);
		ASSIGN_MSF (realstatus->absolute, subchnl.cdsc_absaddr.msf);
		realstatus->track = subchnl.cdsc_trk;		
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
solaris_cdrom_close_tray (GnomeCDRom *cdrom,
			GError **error)
{
	SolarisCDRom *lcd;

	lcd = SOLARIS_CDROM (cdrom);
	if (solaris_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

#if 0
	if (ioctl (cdrom->fd, CDROMCLOSETRAY) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
					      "(solaris_cdrom_close_tray): ioctl failed %s",
					      strerror (errno));
		}

		solaris_cdrom_close (lcd);
		return FALSE;
	}
#endif

	solaris_cdrom_close (lcd);
	return TRUE;
}

static gboolean
solaris_cdrom_set_volume (GnomeCDRom *cdrom,
			  int volume,
                          GError **error)
{
	SolarisCDRom *lcd;
	SolarisCDRomPrivate *priv;
	struct audio_info audioinfo;
	struct cdrom_volctrl vol;
	int vol_fd;

	lcd = SOLARIS_CDROM (cdrom);
	priv = lcd->priv;
	AUDIO_INITINFO (&audioinfo)

	audioinfo.play.gain = volume;

	vol_fd = open ("/dev/audioctl", O_RDWR);
	if (vol_fd < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
                                              GNOME_CDROM_ERROR_SYSTEM_ERROR,
                                              "(solaris_cdrom_set_volume:1): ioctl failed %s",
                                              strerror (errno));
		}
		close (vol_fd);
		return FALSE;
	}
	
	if (ioctl (vol_fd, AUDIO_SETINFO, &audioinfo) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
                                              GNOME_CDROM_ERROR_SYSTEM_ERROR,
                                              "(solaris_cdrom_set_volume:1): ioctl failed %s",
					      strerror (errno));
		}

		close (vol_fd);
		return FALSE;
	}

	close (vol_fd);

	if (solaris_cdrom_open (lcd, error) == FALSE) {
		return FALSE;
	}

	vol.channel0 = volume;
	vol.channel1 = vol.channel2 = vol.channel3 = volume;

	if (ioctl (cdrom->fd, CDROMVOLCTRL, &vol) < 0) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_SYSTEM_ERROR,
              				      "(solaris_cdrom_set_volume:1): ioctl failed %s",
					      strerror (errno));
                }

		solaris_cdrom_close (lcd);
		return FALSE;
	}

	solaris_cdrom_close (lcd);
	return TRUE;
}

static gboolean
solaris_cdrom_is_cdrom_device (GnomeCDRom *cdrom,
			       const char *device,
			       GError **error)
{
	int fd;
	int vol_fd;
	int volume;
	struct audio_info audioinfo;
	struct cdrom_volctrl vol;

	vol_fd = open ( "/dev/audioctl", O_RDWR);
	if (ioctl (vol_fd, AUDIO_GETINFO, &audioinfo) < 0) {
			g_warning ("(solaris_cdrom_is_cdrom_device): AUDIO_GETINFO ioctl failed %s",
                                   strerror (errno));
	} else {
                volume = audioinfo.play.gain;
	}
	close (vol_fd);

	if (device == NULL || *device == 0) {
		return FALSE;
	}
	
	fd = open (device, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		return FALSE;
	}

	vol.channel0 = volume;
	vol.channel1 = vol.channel2 = vol.channel3 = volume;
	/* Fire a harmless ioctl at the device. */
	if (ioctl (fd, CDROMVOLCTRL, &vol) < 0) {
		/* Failed, it's not a CDROM drive */
		close (fd);
		
		return FALSE;
	}
	
	close (fd);

	return TRUE;
}

static gboolean
solaris_cdrom_get_cddb_data (GnomeCDRom *cdrom,
			   GnomeCDRomCDDBData **data,
			   GError **error)
{
	SolarisCDRom *lcd;
	SolarisCDRomPrivate *priv;
	int i, t = 0, n = 0;

	lcd = SOLARIS_CDROM (cdrom);
	priv = lcd->priv;

	if (solaris_cdrom_open (lcd, error) == FALSE) {
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
		g_print ("%d: %u\n", i, msf_to_frames (&priv->track_info[i].address));
	}

	solaris_cdrom_close (lcd);
	return TRUE;
}

static void
solaris_cdrom_class_init (SolarisCDRomClass *klass)
{
	GObjectClass *object_class;
	GnomeCDRomClass *cdrom_class;

	object_class = G_OBJECT_CLASS (klass);
	cdrom_class = GNOME_CDROM_CLASS (klass);

	object_class->finalize = solaris_cdrom_finalize;

	cdrom_class->eject = solaris_cdrom_eject;
	cdrom_class->next = solaris_cdrom_next;
	cdrom_class->ffwd = solaris_cdrom_ffwd;
	cdrom_class->play = solaris_cdrom_play;
	cdrom_class->pause = solaris_cdrom_pause;
	cdrom_class->stop = solaris_cdrom_stop;
	cdrom_class->rewind = solaris_cdrom_rewind;
	cdrom_class->back = solaris_cdrom_back;
	cdrom_class->get_status = solaris_cdrom_get_status;
	cdrom_class->close_tray = solaris_cdrom_close_tray;
	cdrom_class->set_volume = solaris_cdrom_set_volume;
	cdrom_class->is_cdrom_device = solaris_cdrom_is_cdrom_device;
	cdrom_class->update_cd = solaris_cdrom_update_cd;
	
	/* For CDDB */
  	cdrom_class->get_cddb_data = solaris_cdrom_get_cddb_data;

	parent_class = g_type_class_peek_parent (klass);
}

static void
solaris_cdrom_init (SolarisCDRom *cdrom)
{
	cdrom->priv = g_new0 (SolarisCDRomPrivate, 1);
	cdrom->priv->tochdr = g_new0 (struct cdrom_tochdr, 1);
	cdrom->priv->track_info = NULL;
}

/* API */
GType
solaris_cdrom_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		GTypeInfo info = {
			sizeof (SolarisCDRomClass),
			NULL, NULL, (GClassInitFunc) solaris_cdrom_class_init, NULL, NULL,
			sizeof (SolarisCDRom), 0, (GInstanceInitFunc) solaris_cdrom_init,
		};

		type = g_type_register_static (GNOME_CDROM_TYPE, "SolarisCDRom", &info, 0);
	}

	return type;
}

GnomeCDRom *
gnome_cdrom_new (const char      *cdrom_device,
		 GnomeCDRomUpdate update,
		 GError         **error)
{
	return gnome_cdrom_construct (
		g_object_new (solaris_cdrom_get_type (), NULL),
		cdrom_device, update, GNOME_CDROM_DEVICE_TRANSIENT, error);
}
