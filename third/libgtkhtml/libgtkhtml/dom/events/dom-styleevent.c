#include "dom-styleevent.h"

gushort
dom_StyleEvent__get_styleChange (DomStyleEvent *event)
{
	return event->styleChange;
}

void
dom_StyleEvent_initStyleEvent (DomStyleEvent *event, const DomString *typeArg, DomBoolean canBubbleArg, DomBoolean cancelableArg, gushort styleChangeArg)
{
	dom_Event_initEvent (DOM_EVENT (event), typeArg, canBubbleArg, cancelableArg);

	event->styleChange = styleChangeArg;
}

GType
dom_style_event_get_type (void)
{
	static GType dom_style_event_type = 0;

	if (!dom_style_event_type) {
		static const GTypeInfo dom_style_event_info = {
			sizeof (DomStyleEventClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			NULL,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (DomStyleEvent),
			16,   /* n_preallocs */
			NULL,
		};

		dom_style_event_type = g_type_register_static (DOM_TYPE_EVENT, "DomStyleEvent", &dom_style_event_info, 0);
	}

	return dom_style_event_type;
}

