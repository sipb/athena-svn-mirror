#include <gmodule.h>
#include "hc-rc-style.h"
#include "hc-style.h"

G_MODULE_EXPORT void
theme_init (GTypeModule *module)
{
  hc_rc_style_register_type (module);
  hc_style_register_type (module);
  gtk_rc_reparse_all;
}

G_MODULE_EXPORT void
theme_exit (void)
{
}

G_MODULE_EXPORT GtkRcStyle *
theme_create_rc_style (void)
{
  void *ptr;
  
  ptr = GTK_RC_STYLE (g_object_new (HC_TYPE_RC_STYLE, NULL));  
  return (GtkRcStyle *)ptr;
}

/* The following function will be called by GTK+ when the module
 * is loaded and checks to see if we are compatible with the
 * version of GTK+ that loads us.
*/
G_MODULE_EXPORT const gchar *g_module_check_init (GModule * module);

const gchar *

g_module_check_init (GModule * module)
{
  return gtk_check_version (GTK_MAJOR_VERSION,
			    GTK_MINOR_VERSION,
			    GTK_MICRO_VERSION - GTK_INTERFACE_AGE);
}
