#include <atk/atkobjectfactory.h>

#include "gtkhtml.h"
#include "factory.h"
#include "object.h"

static GType
gtk_html_a11y_factory_get_accessible_type (void)
{
	return G_TYPE_GTK_HTML_A11Y;
}

static AtkObject*
gtk_html_a11y_factory_create_accessible (GObject *obj)
{
	GtkWidget *widget;
	AtkObject *accessible;

	g_return_val_if_fail (GTK_IS_WIDGET (obj), NULL);

	widget = GTK_WIDGET (obj);

	accessible = gtk_html_a11y_new (widget);

	return accessible;
}

static void
gtk_html_a11y_factory_class_init (AtkObjectFactoryClass *klass)
{
	klass->create_accessible   = gtk_html_a11y_factory_create_accessible;
	klass->get_accessible_type = gtk_html_a11y_factory_get_accessible_type;
}

static GType
gtk_html_a11y_factory_get_type (void)
{
	static GType t = 0;

	if (!t) {
		static const GTypeInfo tinfo = {
			sizeof (AtkObjectFactoryClass),
			NULL, NULL, (GClassInitFunc) gtk_html_a11y_factory_class_init,
			NULL, NULL, sizeof (AtkObjectFactory), 0, NULL, NULL
		};

		t = g_type_register_static (ATK_TYPE_OBJECT_FACTORY, "GtkHTMLA11YNFactory", &tinfo, 0);
	}
	return t;
}

static int accessibility_initialized = FALSE;

void
gtk_html_accessibility_init (void)
{
	if (accessibility_initialized)
		return;

	atk_registry_set_factory_type (atk_get_default_registry (), GTK_TYPE_HTML, gtk_html_a11y_factory_get_type ());

	accessibility_initialized = TRUE;
}
