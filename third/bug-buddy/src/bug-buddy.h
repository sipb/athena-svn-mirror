/* bug-buddy bug submitting program
 *
 * Copyright (C) 1999 - 2001 Jacob Berkman
 * Copyright 2000 Ximian, Inc.
 *
 * Author:  jacob berkman  <jacob@bug-buddy.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __BUG_BUDDY_H__
#define __BUG_BUDDY_H__

#include "bugzilla.h"

#include <glade/glade.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtktreeviewcolumn.h>
#include <libgnomevfs/gnome-vfs-async-ops.h>
#include <sys/types.h>

#define TEXT_BUG_CRASH "Description of the crash:\n\n\n\
Steps to reproduce the crash:\n\
1. \n\
2. \n\
3. \n\n\
Expected Results:\n\n\n\
How often does this happen?\n\n\n\
Additional Information:\n"

#define TEXT_BUG_WRONG "Description of Problem:\n\n\n\
Steps to reproduce the problem:\n\
1. \n\
2. \n\
3. \n\n\
Actual Results:\n\n\n\
Expected Results:\n\n\n\
How often does this happen?\n\n\n\
Additional Information:\n"


#define TEXT_BUG_I18N "Language:\n\n\
Wrong text location:\n\
Current text:\n\n\n\
Expected text:\n\n\n\
Additional Information:\n"


#define TEXT_BUG_DOC "Language:\n\n\
Documentation chapter/section:\n\
Current text:\n\n\n\
Expected text:\n\n\n\
Additional Information:\n"

#define TEXT_BUG_REQUEST "Please describe your feature request:\n\n"


typedef enum {
	CRASH_DIALOG,
	CRASH_CORE,
	CRASH_NONE
} CrashType;

/* keep in sync with mail config page */
typedef enum {
	SUBMIT_GNOME_MAILER,
	SUBMIT_SENDMAIL,
	SUBMIT_FILE
} SubmitType;

/* The order of this enum matters for the state update functions. */
typedef enum {
	STATE_INTRO,
	STATE_GDB,
	STATE_PRODUCT,
	STATE_COMPONENT,
	STATE_MOSTFREQ,
	STATE_DESC,
	STATE_EMAIL_CONFIG,
	STATE_EMAIL,
	STATE_FINISHED,
	STATE_LAST
} BuddyState;

/* The order of this enum matters for druid_draw_state
 * and mail_write_template. */
typedef enum {
	BUG_CRASH,
	BUG_DEBUG,
	BUG_WRONG,
	BUG_I18N,
	BUG_DOC,
	BUG_REQUEST
} BugType;

typedef struct {
	/* contact page */
	gchar *name;
	gchar *email;
	
	/* package page */
	gchar *package;
	gchar *package_ver;
	
	/* dialog page */
	gchar *app_file;
	gchar *pid;
	
	/* core page */
	gchar *core_file;

	/* file to include */
	gchar *include_file;
} PoptData;
extern PoptData popt_data;

typedef struct {
	GladeXML  *xml;
	BuddyState state;
	BugType bug_type;
	gboolean	product_skipped;
	gboolean	component_skipped;
	gboolean	description_filled;
	gboolean	show_products;

	/* canvas stuff */
	GnomeCanvasItem *title_box;
	GnomeCanvasItem *banner;
	GnomeCanvasItem *logo;

	GnomeCanvasItem *side_box;
	GnomeCanvasItem *side_image;

	/* throbber */
#if 0
	GnomeCanvasItem *throbber;
	GdkPixbuf *throbber_pb;
#endif

	char              *bts_file;

	CrashType  crash_type;

	int selected_row;
	int progress_timeout;


	char      *distro;
	
	/* gdb stuff */
	pid_t       app_pid;
	pid_t       gdb_pid;
	GIOChannel *ioc;
	int         fd;

	GHashTable  *bugzillas;
	GSList *applications;
	GHashTable *program_to_application;
	char *current_appname;

	/* Debian BTS stuff */
	SubmitType    submit_type;
	GSList       *packages;

	GSList       *bugs;

	/* Bugzilla BTS stuff */
	BugzillaProduct   *product;
	BugzillaComponent *component;
	char              *severity;
	char		  *version;

	GList *dlsources;
	GList *dldests;

	int last_update_check;

	GnomeVFSAsyncHandle *vfshandle;
	gboolean need_to_download;

	GtkTreeViewColumn *uri_column;
	gboolean           showing_hand;

	const char *gnome_version;

	char *gnome_platform_version;
	char *gnome_distributor;
	char *gnome_date;

	guint dl_timeout;

} DruidData;

extern DruidData druid_data;

extern const gchar *severity[];
extern const gchar *bug_class[][2];

/* The following functions are used when moving through pages of the
 * druid. The "enter" func tries to go to a page. If the page does
 * not exist in a certain mode, it returns FALSE. Try the next one.
 * The "done" functions are called when moving forward from a state.
 * The functions return FALSE if the user should not be allowed to
 * leave the state (e.g. incorrectly filled form).
 */
typedef gboolean (*FuncPtr) (void);

gboolean enter_intro (void);
gboolean enter_gdb (void);
gboolean enter_product (void);
gboolean enter_component (void);
gboolean enter_mostfreq (void);
gboolean enter_desc (void);
gboolean enter_email_config (void);
gboolean enter_email (void);
gboolean enter_finished (void);

gboolean done_intro (void);
gboolean done_gdb (void);
gboolean done_product (void);
gboolean done_component (void);
gboolean done_mostfreq (void);
gboolean done_desc (void);
gboolean done_email_config (void);
gboolean done_email (void);
gboolean done_finished (void);

void buddy_error (GtkWidget *parent, const char *msg, ...);
void druid_set_state (BuddyState state);

void stop_gdb (void);
void start_gdb (void);

void stop_progress (void);

void append_packages (void);

#define GET_WIDGET(name) (get_widget ((name), (G_STRLOC)))

GtkWidget *get_widget (const char *s, const char *loc);

/* GTK's text widgets have sucky apis */
char *buddy_get_text_widget (GtkWidget *w);
#define buddy_get_text(w) (buddy_get_text_widget (GET_WIDGET (w)))

void  buddy_set_text_widget (GtkWidget *w, const char *s);
#define buddy_set_text(w, s) (buddy_set_text_widget (GET_WIDGET (w), s))

void gconf_buddy_connect_string (const char *key, const char *opt_widget);
void gconf_buddy_connect_bool   (const char *key, const char *opt_widget);

#endif /* __bug_buddy_h__ */
