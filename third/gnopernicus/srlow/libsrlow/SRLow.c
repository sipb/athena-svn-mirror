/* SRLow.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include "SRLow.h"
#include "glib.h"
#include "cspi/spi.h"
#include "SRObject.h"
#include "string.h"
#include  "gdk/gdk.h"

#define	SRL_MAX_CLIENTS			1
#define	SRL_LOG
/* #define SRL_TIMING */

#define srl_check_uninitialized() 	(!srl_initialized)
#define srl_check_initialized() 	(srl_initialized)
#define srl_set_uninitialized() 	srl_initialized = FALSE
#define srl_set_initialized() 		srl_initialized = TRUE

#define SRL_STR(X) 	X ? X : ""
#define SRL_PTR(X)	(unsigned int)X


#define SRL_EVENT_NAME_FOCUS			"focus:"
#define SRL_EVENT_NAME_LINK_SELECTED		"object:link-selected"
#define SRL_EVENT_NAME_TEXT_CARET_MOVED		"object:text-caret-moved"
#define SRL_EVENT_NAME_TEXT_CHANGED_INSERT	"object:text-changed:insert"
#define SRL_EVENT_NAME_TEXT_CHANGED_DELETE	"object:text-changed:delete"
#define SRL_EVENT_NAME_TEXT_SELECTION_CHANGED	"object:text-selection-changed"
#define SRL_EVENT_NAME_VALUE_CHANGED		"object:property-change:accessible-value"
#define SRL_EVENT_NAME_STATE_CHECKED		"object:state-changed:checked"
#define SRL_EVENT_NAME_STATE_SELECTED		"object:state-changed:selected"
#define SRL_EVENT_NAME_STATE_EXPANDED		"object:state-changed:expanded"
#define SRL_EVENT_NAME_STATE_SHOWING		"object:state-changed:showing"
#define SRL_EVENT_NAME_SELECTION_CHANGED	"object:selection-changed"
#define SRL_EVENT_NAME_WINDOW_CREATE		"window:create"
#define SRL_EVENT_NAME_WINDOW_DESTROY		"window:destroy"
#define SRL_EVENT_NAME_WINDOW_MINIMIZE		"window:minimize"
#define SRL_EVENT_NAME_WINDOW_MAXIMIZE		"window:maximize"
#define SRL_EVENT_NAME_WINDOW_RESTORE		"window:restore"
#define SRL_EVENT_NAME_WINDOW_ACTIVATE		"window:activate"
#define SRL_EVENT_NAME_WINDOW_DEACTIVATE	"window:deactivate"
#define SRL_EVENT_NAME_MOUSE_ABS		"mouse:abs"
#define SRL_EVENT_NAME_CONTENT_CHANGED		"object:property-changed:accessible-content"
#define SRL_EVENT_NAME_WINDOW_SWITCH		"window:switch"
#define SRL_EVENT_NAME_TOOLTIP_SHOW		"tooltip:show"
#define SRL_EVENT_NAME_TOOLTIP_HIDE		"tooltip:hide"
#define SRL_EVENT_NAME_WINDOW_TITLELIZE		"window:titlelize"
#define SRL_EVENT_NAME_WINDOW_RENAME		"window:rename"
#define SRL_EVENT_NAME_UNKNOWN			"unknown"
#define SRL_EVENT_NAME_ACTIVE_DESCENDANT_CHANGED "object:active-descendant-changed"
#define SRL_EVENT_NAME_NAME_CHANGED		"object:property-change:accessible-name"
#define SRL_EVENT_NAME_VISIBLE_DATA_CHANGED	"object:visible-data-changed"
#define SRL_EVENT_NAME_CHILDREN_ADDED           "object:children-changed:add"
#define SRL_EVENT_NAME_CHILDREN_REMOVED         "object:children-changed:remove"
#define SRL_EVENT_NAME_TAB_ADDED                "object:tab-added"
#define SRL_EVENT_NAME_TAB_REMOVED              "object:tab-removed"
#define SRL_EVENT_NAME_CONTEXT_SWITCHED         "object:context-switched"

#define SRL_TOOLKIT_MOZILLA			"mozilla"
#define SRL_APP_METACITY			"metacity"

typedef enum _SRLEventType
{
    SRL_EVENT_UNKNOWN = 0,
    SRL_EVENT_FOCUS,
    SRL_EVENT_FOCUS1, /* some events are reported as "focus:",
			but all other events for those objects 
			should not be reported. (eg ALT+CTRL+TAB) */
    SRL_EVENT_FOCUS2, /* some text object are not focused, but user can 
			interact with them */
    SRL_EVENT_LINK_SELECTED,
    SRL_EVENT_TEXT_CARET_MOVED,
    SRL_EVENT_TEXT_CHANGED_INSERT,
    SRL_EVENT_TEXT_CHANGED_DELETE,
    SRL_EVENT_TEXT_SELECTION_CHANGED,
    SRL_EVENT_VALUE_CHANGED,
    SRL_EVENT_STATE_CHECKED,
    SRL_EVENT_STATE_SELECTED, 
    SRL_EVENT_STATE_EXPANDED,
    SRL_EVENT_STATE_SHOWING,
    SRL_EVENT_SELECTION_CHANGED,
    SRL_EVENT_WINDOW_CREATE,
    SRL_EVENT_WINDOW_DESTROY,
    SRL_EVENT_WINDOW_MINIMIZE,
    SRL_EVENT_WINDOW_MAXIMIZE,
    SRL_EVENT_WINDOW_RESTORE,
    SRL_EVENT_WINDOW_ACTIVATE,
    SRL_EVENT_WINDOW_DEACTIVATE,
    SRL_EVENT_WINDOW_TITLELIZE,
    SRL_EVENT_MOUSE_ABS,
    SRL_EVENT_TOOLTIP_SHOW,
    SRL_EVENT_TOOLTIP_HIDE,
    SRL_EVENT_ACTIVE_DESCENDANT_CHANGED,
    SRL_EVENT_NAME_CHANGED,
    SRL_EVENT_VISIBLE_DATA_CHANGED,
    SRL_EVENT_CHILDREN_ADDED,
    SRL_EVENT_CHILDREN_REMOVED,
    SRL_EVENT_TAB_ADDED,
    SRL_EVENT_TAB_REMOVED,
    SRL_EVENT_CONTEXT_SWITCHED,
    SRL_EVENT_WINDOW_RENAME
}SRLEventType;

typedef enum _SRLEventPriority
{
    SRL_EVENT_PRIORITY_MOUSE,
    SRL_EVENT_PRIORITY_TOOLTIP,
    SRL_EVENT_PRIORITY_FOCUS,
    SRL_EVENT_PRIORITY_WATCHED,
    SRL_EVENT_PRIORITY_WINDOW,
    SRL_EVENT_PRIORITY_LAST,
    SRL_EVENT_PRIORITY_UNKNOWN = SRL_EVENT_PRIORITY_LAST
}SRLEventPriority;

typedef struct _SRLEventTypeEventName
{
    SRLEventType type;
    gchar 	 *name;
}SRLEventTypeEventName;

static SRLEventTypeEventName srl_events_type_name[] =
    {
	{SRL_EVENT_FOCUS, 			SRL_EVENT_NAME_FOCUS},
	{SRL_EVENT_LINK_SELECTED,		SRL_EVENT_NAME_LINK_SELECTED},
	{SRL_EVENT_TEXT_CARET_MOVED,		SRL_EVENT_NAME_TEXT_CARET_MOVED},
	{SRL_EVENT_TEXT_CHANGED_INSERT,		SRL_EVENT_NAME_TEXT_CHANGED_INSERT},
	{SRL_EVENT_TEXT_CHANGED_DELETE,		SRL_EVENT_NAME_TEXT_CHANGED_DELETE},
	{SRL_EVENT_TEXT_SELECTION_CHANGED,	SRL_EVENT_NAME_TEXT_SELECTION_CHANGED},
	{SRL_EVENT_VALUE_CHANGED,		SRL_EVENT_NAME_VALUE_CHANGED},
	{SRL_EVENT_STATE_CHECKED,		SRL_EVENT_NAME_STATE_CHECKED},
	{SRL_EVENT_STATE_SELECTED,		SRL_EVENT_NAME_STATE_SELECTED},
	{SRL_EVENT_STATE_EXPANDED,		SRL_EVENT_NAME_STATE_EXPANDED},
	{SRL_EVENT_STATE_SHOWING,		SRL_EVENT_NAME_STATE_SHOWING},
	{SRL_EVENT_SELECTION_CHANGED,		SRL_EVENT_NAME_SELECTION_CHANGED},
	{SRL_EVENT_WINDOW_CREATE,		SRL_EVENT_NAME_WINDOW_CREATE},
	{SRL_EVENT_WINDOW_DESTROY,		SRL_EVENT_NAME_WINDOW_DESTROY},
	{SRL_EVENT_WINDOW_MINIMIZE,		SRL_EVENT_NAME_WINDOW_MINIMIZE},
	{SRL_EVENT_WINDOW_MAXIMIZE,		SRL_EVENT_NAME_WINDOW_MAXIMIZE},
	{SRL_EVENT_WINDOW_RESTORE,		SRL_EVENT_NAME_WINDOW_RESTORE},
	{SRL_EVENT_WINDOW_ACTIVATE,		SRL_EVENT_NAME_WINDOW_ACTIVATE},
	{SRL_EVENT_WINDOW_DEACTIVATE,		SRL_EVENT_NAME_WINDOW_DEACTIVATE},
	{SRL_EVENT_MOUSE_ABS,			SRL_EVENT_NAME_MOUSE_ABS},
	{SRL_EVENT_ACTIVE_DESCENDANT_CHANGED, 	SRL_EVENT_NAME_ACTIVE_DESCENDANT_CHANGED},
	{SRL_EVENT_NAME_CHANGED,		SRL_EVENT_NAME_NAME_CHANGED},
	{SRL_EVENT_VISIBLE_DATA_CHANGED,	SRL_EVENT_NAME_VISIBLE_DATA_CHANGED},
	{SRL_EVENT_CHILDREN_ADDED,              SRL_EVENT_NAME_CHILDREN_ADDED},
	{SRL_EVENT_CHILDREN_REMOVED,            SRL_EVENT_NAME_CHILDREN_REMOVED}
    };

