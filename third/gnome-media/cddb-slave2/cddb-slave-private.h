/*
 * cddb-slave-private.h: Internal stuff
 *
 * Copyright (C) 2004 Thomas Vander Stichele
 */

#ifndef __CDDB_SLAVE_PRIVATE_H__
#define __CDDB_SLAVE_PRIVATE_H__

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

void cs_set_debug (gboolean active);
void cs_debug (const gchar *format, ...) G_GNUC_PRINTF (1, 2);

#ifdef __cplusplus
}
#endif

#endif
