#include "dom-eventlistener.h"

/**
 * dom_EventListener__handleEvent:
 * @listener: a DomEventListener
 * @event: The event that's going to be handled.
 * 
 * This function is called whenever an event occurs of the typ for which the event listener was registered.
 **/
void
dom_EventListener_handleEvent (DomEventListener *listener, DomEvent *event)
{
	DOM_EVENT_LISTENER_GET_IFACE (listener)->handleEvent (listener, event);
}

GType
dom_event_listener_get_type (void)
{
	static GType dom_event_listener_type = 0;

	if (!dom_event_listener_type) {
		static const GTypeInfo dom_event_listener_info = {
			sizeof (DomEventListenerIface),
			NULL, /* base_init */
			NULL, /* base_finalize */
			NULL,
			NULL, /* class_finalize */
			NULL, /* class_data */
			0,
			0,   /* n_preallocs */
			NULL
		};

		dom_event_listener_type = g_type_register_static (G_TYPE_INTERFACE, "DomEventListener", &dom_event_listener_info, 0);
		g_type_interface_add_prerequisite (dom_event_listener_type, G_TYPE_OBJECT);
	}

	return dom_event_listener_type;
}
