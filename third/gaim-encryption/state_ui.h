#ifndef GE_STATE_UI_H
#define GE_STATE_UI_H


#include "gaim-encryption-config.h"

#include <gdk/gdk.h>
#include <gtk/gtkplug.h>

#include "gtkplugin.h"

#include <gtkdialogs.h>
#include <gaim.h>
#include <gtkconv.h>
#include <gtkutils.h>

void GE_set_capable_icon(GaimConversation *conv, gboolean cap);
void GE_set_rx_encryption_icon(GaimConversation *c, gboolean encrypted);
void GE_set_tx_encryption_icon(GaimConversation* c, gboolean encrypt, gboolean is_capable);

void GE_add_buttons(GaimConversation *conv);
void GE_remove_buttons(GaimConversation *conv);
void GE_pixmap_init();
void GE_error_window(const char* message);

#endif
