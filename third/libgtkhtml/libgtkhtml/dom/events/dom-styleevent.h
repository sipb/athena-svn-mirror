#ifndef __DOM_STYLE_EVENT_H__
#define __DOM_STYLE_EVENT_H__

typedef struct _DomStyleEvent DomStyleEvent;
typedef struct _DomStyleEventClass DomStyleEventClass;

#include <glib-object.h>

#include <dom/core/dom-node.h>
#include <dom/events/dom-uievent.h>

#define DOM_TYPE_STYLE_EVENT             (dom_style_event_get_type ())
#define DOM_STYLE_EVENT(object)          (G_TYPE_CHECK_INSTANCE_CAST ((object), DOM_TYPE_STYLE_EVENT, DomStyleEvent))
#define DOM_STYLE_EVENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DOM_TYPE_STYLE_EVENT, DomStyleEventClass))
#define DOM_IS_STYLE_EVENT(object)       (G_TYPE_CHECK_INSTANCE_TYPE ((object),  DOM_TYPE_STYLE_EVENT))
#define DOM_IS_STYLE_EVENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DOM_TYPE_STYLE_EVENT))
#define DOM_STYLE_EVENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DOM_TYPE_STYLE_EVENT, DomStyleEventClass))

struct _DomStyleEvent {
	DomEvent parent;

	gushort styleChange;
};

struct _DomStyleEventClass {
	DomEventClass parent_class;
};

enum {
	DOM_STYLE_CHANGE_NONE,
	DOM_STYLE_CHANGE_REPAINT,
	DOM_STYLE_CHANGE_RELAYOUT,
	DOM_STYLE_CHANGE_RECREATE
};

GType dom_style_event_get_type (void);

gushort dom_StyleEvent__get_styleChange (DomStyleEvent *event);

void dom_StyleEvent_initStyleEvent (DomStyleEvent *event, const DomString *typeArg, DomBoolean canBubbleArg, DomBoolean cancelableArg, gushort styleChangeArg);

#endif /* __DOM_STYLE_EVENT_H__ */
