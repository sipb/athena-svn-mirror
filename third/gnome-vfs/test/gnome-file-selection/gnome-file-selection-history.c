#include <stdio.h>              /* FIXME bugzilla.eazel.com 1127: We should remove this debug code some day. */

#include "gnome-file-selection-history.h"
#include <string.h>


static guint get_offset (GnomeFileSelectionHistory *h);
static void print_history (GnomeFileSelectionHistory *h);

static guint
get_offset (GnomeFileSelectionHistory *h)
{
  guint offset;

  if (h->ptr >= h->head)
    offset = h->ptr - h->head;
  else
    offset = h->ptr + h->size - h->head;

  return offset;
}

/* FIXME bugzilla.eazel.com 1127: We should remove this debug code some day. */
static void
print_history (GnomeFileSelectionHistory *h)
{
  guint ptr, i;

  printf ("History is:\n");
  for (ptr = h->head, i = 0; i < h->num; i++, ptr = (ptr + 1) % h->size)
    {
      printf ("  %d: %s %s\n",
              ptr,
              ptr == h->ptr ? "=>" : "  ",
              h->values[ptr]);
    }
}



GnomeFileSelectionHistory *
gnome_file_selection_history_new (guint size)
{
  GnomeFileSelectionHistory *new;
  guint i;

  g_return_val_if_fail (size > 0, NULL);

  new = g_new (GnomeFileSelectionHistory, 1);
  new->num = 0;
  new->ptr = 0;
  new->head = 0;
  new->size = size;

  new->values = g_new (gchar *, new->size);
  for (i = 0; i < new->size; i++)
    new->values[i] = NULL;

  return new;
}

gchar *
gnome_file_selection_history_get_current (GnomeFileSelectionHistory *h)
{
  if (h->num == 0)
    return NULL;
  
  return h->values[h->ptr];
}

gboolean
gnome_file_selection_history_can_back (GnomeFileSelectionHistory *h)
{
  return h->ptr != h->head;
}

gchar *
gnome_file_selection_history_back (GnomeFileSelectionHistory *h)
{
  if (gnome_file_selection_history_can_back (h))
    {
      if (h->ptr > 0)
        h->ptr--;
      else
        h->ptr = h->size - 1;
      print_history (h);
      return gnome_file_selection_history_get_current (h);
    }
  else
    return NULL;
}

gboolean
gnome_file_selection_history_can_forward (GnomeFileSelectionHistory *h)
{
  guint offset;

  offset = get_offset (h);

  return offset < h->num -1;
}

gchar *
gnome_file_selection_history_forward (GnomeFileSelectionHistory *h)
{
  guint offset;

  offset = get_offset (h);
  if (offset < h->num - 1)
    {
      h->ptr = (h->ptr + 1) % h->size;
      print_history (h);
      return gnome_file_selection_history_get_current (h);
    }
  else
    return NULL;
}

gboolean
gnome_file_selection_history_add (GnomeFileSelectionHistory *h,
                                  const gchar *value)
{
  guint offset;
  guint next_ptr;

  offset = get_offset (h);

  if (offset < h->num && strcmp (value, h->values[h->ptr]) == 0)
    return TRUE;

  if (h->num == 0)
    next_ptr = h->head;
  else
    next_ptr = (h->ptr + 1) % h->size;

  if (offset + 1 < h->num && strcmp (value, h->values[next_ptr]) == 0)
    {
      h->ptr = next_ptr;
      return TRUE;
    }

  if (offset == h->size - 1)
    {
      h->head = (h->head + 1) % h->size;
      h->ptr = (h->ptr + 1) % h->size;
      g_free (h->values[h->ptr]);
      h->values[h->ptr] = g_strdup (value);
    }
  else
    {
      guint ptr;
      guint i;

      for (i = offset + 1, ptr = next_ptr;
           i < h->num;
           i++, ptr = (ptr + 1) % h->size)
        {
          g_free (h->values[ptr]);
          h->values[ptr] = NULL;
        }

      h->values[next_ptr] = g_strdup (value);
      h->ptr = next_ptr;
    }

  if (h->ptr >= h->head)
    h->num = h->ptr - h->head + 1;
  else
    h->num = h->ptr + (h->size - h->head) + 1;

  print_history (h);

  return TRUE;
}
