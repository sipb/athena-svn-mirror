#ifndef __DOM_EVENTLISTENER_H__
#define __DOM_EVENTLISTENER_H__

typedef struct _DomEventListener DomEventListener;
typedef struct _DomEventListenerIface DomEventListenerIface;

#include <libgtkhtml/dom/dom-types.h>
#include <libgtkhtml/dom/events/dom-event.h>

#define DOM_TYPE_EVENT_LISTENER             (dom_event_listener_get_type ())
#define DOM_EVENT_LISTENER(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_EVENT_LISTENER, DomEventListener))
#define DOM_EVENT_LISTENER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_EVENT_LISTENER, DomEventListenerClass))
#define DOM_IS_EVENT_LISTENER(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_EVENT_LISTENER))
#define DOM_IS_EVENT_LISTENER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_EVENT_LISTENER))
#define DOM_EVENT_LISTENER_GET_IFACE(obj)  ((DomEventListenerIface *)g_type_interface_peek (((GTypeInstance *)DOM_EVENT_LISTENER (obj))->g_class, DOM_TYPE_EVENT_LISTENER))


struct _DomEventListenerIface {
	GTypeInterface g_iface;

	void (*handleEvent) (DomEventListener *listener, DomEvent *event);
};


GType dom_event_listener_get_type (void);

void dom_EventListener_handleEvent (DomEventListener *listener, DomEvent *event);

#endif /* __DOM_EVENTLISTENER_H__ */
