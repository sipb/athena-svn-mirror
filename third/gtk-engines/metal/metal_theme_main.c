#include <gmodule.h>
#include <gtk/gtk.h>

#include "metal_style.h"
#include "metal_rc_style.h"

G_MODULE_EXPORT void
theme_init (GTypeModule *module)
{
  metal_rc_style_register_type (module);
  metal_style_register_type (module);
}

G_MODULE_EXPORT void
theme_exit (void)
{
}

G_MODULE_EXPORT GtkRcStyle *
theme_create_rc_style (void)
{
  return GTK_RC_STYLE (g_object_new (METAL_TYPE_RC_STYLE, NULL));  
}

/* The following function will be called by GTK+ when the module
 * is loaded and checks to see if we are compatible with the
 * version of GTK+ that loads us.
 */
G_MODULE_EXPORT const gchar* g_module_check_init (GModule *module);
const gchar*
g_module_check_init (GModule *module)
{
  return gtk_check_version (GTK_MAJOR_VERSION,
			    GTK_MINOR_VERSION,
			    GTK_MICRO_VERSION - GTK_INTERFACE_AGE);
}