#ifndef __DOM_EVENT_UTILS_H__
#define __DOM_EVENT_UTILS_H__

#include <libgtkhtml/dom/events/dom-mutationevent.h>
#include <libgtkhtml/dom/events/dom-styleevent.h>
#include <libgtkhtml/dom/events/dom-mouseevent.h>

DomEventListener *dom_event_listener_signal_new (void);

typedef enum {
	DOM_EVENT_TRAVERSER_PRE_ORDER,
	DOM_EVENT_TRAVERSER_POST_ORDER
} DomEventTraverserType;


void
dom_MutationEvent_invoke (DomEventTarget *target, const gchar *eventType, DomBoolean canBubble, DomBoolean cancelable, DomNode *relatedNode, const gchar *prevValue, const gchar *newValue, const gchar *attrName, gushort attrChange);

void
dom_Event_invoke (DomEventTarget *target, const gchar *eventType, DomBoolean canBubble, DomBoolean cancelable);

void dom_StyleEvent_invoke (DomEventTarget *target, const gchar *eventType, DomBoolean canBubble, DomBoolean cancelable, gushort styleChange);

void dom_MutationEvent_invoke_recursively (DomEventTarget *target, const gchar *eventType, DomBoolean canBubble, DomBoolean cancelable,
					   DomNode *relatedNode, const gchar *prevValue, const gchar *newValue, const gchar *attrName, gushort attrChange, DomEventTraverserType traverser_type);

gboolean dom_MouseEvent_invoke (DomEventTarget *target, const gchar *eventType, gboolean canBubble, gboolean cancelable, DomAbstractView *viewArg, glong detailArg, glong screenXArg, glong screenYArg, glong clientXArg, glong clientYArg, DomBoolean ctrlKeyArg, DomBoolean altKeyArg, DomBoolean shiftKeyArg, DomBoolean metaKeyArg, gushort buttonArg, DomEventTarget *relatedTargetArg);

DomEventListener *dom_event_listener_signal_new (void);


#endif /* __DOM_EVENT_UTILS_H__ */
