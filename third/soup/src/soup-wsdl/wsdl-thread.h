/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-thread.h: Thread together all the pieces of the WSDL document
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_THREAD_H_
#define _WSDL_THREAD_H_

#include "wsdl-parse.h"

int wsdl_thread (wsdl_definitions *definitions);

#endif /* _WSDL_THREAD_H_ */
