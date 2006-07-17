#ifndef GE_STATE_H
#define GE_STATE_H

#include <gdk/gdk.h>

typedef struct EncryptionState {
   gboolean incoming_encrypted;
   gboolean outgoing_encrypted;
   gboolean has_been_notified;
   gboolean is_capable;
} EncryptionState;

void GE_state_init();
void GE_state_delete();


EncryptionState* GE_get_state(const GaimAccount *account, const gchar* name);
void GE_reset_state(const GaimAccount *account, const gchar* name);

gboolean GE_get_tx_encryption(const GaimAccount *account, const gchar* name);
void     GE_set_tx_encryption(const GaimAccount *account, 
                              const gchar* name, gboolean new_state);

void     GE_set_capable(const GaimAccount *account, const gchar* name,
                        gboolean cap);

gboolean GE_has_been_notified(const GaimAccount *account, const gchar* name);
void     GE_set_notified(const GaimAccount *account, const gchar* name,
                         gboolean newstate);

void     GE_set_rx_encryption(const GaimAccount *account, const gchar* name,
                              gboolean encrypted);

gboolean  GE_get_default_notified(const GaimAccount *account, const gchar* name);

#endif
