#include "signal-buddy.h"

typedef struct _SignalSource SignalSource;
struct _SignalSource {
	GSource     source;

	gint8       signal;
	guint8      index;
	guint8      shift;
};

static	guint32	signals_notified[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

static gboolean
bb_signal_prepare (GSource  *source,
		    gint     *timeout)
{
	SignalSource *ss = (SignalSource *)source;

	return signals_notified[ss->index] & (1 << ss->shift);
}

static gboolean
bb_signal_check (GSource *source)
{
	SignalSource *ss = (SignalSource *)source;

	return signals_notified[ss->index] & (1 << ss->shift);
}

static gboolean
bb_signal_dispatch (GSource     *source,
		     GSourceFunc  callback,
		     gpointer     user_data)
{
	SignalSource *ss = (SignalSource *)source;

	signals_notified[ss->index] &= ~(1 << ss->shift);

	return ((BBSignalFunc)callback) (ss->signal, user_data);
}

static GSourceFuncs signal_funcs = {
	bb_signal_prepare,
	bb_signal_check,
	bb_signal_dispatch
};

guint
bb_signal_add (gint8	      signal,
		BBSignalFunc function,
		gpointer      data)
{
	return bb_signal_add_full (G_PRIORITY_DEFAULT, signal, function, data, NULL);
}

guint
bb_signal_add_full (gint           priority,
		     gint8          signal,
		     BBSignalFunc  function,
		     gpointer       data,
		     GDestroyNotify destroy)
{
	GSource *source;
	SignalSource *ss;
	guint s = 128 + signal;

	g_return_val_if_fail (function != NULL, 0);

	source = g_source_new (&signal_funcs, sizeof (SignalSource));
	ss = (SignalSource *)source;

	ss->signal = signal;
	ss->index = s / 32;
	ss->shift = s % 32;

	g_source_set_priority (source, priority);
	g_source_set_callback (source, (GSourceFunc)function, data, destroy);
	g_source_set_can_recurse (source, TRUE);

	return g_source_attach (source, NULL);
}

void
bb_signal_notify (gint8 signal)
{
	guint index, shift;
	guint s = 128 + signal;

	index = s / 32;
	shift = s % 32;

	signals_notified[index] |= 1 << shift;
}
