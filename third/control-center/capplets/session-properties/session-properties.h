#include <gtk/gtk.h>
#include <tree.h>

/* Create the chooser */
void chooser_build (void);

/* Functions for manipulating a list of non-session-managed
 * programs to be started.
 */

/* Startup list maintanence */
GSList *startup_list_read (const gchar *name);
void    startup_list_write (GSList *cl, const gchar *name);
GSList *startup_list_duplicate (GSList *sl);
void startup_list_free (GSList *sl);

GSList *startup_list_read_from_xml (xmlNodePtr node);
xmlNodePtr startup_list_write_to_xml (GSList *sl);

/* Display the list in a CList, and modify it */
void startup_list_update_gui (GSList *sl, GtkWidget *clist);

void startup_list_add_dialog (GSList **sl, GtkWidget *clist, GtkWidget **dialog);
void startup_list_edit_dialog (GSList **sl, GtkWidget *clist, GtkWidget **dialog);
void startup_list_delete (GSList **sl, GtkWidget *clist);
