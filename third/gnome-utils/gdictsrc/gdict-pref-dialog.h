#ifndef __GDICT_PREF_DIALOG_H_
#define __GDICT_PREF_DIALOG_H_

/* $Id: gdict-pref-dialog.h,v 1.1.1.3 2004-10-04 05:06:04 ghudson Exp $ */

/*
 *  Mike Hughes <mfh@psilord.com>
 *  Papadimitriou Spiros <spapadim@cs.cmu.edu>
 *  Bradford Hovinen <hovinen@udel.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict preferences window
 *
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtk/gtk.h>

#include "dict.h"
#include "gdict-pref.h"


#define GDICT_TYPE_PREF_DIALOG            (gdict_pref_dialog_get_type ())
#define GDICT_PREF_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDICT_TYPE_PREF_DIALOG, GDictPrefDialog))
#define GDICT_PREF_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GDICT_TYPE_PREF_DIALOG, GDictPrefDialogClass))
#define GDICT_IS_PREF_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDICT_TYPE_PREF_DIALOG))
#define GDICT_IS_PREF_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GDICT_TYPE_PREF_DIALOG))
#define GDICT_PREF_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GDICT_TYPE_PREF_DIALOG, GDictPrefDialogClass))


typedef struct _GDictPrefDialog        GDictPrefDialog;
typedef struct _GDictPrefDialogClass   GDictPrefDialogClass;

struct _GDictPrefDialog {
    GtkDialog       dialog;
    
    GtkTable         *table;
    GtkEntry         *server_entry;
    GtkEntry         *port_entry;
    GtkCheckButton   *smart_lookup_btn;
    GtkCheckButton   *applet_handle_btn;
    GtkWidget        *db_label;
    GtkWidget        *strat_label;
    GtkOptionMenu    *db_sel;
    GtkMenu          *db_list;
    GtkOptionMenu    *strat_sel;
    GtkMenu          *strat_list;

    dict_context_t   *context;
    dict_command_t   *get_db_cmd;
    dict_command_t   *get_strat_cmd;
    
    gchar            *database;
    guint             database_idx;
    gchar            *dfl_strat;
    guint             dfl_strat_idx;
};

struct _GDictPrefDialogClass {
    GtkDialogClass parent_class;
    
    void (*socket_error)  (GDictPrefDialog *, gchar *);
};

GType      gdict_pref_dialog_get_type   (void);

GtkWidget *gdict_pref_dialog_new        (dict_context_t *context);
void gdict_pref_dialog_destroy          (GDictPrefDialog *dialog);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GDICT_PREF_DIALOG_H_ */