typedef struct _SRLEvent
{
    SRLEventType    type;
    Accessible	    *acc;
    AccessibleEvent *acc_ev;
}SRLEvent;

typedef struct _SRLLastInfo
{
    SRLong	text_char_count;
    SRLong	text_caret_offset;
    SRLong	text_selections_count;
    SRLong	text_crt_selection_length;
    gdouble	value_crt_value;
}SRLLastInfo;

typedef gboolean (*SRLNotify) (SRLEvent *event, SREventType type);

static SRLClient 		srl_clients[SRL_MAX_CLIENTS];
static SRLEvent			*srl_last_events[SRL_EVENT_PRIORITY_LAST];
static gboolean			srl_initialized		= FALSE;
static gboolean			srl_idle_need		= FALSE;
static gboolean			srl_idle_installed	= FALSE;
static GQueue			*srl_event_queue	= NULL;
static AccessibleEventListener 	*srl_event_listeners[G_N_ELEMENTS (srl_events_type_name)];
static Accessible		*srl_last_focus		= NULL;
static Accessible		*srl_last_focus2	= NULL;
static Accessible		*srl_watched_acc	= NULL;
static Accessible 		*srl_last_table 	= NULL;
static Accessible 		*srl_last_context 	= NULL;
static Accessible		*srl_last_create	= NULL;
/*FIXME:next varaible should be in SRObject.c file */
Accessible 			*srl_last_edit		= NULL;

extern gboolean sro_set_difference (SRObject *obj, gchar *difference);
extern gboolean sro_set_name (SRObject *obj, gchar *name);
extern gboolean sro_get_from_accessible_event (Accessible *acc, gchar *reason, 
						SRObject **obj);
extern Accessible* sro_get_acc (SRObject *obj);
extern gint sr_acc_get_link_index (Accessible *acc);

#define sro_get_from_acc	sro_get_from_accessible_event

#ifdef SRL_TIMING

#define SRL_TIMING_PROCESS_TIMER	0
#define SRL_TIMING_REPORT_TIMER		1
#define SRL_TIMING_LAST_TIMER		SRL_TIMING_REPORT_TIMER

typedef struct _STRLTimingTimer
{
    gchar *id;
    GTimer *timer;
    gpointer data;
}STRLTimingTimer;

static STRLTimingTimer srl_timing_timers[] =
{
    { "process", NULL, NULL},
    { "report" , NULL, NULL}
};

static void
srl_timing_show_data (gchar *id,
		      GTimer *timer,
		      gpointer *data)
{
    gulong ms;
    gdouble s, res;

    srl_assert (id && timer);

    s = g_timer_elapsed (timer, &ms);
    res = s *1000000.0 + ms;
    fprintf (stderr, "\n%s for %xp event takes %.2f microseconds",
		    id, SRL_PTR (data), res);
}

static void
srl_timing_timer_start (gint index,
			gpointer data)
{
    srl_assert (0 <= index && index < G_N_ELEMENTS (srl_timing_timers));
    srl_assert (srl_timing_timers[index].timer);

    srl_timing_timers[index].data = data;
    g_timer_start (srl_timing_timers[index].timer);
}

static void
srl_timing_timer_stop (gint index)
{
    srl_assert (0 <= index && index < G_N_ELEMENTS (srl_timing_timers));
    srl_assert (srl_timing_timers[index].timer);

    g_timer_stop (srl_timing_timers[index].timer);
    srl_timing_show_data (srl_timing_timers[index].id,
		srl_timing_timers[index].timer, srl_timing_timers[index].data);
}

static void
srl_timing_process_start (gpointer data)
{
    srl_timing_timer_start (SRL_TIMING_PROCESS_TIMER, data);
}

static void
srl_timing_process_stop ()
{
    srl_timing_timer_stop (SRL_TIMING_PROCESS_TIMER);
}

static void
srl_timing_report_start (gpointer data)
{
    srl_timing_timer_start (SRL_TIMING_REPORT_TIMER, data);
}

static void
srl_timing_report_stop ()
{
    srl_timing_timer_stop (SRL_TIMING_REPORT_TIMER);
}

static gboolean
srl_timing_init ()
{
    gint i;

    srl_assert (SRL_TIMING_LAST_TIMER + 1 == G_N_ELEMENTS (srl_timing_timers));

    for (i = 0; i <= SRL_TIMING_LAST_TIMER; i++)
    {
	srl_timing_timers[i].timer = g_timer_new ();
	srl_timing_timers[i].data = NULL;
    }

    return TRUE;
}
static void
srl_timing_terminate ()
{
    gint i;

    for (i = 0; i <= SRL_TIMING_LAST_TIMER; i++)
	g_timer_destroy (srl_timing_timers[i].timer);
}
#else /* SRL_TIMING */
#define srl_timing_process_start(X)
#define srl_timing_process_stop()
#define srl_timing_report_start(X)
#define srl_timing_report_stop()
#define srl_timing_init()
#define srl_timing_terminate()
#endif /* SRL_TIMING */

#ifdef SRL_LOG

#define SRL_LOG_AT_SPI		1
#define SRL_LOG_GNOPERNICUS	2
#define SRL_LOG_IMPORTANT	4
#define SRL_LOG_TERMINAL	8
#define SRL_LOG_REENTRANCY	16

#define srl_log_is_at_spi_event_logged()	(srl_log_flags & SRL_LOG_AT_SPI)
#define srl_log_is_terminal_event_logged()	(srl_log_flags & SRL_LOG_TERMINAL)
#define srl_log_is_gnopernicus_event_logged()	(srl_log_flags & SRL_LOG_GNOPERNICUS)
#define srl_log_is_important_event_logged()	(srl_log_flags & SRL_LOG_IMPORTANT)
#define srl_log_is_reentrancy_event_logged()	(srl_log_flags & SRL_LOG_REENTRANCY)

static gint srl_log_flags = 0;

static gchar* srl_acc_get_toolkit_name (const Accessible *acc);

static void
srl_log_at_spi_event (const AccessibleEvent *event)
{
    gchar *name, *role, *toolkit;

    srl_assert (event);

    if (!srl_log_is_at_spi_event_logged ())
	return;
    if (!srl_log_is_terminal_event_logged () && 
	    Accessible_getRole (event->source) == SPI_ROLE_TERMINAL)
	return;

    name = Accessible_getName (event->source),
    role = Accessible_getRoleName (event->source);
    toolkit = srl_acc_get_toolkit_name (event->source);
    fprintf (stderr, "\nAT:%xp----\"%s\" "
			"for %xp \"%s\" role \"%s\" "
			"from \"%s\" with details %ld and %ld", 
		SRL_PTR (event), event->type, SRL_PTR (event->source), 
		SRL_STR (name), SRL_STR (role), SRL_STR (toolkit),
		event->detail1, event->detail2);
    SPI_freeString (name);
    SPI_freeString (role);
    SPI_freeString (toolkit);
}

static void
srl_log_gnopernicus_event_user_obj (SRLEvent *event,
				    SREvent *ev)
{
    SRObject *obj;
    gchar *name, *role, *type;

    srl_assert (event && ev);

    if (!srl_log_is_gnopernicus_event_logged ())
	return;
    if (!srl_log_is_terminal_event_logged () && 
	    Accessible_getRole (event->acc_ev->source) == SPI_ROLE_TERMINAL)
	return;

    sre_get_event_data (ev, (void**)&obj);
    sro_get_name (obj, &name, SR_INDEX_CONTAINER);
    sro_get_role_name (obj, &role, SR_INDEX_CONTAINER);
    sro_get_reason (obj, &type);
    fprintf (stderr, "\nGN:%xp--\"%s\" "
			"for %xp \"%s\" role \"%s\" ", 
		    SRL_PTR (event), SRL_STR (type),
		    SRL_PTR (obj), SRL_STR (name), SRL_STR (role));
    SR_freeString (name);
    SR_freeString (role);
    SR_freeString (type);
}

static void
srl_log_gnopernicus_event_user_mouse (SRLEvent *event,
				      SREvent *ev)
{
    SRPoint *point;

    srl_assert (event && ev);

    if (!srl_log_is_gnopernicus_event_logged ())
	return;

    sre_get_event_data (ev, (void**)&point);
    fprintf (stderr, "\nGN:%xp--\"mouse:move\" "
			"at %d, %d", SRL_PTR (event), point->x, point->y);
}

