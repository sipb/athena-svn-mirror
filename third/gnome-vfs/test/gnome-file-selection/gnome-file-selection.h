#ifndef _GNOME_FILE_SELECTION_H
#define _GNOME_FILE_SELECTION_H

#include <gtk/gtk.h>
#include <gdk_imlib.h>

#include <libgnome/gnome-defs.h>
#include <libgnomeui/gnome-dialog.h>

BEGIN_GNOME_DECLS

#define GNOME_FILE_SELECTION(obj) \
	GTK_CHECK_CAST (obj, gnome_file_selection_get_type (), GnomeFileSelection)
#define GNOME_FILE_SELECTION_CLASS(klass) \
	GTK_CHECK_CLASS_CAST (klass, gnome_file_selection_get_type (), GnomeFileSelectionClass)
#define GNOME_IS_FILE_SELECTION(obj) \
	GTK_CHECK_TYPE (obj, gnome_file_selection_get_type ())

typedef struct _GnomeFileSelection GnomeFileSelection;
typedef struct _GnomeFileSelectionPrivate GnomeFileSelectionPrivate;
typedef struct _GnomeFileSelectionFilter GnomeFileSelectionFilter;
typedef struct _GnomeFileSelectionClass GnomeFileSelectionClass;

typedef enum {
	GNOME_FILE_SELECTION_LIST_ICONS,
	GNOME_FILE_SELECTION_LIST_ICONS_SMALL,
	GNOME_FILE_SELECTION_LIST_DETAILED
} GnomeFileSelectionListType;

struct _GnomeFileSelectionFilter {
	gchar *description;
	gchar *pattern;
};

struct _GnomeFileSelection {
	GnomeDialog dialog;

	GtkWidget *directory_combo;

	GtkWidget *paned;
	GtkWidget *directory_clist;
	GtkWidget *file_clist;
	GtkWidget *file_icon_list;
	GtkWidget *file_scrolled_window;

	GtkWidget *selection_text;
	GtkWidget *selection_entry;

	GtkWidget *back_button;
	GtkWidget *forward_button;
	GtkWidget *up_button;
	GtkWidget *rescan_button;
	GtkWidget *home_button;

	GtkWidget *filter_combo;
	GtkWidget *dotfiles_check_button;

	gchar *directory;

	GnomeFileSelectionListType list_type;

	GList *filter_list;           /* GnomeFileSelectionFilter */
	GList *current_filter;

	gboolean show_dotfiles : 1;

	GnomeFileSelectionPrivate *priv;
};

struct _GnomeFileSelectionClass {
	GnomeDialogClass parent_class;
};


GtkType gnome_file_selection_get_type (void);
GtkWidget *gnome_file_selection_new (const gchar *title);
void gnome_file_selection_set_list_type (GnomeFileSelection *fs,
                                         GnomeFileSelectionListType type);
void gnome_file_selection_show_dotfiles (GnomeFileSelection *fs,
                                         gboolean enable);
void gnome_file_selection_append_filter (GnomeFileSelection *fs,
                                         const gchar *description,
                                         const gchar *pattern);

#endif
