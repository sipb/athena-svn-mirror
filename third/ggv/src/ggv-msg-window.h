/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * ggv-msg-window.h: display of GhostScript's output
 *
 * Copyright (C) 2001, 2002 the Free Software Foundation
 */
#ifndef __GGV_MSG_WINDOW_H__
#define __GGV_MSG_WINDOW_H__

#include <gtk/gtk.h>

typedef struct _GgvMsgWindow GgvMsgWindow;

struct _GgvMsgWindow {
        GtkWidget *window;
        GtkWidget *error_text;
};

GgvMsgWindow *ggv_msg_window_new(const gchar *title);
void ggv_msg_window_free(GgvMsgWindow *win);
void ggv_msg_window_init(GgvMsgWindow *win, const gchar *title);
void ggv_msg_window_show(GgvMsgWindow *win);
void ggv_msg_window_add_text(GgvMsgWindow *win, const gchar *text, gint show);

#endif