static G_CONST_RETURN Accessible* srle_get_acc (SRLEvent *event);
static G_CONST_RETURN gchar* 	  srle_get_reason (SRLEvent *event);

static void
srl_log_important_event (SRLEvent *event)
{
    gchar *name, *role;

    srl_assert (event);

    if (!srl_log_is_important_event_logged ())
	return;
    if (!srl_log_is_terminal_event_logged () && 
	    Accessible_getRole (event->acc_ev->source) == SPI_ROLE_TERMINAL)
	return;

    name = Accessible_getName ((Accessible*)srle_get_acc (event)),
    role = Accessible_getRoleName ((Accessible*)srle_get_acc (event));
    fprintf (stderr, "\nIMP:%xp----\"%s\" "
			"for %xp \"%s\" role \"%s\" ", 
		SRL_PTR (event->acc_ev), srle_get_reason (event),
		SRL_PTR ((Accessible*)srle_get_acc (event)), 
		SRL_STR (name), SRL_STR (role));
    SPI_freeString (name);
    SPI_freeString (role);

    return;
}

static void
srl_log_reentrancy_event (const AccessibleEvent *event)
{
    if (!srl_log_is_reentrancy_event_logged ())
	return;
    if (!srl_log_is_terminal_event_logged () && 
	    Accessible_getRole (event->source) == SPI_ROLE_TERMINAL)
	return;

    fprintf (stderr, "\nQU:%xp will be really added to gnopernicus queue",
    		SRL_PTR (event));
}

static gboolean
srl_log_init ()
{
    const gchar *log;
    gchar **tokens;
    gint i;

    log = g_getenv ("GNOPERNICUS_LOG");
    if (!log)
	log = "";
    srl_log_flags = 0;
    tokens = g_strsplit (log, ":", 0);
    for (i = 0; tokens[i]; i++)
    {
	if (strcmp (tokens[i], "at-spi") == 0)
	    srl_log_flags |= SRL_LOG_AT_SPI;
	else if (strcmp (tokens[i], "gnopernicus") == 0)
	    srl_log_flags |= SRL_LOG_GNOPERNICUS;
	else if (strcmp (tokens[i], "important") == 0)
	    srl_log_flags |= SRL_LOG_IMPORTANT;
	else if (strcmp (tokens[i], "terminal") == 0)
	    srl_log_flags |= SRL_LOG_TERMINAL;
	else if (strcmp (tokens[i], "reentrancy") == 0)
	    srl_log_flags |= SRL_LOG_REENTRANCY;
	else
	    srl_warning ("Unknown value \"%s\" for \"GNOPERNICUS_LOG\".", tokens[i]);
    }
    g_strfreev(tokens);

    return TRUE;
}

static void
srl_log_terminate ()
{
}
#else /* SRL_LOG */
#define srl_log_at_spi_event(X)
#define srl_log_gnopernicus_event_user_obj(X, Y)
#define srl_log_gnopernicus_event_user_mouse(X, Y)
#define srl_log_important_event(X)
#define srl_log_reentrancy_event(X)
#define srl_log_init()
#define srl_log_terminate()
#endif /* SRL_LOG */

static SRLEvent*
srle_new ()
{
    return g_new0 (SRLEvent, 1);
}

static void
srle_free (SRLEvent *event)
{
    srl_assert (event);

    if (event->acc)
	Accessible_unref (event->acc);
    if (event->acc_ev)
    AccessibleEvent_unref (event->acc_ev);
    g_free (event);
}

static SRLEvent*
srle_dup (SRLEvent *event)
{
    SRLEvent *ev;

    srl_assert (event);

    ev = srle_new ();
    ev->type = event->type;
    if (event->acc)
    {
	ev->acc = event->acc;
	Accessible_ref (ev->acc);
    }
    ev->acc_ev = event->acc_ev;
    AccessibleEvent_ref (ev->acc_ev);

    return ev;
}

static gboolean
srle_has_type (SRLEvent *event,
	       SRLEventType type)
{
    srl_assert (event);
    srl_assert (event->type != SRL_EVENT_UNKNOWN);
    
    return event->type == type;
}

static AccessibleRole
srle_get_acc_role (SRLEvent *event)
{
    srl_assert (event);
    
    return Accessible_getRole (event->acc ? event->acc : event->acc_ev->source);
}

static gboolean
srle_acc_has_role (SRLEvent *event,
		   AccessibleRole role)
{
    srl_assert (event);
    
    return role == srle_get_acc_role (event);
}

static G_CONST_RETURN Accessible*
srle_get_acc (SRLEvent *event)
{
    srl_assert (event);
    
    return event->acc ? event->acc : event->acc_ev->source;
}

static gboolean
srl_acc_has_state (Accessible *acc,
		   AccessibleState state)
{
    gboolean rv = FALSE;
    AccessibleStateSet *states;

    srl_assert (acc);
    
    states = Accessible_getStateSet (acc);
    if (!states)
	return FALSE;
    rv = AccessibleStateSet_contains (states, state);

    AccessibleStateSet_unref (states);
    return rv;
}

static gboolean
srle_acc_has_state (SRLEvent *event,
		    AccessibleState state)
{
    return srl_acc_has_state ((Accessible*)srle_get_acc (event), state);
}

static gboolean srl_acc_has_toolkit (const Accessible *acc, const gchar *toolkit);

static gboolean
srle_set_acc (SRLEvent *event)
{
    AccessibleRole role;

    srl_assert (event);
    
    role = Accessible_getRole (event->acc_ev->source);
    if (role == SPI_ROLE_LABEL ||
	role == SPI_ROLE_TEXT)
    {
	Accessible *parent = Accessible_getParent (event->acc_ev->source);
	if (parent)
	{
	    if (Accessible_getRole (parent) == SPI_ROLE_COMBO_BOX)
		event->acc = parent;
	    else
		Accessible_unref (parent);
	}
    }
    else if (srle_has_type (event, SRL_EVENT_FOCUS) &&
	srle_acc_has_state (event, SPI_STATE_MANAGES_DESCENDANTS))
    {
	AccessibleSelection *selection;
	selection = Accessible_getSelection ((Accessible*)srle_get_acc (event));
	if (selection)
	{
	    gint i, cnt;
	    cnt = AccessibleSelection_getNSelectedChildren (selection);
	    for (i = 0; i < cnt; i++)
	    {
		Accessible *child;
		child = AccessibleSelection_getSelectedChild (selection, i);
		if (child)
		{
		    if (srl_acc_has_state (child, SPI_STATE_FOCUSED))
		    {
			event->acc = child;
			break;
		    }
		    else
			Accessible_unref (child);
		}
	    }
	    AccessibleSelection_unref (selection);
	}
    }
    else if (srle_has_type (event, SRL_EVENT_ACTIVE_DESCENDANT_CHANGED))
    {
	event->acc = AccessibleActiveDescendantChangedEvent_getActiveDescendant (event->acc_ev);
    }
    if (srle_has_type (event, SRL_EVENT_CHILDREN_ADDED) ||
        srle_has_type (event, SRL_EVENT_CHILDREN_REMOVED))
    {
	event->acc = AccessibleChildChangedEvent_getChildAccessible (event->acc_ev);
    }	
    if (srle_has_type (event, SRL_EVENT_LINK_SELECTED))
    {
	if (srl_acc_has_toolkit (srle_get_acc (event), SRL_TOOLKIT_MOZILLA))
	{
	    AccessibleHypertext *hyper;
	    hyper = Accessible_getHypertext ((Accessible *)srle_get_acc (event));
	    if (hyper)
	    {
		AccessibleHyperlink *link = AccessibleHypertext_getLink (hyper,
					    event->acc_ev->detail1);
		if (link)
		{
		    event->acc = AccessibleHyperlink_getObject (link, 0);
		    AccessibleHyperlink_unref (link);
		}
		AccessibleHypertext_unref (hyper);
	    }
	}
    }

    return TRUE;
}

static G_CONST_RETURN gchar*
srle_get_reason (SRLEvent *event)
{
    gint i;

    srl_assert (event);

    if (srle_has_type (event, SRL_EVENT_TAB_ADDED))
	return SRL_EVENT_NAME_TAB_ADDED;
    if (srle_has_type (event, SRL_EVENT_TAB_REMOVED))
	return SRL_EVENT_NAME_TAB_REMOVED;		
	
    if (srle_has_type (event, SRL_EVENT_SELECTION_CHANGED) &&
	srle_acc_has_role (event, SPI_ROLE_COMBO_BOX))
	return SRL_EVENT_NAME_CONTENT_CHANGED;
    if (srle_has_type (event, SRL_EVENT_WINDOW_ACTIVATE))
	return SRL_EVENT_NAME_WINDOW_SWITCH;		

    for (i = 0; i < G_N_ELEMENTS (srl_events_type_name); i++)
    	if (srle_has_type (event, srl_events_type_name[i].type))
	    return srl_events_type_name[i].name;

    if (srle_has_type (event, SRL_EVENT_TOOLTIP_SHOW))
	return SRL_EVENT_NAME_TOOLTIP_SHOW;
    if (srle_has_type (event, SRL_EVENT_TOOLTIP_HIDE))
	return SRL_EVENT_NAME_TOOLTIP_HIDE;
    if (srle_has_type (event, SRL_EVENT_WINDOW_TITLELIZE))
	return SRL_EVENT_NAME_WINDOW_TITLELIZE;
    if (srle_has_type (event, SRL_EVENT_LINK_SELECTED))
	return SRL_EVENT_NAME_FOCUS;
    if (srle_has_type (event, SRL_EVENT_WINDOW_RENAME))
	return SRL_EVENT_NAME_WINDOW_RENAME;
    if (srle_has_type (event, SRL_EVENT_FOCUS1))
	return SRL_EVENT_NAME_FOCUS;
    if (srle_has_type (event, SRL_EVENT_FOCUS2))
	return SRL_EVENT_NAME_FOCUS;
    if (srle_has_type (event, SRL_EVENT_CONTEXT_SWITCHED))
	return SRL_EVENT_NAME_CONTEXT_SWITCHED;

    srl_assert_not_reached ();
    return SRL_EVENT_NAME_UNKNOWN;
}

