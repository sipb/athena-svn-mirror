/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <gnome.h>
#include <libgnomeui/gnome-help.h>

#include "gswitchit_util.h"

#include "libxklavier/xklavier.h"

void
GSwitchItHelp (GtkWidget * appWidget, const gchar * bookmark)
{
	GError *error = NULL;

	gnome_help_display_on_screen ("gswitchit", bookmark,
				    gtk_widget_get_screen (appWidget),
				    &error);

	if (error != NULL) {
		GtkWidget *dialog = gtk_message_dialog_new (NULL,
							    GTK_DIALOG_DESTROY_WITH_PARENT,
							    GTK_MESSAGE_ERROR,
							    GTK_BUTTONS_CLOSE,
							    _
							    ("There was an error displaying help: %s"),
							    error->
							    message);

		g_signal_connect (G_OBJECT (dialog),
				  "response",
				  G_CALLBACK (gtk_widget_destroy), NULL);

		gtk_window_set_screen (GTK_WINDOW (dialog),
				       gtk_widget_get_screen (appWidget));
		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

		gtk_widget_show (dialog);
		g_error_free (error);
	}
}

static void
GSwitchItLogAppender (const char file[], const char function[],
		      int level, const char format[], va_list args)
{
	time_t now = time (NULL);
	g_log (NULL, G_LOG_LEVEL_DEBUG, "[%08ld,%03d,%s:%s/] \t",
	       (long)now, level, file, function);
	g_logv (NULL, G_LOG_LEVEL_DEBUG, format, args);
}

void
GSwitchItInstallGlibLogAppender (void)
{
	XklSetLogAppender (GSwitchItLogAppender);
}
