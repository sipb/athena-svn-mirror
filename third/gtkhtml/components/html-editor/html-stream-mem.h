/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * html-stream-memory.h: blah
 *
 * Author: Larry Ewing <lewing@ximian.com>
 *
 * Copyright 2001, Ximian, Inc.
 */
#ifndef _HTML_STREAM_MEM_H_
#define _HTML_STREAM_MEM_H_

#include <bonobo/bonobo-stream-memory.h>

typedef struct _HTMLStreamMem      HTMLStreamMem;
typedef struct _HTMLStreamMemClass HTMLStreamMemClass;

#define HTML_STREAM_MEM_TYPE          (html_stream_mem_get_type ())
#define HTML_STREAM_MEM(o)            (GTK_CHECK_CAST ((o), HTML_STREAM_MEM_TYPE, HTMLStreamMem))
#define HTML_STREAM_MEM_CLASS(k)      (GTK_CHECK_CLASS_CAST((k), HTML_STREAM_MEM_TYPE, HTMLStreamMemClass))
#define HTML_IS_STREAM_MEM(o)         (GTK_CHECK_TYPE ((o), HTML_STREAM_MEM_TYPE))
#define HTML_IS_STREAM_MEM_CLASS(k)   (GTK_CHECK_CLASS_TYPE ((k), HTML_STREAM_MEM_TYPE))

struct _HTMLStreamMem {
	BonoboStreamMem  stream_mem;

	GtkHTMLStream   *html_stream;
};
  
struct _HTMLStreamMemClass {
	BonoboStreamMemClass parent_class;
};

GtkType         html_stream_mem_get_type     (void);
BonoboStream   *html_stream_mem_create       (GtkHTMLStream *html_stream);
HTMLStreamMem  *html_stream_mem_constuct     (HTMLStreamMem *bhtml,
					      Bonobo_Stream corba_stream,
					      GtkHTMLStream *html_stream);

#endif /* _HTML_STREAM_MEM_H_ */
