#include <glib.h>

#ifndef GNOME_FILE_SELECTION_HISTORY_H
#define GNOME_FILE_SELECTION_HISTORY_H

struct _GnomeFileSelectionHistory
{
  /* Current item.  */
  guint ptr;

  /* Start of the list.  */
  guint head;

  /* Number of items in the list.  */
  guint num;

  /* Size of history (maximum number of history elements).  */
  guint size;

  /* History values.  */
  gchar **values;
};
typedef struct _GnomeFileSelectionHistory GnomeFileSelectionHistory;

GnomeFileSelectionHistory *gnome_file_selection_history_new (guint size);
void     gnome_file_selection_history_destroy   (GnomeFileSelectionHistory *h);
gchar   *gnome_file_selection_history_get_current
                                                (GnomeFileSelectionHistory *h);
gboolean gnome_file_selection_history_can_back  (GnomeFileSelectionHistory *h);
gchar   *gnome_file_selection_history_back      (GnomeFileSelectionHistory *h);
gboolean gnome_file_selection_history_can_forward
                                                (GnomeFileSelectionHistory *h);
gchar   *gnome_file_selection_history_forward   (GnomeFileSelectionHistory *h);
gboolean gnome_file_selection_history_add       (GnomeFileSelectionHistory *h,
                                                 const gchar *value);
                                              
#endif
