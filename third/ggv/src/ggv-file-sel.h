/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * ggv-file-sel.h: a file selector for ggv
 *
 * Copyright (C) 2002 the Free Software Foundation
 *
 * Author: Jaka Mocnik  <jaka@gnu.org>
 */

#ifndef __GGV_FILE_SEL_H__
#define __GGV_FILE_SEL_H__

#include <config.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS 

#define GGV_TYPE_FILE_SEL            (ggv_file_sel_get_type ())
#define GGV_FILE_SEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GGV_TYPE_FILE_SEL, GgvFileSel))
#define GGV_FILE_SEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GGV_TYPE_FILE_SEL, GgvFileSelClass))
#define GGV_IS_FILE_SEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GGV_TYPE_FILE_SEL))
#define GGV_IS_FILE_SEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GGV_TYPE_FILE_SEL))
#define GGV_FILE_SEL_GET_CLASS(o)    (G_TYPE_INSTANCE_GET_CLASS ((o), GGV_TYPE_FILE_SEL, GgvFileSelClass))

typedef struct _GgvFileSel      GgvFileSel;
typedef struct _GgvFileSelClass GgvFileSelClass;

struct _GgvFileSel 
{
        GtkFileSelection file_sel;
        gchar *uri;
};

struct _GgvFileSelClass
{
        GtkFileSelectionClass klass;
};

GType        ggv_file_sel_get_type           (void);
GtkWidget   *ggv_file_sel_new                (void);
void         ggv_file_sel_set_uri            (GgvFileSel *sel, const gchar *uri);
gchar       *ggv_file_sel_get_uri            (GgvFileSel *sel);
gchar       *ggv_file_sel_request_uri        (const gchar *title, const gchar *def_uri);

G_END_DECLS

#endif /* __GGV_FILE_SEL_H__ */
