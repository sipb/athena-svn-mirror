#ifndef GE_BLIST_H
#define GE_BLIST_H

#include <gdk/gdk.h>
#include <gtk/gtkplug.h>

#include <gtkplugin.h>
#include <blist.h>

#include "gaim-encryption-config.h"

void GE_buddy_menu_cb(GaimBlistNode* node, GList **menu, void* data);

gboolean GE_get_buddy_default_autoencrypt(const GaimAccount* account, const char* buddyname);

#endif
