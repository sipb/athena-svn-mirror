/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * gsio.h: functions for document extraction
 *
 * Copyright (c) 2002 Free Software Foundation
 *
 * Author: jaKa Mocnik <jaka@gnu.org>
 */

#ifndef __GS_IO_H__
#define __GS_IO_H__

#include <gtkgs.h>

#ifdef __cplusplus
extern "C" {
#if 0 /* let's fool emacs */
}
#endif
#endif /* __cplusplus */

typedef struct _GtkGSDocSink GtkGSDocSink;

gchar *gtk_gs_get_pages(GtkGS *gs, gint *pages);
gchar *gtk_gs_get_document(GtkGS *gs);

GtkGSDocSink *gtk_gs_doc_sink_new();
void gtk_gs_doc_sink_free(GtkGSDocSink *sink);
void gtk_gs_doc_sink_write(GtkGSDocSink *sink, const gchar *buf, int len);
void gtk_gs_doc_sink_printf_v(GtkGSDocSink *sink, const gchar *fmt, va_list ap);
void gtk_gs_doc_sink_printf(GtkGSDocSink *sink, const gchar *fmt, ...);
gchar *gtk_gs_doc_sink_get_buffer(GtkGSDocSink *sink);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GS_IO_H__ */
