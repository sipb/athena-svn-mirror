/*  history-combo.h
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2002 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *  Authors: Damien Sandras <dsandras@seconix.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 *           Miguel Rodríguez <migrax@terra.es>
 *           De Michele Cristiano
 */

/*
 *                         history-combo.h -  description
 *                         ------------------------------
 *   begin                : Mon Jun 17 2002 (original: Thu Nov 22 2001)
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create a new history combo box GObject.
 */


#ifndef __GM_HISTORY_COMBO_H
#define __GM_HISTORY_COMBO_H

#include <glib-object.h>
#include <gtk/gtk.h>

#include <string.h>

#ifndef DISABLE_GCONF
#include <gconf/gconf-client.h>
#else
#include "win32/gconf-simu.h"
#endif

G_BEGIN_DECLS

#define GM_TYPE_HISTORY_COMBO         (gm_history_combo_get_type ())
#define GM_HISTORY_COMBO(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GM_TYPE_HISTORY_COMBO, GmHistoryCombo))
#define GM_HISTORY_COMBO_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), GM_TYPE_HISTORY_COMBO, GmHistoryComboClass))
#define GM_IS_HISTORY_COMBO(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GM_TYPE_HISTORY_COMBO))
#define GM_IS_HISTORY_COMBO_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GM_TYPE_HISTORY_COMBO))
#define GM_HISTORY_COMBO_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GM_TYPE_HISTORY_COMBO, GmHistoryComboClass))

typedef struct GmHistoryComboPrivate GmHistoryComboPrivate;

typedef struct
{
  GtkCombo parent;
  GList   *contact_list;
  
  GmHistoryComboPrivate *priv;
} GmHistoryCombo;

typedef struct
{
  GtkComboClass parent_class;
  
  /* signals */
  /* implementation */
} GmHistoryComboClass;

GType gm_history_combo_get_type (void);

/**
 * gm_history_combo_new:
 *
 * @key is the GConf key where you want to store
 * the history.
 *
 * Creates a new combo box with history.
 **/
GtkWidget *gm_history_combo_new (const char *key);

/**
 * gm_history_combo_add_entry:
 *
 * @key is the gconf key used to store the history.
 * 
 * Add a new entry to the history combo and saves it
 * in the GConf database.
 **/
void gm_history_combo_add_entry (GmHistoryCombo *combo, 
                                 const char *key,
                                 const char *new_entry);
/**
 * gm_history_combo_update:
 *
 * Updates the history list. If the GM_HISTORY_NUM key
 * has been changed it will not max show this number.
 **/
void gm_history_combo_update (GmHistoryCombo *combo);

G_END_DECLS

#endif /* __GM_HISTORY_COMBO_H */
