#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <glib.h>
#include <glib/gi18n.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifdef __FreeBSD__
#include <sys/uio.h>
#endif /* __FreeBSD__ */
#include <signal.h>

#ifdef USE_HAL
#include <libhal.h>
#endif

#include "cd-recorder.h"
#include "cd-recorder-marshal.h"

struct CDRecorderPrivate {
	GMainLoop *loop;
	int result;
	int pid;
	int cdr_stdin;
	GString *line;
	GString *cdr_stderr;
	gboolean changed_text;
	gboolean expect_cdrecord_to_die;
	gboolean dangerous;

	char *last_error;

	GList *tracks;
	int track_count;
	gboolean debug;
	gboolean can_rewrite;

#ifdef USE_HAL
	LibHalContext *ctx;
#endif
};

/* Signals */
enum {
	PROGRESS_CHANGED,
	ACTION_CHANGED,
	ANIMATION_CHANGED,
	INSERT_CD_REQUEST,
	LAST_SIGNAL
};

static int cd_recorder_table_signals[LAST_SIGNAL] = { 0 };

static GObjectClass *parent_class = NULL;

static void cd_recorder_class_init (CDRecorderClass *class);
static void cd_recorder_init       (CDRecorder      *cdrecorder);

static int cd_recorder_write_cdrecord (CDRecorder *cdrecorder,
		CDDrive *recorder, GList *tracks,
		int speed, CDRecorderWriteFlags flags);
static int cd_recorder_write_growisofs (CDRecorder *cdrecorder,
		CDDrive *recorder, GList *tracks, int speed,
		CDRecorderWriteFlags flags);

G_DEFINE_TYPE(CDRecorder, cd_recorder, G_TYPE_OBJECT)

void
cd_recorder_track_free (Track *track)
{
	switch (track->type) {
	case TRACK_TYPE_DATA:
		g_free (track->contents.data.filename);
		break;
	case TRACK_TYPE_AUDIO:
		g_free (track->contents.audio.filename);
		g_free (track->contents.audio.cdtext);
		break;
	default:
		g_warning ("Invalid track type %d", track->type);
	}

	g_free (track);
}