static gboolean
srl_is_window_event (SRLEvent *event)
{
    srl_assert (event);
    
    if (srle_acc_has_role (event, SPI_ROLE_TOOL_TIP) ||
	srle_acc_has_role (event, SPI_ROLE_WINDOW))
	return FALSE;
    return  event->type == SRL_EVENT_WINDOW_CREATE   ||
	    event->type == SRL_EVENT_WINDOW_DESTROY  ||
	    event->type == SRL_EVENT_WINDOW_MINIMIZE ||
	    event->type == SRL_EVENT_WINDOW_MAXIMIZE ||
	    event->type == SRL_EVENT_WINDOW_RESTORE  ||
	    event->type == SRL_EVENT_WINDOW_ACTIVATE ||
	    event->type == SRL_EVENT_WINDOW_TITLELIZE||
	    event->type == SRL_EVENT_WINDOW_DEACTIVATE ||
	    event->type == SRL_EVENT_WINDOW_RENAME ||
	    event->type == SRL_EVENT_TAB_ADDED ||
	    event->type == SRL_EVENT_CONTEXT_SWITCHED ||
	    event->type == SRL_EVENT_TAB_REMOVED;
}
static gboolean
srle_is_for_watched_acc (SRLEvent *event)
{
    srl_assert (event);
    
    return srl_watched_acc == event->acc_ev->source;
}

static gboolean
srle_is_for_focused_acc (SRLEvent *event)
{
    srl_assert (event);

    return srl_last_focus == srle_get_acc (event);
}

/*
static gboolean
srl_is_focus_event (SRLEvent *event)
{
    srl_assert (event);
    
    return  (event->type == SRL_EVENT_FOCUS		||
	    event->type == SRL_EVENT_FOCUS1		||
	    event->type == SRL_EVENT_FOCUS2		||
	    event->type == SRL_EVENT_LINK_SELECTED	||
	    event->type == SRL_EVENT_TEXT_CARET_MOVED	||
	    event->type == SRL_EVENT_TEXT_CHANGED_INSERT||
	    event->type == SRL_EVENT_TEXT_CHANGED_DELETE||
	    event->type == SRL_EVENT_TEXT_SELECTION_CHANGED||
	    event->type == SRL_EVENT_VALUE_CHANGED	||
	    event->type == SRL_EVENT_NAME_CHANGED	||
	    event->type == SRL_EVENT_STATE_CHECKED	||
	    event->type == SRL_EVENT_STATE_SELECTED	||
	    event->type == SRL_EVENT_STATE_EXPANDED	||
	    event->type == SRL_EVENT_STATE_SHOWING	||
	    event->type == SRL_EVENT_SELECTION_CHANGED	||
	    event->type == SRL_EVENT_VISIBLE_DATA_CHANGED) &&
	    srle_is_for_focused_acc (event);
}
*/

static gboolean
srl_is_watched_event (SRLEvent *event)
{
    srl_assert (event);
    
    return  (event->type == SRL_EVENT_FOCUS		||
	    event->type == SRL_EVENT_FOCUS1		||
	    event->type == SRL_EVENT_FOCUS2		||
	    event->type == SRL_EVENT_LINK_SELECTED	||
	    event->type == SRL_EVENT_TEXT_CARET_MOVED	||
	    event->type == SRL_EVENT_TEXT_CHANGED_INSERT||
	    event->type == SRL_EVENT_TEXT_CHANGED_DELETE||
	    event->type == SRL_EVENT_TEXT_SELECTION_CHANGED||
	    event->type == SRL_EVENT_VALUE_CHANGED	||
	    event->type == SRL_EVENT_NAME_CHANGED	||
	    event->type == SRL_EVENT_STATE_CHECKED	||
	    event->type == SRL_EVENT_STATE_SELECTED	||
	    event->type == SRL_EVENT_STATE_EXPANDED	||
	    event->type == SRL_EVENT_STATE_SHOWING	||
	    event->type == SRL_EVENT_SELECTION_CHANGED	||
	    event->type == SRL_EVENT_VISIBLE_DATA_CHANGED) &&
	    srle_is_for_watched_acc (event);
}

static gboolean
srl_is_tooltip_event (SRLEvent *event)
{
    srl_assert (event);
    
    return  event->type == SRL_EVENT_TOOLTIP_SHOW;
}

static gboolean
srl_is_mouse_event (SRLEvent *event)
{
    srl_assert (event);
    
    return  event->type == SRL_EVENT_MOUSE_ABS;
}

static SRLEventPriority
srle_get_priority (SRLEvent *event)
{
    srl_assert (event);
	
    if (srl_is_watched_event (event))
	return SRL_EVENT_PRIORITY_WATCHED;
    if (srl_is_window_event (event))
	return SRL_EVENT_PRIORITY_WINDOW;
    if (srl_is_mouse_event (event))
	return SRL_EVENT_PRIORITY_MOUSE;
    if (srl_is_tooltip_event (event))
	return SRL_EVENT_PRIORITY_TOOLTIP;
    return SRL_EVENT_PRIORITY_FOCUS;
}

static gboolean
srl_notify_all_clients (SREvent *event,
			unsigned long flags)
{
    gint i;

    srl_assert (event);

    for (i = 0; i < SRL_MAX_CLIENTS; i++)
        if (srl_clients[i].event_proc)
	    srl_clients[i].event_proc (event, flags);

    return TRUE;
}

static gboolean
srl_set_last_table (Accessible *acc)
{
    if (srl_last_table)
	Accessible_unref (srl_last_table);
    srl_last_table = acc;
    if (srl_last_table)
	Accessible_ref (srl_last_table);
    return TRUE;
}

static gboolean
srl_table_same_line (Accessible *acc)
{
    static gint last_row = -1;
    Accessible *parent;
    gboolean rv = FALSE;

    srl_assert (acc);

    parent = Accessible_getParent (acc);
    if (parent)
    {
	AccessibleTable *table= Accessible_getTable (parent);
	if (table)
	{
	    gint row = AccessibleTable_getRowAtIndex (table, 
				    Accessible_getIndexInParent (acc));
	    if (row == last_row && parent == srl_last_table)
	        rv = TRUE;
	    last_row = row;
	    AccessibleTable_unref (table);
	}
	Accessible_unref (parent);
    }
    return rv;
}


static gboolean
srl_table_same_header (Accessible *acc)
{
    Accessible *parent;
    gboolean rv = FALSE;

    srl_assert (acc);

    parent = Accessible_getParent (acc);
    if (parent)
    {
	if (parent == srl_last_table)
	    rv = TRUE;
	Accessible_unref (parent);
    }
    return rv;
}

