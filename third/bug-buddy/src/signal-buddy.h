#ifndef SIGNAL_BUDDY_H
#define SIGNAL_BUDDY_H

#include <glib.h>

typedef gboolean (*BBSignalFunc) (gint8	signal,
				 gpointer	data);
guint	bb_signal_add		(gint8		signal,
				 BBSignalFunc	function,
				 gpointer    	data);
guint   bb_signal_add_full     (gint           priority,
				 gint8          signal,
				 BBSignalFunc  function,
				 gpointer       data,
				 GDestroyNotify destroy);
void    bb_signal_notify       (gint8          signal);

#endif