static gboolean
can_burn_dvds (CDDrive *recorder)
{
	if (!(recorder->type & CDDRIVE_TYPE_DVD_RW_RECORDER) &&
	    !(recorder->type & CDDRIVE_TYPE_DVD_PLUS_R_RECORDER) &&
	    !(recorder->type & CDDRIVE_TYPE_DVD_PLUS_RW_RECORDER)) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
cd_needs_growisofs (CDDrive *recorder, CDMediaType type, GList *tracks)
{
	GList *l;

	/* If we cannot burn DVDs, then we don't need growisofs */
	if (can_burn_dvds (recorder) == FALSE) {
		return FALSE;
	}

	if (type == CD_MEDIA_TYPE_DVDR
			|| type == CD_MEDIA_TYPE_DVDRW
			|| type == CD_MEDIA_TYPE_DVD_PLUS_R
			|| type == CD_MEDIA_TYPE_DVD_PLUS_RW) {
		return TRUE;
	}

	/* If we have audio tracks, we're using cdrecord */
	for (l = tracks; l != NULL; l = l->next) {
		Track *track = l->data;
		if (track->type == TRACK_TYPE_AUDIO) {
			return FALSE;
		}
	}

	return FALSE;
}

gboolean
cd_recorder_cancel (CDRecorder *cdrecorder, gboolean skip_if_dangerous)
{
	if (!cdrecorder->priv->dangerous || !skip_if_dangerous) {
		kill (cdrecorder->priv->pid, SIGINT);

		cdrecorder->priv->result = RESULT_CANCEL;
		g_main_loop_quit (cdrecorder->priv->loop);

		return FALSE;
	}

	return cdrecorder->priv->dangerous;
}

static gboolean
growisofs_stdout_read (GIOChannel   *source,
		      GIOCondition  condition,
		      gpointer      data)
{
	CDRecorder *cdrecorder = (CDRecorder *) data;
	char *line;
	char buf[1];
	GIOStatus status;

	status = g_io_channel_read_line (source,
					 &line, NULL, NULL, NULL);

	if (line && cdrecorder->priv->debug) {
		g_print ("growisofs stdout: %s", line);
	}

	if (status == G_IO_STATUS_NORMAL) {
		double percent;
		int perc_1, perc_2;
		long long tmp;

		if (cdrecorder->priv->line) {
			g_string_append (cdrecorder->priv->line, line);
			g_free (line);
			line = g_string_free (cdrecorder->priv->line, FALSE);
			cdrecorder->priv->line = NULL;
		}

		if (sscanf (line, "%10lld/%lld ( %2d.%1d%%)", &tmp, &tmp, &perc_1, &perc_2) == 4) {
			if (!cdrecorder->priv->changed_text) {
				g_signal_emit (G_OBJECT (cdrecorder),
					       cd_recorder_table_signals[ACTION_CHANGED], 0,
					       WRITING, MEDIA_DVD);
			}

			percent = (perc_1 + ((float) perc_2 / 10)) / 100;

			g_signal_emit (G_OBJECT (cdrecorder),
				       cd_recorder_table_signals[PROGRESS_CHANGED], 0,
				       percent);
		} else if (strstr (line, "About to execute") != NULL) {
			cdrecorder->priv->dangerous = TRUE;
		}

		g_free (line);
	} else if (status == G_IO_STATUS_AGAIN) {
		/* A non-terminated line was read, read the data into the buffer. */
		status = g_io_channel_read_chars (source, buf, 1, NULL, NULL);
		if (status == G_IO_STATUS_NORMAL) {
			if (cdrecorder->priv->line == NULL) {
				cdrecorder->priv->line = g_string_new (NULL);
			}
			g_string_append_c (cdrecorder->priv->line, buf[0]);
		}
	} else if (status == G_IO_STATUS_EOF) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
cdrecord_stdout_read (GIOChannel   *source,
		      GIOCondition  condition,
		      gpointer      data)
{
	CDRecorder *cdrecorder = (CDRecorder *) data;
	char *line;
	char buf[1];
	unsigned int track, mb_written, mb_total;
	GIOStatus status;

	status = g_io_channel_read_line (source,
					 &line, NULL, NULL, NULL);
	
	if (line && cdrecorder->priv->debug) {
		g_print ("cdrecord stdout: %s", line);
	}

	if (status == G_IO_STATUS_NORMAL) {
		if (cdrecorder->priv->line) {
			g_string_append (cdrecorder->priv->line, line);
			g_free (line);
			line = g_string_free (cdrecorder->priv->line, FALSE);
			cdrecorder->priv->line = NULL;
		}
		
		if (sscanf (line, "Track %2u: %d of %d MB written",
			    &track, &mb_written, &mb_total) == 3) {
			double percent;
			if (!cdrecorder->priv->changed_text) {
				g_signal_emit (G_OBJECT (cdrecorder),
					       cd_recorder_table_signals[ACTION_CHANGED], 0,
					       WRITING, MEDIA_CD);
			}
			percent = 0.98 * ((double)((track-1)/(double)cdrecorder->priv->track_count)
					  + ((double)mb_written/mb_total) / (double)cdrecorder->priv->track_count);
			g_signal_emit (G_OBJECT (cdrecorder),
				       cd_recorder_table_signals[PROGRESS_CHANGED], 0,
				       percent);
		} else if (g_str_has_prefix (line, "Re-load disk and hit <CR>") ||
			   g_str_has_prefix (line, "send SIGUSR1 to continue")) {
			g_assert_not_reached ();
		} else if (g_str_has_prefix (line, "Fixating...")) {
			g_signal_emit (G_OBJECT (cdrecorder),
				       cd_recorder_table_signals[ACTION_CHANGED], 0,
				       FIXATING, MEDIA_CD);
		} else if (g_str_has_prefix (line, "Fixating time:")) {
			g_signal_emit (G_OBJECT (cdrecorder),
				       cd_recorder_table_signals[PROGRESS_CHANGED], 0,
				       1.0);
			cdrecorder->priv->result = RESULT_FINISHED;
		} else if (g_str_has_prefix (line, "Last chance to quit, ")) {
			cdrecorder->priv->dangerous = TRUE;
		} else if (g_str_has_prefix (line, "Blanking PMA, TOC, pregap")) {
			g_signal_emit (G_OBJECT (cdrecorder),
				       cd_recorder_table_signals[ACTION_CHANGED], 0,
				       BLANKING, MEDIA_CD);
		}

		g_free (line);
		
	} else if (status == G_IO_STATUS_AGAIN) {
		/* A non-terminated line was read, read the data into the buffer. */
		status = g_io_channel_read_chars (source, buf, 1, NULL, NULL);
		if (status == G_IO_STATUS_NORMAL) {
			if (cdrecorder->priv->line == NULL) {
				cdrecorder->priv->line = g_string_new (NULL);
			}
			g_string_append_c (cdrecorder->priv->line, buf[0]);
		}
	} else if (status == G_IO_STATUS_EOF) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
growisofs_stderr_read (GIOChannel   *source,
		      GIOCondition  condition,
		      gpointer      data)
{
	CDRecorder *cdrecorder = (CDRecorder *) data;
	char *line;
	GIOStatus status;

	status = g_io_channel_read_line (source,
					 &line, NULL, NULL, NULL);
	if (line && cdrecorder->priv->debug) {
		g_print ("growisofs stderr: %s", line);
	}

	/* TODO: Handle errors */
	if (status == G_IO_STATUS_NORMAL && !cdrecorder->priv->expect_cdrecord_to_die) {
		g_string_prepend (cdrecorder->priv->cdr_stderr, line);
		if (strstr (line, "unsupported MMC profile") != NULL || (strstr (line, "already carries isofs") != NULL && strstr (line, "FATAL:") != NULL)) {
			g_assert_not_reached ();
		} else if (strstr (line, "unable to open") != NULL || strstr (line, "unable to stat") != NULL) {
			/* This fits the "open64" and "open"-like messages */
			cdrecorder->priv->last_error = g_strdup (_("The recorder could not be accessed"));
			cdrecorder->priv->result = RESULT_ERROR;
		} else if (strstr (line, "not enough space available") != NULL) {
			cdrecorder->priv->last_error = g_strdup (_("Not enough space available on the disc"));
			cdrecorder->priv->result = RESULT_ERROR;
		} else if (strstr (line, "end of user area encountered on this track") != NULL) {
			cdrecorder->priv->last_error = g_strdup (_("The files selected did not fit on the CD"));
			cdrecorder->priv->result = RESULT_ERROR;
		} else if (strstr (line, "blocks are free") != NULL) {
			cdrecorder->priv->last_error = g_strdup (_("The files selected did not fit on the CD"));
			cdrecorder->priv->result = RESULT_ERROR;
		} else if (strstr (line, "flushing cache") != NULL) {
			g_signal_emit (G_OBJECT (cdrecorder),
				       cd_recorder_table_signals[ACTION_CHANGED], 0,
				       FIXATING, MEDIA_DVD);
			cdrecorder->priv->result = RESULT_FINISHED;
		} else if (strstr (line, ":-(") != NULL || strstr (line, "FATAL") != NULL) {
			cdrecorder->priv->last_error = g_strdup (_("Unhandled error, aborting"));
			cdrecorder->priv->result = RESULT_ERROR;
		}
	} else if (status == G_IO_STATUS_EOF) {
		if (!cdrecorder->priv->expect_cdrecord_to_die) {
			g_main_loop_quit (cdrecorder->priv->loop);
		}
		return FALSE;
	} else {
		g_print ("growisofs stderr read failed, status: %d\n", status);
	}

	return TRUE;
}

static gboolean  
cdrecord_stderr_read (GIOChannel   *source,
		      GIOCondition  condition,
		      gpointer      data)
{
	CDRecorder *cdrecorder = (CDRecorder *) data;
	char *line;
	GIOStatus status;

	status = g_io_channel_read_line (source,
					 &line, NULL, NULL, NULL);
	if (line && cdrecorder->priv->debug) {
		g_print ("cdrecord stderr: %s", line);
	}

	/* TODO: Handle errors */
	if (status == G_IO_STATUS_NORMAL && !cdrecorder->priv->expect_cdrecord_to_die) {
		g_string_prepend (cdrecorder->priv->cdr_stderr, line);
		if (strstr (line, "No disk / Wrong disk!") != NULL) {
			g_warning ("No disk in drive, shouldn't happen");
			g_assert_not_reached ();
		} else if (strstr (line, "This means that we are checking recorded media.") != NULL) {
			cdrecorder->priv->last_error = g_strdup (_("The CD has already been recorded"));
			cdrecorder->priv->result = RESULT_ERROR;
		} else if (strstr (line, "Cannot blank disk, aborting.") != NULL) {
			g_warning ("Right CD type wasn't loaded, shouldn't happen");
			g_assert_not_reached ();
		} else if (strstr (line, "Data may not fit on current disk") != NULL) {
			cdrecorder->priv->last_error = g_strdup (_("The files selected did not fit on the CD"));
			/* FIXME should we error out in that case?
			cdrecorder->priv->result = RESULT_ERROR; */
		} else if (strstr (line, "Inappropriate audio coding") != NULL) {
			cdrecorder->priv->last_error = g_strdup (_("All audio files must be stereo, 16-bit digital audio with 44100Hz samples"));
			cdrecorder->priv->result = RESULT_ERROR;
		} else if (strstr (line, "cannot write medium - incompatible format") != NULL) {
			g_warning ("Right CD type wasn't loaded, shouldn't happen");
			g_assert_not_reached ();
			cdrecorder->priv->expect_cdrecord_to_die = TRUE;
		} else if (strstr (line, "DMA speed too slow") != NULL) {
			cdrecorder->priv->last_error = g_strdup (_("The system is too slow to write the CD at this speed. Try a lower speed."));
		}

		g_free (line);
	} else if (status == G_IO_STATUS_EOF) {
		if (!cdrecorder->priv->expect_cdrecord_to_die) {
			g_main_loop_quit (cdrecorder->priv->loop);
		}
		return FALSE;
	} else {
		g_print ("cdrecord stderr read failed, status: %d\n", status);
	}

	return TRUE;
}

static gboolean
media_type_matches (CDMediaType type, gboolean *needs_blank)
{
	*needs_blank = FALSE;

	switch (type)
	{
	case CD_MEDIA_TYPE_ERROR:
	case CD_MEDIA_TYPE_BUSY:
		return FALSE;
	case CD_MEDIA_TYPE_UNKNOWN:
		return TRUE;
	case CD_MEDIA_TYPE_CD:
	case CD_MEDIA_TYPE_DVD:
	case CD_MEDIA_TYPE_DVD_RAM:
		return FALSE;
	case CD_MEDIA_TYPE_CDR:
		return TRUE;
	case CD_MEDIA_TYPE_CDRW:
		*needs_blank = TRUE;
		return TRUE;
	case CD_MEDIA_TYPE_DVDR:
		return TRUE;
	case CD_MEDIA_TYPE_DVDRW:
		*needs_blank = TRUE;
		return TRUE;
	case CD_MEDIA_TYPE_DVD_PLUS_R:
		return TRUE;
	case CD_MEDIA_TYPE_DVD_PLUS_RW:
		*needs_blank = TRUE;
		return TRUE;
	}

	return FALSE;
}

static CDMediaType
cd_recorder_wait_for_insertion (CDRecorder *cdrecorder, CDDrive *drive,
		gboolean *needs_blank)
{
	CDMediaType type;
	gboolean reload;

	reload = FALSE;
	type = cd_drive_get_media_type (drive);

	if (type == CD_MEDIA_TYPE_ERROR) {
		reload = TRUE;
	}

	while (!media_type_matches (type, needs_blank)) {
		gboolean res, busy_cd;

		busy_cd = (type == CD_MEDIA_TYPE_BUSY);

#ifdef USE_HAL
		if (busy_cd != FALSE) {
			/* TODO umount the disc if we find it,
			 * and try it again */
		}
#endif

		g_signal_emit (G_OBJECT (cdrecorder),
				cd_recorder_table_signals[INSERT_CD_REQUEST],
				0, reload, cdrecorder->priv->can_rewrite,
				busy_cd, &res);

		if (res == FALSE) {
			return CD_MEDIA_TYPE_ERROR;
		}

		type = cd_drive_get_media_type (drive);
		reload = FALSE;
		if (type == CD_MEDIA_TYPE_UNKNOWN
				|| type == CD_MEDIA_TYPE_ERROR) {
			reload = TRUE;
		}
	}

	return type;
}

int
cd_recorder_write_tracks (CDRecorder *cdrecorder,
		       CDDrive *drive,
		       GList *tracks,
		       int speed,
		       CDRecorderWriteFlags flags)
		       /* TODO: GError **error */
{
	CDMediaType type;
	gboolean needs_blank;

	g_return_val_if_fail (tracks != NULL, RESULT_ERROR);
	cdrecorder->priv->tracks = tracks;
	cdrecorder->priv->track_count = g_list_length (tracks);
	cdrecorder->priv->debug = (flags & CDRECORDER_DEBUG);

	/* Let's hope that DVD+RW and DVD-RW drives can also do CD-RWs */
	cdrecorder->priv->can_rewrite =
		(drive->type & CDDRIVE_TYPE_CDRW_RECORDER);

	if (cdrecorder->priv->track_count > 99) {
		cdrecorder->priv->last_error = g_strdup (_("You can only burn 99 tracks on one disc"));
		cdrecorder->priv->result = RESULT_ERROR;
		return cdrecorder->priv->result;
	}

	type = cd_recorder_wait_for_insertion (cdrecorder,
			drive, &needs_blank);

	if (type == CD_MEDIA_TYPE_ERROR) {
		cdrecorder->priv->result = RESULT_CANCEL;
		return cdrecorder->priv->result;
	}

	if (needs_blank != FALSE) {
		flags |= CDRECORDER_BLANK;
	}

	if (cd_needs_growisofs (drive, type, tracks)) {
		return cd_recorder_write_growisofs (cdrecorder,
						    drive, tracks, speed,
						    flags);
	} else {
		return cd_recorder_write_cdrecord (cdrecorder,
						   drive, tracks, speed,
						   flags);
	}
}

static int
cd_recorder_write_growisofs (CDRecorder *cdrecorder,
				 CDDrive *recorder,
				 GList *tracks,
				 int speed,
				 CDRecorderWriteFlags flags)
{
	GPtrArray *argv;
	char *speed_str, *dev_str;
	int stdout_pipe, stderr_pipe;
	guint stdout_tag, stderr_tag;
	GIOChannel *channel;
	GError *error;
	Track *t;

	if (g_list_length (tracks) != 1) {
		g_warning ("Can only use growisofs on a single track");
		return RESULT_ERROR;
	}
	t = (Track*)tracks->data;
	if (t->type != TRACK_TYPE_DATA) {
		g_warning ("Can only use growisofs on a data track");
		return RESULT_ERROR;
	}

	argv = g_ptr_array_new ();

	g_ptr_array_add (argv, "growisofs");
	speed_str = g_strdup_printf ("-speed=%d", speed);
	if (speed != 0) {
		g_ptr_array_add (argv, speed_str);
	}

	g_ptr_array_add (argv, "-dvd-compat");

	/* Weird, innit? We tell growisofs we have a tty so it ignores
	 * the fact that the DVD+ has an ISO fs already */
	if (flags & CDRECORDER_BLANK) {
		g_ptr_array_add (argv, "-use-the-force-luke=tty");
	}

	g_ptr_array_add (argv, "-Z");

	dev_str = g_strdup_printf ("%s=%s", recorder->device, t->contents.data.filename);
	g_ptr_array_add (argv, dev_str);

	g_ptr_array_add (argv, NULL);

	cdrecorder->priv->cdr_stderr = NULL;
retry_growisofs:
	cdrecorder->priv->result = RESULT_ERROR;
	cdrecorder->priv->expect_cdrecord_to_die = FALSE;
	cdrecorder->priv->line = NULL;
	if (cdrecorder->priv->cdr_stderr != NULL) {
		g_string_truncate (cdrecorder->priv->cdr_stderr, 0);
	} else {
		cdrecorder->priv->cdr_stderr = g_string_new (NULL);
	}

	g_signal_emit (G_OBJECT (cdrecorder),
			cd_recorder_table_signals[ACTION_CHANGED], 0,
			PREPARING_WRITE, MEDIA_DVD);
	cdrecorder->priv->changed_text = FALSE;
	g_signal_emit (G_OBJECT (cdrecorder),
			cd_recorder_table_signals[PROGRESS_CHANGED], 0,
			0.0);
	g_signal_emit (G_OBJECT (cdrecorder),
			cd_recorder_table_signals[ANIMATION_CHANGED], 0,
			TRUE);
	cdrecorder->priv->dangerous = FALSE;

	if (cdrecorder->priv->debug)
	{
		guint i;
		g_print ("launching command: ");
		for (i = 0; i < argv->len - 1; i++) {
			g_print ("%s ", (char*)g_ptr_array_index (argv, i));
		}
		g_print ("\n");
	}

	error = NULL;
	if (!g_spawn_async_with_pipes  (NULL,
					(char**)argv->pdata,
					NULL,
					G_SPAWN_SEARCH_PATH,
					NULL, NULL,
					&cdrecorder->priv->pid,
					&cdrecorder->priv->cdr_stdin,
					&stdout_pipe,
					&stderr_pipe,
					&error)) {
		guint i;
		g_warning ("growisofs command failed: %s\n", error->message);
		for (i = 0; i < argv->len - 1; i++) {
			g_print ("%s ", (char*)g_ptr_array_index (argv, i));
		}
		g_print ("\n");
		g_error_free (error);
		/* TODO: Better error handling */
	} else {
		/* Make sure we don't block on a read. */
		fcntl (stdout_pipe, F_SETFL, O_NONBLOCK);
		fcntl (stdout_pipe, F_SETFL, O_NONBLOCK);

		cdrecorder->priv->loop = g_main_loop_new (NULL, FALSE);

		channel = g_io_channel_unix_new (stdout_pipe);
		g_io_channel_set_encoding (channel, NULL, NULL);
		stdout_tag = g_io_add_watch (channel,
					     (G_IO_IN | G_IO_HUP | G_IO_ERR),
					     growisofs_stdout_read,
					     cdrecorder);
		g_io_channel_unref (channel);
		channel = g_io_channel_unix_new (stderr_pipe);
		g_io_channel_set_encoding (channel, NULL, NULL);
		stderr_tag = g_io_add_watch (channel,
					     (G_IO_IN | G_IO_HUP | G_IO_ERR),
					     growisofs_stderr_read,
					     cdrecorder);
		g_io_channel_unref (channel);

		cdrecorder->priv->dangerous = FALSE;

		g_main_loop_run (cdrecorder->priv->loop);
		g_main_loop_unref (cdrecorder->priv->loop);

		g_source_remove (stdout_tag);
		g_source_remove (stderr_tag);

		if (cdrecorder->priv->result == RESULT_RETRY) {
			goto retry_growisofs;
		}
	}

	g_free (speed_str);
	g_free (dev_str);
	g_ptr_array_free (argv, TRUE);
	
	g_signal_emit (G_OBJECT (cdrecorder),
			cd_recorder_table_signals[ANIMATION_CHANGED], 0,
			FALSE);

	if (flags & CDRECORDER_EJECT && cdrecorder->priv->result == RESULT_FINISHED) {
		char *cmd;

		cmd = g_strdup_printf ("eject %s", recorder->device);
		g_spawn_command_line_sync (cmd, NULL, NULL, NULL, NULL);
		g_free (cmd);
	}

	return cdrecorder->priv->result;
}

static int
cd_recorder_write_cdrecord (CDRecorder *cdrecorder,
		       CDDrive *recorder,
		       GList *tracks,
		       int speed,
		       CDRecorderWriteFlags flags)
{
	GPtrArray *argv;
	char *speed_str, *dev_str;
	int stdout_pipe, stderr_pipe;
	guint stdout_tag, stderr_tag;
	GIOChannel *channel;
	GError *error;
	GList *l;

	g_return_val_if_fail (tracks != NULL, RESULT_ERROR);

	argv = g_ptr_array_new ();
	g_ptr_array_add (argv, "cdrecord");

	speed_str = g_strdup_printf ("speed=%d", speed);
	if (speed != 0) {
		g_ptr_array_add (argv, speed_str);
	}
	dev_str = g_strdup_printf ("dev=%s", recorder->cdrecord_id);
	g_ptr_array_add (argv, dev_str);
	if (flags & CDRECORDER_DUMMY_WRITE) {
		g_ptr_array_add (argv, "-dummy");
	}
	if (flags & CDRECORDER_EJECT) {
		g_ptr_array_add (argv, "-eject");
	}
	if (flags & CDRECORDER_BLANK) {
		g_ptr_array_add (argv, "blank=fast");
	}
	if (flags & CDRECORDER_DISC_AT_ONCE) {
		g_ptr_array_add (argv, "-dao");
	}
	if (flags & CDRECORDER_OVERBURN) {
		g_ptr_array_add (argv, "-overburn");
	}
	if (flags & CDRECORDER_BURNPROOF) {
		g_ptr_array_add (argv, "driveropts=burnfree");
	}
	g_ptr_array_add (argv, "-v");

	l = tracks;
	while (l != NULL && l->data != NULL) {
		Track *track = l->data;
		switch (track->type) {
		case TRACK_TYPE_DATA:
			g_ptr_array_add (argv, "-data");
			g_ptr_array_add (argv, "-nopad"); /* TODO? */
			g_ptr_array_add (argv, track->contents.data.filename);
			break;
		case TRACK_TYPE_AUDIO:
			g_ptr_array_add (argv, "-copy");
			g_ptr_array_add (argv, "-audio");
			g_ptr_array_add (argv, "-pad");
			g_ptr_array_add (argv, track->contents.audio.filename);
			/* TODO: handle CD-TEXT somehow */
			break;
		default:
			g_warning ("Unknown track type %d", track->type);
		}
		l = g_list_next (l);
	}
	g_ptr_array_add (argv, NULL);

	cdrecorder->priv->cdr_stderr = NULL;
 retry:
	cdrecorder->priv->result = RESULT_ERROR;
	cdrecorder->priv->expect_cdrecord_to_die = FALSE;
	cdrecorder->priv->line = NULL;
	if (cdrecorder->priv->cdr_stderr != NULL) {
		g_string_truncate (cdrecorder->priv->cdr_stderr, 0);
	} else {
		cdrecorder->priv->cdr_stderr = g_string_new (NULL);
	}

	g_signal_emit (G_OBJECT (cdrecorder),
		       cd_recorder_table_signals[ACTION_CHANGED], 0,
		       PREPARING_WRITE, MEDIA_CD);
	cdrecorder->priv->changed_text = FALSE;
	g_signal_emit (G_OBJECT (cdrecorder),
		       cd_recorder_table_signals[PROGRESS_CHANGED], 0,
		       0.0);
	g_signal_emit (G_OBJECT (cdrecorder),
		       cd_recorder_table_signals[ANIMATION_CHANGED], 0,
		       TRUE);
	cdrecorder->priv->dangerous = FALSE;

	if (cdrecorder->priv->debug)
	{
		guint i;
		g_print ("launching command: ");
		for (i = 0; i < argv->len - 1; i++) {
			g_print ("%s ", (char*)g_ptr_array_index (argv, i));
		}
		g_print ("\n");
	}

	error = NULL;
	if (!g_spawn_async_with_pipes  (NULL,
					(char**)argv->pdata,
					NULL,
					G_SPAWN_SEARCH_PATH,
					NULL, NULL,
					&cdrecorder->priv->pid,
					&cdrecorder->priv->cdr_stdin,
					&stdout_pipe,
					&stderr_pipe,
					&error)) {
		g_warning ("cdrecord command failed: %s\n", error->message);
		g_error_free (error);
		/* TODO: Better error handling */
	} else {
		/* Make sure we don't block on a read. */
		fcntl (stdout_pipe, F_SETFL, O_NONBLOCK);
		fcntl (stderr_pipe, F_SETFL, O_NONBLOCK);

		cdrecorder->priv->loop = g_main_loop_new (NULL, FALSE);

		channel = g_io_channel_unix_new (stdout_pipe);
		g_io_channel_set_encoding (channel, NULL, NULL);
		stdout_tag = g_io_add_watch (channel, 
					     (G_IO_IN | G_IO_HUP | G_IO_ERR), 
					     cdrecord_stdout_read,
					     cdrecorder);
		g_io_channel_unref (channel);
		channel = g_io_channel_unix_new (stderr_pipe);
		g_io_channel_set_encoding (channel, NULL, NULL);
		stderr_tag = g_io_add_watch (channel, 
					     (G_IO_IN | G_IO_HUP | G_IO_ERR), 
					     cdrecord_stderr_read,
					     cdrecorder);
		g_io_channel_unref (channel);

		cdrecorder->priv->dangerous = FALSE;

		g_main_loop_run (cdrecorder->priv->loop);
		g_main_loop_unref (cdrecorder->priv->loop);
		
		g_source_remove (stdout_tag);
		g_source_remove (stderr_tag);

		if (cdrecorder->priv->result == RESULT_RETRY) {
			goto retry;
		}
	}

	g_free (speed_str);
	g_free (dev_str);
	g_ptr_array_free (argv, TRUE);

	g_signal_emit (G_OBJECT (cdrecorder),
		       cd_recorder_table_signals[ANIMATION_CHANGED], 0,
		       FALSE);

	return cdrecorder->priv->result;
}

const char *
cd_recorder_get_error_message_details (CDRecorder *cdrecorder)
{
	g_return_val_if_fail (cdrecorder->priv->result == RESULT_ERROR, NULL);

	return (const char *)cdrecorder->priv->cdr_stderr->str;
}

const char *
cd_recorder_get_error_message (CDRecorder *cdrecorder)
{
	g_return_val_if_fail (cdrecorder->priv->result == RESULT_ERROR, NULL);

	return (const char *)cdrecorder->priv->last_error;
}

static void
cd_recorder_finalize (GObject *object)
{
	CDRecorder *cdrecorder = CD_RECORDER (object);

	g_return_if_fail (object != NULL);

	if (cdrecorder->priv->cdr_stderr != NULL) {
		g_string_free (cdrecorder->priv->cdr_stderr, TRUE);
	}

	if (cdrecorder->priv->line != NULL) {
		g_string_free (cdrecorder->priv->line, TRUE);
	}

	g_free (cdrecorder->priv->last_error);

	if (G_OBJECT_CLASS (parent_class)->finalize != NULL) {
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
	}
}

static void
cd_recorder_init (CDRecorder *cdrecorder)
{
	cdrecorder->priv = g_new0 (CDRecorderPrivate, 1);
}

CDRecorder *
cd_recorder_new (void)
{
	return g_object_new (cd_recorder_get_type (), NULL);
}

static void
cd_recorder_class_init (CDRecorderClass *klass)
{
	parent_class = g_type_class_ref (G_TYPE_OBJECT);

	G_OBJECT_CLASS (klass)->finalize = cd_recorder_finalize;

	/* Signals */
	cd_recorder_table_signals[PROGRESS_CHANGED] =
		g_signal_new ("progress-changed",
				G_TYPE_FROM_CLASS (klass),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET (CDRecorderClass,
					progress_changed),
				NULL, NULL,
				g_cclosure_marshal_VOID__DOUBLE,
				G_TYPE_NONE, 1, G_TYPE_DOUBLE);
	cd_recorder_table_signals[ACTION_CHANGED] =
		g_signal_new ("action-changed",
				G_TYPE_FROM_CLASS (klass),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET (CDRecorderClass,
					action_changed),
				NULL, NULL,
				cdrecorder_marshal_VOID__INT_INT,
				G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);
	cd_recorder_table_signals[ANIMATION_CHANGED] =
		g_signal_new ("animation-changed",
				G_TYPE_FROM_CLASS (klass),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET (CDRecorderClass,
					animation_changed),
				NULL, NULL,
				g_cclosure_marshal_VOID__BOOLEAN,
				G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
	cd_recorder_table_signals[INSERT_CD_REQUEST] =
		g_signal_new ("insert-cd-request",
				G_TYPE_FROM_CLASS (klass),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET (CDRecorderClass,
					insert_cd_request),
				NULL, NULL,
				cdrecorder_marshal_BOOLEAN__BOOLEAN_BOOLEAN_BOOLEAN,
				G_TYPE_BOOLEAN, 3, G_TYPE_BOOLEAN,
				G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);
}

