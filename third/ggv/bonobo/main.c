/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*-
 *
 * GGV Bonobo stuff server
 *
 * Author:
 *   Jaka Mocnik  <jaka@gnu.org>
 *
 * Inspired by Martin Baulig's EOG image viewer
 */

#include <config.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <gnome.h>

#include <bonobo/bonobo-ui-main.h> 
#include <bonobo/bonobo-generic-factory.h>

#include <gdk/gdkx.h>

#include <ggv-control.h>
#if 0
#  include <ggv-embeddable.h>
#endif /* 0 */

#undef BONOBO_DEBUG

#ifdef BONOBO_DEBUG
#  define LOG_FILE_NAME "/tmp/ggv-postscript-viewer.log"

static FILE *log_file = NULL;
static void ggv_postscript_viewer_log_func(const gchar *log_domain,
										   GLogLevelFlags log_level,
										   const gchar *message,
										   gpointer user_data)
{
	FILE *f = (FILE *)user_data;
	
	fprintf(f, "%s [%d]: %s\n", log_domain?log_domain:"", log_level, message);
	fflush(f);
}

#endif /* BONOBO_DEBUG */

static BonoboObject *
ggv_postscript_viewer_factory (BonoboGenericFactory *this, const char *oaf_iid,
							   void *data)
{
	GtkWidget *gs;
	GgvPostScriptView *ps_view;
	GtkAdjustment *hadj, *vadj;
	BonoboObject *retval;

	g_return_val_if_fail (this != NULL, NULL);
	g_return_val_if_fail (oaf_iid != NULL, NULL);

#ifdef BONOBO_DEBUG
	if(!log_file) {
		log_file = fopen(LOG_FILE_NAME, "w");
		if(log_file) {
			fprintf(log_file, "Log started.\n"); fflush(log_file);
			g_log_set_handler("GGV",
							  G_LOG_LEVEL_MASK |
							  G_LOG_FLAG_FATAL |
							  G_LOG_FLAG_RECURSION,
							  (GLogFunc)ggv_postscript_viewer_log_func,
							  log_file);
		}
	}
#endif /* BONOBO_DEBUG */

	hadj = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 1.0, 0.01, 0.1, 0.09));
	vadj = GTK_ADJUSTMENT(gtk_adjustment_new(0.0, 0.0, 1.0, 0.01, 0.1, 0.09));
	gs = gtk_gs_new(hadj, vadj);

	ps_view = ggv_postscript_view_new(GTK_GS(gs), FALSE);

	if (!strcmp (oaf_iid, "OAFIID:GNOME_GGV_Control"))
		retval = BONOBO_OBJECT (ggv_control_new (ps_view));
#if 0
	else if (!strcmp (oaf_iid, "OAFIID:GNOME_GGV_Embeddable"))
		retval = BONOBO_OBJECT (ggv_embeddable_new (ps_view));
#endif /* 0 */
	else if(!strcmp(oaf_iid, "OAFIID:GNOME_GGV_PostScriptView")) {
		retval = BONOBO_OBJECT(ps_view);
		bonobo_object_ref (BONOBO_OBJECT (ps_view));
	}
	else {
		g_warning ("Unknown IID `%s' requested", oaf_iid);
		return NULL;
	}

	bonobo_object_unref (BONOBO_OBJECT (ps_view));

	return retval;
}

static char *
make_reg_id(const char *iid)
{
	return
		bonobo_activation_make_registration_id(iid,
											   DisplayString(gdk_display));
}

int
main (int argc, char *argv [])					
{									
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);                       
	bind_textdomain_codeset (PACKAGE, "UTF-8");                     
	textdomain (PACKAGE);                                           

	BONOBO_FACTORY_INIT ("ggv-postscript-viewer", VERSION, &argc, argv);		
									
	return bonobo_generic_factory_main (make_reg_id("OAFIID:GNOME_GGV_Factory"),
										ggv_postscript_viewer_factory, NULL);	
}                                                                       