static gboolean
srl_notify_clients_obj (SRLEvent *event,
			SREventType type)
{
    SREvent *ev;
    SRObject *obj;
    
    srl_assert (event);

    ev = sre_new ();
    if (!ev)
	return FALSE;
    if (!sro_get_from_acc ((Accessible*)srle_get_acc (event),
				    (gchar*)srle_get_reason (event), &obj))
	obj = NULL;

    if (obj)
    {
	SRObjectRoles role;
	sro_get_role (obj, &role, SR_INDEX_CONTAINER);
	
	if ((role == SR_ROLE_TABLE_LINE || role == SR_ROLE_TABLE_COLUMNS_HEADER) &&
	    srle_has_type (event, SRL_EVENT_FOCUS))
	{
	    SRObject *pobj = NULL;
	    Accessible *pacc;
 
	    pacc = Accessible_getParent ((Accessible*)srle_get_acc (event));

	    if (pacc != srl_last_table)
	    {
		if (pacc)
		    if (!sro_get_from_acc (pacc, SRL_EVENT_NAME_FOCUS, &pobj))
			pobj = NULL;
		if (pobj)
		{
		    SREvent *pev = NULL;
		    pev = sre_new ();
		    pev->type = type;
		    pev->data = pobj;
    		    pev->data_destructor = (SREventDataDestructor) sro_release_reference;
		    srl_notify_all_clients (pev, 0);
		    sre_release_reference (pev);
		}
	    }
	    if (pacc)
		Accessible_unref (pacc);
	}
    }

    if (obj)
    {
	static SRObjectRoles last_role = SR_ROLE_UNKNOWN; /* to report all headers
		in case of click from a table cell to a column header */
	SRObjectRoles role;
	sro_get_role (obj, &role, SR_INDEX_CONTAINER);
	if (srle_has_type (event, SRL_EVENT_FOCUS))
	    if ((role == SR_ROLE_TABLE_LINE && 
		    srl_table_same_line ((Accessible*)srle_get_acc (event))) ||
	        (role == SR_ROLE_TABLE_COLUMNS_HEADER && role == last_role &&
	    	    srl_table_same_header ((Accessible*)srle_get_acc (event))))
		{
		    sro_release_reference (obj);
		    if (!sro_get_from_acc ((Accessible*)srle_get_acc (event),
				SRL_EVENT_NAME_SELECTION_CHANGED, &obj))
			obj = NULL;
		}
	if (role == SR_ROLE_TABLE_LINE || role == SR_ROLE_TABLE_COLUMNS_HEADER)
	{
	    Accessible *parent = Accessible_getParent ((Accessible*)srle_get_acc (event));
	    srl_set_last_table (parent);
	    Accessible_unref (parent);
	    last_role = role;
	}
    }

    if (obj)
    {
	ev->type = type;
	ev->data = obj;
        ev->data_destructor = (SREventDataDestructor) sro_release_reference;
	if (srle_has_type (event, SRL_EVENT_TEXT_CHANGED_INSERT) ||
	    srle_has_type (event, SRL_EVENT_TEXT_CHANGED_DELETE))
	{
	    gchar *diff = AccessibleTextChangedEvent_getChangeString (event->acc_ev);
	    if (diff)
		sro_set_difference (obj, diff);
	    SPI_freeString (diff);
	}
	if (srl_is_window_event (event) && event->type != SRL_EVENT_CONTEXT_SWITCHED)
	{
	    gchar *name = AccessibleWindowEvent_getTitleString (event->acc_ev);
	    if (name)
		sro_set_name (obj, name);
	    SPI_freeString (name);
	}
/*FIXME: AccessibleTextSelectionChangedEvent_getSelectionString */
	srl_log_gnopernicus_event_user_obj (event, ev);
	srl_notify_all_clients (ev, 0);
    }

    sre_release_reference (ev);

    return obj != NULL;
}

static gboolean
srl_notify_clients_mouse (SRLEvent *event,
			  SREventType type)
{
    SRPoint *point;
    SREvent *ev;

    srl_assert (event);

    ev = sre_new ();
    point = g_new0 (SRPoint, 1);

    ev->type = type;
    ev->data = point;
    point->x = event->acc_ev->detail1;
    point->y = event->acc_ev->detail2;
    ev->data_destructor = (SREventDataDestructor) g_free;
    srl_log_gnopernicus_event_user_mouse (event, ev);
    srl_notify_all_clients (ev, 0);
    sre_release_reference (ev);

    return TRUE;
}

static gboolean
srl_report_event_from_lasts_to_clients (gint index,
				        SREventType type,
					SRLNotify notify)
{
    SRLEvent *event;

    srl_assert (0 <= index && index < SRL_EVENT_PRIORITY_LAST);
    srl_assert (notify);

    event = srl_last_events[index];
    srl_last_events[index] = NULL;
    if (event)
    {
	srl_timing_report_start ((gpointer) event);
	notify (event, type);
	srl_timing_report_stop ();
	srle_free (event);	
    }

    return TRUE;
}


static gboolean
srl_report_obj_event_to_clients (gint index,
				 SREventType type)
{
    srl_assert (0 <= index && index < SRL_EVENT_PRIORITY_LAST);

    srl_report_event_from_lasts_to_clients (index, type,
						srl_notify_clients_obj);

    return TRUE;
}

static gboolean
srl_report_mouse_event_to_clients (gint index,
				   SREventType type)
{
    srl_assert (0 <= index && index < SRL_EVENT_PRIORITY_LAST);

    srl_report_event_from_lasts_to_clients (index, type,
						srl_notify_clients_mouse);

    return TRUE;
}


static gboolean
srl_report_event_to_clients (gpointer data)
{
    srl_report_obj_event_to_clients (SRL_EVENT_PRIORITY_WINDOW, 
					    SR_EVENT_WINDOW);    
    srl_report_obj_event_to_clients (SRL_EVENT_PRIORITY_WATCHED,
					    SR_EVENT_SRO);
    srl_report_obj_event_to_clients (SRL_EVENT_PRIORITY_FOCUS,
					    SR_EVENT_SRO);
    srl_report_obj_event_to_clients (SRL_EVENT_PRIORITY_TOOLTIP,
					    SR_EVENT_TOOLTIP);
    srl_report_mouse_event_to_clients (SRL_EVENT_PRIORITY_MOUSE,
					    SR_EVENT_MOUSE);

    srl_idle_installed = srl_idle_need;
    srl_idle_need = FALSE;

    return srl_idle_installed;
}


static gboolean
srl_report_event (SRLEvent *event)
{
    SRLEventPriority priority;
    gint i;

    srl_assert (event);

    srl_log_important_event (event);
    priority = srle_get_priority (event);
    srl_assert (0 <= priority && priority < SRL_EVENT_PRIORITY_LAST);
    for (i = 0; i <= priority; i++)
    {
	SRLEvent *ev;
	ev = srl_last_events[i];
	srl_last_events[i] = NULL;
	if (ev)
	    srle_free (ev);
    }
	    
    srl_last_events[priority] = srle_dup (event);

    /* ideea is to have only one idle callback active.
       That callback should be installed only if it is really needed 
       but not yet installed */
    if (srl_idle_installed)
	srl_idle_need = TRUE;
    else
    {
	srl_idle_installed = TRUE;
	g_idle_add (srl_report_event_to_clients, NULL);
    }

    return TRUE;
}


static gchar*
srl_acc_get_toolkit_name (const Accessible *acc)
{
    Accessible *parent;
    gchar *rv = NULL;
    
    srl_return_val_if_fail (acc, NULL);
    
    parent = (Accessible*) acc;
    Accessible_ref (parent);
    
    while (parent && !Accessible_isApplication (parent))
    {
	Accessible *tmp = parent;
	parent = Accessible_getParent (parent);
	Accessible_unref (tmp);
    }
    if (parent)
    {
	AccessibleApplication *app;
	app = Accessible_getApplication (parent);
	if (app)
	{
	    rv = AccessibleApplication_getToolkitName (app);
	    AccessibleApplication_unref (app);
	}
	Accessible_unref (parent);
    }
    return rv;
}


static gboolean
srl_acc_has_toolkit (const Accessible *acc,
		     const gchar *toolkit)
{
    gboolean rv = FALSE;
    gchar *name;

    srl_assert (acc && toolkit);

    name = srl_acc_get_toolkit_name (acc);
    if (name && strcmp (toolkit, name) == 0)
	rv = TRUE;
    SPI_freeString (name);

    return rv;    
}

static gboolean
srle_is_for_focused_ancestor_acc (SRLEvent *event)
{
    Accessible *acc;
    gboolean rv = FALSE;
    srl_assert (event);

    acc = event->acc_ev->source;
    Accessible_ref (acc);
    while (acc)
    {
	Accessible *tmp;
	rv = srl_acc_has_state (acc, SPI_STATE_FOCUSED);
	if (rv)
	    break;
	tmp = acc;
	acc = Accessible_getParent (tmp);
	Accessible_unref (tmp);
    }
    if (acc)
	Accessible_unref (acc);
    return rv;
}


static gboolean
srl_event_is_for_metacity (SRLEvent *event)
{
    gboolean rv = FALSE;
    Accessible *acc;

    srl_assert (event);

    acc = (Accessible*) srle_get_acc (event);
    Accessible_ref (acc);
    while (acc && !Accessible_isApplication (acc))
    {
	Accessible *parent = Accessible_getParent (acc);
	Accessible_unref (acc);
	acc = parent;
    }
    if (acc)
    {
    	gchar *name = Accessible_getName (acc);
	if (name && strcmp (name, SRL_APP_METACITY) == 0)
	    rv = TRUE;
	SPI_freeString (name);
	Accessible_unref (acc);
    }

    return rv;
}

