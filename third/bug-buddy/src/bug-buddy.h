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

typedef enum {
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

typedef struct {
	const char *name;
	const char *command;
	gboolean    start_in_terminal;
	gboolean    use_moz_remote;
} MailerItem;

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

	/* canvas stuff */
	GnomeCanvasItem *title_box;
	GnomeCanvasItem *banner;
	GnomeCanvasItem *logo;

	GnomeCanvasItem *side_box;
	GnomeCanvasItem *side_image;

	/* throbber */
	GnomeCanvasItem *throbber;
	GdkPixbuf *throbber_pb;
	guint            throbber_id;

	gboolean already_run;

#if 0
	Distribution      *distro;
#endif
	char              *bts_file;

	CrashType  crash_type;

	int selected_row;
	int progress_timeout;

	/* gdb stuff */
	pid_t       app_pid;
	pid_t       gdb_pid;
	GIOChannel *ioc;
	int         fd;
	gboolean    explicit_dirty;

	BugzillaBTS *all_products;
	GSList      *bugzillas;

	/* Debian BTS stuff */
	SubmitType    submit_type;
#if 0
	BugType       bug_type; 
	SeverityType  severity;
	BugClassType  bug_class;
#endif
	GSList       *packages;

	GSList       *bugs;

	/* Bugzilla BTS stuff */
	BugzillaProduct   *product;
	BugzillaComponent *component;
	char              *severity;

	GList *dlsources;
	GList *dldests;

	gboolean download_in_progress;

	GnomeVFSAsyncHandle *vfshandle;
	gboolean need_to_download;

	GtkTreeViewColumn *uri_column;
	gboolean           showing_hand;

	const char *gnome_version;

	GHashTable *mailer_hash;
	MailerItem *mailer;
	MailerItem  custom_mailer;

	guint dl_timeout;
} DruidData;

extern DruidData druid_data;

extern const gchar *severity[];
extern const gchar *bug_class[][2];

void druid_set_sensitive (gboolean prev, gboolean next, gboolean cancel);
void druid_set_state (BuddyState state);

void get_trace_from_core (const gchar *core_file);
void get_trace_from_pair (const gchar *app, const gchar *extra);
void stop_gdb (void);
void start_gdb (void);

void stop_progress (void);

void append_packages (void);

void load_config (void);
void save_config (void);

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
