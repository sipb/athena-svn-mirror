/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * cdrom.c
 *
 * Copyright (C) 2001 Iain Holmes
 * Authors: Iain Holmes <iain@ximian.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glib.h>
#include <glib-object.h>
#include <libgnome/gnome-i18n.h>

#include "cdrom.h"
#include "gnome-cd.h"

/* how frequently we poke at the cdrom (ms) */
#define CDROM_POLL_TIMEOUT 1000

static GObjectClass *parent_class = NULL;

enum {
	STATUS_CHANGED,
	LAST_SIGNAL
};

static gulong gnome_cdrom_signals[LAST_SIGNAL] = { 0, };

struct _GnomeCDRomPrivate {
	char             *device;
	int               fd;
	int               open_count;
	guint             update_id;
	GnomeCDRomUpdate  update;
	GnomeCDRomStatus *recent_status;
};

GQuark
gnome_cdrom_error_quark (void)
{
	static GQuark quark = 0;
	if (quark == 0) {
		quark = g_quark_from_static_string ("gnome-cdrom-error-quark");
	}

	return quark;
}

static void
cdrom_finalize (GObject *object)
{
	GnomeCDRom *cdrom = (GnomeCDRom *) object;

	g_free (cdrom->priv->recent_status);
	g_free (cdrom->priv->device);
	g_free (cdrom->priv);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
cdrom_dispose (GObject *object)
{
	GnomeCDRom *cdrom = (GnomeCDRom *) object;

	if (cdrom->priv->update_id > 0) {
		g_source_remove (cdrom->priv->update_id);
		cdrom->priv->update_id = -1;
	}

	gnome_cdrom_close_dev (cdrom, TRUE);

	G_OBJECT_CLASS (parent_class)->dispose (object);
}

static gboolean
cdrom_eject (GnomeCDRom *cdrom,
	     GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION, 
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;
}

static gboolean
cdrom_next (GnomeCDRom *cdrom,
	    GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION,
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;

}

static gboolean
cdrom_ffwd (GnomeCDRom *cdrom,
	    GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION,
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;
}

static gboolean
cdrom_play (GnomeCDRom *cdrom,
	    int start_track,
	    GnomeCDRomMSF *start,
	    int finish_track,
	    GnomeCDRomMSF *finish,
	    GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION,
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;
}

static gboolean
cdrom_pause (GnomeCDRom *cdrom,
	     GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION,
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;
}

static gboolean
cdrom_stop (GnomeCDRom *cdrom,
	    GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION, 
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;
}

static gboolean
cdrom_rewind (GnomeCDRom *cdrom,
	      GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION,
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;
}

static gboolean
cdrom_back (GnomeCDRom *cdrom,
	    GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION,
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;
}

static gboolean
cdrom_get_status (GnomeCDRom *cdrom,
		  GnomeCDRomStatus **status,
		  GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION,
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	*status = NULL;
	return FALSE;
}

static gboolean
cdrom_close_tray (GnomeCDRom *cdrom,
		  GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION,
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;
}

static gboolean
cdrom_get_cddb_data (GnomeCDRom *cdrom,
		     GnomeCDRomCDDBData **data,
		     GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION,
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;
}

static gboolean
cdrom_set_volume (GnomeCDRom *cdrom,
		  int volume,
		  GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION,
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;
}

static gboolean
cdrom_is_cdrom_device (GnomeCDRom *cdrom,
		       const char *device,
		       GError **error)
{
	if (error) {
		*error = g_error_new (GNOME_CDROM_ERROR,
				      GNOME_CDROM_ERROR_NOT_IMPLEMENTED,
				      "%s has not been implemented in %s",
				      G_GNUC_FUNCTION,
				      G_OBJECT_TYPE_NAME (cdrom));
	}

	return FALSE;
}

static gboolean
cdrom_set_device (GnomeCDRom *cdrom,
		  const char *device,
		  GError    **error)
{
	GnomeCDRomStatus *status;
	GnomeCDRomPrivate *priv;

	g_return_val_if_fail (cdrom != NULL, FALSE);

	priv = cdrom->priv;

	if (priv->device && !strcmp (priv->device, device))
		return TRUE;

	gnome_cdrom_close_dev (cdrom, TRUE);
	
	g_free (priv->device);
	priv->device = g_strdup (device);
	if (gnome_cdrom_get_status (cdrom, &status, NULL) == TRUE) {
		if (status->audio == GNOME_CDROM_AUDIO_PLAY) {
			if (gnome_cdrom_stop (cdrom, error) == FALSE) {
				g_free (status);
				return FALSE;
			}
		}

		g_free (status);
	}


	switch (cdrom->lifetime) {
		case  GNOME_CDROM_DEVICE_STATIC :
		case  GNOME_CDROM_DEVICE_TRANSIENT :
			if (!gnome_cdrom_open_dev (cdrom, error))
				return FALSE;
			gnome_cdrom_close_dev (cdrom, FALSE);
			break;
		default :
			break;
	}

	if (gnome_cdrom_is_cdrom_device (GNOME_CDROM (cdrom),
					 cdrom->priv->device,
					 error) == FALSE) {
		if (error) {
			*error = g_error_new (GNOME_CDROM_ERROR,
					      GNOME_CDROM_ERROR_NOT_OPENED,
					      "%s does not point to a valid CDRom device. This may be caused by:\n"
					      "a) CD support is not present in your machine\n"
					      "b) You do not have the correct permissions to access the CD drive\n"
					      "c) %s is not the CD drive.\n",					     
					      cdrom->priv->device, cdrom->priv->device);
		}
			return FALSE;
	}

	gnome_cdrom_force_status_rescan (cdrom);

	return TRUE;	
}

static gboolean
cdrom_open_dev (GnomeCDRom *cdrom,
		GError    **error)
{
	if (cdrom->priv->open_count++ == 0) {
		cdrom->fd = open (
			cdrom->priv->device, O_RDONLY | O_NONBLOCK);

		if (cdrom->fd < 0) {
			if (errno == EACCES && error != NULL) {
				*error = g_error_new (
					GNOME_CDROM_ERROR,
					GNOME_CDROM_ERROR_NOT_OPENED,
					_("You do not seem to have permission to access %s."),
					cdrom->priv->device);
			} else if (error != NULL) {
				*error = g_error_new (
					GNOME_CDROM_ERROR,
					GNOME_CDROM_ERROR_NOT_OPENED,
					_("%s does not appear to point to a valid CD device. This may be because:\n"
					  "a) CD support is not present in your machine\n"
					  "b) You do not have the correct permissions to access the CD drive\n"
					  "c) %s is not the CD drive.\n"),					     
					cdrom->priv->device,
					cdrom->priv->device);
			}
				
			cdrom->priv->open_count = 0;
			return FALSE;
		}
	}
	
	return (cdrom->fd >= 0);
}

static void
cdrom_close_dev (GnomeCDRom *cdrom, gboolean force_close)
{
	if (--cdrom->priv->open_count < 0 || force_close)
		cdrom->priv->open_count = 0;

	if (cdrom->priv->open_count == 0) {
		if (cdrom->fd >= 0)
			close (cdrom->fd);
		cdrom->fd = -1;
	}
}

static void
cdrom_update_cd (GnomeCDRom *cdrom)

{
	g_error ("cdrom_update_cd not implmented");
}

static void
cdrom_class_init (GnomeCDRomClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
	object_class->dispose  = cdrom_dispose;
	object_class->finalize = cdrom_finalize;

	klass->eject = cdrom_eject;
	klass->next = cdrom_next;
	klass->ffwd = cdrom_ffwd;
	klass->play = cdrom_play;
	klass->pause = cdrom_pause;
	klass->stop = cdrom_stop;
	klass->rewind = cdrom_rewind;
	klass->back = cdrom_back;
	klass->get_status = cdrom_get_status;
	klass->close_tray = cdrom_close_tray;
	klass->set_volume = cdrom_set_volume;
	klass->set_device = cdrom_set_device;
	klass->is_cdrom_device = cdrom_is_cdrom_device;

	klass->open_dev  = cdrom_open_dev;
	klass->close_dev = cdrom_close_dev;
	klass->update_cd = cdrom_update_cd;
	
	/* CDDB stuff */
	klass->get_cddb_data = cdrom_get_cddb_data;

	/* Signals */
	gnome_cdrom_signals[STATUS_CHANGED] = g_signal_new ("status-changed",
							    G_TYPE_FROM_CLASS (klass),
							    G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
							    G_STRUCT_OFFSET (GnomeCDRomClass, status_changed),
							    NULL, NULL,
							    g_cclosure_marshal_VOID__POINTER,
							    G_TYPE_NONE,
							    1, G_TYPE_POINTER);
	parent_class = g_type_class_peek_parent (klass);
}

static void
cdrom_init (GnomeCDRom *cdrom)
{
	cdrom->priv = g_new0 (GnomeCDRomPrivate, 1);
	cdrom->playmode = GNOME_CDROM_WHOLE_CD;
	cdrom->loopmode = GNOME_CDROM_PLAY_ONCE;
	cdrom->priv->device = g_strdup (default_cd_device);
}

/* API */
GType
gnome_cdrom_get_type (void)
{
	static GType type = 0;

	if (type == 0) {
		GTypeInfo info = {
			sizeof (GnomeCDRomClass),
			NULL, NULL, (GClassInitFunc) cdrom_class_init, NULL, NULL,
			sizeof (GnomeCDRom), 0, (GInstanceInitFunc) cdrom_init,
		};

		type = g_type_register_static (G_TYPE_OBJECT, "GnomeCDRom", &info, 0);
	}

	return type;
}

gboolean
gnome_cdrom_eject (GnomeCDRom *cdrom,
		   GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->eject (cdrom, error);
}

gboolean
gnome_cdrom_next (GnomeCDRom *cdrom,
		  GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->next (cdrom, error);
}

gboolean
gnome_cdrom_fast_forward (GnomeCDRom *cdrom,
			  GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->ffwd (cdrom, error);
}

gboolean
gnome_cdrom_play (GnomeCDRom *cdrom,
		  int start_track,
		  GnomeCDRomMSF *start,
		  int finish_track,
		  GnomeCDRomMSF *finish,
		  GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->play (cdrom, start_track, start, finish_track, finish, error);
}

gboolean
gnome_cdrom_pause (GnomeCDRom *cdrom,
		   GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->pause (cdrom, error);
}

gboolean
gnome_cdrom_stop (GnomeCDRom *cdrom,
		  GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->stop (cdrom, error);
}

gboolean
gnome_cdrom_rewind (GnomeCDRom *cdrom,
		    GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->rewind (cdrom, error);
}

gboolean
gnome_cdrom_back (GnomeCDRom *cdrom,
		  GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->back (cdrom, error);
}

gboolean
gnome_cdrom_get_status (GnomeCDRom *cdrom,
			GnomeCDRomStatus **status,
			GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->get_status (cdrom, status, error);
}

gboolean
gnome_cdrom_close_tray (GnomeCDRom *cdrom,
			GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->close_tray (cdrom, error);
}

gboolean
gnome_cdrom_get_cddb_data (GnomeCDRom *cdrom,
			   GnomeCDRomCDDBData **data,
			   GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->get_cddb_data (cdrom, data, error);
}

void
gnome_cdrom_free_cddb_data (GnomeCDRomCDDBData *data)
{
	g_free (data->offsets);
	g_free (data);
}

void
gnome_cdrom_status_changed (GnomeCDRom *cdrom,
			    GnomeCDRomStatus *new_status)
{
	GnomeCDRomStatus *status_copy;

	status_copy = gnome_cdrom_copy_status (new_status);
	
	g_signal_emit (G_OBJECT (cdrom),
		       gnome_cdrom_signals[STATUS_CHANGED],
		       0, status_copy);

	g_free (status_copy);
}

static gboolean
msf_equals (GnomeCDRomMSF *msf1,
	    GnomeCDRomMSF *msf2)
{
	if (msf1 == msf2) {
		return TRUE;
	}

	if (msf1->minute == msf2->minute &&
	    msf1->second == msf2->second &&
	    msf1->frame == msf2->frame) {
		return TRUE;
	}

	return FALSE;
}

gboolean
gnome_cdrom_status_equal (GnomeCDRomStatus *status1,
			  GnomeCDRomStatus *status2)
{
	if (status1 == status2) {
		return TRUE;
	}

	if (status1->cd != status2->cd) {
		return FALSE;
	}

	if (status1->audio != status2->audio) {
		return FALSE;
	}

	if (status1->track != status2->track) {
		return FALSE;
	}

	if (msf_equals (&status1->relative, &status2->relative) &&
	    msf_equals (&status1->absolute, &status2->absolute)) {
		return TRUE;
	}

	return FALSE;
}

GnomeCDRomStatus *
gnome_cdrom_copy_status (GnomeCDRomStatus *original)
{
	GnomeCDRomStatus *status;

	status = g_new (GnomeCDRomStatus, 1);
	memcpy (status, original, sizeof (GnomeCDRomStatus));

	return status;
}

gboolean
gnome_cdrom_set_device (GnomeCDRom *cdrom,
			const char *device,
			GError **error)
{
	GnomeCDRomClass *klass;
	
	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->set_device (cdrom, device, error);
}

gboolean
gnome_cdrom_set_volume (GnomeCDRom *cdrom,
			int volume,
			GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->set_volume (cdrom, volume, error);
}

gboolean
gnome_cdrom_is_cdrom_device (GnomeCDRom *cdrom,
			     const char *device,
			     GError **error)
{
	GnomeCDRomClass *klass;

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	return klass->is_cdrom_device (cdrom, device, error);
}

gboolean
gnome_cdrom_open_dev (GnomeCDRom *cdrom,
		      GError    **error)
{
	GnomeCDRomClass *klass;
	
	klass = GNOME_CDROM_GET_CLASS (cdrom);

	g_return_val_if_fail (GNOME_IS_CDROM (cdrom), FALSE);
	
	return klass->open_dev (cdrom, error);
}

void
gnome_cdrom_close_dev (GnomeCDRom *cdrom, gboolean force)
{
	GnomeCDRomClass *klass;
	
	g_return_if_fail (GNOME_IS_CDROM (cdrom));

	klass = GNOME_CDROM_GET_CLASS (cdrom);
	
	klass->close_dev (cdrom, force);
}

int
gnome_cdrom_get_dev_fd (GnomeCDRom *cdrom)
{
	g_return_val_if_fail (GNOME_IS_CDROM (cdrom), -1);

	return cdrom->fd;
}

static void
gnome_cdrom_update_cd (GnomeCDRom *cdrom)
{
	GnomeCDRomClass *klass;

	g_return_if_fail (GNOME_IS_CDROM (cdrom));
	
	klass = GNOME_CDROM_GET_CLASS (cdrom);
	
	klass->update_cd (cdrom);
}

static GnomeCDRomStatus *
not_ready_status_new (void)
{
	GnomeCDRomStatus *status;

	status = g_new0 (GnomeCDRomStatus, 1);

	status->cd = GNOME_CDROM_STATUS_DRIVE_NOT_READY;
	status->audio = GNOME_CDROM_AUDIO_NOTHING;
	status->track = -1;

	return status;
}

static gboolean
timeout_update_cd (gpointer data)
{
	GError *error;
	GnomeCDRom *cdrom = data;
	GnomeCDRomStatus *status;
	GnomeCDRomPrivate *priv;

	g_return_val_if_fail (GNOME_IS_CDROM (cdrom), FALSE);

	g_object_ref (G_OBJECT (cdrom));

	priv = cdrom->priv;

	/* Do an update */
	if (gnome_cdrom_get_status (GNOME_CDROM (cdrom), &status, &error) == FALSE) {
		g_free (priv->recent_status);
		priv->recent_status = not_ready_status_new ();
		if (status != NULL) {
			if (status->cd == GNOME_CDROM_STATUS_NO_DISC ||
			    status->cd == GNOME_CDROM_STATUS_NO_CDROM )
				priv->recent_status->cd = status->cd;
			g_free (status);
		}
		if (priv->update != GNOME_CDROM_UPDATE_NEVER)
			gnome_cdrom_status_changed (GNOME_CDROM (cdrom), priv->recent_status);
		gcd_warning ("%s", error);
		g_error_free (error);
		g_object_unref (G_OBJECT (cdrom));

		return TRUE;
	}
	
	if (priv->recent_status == NULL) {
		priv->recent_status = status;
		
		gnome_cdrom_update_cd (cdrom);
		if (priv->update != GNOME_CDROM_UPDATE_NEVER)
			gnome_cdrom_status_changed (GNOME_CDROM (cdrom), priv->recent_status);

	} else {
		if (gnome_cdrom_status_equal (status, priv->recent_status)) {
			if (priv->update == GNOME_CDROM_UPDATE_CONTINOUS)
				gnome_cdrom_status_changed (
					GNOME_CDROM (cdrom), priv->recent_status);

		} else {
			if (priv->recent_status->cd != GNOME_CDROM_STATUS_OK &&
			    status->cd == GNOME_CDROM_STATUS_OK)
				gnome_cdrom_update_cd (cdrom);
			
			g_free (priv->recent_status);
			priv->recent_status = status;

			if (priv->update != GNOME_CDROM_UPDATE_NEVER)
				gnome_cdrom_status_changed (
					GNOME_CDROM (cdrom), priv->recent_status);
		}
	}

	g_object_unref (G_OBJECT (cdrom));

	return TRUE;
}

GnomeCDRom *
gnome_cdrom_construct (GnomeCDRom      *cdrom,
		       const char      *device,
		       GnomeCDRomUpdate update,
		       GnomeCDRomDeviceLifetime lifetime,
		       GError         **error)
{
	g_return_val_if_fail (GNOME_IS_CDROM (cdrom), NULL);

	cdrom->lifetime = lifetime;
	cdrom->priv->update = update;

	if (!gnome_cdrom_set_device (cdrom, device, error) &&
	    lifetime != GNOME_CDROM_DEVICE_TRANSIENT) {
		g_object_unref (cdrom);
		return NULL;
	}

	/* Force an update so that a status will always exist */
	gnome_cdrom_update_cd (cdrom);
	
	/* All worked, start the counter */
	switch (update) {
	case GNOME_CDROM_UPDATE_NEVER:
		break;
		
	case GNOME_CDROM_UPDATE_WHEN_CHANGED:
	case GNOME_CDROM_UPDATE_CONTINOUS:
		cdrom->priv->update_id = g_timeout_add (
			CDROM_POLL_TIMEOUT, timeout_update_cd, cdrom);
		break;
		
	default:
		break;
	}

	return GNOME_CDROM (cdrom);
}

void
gnome_cdrom_force_status_rescan (GnomeCDRom *cdrom)
{
	g_return_if_fail (GNOME_IS_CDROM (cdrom));

	/* Force the new CD to be scanned at next update cycle */
	g_free (cdrom->priv->recent_status);
	cdrom->priv->recent_status = NULL;
}
