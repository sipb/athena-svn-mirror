#ifndef __DOM_EVENTTARGET_H__
#define __DOM_EVENTTARGET_H__

typedef struct _DomEventTarget DomEventTarget;
typedef struct _DomEventTargetIface DomEventTargetIface;

#include <glib-object.h>

#include "dom-eventlistener.h"

#define DOM_TYPE_EVENT_TARGET             (dom_event_target_get_type ())
#define DOM_EVENT_TARGET(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_EVENT_TARGET, DomEventTarget))
#define DOM_EVENT_TARGET_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_EVENT_TARGET, DomEventTargetClass))
#define DOM_IS_EVENT_TARGET(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_EVENT_TARGET))
#define DOM_IS_EVENT_TARGET_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_EVENT_TARGET))
#define DOM_EVENT_TARGET_GET_IFACE(obj)  ((DomEventTargetIface *)g_type_interface_peek (((GTypeInstance *)DOM_EVENT_TARGET (obj))->g_class, DOM_TYPE_EVENT_TARGET))

struct _DomEventTargetIface {
	GTypeInterface g_iface;

	void (* addEventListener) (DomEventTarget *target, const DomString *type, DomEventListener *listener, DomBoolean useCapture);
	void (* removeEventListener) (DomEventTarget *target, const DomString *type, DomEventListener *listener, DomBoolean useCapture);
	DomBoolean (* dispatchEvent) (DomEventTarget *target, DomEvent *event);
};

GType dom_event_target_get_type (void);

void dom_EventTarget_addEventListener (DomEventTarget *target, const DomString *type, DomEventListener *listener, DomBoolean useCapture);
void dom_EventTarget_removeEventListener (DomEventTarget *target, const DomString *type, DomEventListener *listener, DomBoolean useCapture);
DomBoolean dom_EventTarget_dispatchEvent (DomEventTarget *target, DomEvent *event, DomException *exc);


#endif /* __DOM_EVENTTARGET_H__ */