static gboolean
srle_change_type (SRLEvent *event)
{
    srl_assert (event);
    
    if (srle_has_type (event, SRL_EVENT_ACTIVE_DESCENDANT_CHANGED))
    {
	event->type = SRL_EVENT_FOCUS;
    }
    else if (srle_has_type (event, SRL_EVENT_STATE_SHOWING))
    {
	if (srle_acc_has_role (event, SPI_ROLE_TOOL_TIP))
	    event->type = event->acc_ev->detail1 ? 
				SRL_EVENT_TOOLTIP_SHOW : SRL_EVENT_TOOLTIP_HIDE;
    }
    else if (srle_has_type (event, SRL_EVENT_WINDOW_MINIMIZE))
    {
	if (srle_acc_has_state (event, SPI_STATE_ACTIVE))
	    event->type = SRL_EVENT_WINDOW_TITLELIZE;
    }
    else if (srle_has_type (event, SRL_EVENT_TEXT_CARET_MOVED))
    {
	static gint last_link_index = -1;
	gint new_link_index = sr_acc_get_link_index ((Accessible*)srle_get_acc (event));
/*	if (!srle_is_for_focused_acc (event) &&
	    srl_acc_has_toolkit (srle_get_acc (event), SRL_TOOLKIT_MOZILLA))
	    event->type = SRL_EVENT_FOCUS;
*/
	if (!srle_is_for_focused_acc (event) &&
	    !srle_is_for_watched_acc (event) &&
	    srle_is_for_focused_ancestor_acc (event))
	    event->type = SRL_EVENT_FOCUS2;
	if (last_link_index != new_link_index && new_link_index != -1)
	    event->type = SRL_EVENT_FOCUS;
	last_link_index = new_link_index;
    }
    else if (srle_has_type (event, SRL_EVENT_NAME_CHANGED))
    {
	if (srle_acc_has_role (event, SPI_ROLE_STATUS_BAR) &&
	    srl_event_is_for_metacity (event))
	    event->type = SRL_EVENT_FOCUS1;
	else
	{
	    Accessible *parent = Accessible_getParent (event->acc_ev->source);
	    if (parent)
	    {
		if (Accessible_isApplication (parent))
		    event->type = SRL_EVENT_WINDOW_RENAME;
		Accessible_unref (parent);
	    }
	}
    }
    else if (srle_has_type (event, SRL_EVENT_LINK_SELECTED))
    {
	if (srl_acc_has_toolkit (srle_get_acc (event), SRL_TOOLKIT_MOZILLA))
	    event->type = SRL_EVENT_FOCUS;
    }
    
    if (srle_has_type (event, SRL_EVENT_CHILDREN_ADDED))
	event->type = SRL_EVENT_TAB_ADDED;
    else if (srle_has_type (event, SRL_EVENT_CHILDREN_REMOVED))
	event->type = SRL_EVENT_TAB_REMOVED;
	
    /* Convert the "state-changed:selected" event for a _selected_ page-tab
       into a "focus:" event */
    if (srle_has_type (event, SRL_EVENT_STATE_SELECTED) &&
        srle_acc_has_role (event, SPI_ROLE_PAGE_TAB)    &&
	srl_acc_has_state (event->acc_ev->source, SPI_STATE_SELECTED))
    {
	event->type = SRL_EVENT_FOCUS;
    }	       	
    
    return TRUE;
}

static gboolean
srl_set_text_info (AccessibleText *text,
		   SRLLastInfo *info)
{
    gint i;

    srl_assert (text && info);
    
    info->text_caret_offset  	= AccessibleText_getCaretOffset (text);
    info->text_char_count    	= AccessibleText_getCharacterCount (text);
    info->text_selections_count = AccessibleText_getNSelections (text);
    info->text_selections_count = info->text_selections_count < 0 ? 
					0 : info->text_selections_count;
    info->text_crt_selection_length = 0;
    for (i = 0; i < info->text_selections_count; i++)
    {
	long int start, end;
	AccessibleText_getSelection (text, i, &start, &end);
	if (start == info->text_caret_offset || end == info->text_caret_offset)
	{
	    info->text_crt_selection_length = end - start;
	    break;
	}
    }

    return TRUE;
}

static gboolean
srl_set_value_info (AccessibleValue *value,
		    SRLLastInfo *info)
{
    srl_assert (value && info);

    info->value_crt_value = AccessibleValue_getCurrentValue (value);

    return TRUE;
}

static gboolean
srl_set_info (Accessible *acc,
	      SRLLastInfo *info)
{
    Accessible *value;
    AccessibleText *text;

    srl_assert (info && acc);
    
    value = Accessible_getValue (acc);
    if (value)
    {
	srl_set_value_info (value, info);
	AccessibleValue_unref (value);
    }

    text = Accessible_getText (acc);
    if (text)
    {
        srl_set_text_info (text, info);
        AccessibleText_unref (text);
    }

    return TRUE;
}

static gboolean
srl_combo_has_selection (const Accessible *acc)
{
    gboolean rv = FALSE;
    AccessibleSelection *selection;

    srl_assert (acc);

    selection = Accessible_getSelection ((Accessible*)acc);
    if (!selection)
	return FALSE;
    if (AccessibleSelection_getNSelectedChildren (selection) > 0)
	rv = TRUE;
    AccessibleSelection_unref (selection); 

    return rv;
}

static gboolean
srl_info_is_selection_changed (SRLLastInfo info,
			       SRLLastInfo info2)
{
    return (info2.text_selections_count != info.text_selections_count &&
	     info2.text_char_count == info.text_char_count) ||
	    (info2.text_selections_count != 0 && 
	     info2.text_crt_selection_length != info.text_crt_selection_length);
}

static gboolean
srl_info_is_caret_moved (SRLLastInfo info,
			 SRLLastInfo info2)
{
    return info2.text_selections_count == info.text_selections_count 
		&& info2.text_selections_count == 0
		&& info2.text_caret_offset != info.text_caret_offset
		&& info2.text_char_count == info.text_char_count;
}

static gboolean
srl_info_is_value_changed (SRLLastInfo info,
			  SRLLastInfo info2)
{
    return info2.value_crt_value != info.value_crt_value;
}

static gboolean
srl_text_event_is_reported (SRLEvent *event,
			    SRLLastInfo info)
{
    gboolean rv = FALSE;
    SRLLastInfo info2;

    srl_set_info (event->acc_ev->source, &info2);
/*FIXME: this code is only for debug purpose */
/*    fprintf (stderr, "\nCRT:CC=%ld SC=%ld SL=%ld CO=%ld", info2.text_char_count,
		info2.text_selections_count, info2.text_crt_selection_length,
		info2.text_caret_offset);
    fprintf (stderr, "\nOLD:CC=%ld SC=%ld SL=%ld CO=%ld", info.text_char_count,
		info.text_selections_count, info.text_crt_selection_length,
		info.text_caret_offset);
*/
    if (srle_has_type (event, SRL_EVENT_TEXT_SELECTION_CHANGED))
	rv = srl_info_is_selection_changed (info, info2);
    else if (srle_has_type (event, SRL_EVENT_TEXT_CARET_MOVED))
	rv = srl_info_is_caret_moved (info, info2);
    else
	rv = TRUE;

    if (srle_acc_has_role (event, SPI_ROLE_SPIN_BUTTON) ||
        srle_acc_has_role (event, SPI_ROLE_SLIDER))
        rv = !srl_info_is_value_changed (info, info2);

    /* when a text-changed event for child of combos is emitted,
       if there is a selection,then this change take place 
       because of changing the selected item (by pressing up or down arrow) 
       and is not really a text-changed.
       If is caret moved and is a selection, then user moved caret
       and this event should be reported*/
    if (srle_acc_has_role (event, SPI_ROLE_COMBO_BOX))
    {
	gboolean is_selection = srl_combo_has_selection (srle_get_acc (event));
	if (is_selection)
	{
	    if (srle_has_type (event, SRL_EVENT_TEXT_CARET_MOVED))
	    {
		if (info2.text_crt_selection_length == 0)
		    rv = is_selection; /*is_selection is for first caret-moved
					 if no selection */
	    }
	    else if (srle_has_type (event, SRL_EVENT_TEXT_CHANGED_DELETE))
	    {
		if (info2.text_crt_selection_length == 0)
		    rv = (event->acc_ev->detail2 == 1); /*if only one char is deleted */
		else 
		    rv = TRUE;
	    }
	    else if (srle_has_type (event, SRL_EVENT_TEXT_SELECTION_CHANGED))
		rv = TRUE;
	    else
		rv = FALSE;
	}
    }

    return rv;
}

static gboolean
srl_selection_event_is_reported (SRLEvent *event)
{
    gboolean rv = FALSE;

    srl_assert (event);

    if (srle_acc_has_role (event, SPI_ROLE_COMBO_BOX))
    {
    	static gint last_index = -1;
	static const Accessible *last_combo = NULL;

	if (srl_combo_has_selection (srle_get_acc (event)))
	{
	    if (last_combo != srle_get_acc (event))
	    	rv = TRUE;
	    else
	    {
		gint index = -1;
		AccessibleSelection *selection;

		selection = Accessible_getSelection ((Accessible*)srle_get_acc (event));
		if(selection)
		{
		    Accessible *child;
		    child = AccessibleSelection_getSelectedChild (selection, 0);
		    if (child)
		    {
			index = Accessible_getIndexInParent (child);
			Accessible_unref (child);
		    }
		    AccessibleSelection_unref (selection);
		}
		if (last_index == -1 || last_index != index)
		    rv = TRUE;
		last_index = index;
	    }
	}
	else
	    last_index = -1;
	last_combo = srle_get_acc (event);
    }

    return rv;
}



static gboolean
srl_event_is_reported (SRLEvent *event)
{
    gboolean rv = FALSE;
    static SRLLastInfo last_info_focus, last_info_watched;
    SRLLastInfo *last_info = NULL;

    srl_assert (event);
    last_info = srle_is_for_focused_acc (event) ? &last_info_focus : &last_info_watched;

    if (srl_is_window_event (event))
    {
	rv = TRUE;
	if (srle_has_type (event, SRL_EVENT_WINDOW_CREATE))
	{
	    if (srl_last_create)
		Accessible_unref (srl_last_create);
	    srl_last_create = event->acc_ev->source;
	    Accessible_ref (srl_last_create);
	}
	else
	{
	    if (srl_last_create == event->acc_ev->source)
		rv = FALSE;
	}
	if (srle_has_type (event, SRL_EVENT_WINDOW_ACTIVATE))
	{
	    if (srl_last_create != event->acc_ev->source)
	    {
		if (srl_last_create)
		    Accessible_unref (srl_last_create);
		srl_last_create = NULL;
	    }
	}
    }
    else if (srl_is_mouse_event (event)	||
	srl_is_tooltip_event (event))
	rv = TRUE;
    else if (srle_has_type (event, SRL_EVENT_FOCUS))
	rv = srle_acc_has_state (event, SPI_STATE_FOCUSED) ||
	     srle_acc_has_state (event, SPI_STATE_TRANSIENT);
    else if (srle_has_type (event, SRL_EVENT_FOCUS1))
	rv = TRUE;
    else if (srle_has_type (event, SRL_EVENT_FOCUS2))
	rv = TRUE;
    else if (srle_is_for_focused_acc (event) ||
	     srle_is_for_watched_acc (event))
    {
	if (srle_has_type (event, SRL_EVENT_STATE_EXPANDED)) 
	    rv = srle_acc_has_role (event, SPI_ROLE_TABLE_CELL);
	else if (srle_has_type (event, SRL_EVENT_STATE_CHECKED))
	    rv = srle_acc_has_role (event, SPI_ROLE_CHECK_BOX)		||
		    srle_acc_has_role (event, SPI_ROLE_RADIO_BUTTON)	||
		    srle_acc_has_role (event, SPI_ROLE_TABLE_CELL)	||
		    srle_acc_has_role (event, SPI_ROLE_TOGGLE_BUTTON)	||
		    srle_acc_has_role (event, SPI_ROLE_RADIO_MENU_ITEM)	||
		    srle_acc_has_role (event, SPI_ROLE_CHECK_MENU_ITEM);
/*FIXME: next code is needed? */
/*	else if (srle_has_type (event, SRL_EVENT_STATE_SELECTED))
	    rv = srle_acc_has_role (event, SPI_ROLE_PAGE_TAB); */
	else if (srle_has_type (event, SRL_EVENT_TEXT_CHANGED_INSERT) ||
		 srle_has_type (event, SRL_EVENT_TEXT_CHANGED_DELETE) ||
		 srle_has_type (event, SRL_EVENT_TEXT_CARET_MOVED)    ||
		 srle_has_type (event, SRL_EVENT_TEXT_SELECTION_CHANGED))
	    rv = srl_text_event_is_reported (event, *last_info);
	else if (srle_has_type (event, SRL_EVENT_SELECTION_CHANGED))
	    rv = srl_selection_event_is_reported (event);
	else if (srle_has_type (event, SRL_EVENT_NAME_CHANGED))
	    rv = TRUE;
	else if (srle_has_type (event, SRL_EVENT_VALUE_CHANGED))
	    rv = srle_acc_has_role (event, SPI_ROLE_SPIN_BUTTON) ||
		 srle_acc_has_role (event, SPI_ROLE_SLIDER);
	else if (srle_has_type (event, SRL_EVENT_VISIBLE_DATA_CHANGED))
	    rv = TRUE;
    }

    if (srle_has_type (event, SRL_EVENT_WINDOW_DEACTIVATE))
	rv = FALSE;

    if (rv && (srle_has_type (event, SRL_EVENT_FOCUS) ||
	      srle_is_for_watched_acc (event) ||
	      srle_is_for_focused_acc (event)))
    {
	srl_assert (last_info);
	if (srle_has_type (event, SRL_EVENT_TEXT_CHANGED_DELETE))
	{
	    last_info->text_caret_offset  = event->acc_ev->detail1;
	    last_info->text_selections_count  = 0;
	    last_info->text_char_count -= event->acc_ev->detail2;
	    last_info->text_crt_selection_length = 0;
	}
	else
	    srl_set_info (event->acc_ev->source, last_info);

	if (srle_is_for_watched_acc (event))
	{
	    if (last_info != &last_info_watched)
		last_info_watched = *last_info;
	}else
	{
	    if (last_info != &last_info_focus)
		last_info_focus = *last_info;
	}
    }
    
    if (srle_acc_has_role (event, SPI_ROLE_EDITBAR))
	rv = TRUE;
    
    /*made the traversal in order to receive
     "object:children-change:add" and "object:children-change:remove"
     events for SPI_ROLE_PAGE_TAB_LIST objects
     */
    if (srle_has_type (event, SRL_EVENT_FOCUS) && 
        srle_acc_has_role (event, SPI_ROLE_TERMINAL))
    {
	gint childrencnt = 0;		
        Accessible *parent = Accessible_getParent (event->acc_ev->source);
	Accessible *child = NULL;
	
        while (Accessible_getRole (parent) != SPI_ROLE_PAGE_TAB_LIST)
	    parent = Accessible_getParent (parent);
	
	if (parent)
	{   
	    gint i;
	    childrencnt = Accessible_getChildCount (parent);
	    for (i = 0; i < childrencnt; i++)
	    {
		child = Accessible_getChildAtIndex (parent, i);
	    }
	    
	    if (child)
		Accessible_unref (child);
	    Accessible_unref (parent);	
	}
    }
    if ((srle_has_type (event, SRL_EVENT_TAB_ADDED) || 
         srle_has_type (event, SRL_EVENT_TAB_REMOVED))) 
	rv = srle_acc_has_role (event, SPI_ROLE_PAGE_TAB);
	
    /*In some situations we have two 'focus:' events for a page-tab 
     (an "object:state-changed:selected" event which was converted into
      a focus event _and_ a "focus:" event) => report a single "focus:" event*/	
    if (srle_has_type (event, SRL_EVENT_FOCUS) &&
        srle_acc_has_role (event, SPI_ROLE_PAGE_TAB))
    {
	if (srl_last_focus2 == event->acc_ev->source)
	    rv = FALSE;
	else
	    rv = TRUE;    
    }	     
    
    if (srle_has_type (event, SRL_EVENT_TEXT_CARET_MOVED) &&
        (event->acc_ev->detail1 == -1 ||
	 event->acc_ev->detail2 == -1))
	rv = FALSE;			 
	
    return rv;
}

static gboolean
srl_set_last_focus (Accessible *acc)
{
    if (srl_last_focus)
	Accessible_unref (srl_last_focus);
    srl_last_focus = acc;
    if (srl_last_focus)
	Accessible_ref (srl_last_focus);
    return TRUE;
}

static gboolean
srl_set_last_focus2 (Accessible *acc)
{
    if (srl_last_focus2)
	Accessible_unref (srl_last_focus2);
    srl_last_focus2 = acc;
    if (srl_last_focus2)
	Accessible_ref (srl_last_focus2);
    return TRUE;
}

static gboolean
srl_is_label_for (Accessible *label, Accessible *acc)
{
    AccessibleRelation **relation;
    gint i;
    gboolean rv = FALSE;

    srl_assert (acc && label);

    relation = Accessible_getRelationSet (acc);
    if (!relation)
	return FALSE;
    for (i = 0; !rv && relation[i]; ++i)
    {
	if (AccessibleRelation_getRelationType (relation[i]) == SPI_RELATION_LABELED_BY)
	{
	    gint j, cnt;
	    cnt = AccessibleRelation_getNTargets (relation[i]);
	    for (j = 0; !rv && j < cnt; j++)
	    {
		Accessible *label_;
		label_ = AccessibleRelation_getTarget (relation[i], j);
		if (label_ == label)
		    rv = TRUE;
		if (label_)
		    Accessible_unref (label_);
	    }
	}
    }
    for (i = 0; relation[i]; ++i)
	AccessibleRelation_unref (relation[i]);
    g_free (relation);

    return rv;
}


#define SRL_MAX_LEVELS_UP 5
static Accessible*
srl_get_context (Accessible *acc)
{
    Accessible *context;
    gint i;

    sru_assert (acc);

    context = NULL;
    Accessible_ref (acc);
    for (i = 0; i < SRL_MAX_LEVELS_UP; i++)
    {
	Accessible *parent = Accessible_getParent (acc);
	if (parent)
	{
	    AccessibleRole role;
	    Accessible_unref (acc);
	    acc = parent;
	    role = Accessible_getRole (acc);
	    if (role == SPI_ROLE_EMBEDDED)
	    {
		context = acc;
		Accessible_ref (context);
	    }
	    if (srl_acc_has_state (acc, SPI_STATE_VERTICAL) && 
		    (role == SPI_ROLE_FILLER || role == SPI_ROLE_PANEL))
	    {
		gint cnt;
		cnt = Accessible_getChildCount (acc);
		if (cnt == 2)
		{
		    Accessible *label, *container;
		    label = Accessible_getChildAtIndex (acc, 0);
		    container = Accessible_getChildAtIndex (acc, 1);
		    if (label && container)
		    {
			if (Accessible_getRole (label) == SPI_ROLE_LABEL &&
			    Accessible_getRole (container) == SPI_ROLE_FILLER)
			{
			    gchar *name = Accessible_getName (label);
			    if (name && name[0])
				if (!srl_is_label_for (label, acc))
				{
				    context = label;
				    Accessible_ref (label);
				}
			    SPI_freeString (name);
			}
		    }
		    if (label)
		        Accessible_unref (label);
		    if (container)
		        Accessible_unref (container);
		}
	    }
	}
	else
	    break;
	if (context)
	    break;
    }
    if (acc)
	Accessible_unref (acc);
    return context;
}

static gboolean
srl_check_context_changed (SRLEvent *event)
{
    Accessible *context;

    srl_assert (event);

    context = srl_get_context (event->acc_ev->source);
    if (context)
    {
	if (context != srl_last_context)
	{
	    SRLEvent *ev;
	    if (srl_last_context)
		Accessible_unref (srl_last_context);
	    srl_last_context = context;
	    ev = srle_new ();
	    ev->acc = srl_last_context;
	    Accessible_ref (srl_last_context);
	    ev->type = SRL_EVENT_CONTEXT_SWITCHED;
	    srl_notify_clients_obj (ev, SR_EVENT_WINDOW);
	    srle_free (ev);
	}
	else
	{
	    Accessible_unref (context);
	}
    }
    else
    {
	if (srl_last_context)
	    Accessible_unref (srl_last_context);
	srl_last_context = NULL;    
    }
    return TRUE;
}


static gboolean
srl_process_event (SRLEvent *event)
{
    gboolean process = FALSE;

    srl_assert (event);

    srl_timing_process_start ((gpointer) event->acc_ev);
    srle_set_acc (event);
    srle_change_type (event);

    if (srle_has_type (event, SRL_EVENT_FOCUS))
	srl_check_context_changed (event);

    if (srle_has_type (event, SRL_EVENT_FOCUS) ||
	srle_has_type (event, SRL_EVENT_FOCUS2))
	    srl_set_last_focus ((Accessible*)srle_get_acc (event));
    else if (srle_has_type (event, SRL_EVENT_FOCUS1))
	srl_set_last_focus (NULL);

/* FIXME: 
    if (srle_has_type (event, SRL_EVENT_WINDOW_DEACTIVATE))
	report no focus if no other event */
    process = srl_event_is_reported (event);	
    
    if (srle_has_type (event, SRL_EVENT_FOCUS) ||
	srle_has_type (event, SRL_EVENT_FOCUS2))
	    srl_set_last_focus2 ((Accessible*)srle_get_acc (event));
    else if (srle_has_type (event, SRL_EVENT_FOCUS1))
	srl_set_last_focus2 (NULL);
	
    if (process)
	srl_report_event (event);

    if (process)
    {
	if (srl_last_edit)
    	    Accessible_unref (srl_last_edit);
	srl_last_edit = NULL;
	if (Accessible_isEditableText (event->acc_ev->source))
	{
	    srl_last_edit = event->acc_ev->source;
	    Accessible_ref (srl_last_edit);
	}
    }
    srl_timing_process_stop ();

    return process;
}

static void 
srl_event_listener (const AccessibleEvent *event,
		    void *data)
{
    static gboolean busy = FALSE;
    SRLEvent *ev;

    srl_assert (event && event->source);
    srl_assert (srl_event_queue);

    ev = srle_new ();
    ev->type = GPOINTER_TO_INT (data);
    ev->acc_ev = (AccessibleEvent*)event;
    AccessibleEvent_ref (ev->acc_ev);
    g_queue_push_head (srl_event_queue, ev);
    srl_log_at_spi_event (event);

    if (busy)
    {
	srl_log_reentrancy_event (event);
	return;
    }
    busy = TRUE;
    
    while (!g_queue_is_empty (srl_event_queue))
    {
	SRLEvent *ev1;
	ev1 = (SRLEvent *) g_queue_pop_tail (srl_event_queue);
	srl_process_event (ev1);
	srle_free (ev1);
    }

    busy = FALSE;
}

gboolean 
srl_init ()
{
    gboolean rv = TRUE;
    gint i;
    
    srl_assert (srl_check_uninitialized ());

    for (i = 0; i < SRL_MAX_CLIENTS; i++)
    	srl_clients[i].event_proc = NULL;
    srl_event_queue = g_queue_new ();

    for (i = 0 ; i < SRL_EVENT_PRIORITY_LAST; i++)
	srl_last_events[i] = NULL;
    srl_last_focus	= NULL;
    srl_last_focus2    	= NULL;
    srl_last_edit	= NULL;
    srl_idle_need	= FALSE;
    srl_idle_installed	= FALSE;
    srl_watched_acc	= NULL;
    srl_last_table	= NULL;
    srl_last_context 	= NULL;
    srl_last_create 	= NULL;
    
    srl_log_init ();
    srl_timing_init ();

    for (i = 0; i < G_N_ELEMENTS (srl_events_type_name); i++)
    {
	srl_event_listeners[i] = SPI_createAccessibleEventListener (srl_event_listener,
			    GINT_TO_POINTER (srl_events_type_name[i].type));
	if (srl_event_listeners[i])
	{
	    rv = SPI_registerGlobalEventListener (srl_event_listeners[i],
				srl_events_type_name[i].name);
	    if (!rv)
	    {
		gdk_beep ();
	    	srl_warning ("Cannot register a listener for event \"%s\".",
				srl_events_type_name[i].name);
	    }
	}
	else
	{
	    srl_warning ("Cannot create a listener for event \"%s\"",
				srl_events_type_name[i].name);		
	    rv = TRUE;
	}
    }
    rv = TRUE;

    if (rv)
	srl_set_initialized ();
	
    return rv;
}

gboolean 
srl_terminate ()
{
    gboolean rv = TRUE;
    gint i;
    
    srl_assert (srl_check_initialized ());

    for (i = 0; i < G_N_ELEMENTS (srl_events_type_name); i++)
    {
	SPI_deregisterGlobalEventListenerAll (srl_event_listeners[i]);
	AccessibleEventListener_unref (srl_event_listeners[i]);
    }

    while (!g_queue_is_empty (srl_event_queue))
	srle_free ((SRLEvent *) g_queue_pop_tail (srl_event_queue));
    g_queue_free (srl_event_queue);
    for (i = 0 ; i < SRL_EVENT_PRIORITY_LAST; i++)
	if (srl_last_events[i])
	    srle_free (srl_last_events[i]);
    if (srl_last_focus)
	Accessible_unref (srl_last_focus);
    if (srl_last_focus2)
	Accessible_unref (srl_last_focus2);	
    if (srl_last_edit)
        Accessible_unref (srl_last_edit);
    if (srl_watched_acc)
	Accessible_unref (srl_watched_acc);
    if (srl_last_table)
	Accessible_unref (srl_last_table);
    if (srl_last_context)
	Accessible_unref (srl_last_context);
    if (srl_last_create)
	Accessible_unref (srl_last_create);

    srl_log_terminate ();
    srl_timing_terminate ();
    srl_set_uninitialized ();

    return rv;
}

static inline gboolean 
srl_is_client_empty	(int index)
{
    return srl_clients[index].event_proc == NULL;
}

SRLClientHandle 
srl_add_client (const SRLClient *client)
{
    int i;	
    SRLClientHandle client_handle = SRL_CLIENT_HANDLE_INVALID;
	
    srl_assert (srl_check_initialized ());
    
    if (!client)
	return SRL_CLIENT_HANDLE_INVALID;

    for (i = 0; i < SRL_MAX_CLIENTS; i++)
    {
	if (srl_is_client_empty (i))
	{
	    srl_clients[i] = *client;
	    client_handle = i;
	    break;
	};	
    }	
    return client_handle;
}

gboolean 
srl_remove_client (SRLClientHandle client)
{	
    int i = client;

    srl_assert (srl_check_initialized ());
    
    if (0 <= i && SRL_MAX_CLIENTS > i )
    {
	srl_clients[i].event_proc = NULL;
	return TRUE;     
    }    
	
    return FALSE;
}


gboolean
srl_mouse_move (gint x,
		gint y)
{
    return SPI_generateMouseEvent (x, y, "abs") ? TRUE : FALSE;
}		

gboolean
srl_mouse_click (gint button)
{
    gchar action[] = "b1c";
    
    if (button == SR_MOUSE_BUTTON_LEFT)
	action[1] = '1';
    else if (button == SR_MOUSE_BUTTON_RIGHT)
	action[1] = '2';
    else
	srl_assert_not_reached ();
    
    return SPI_generateMouseEvent (-1, -1, action) ? TRUE : FALSE;
}

gboolean
srl_mouse_button_down (gint button)
{
    gchar action[] = "b1p";
    
    if (button == SR_MOUSE_BUTTON_LEFT)
	action[1] = '1';
    else if (button == SR_MOUSE_BUTTON_RIGHT)
	action[1] = '2';
    else
	srl_assert_not_reached ();
    return SPI_generateMouseEvent (-1,-1 , action) ? TRUE : FALSE;
}		

gboolean
srl_mouse_button_up (gint button)
{
    gchar action[] = "b1r";
    
    if (button == SR_MOUSE_BUTTON_LEFT)
	action[1] = '1';
    else if (button == SR_MOUSE_BUTTON_RIGHT)
	action[1] = '2';
    else
	srl_assert_not_reached ();
    
    return SPI_generateMouseEvent (-1,-1 , action) ? TRUE : FALSE;
}		


gboolean
srl_set_watch_for_object (SRObject *obj)
{
    Accessible *acc;

    srl_assert (obj);

    srl_unwatch_all_objects ();
    acc = sro_get_acc (obj);
    Accessible_ref (acc);
    if (srl_watched_acc)
	Accessible_unref (srl_watched_acc);
    srl_watched_acc = acc;

    return TRUE;
}

void
srl_unwatch_all_objects ()
{
    if (srl_watched_acc)
	Accessible_unref (srl_watched_acc);
    srl_watched_acc = NULL;
}
