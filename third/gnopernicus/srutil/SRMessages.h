/* SRMessages.h
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _SR_MESSAGES_H_
#define _SR_MESSAGES_H_

#include <stdio.h>
#include <stdlib.h>
#include "glib.h"

/* #define SRU_PARANOIA */

#ifdef SRU_PARANOIA
#ifdef G_HAVE_ISO_VARARGS
#define sru_info_(...)	g_log (G_LOG_DOMAIN,		\
			       G_LOG_LEVEL_INFO,	\
			       __VA_ARGS__)
#define sru_debug_(...)	g_log (G_LOG_DOMAIN,		\
			       G_LOG_LEVEL_DEBUG,	\
			       __VA_ARGS__)
#elif defined(G_HAVE_GNUC_VARARGS)
#define sru_info_(format...)	g_log (G_LOG_DOMAIN,		\
			    		G_LOG_LEVEL_INFO,	\
			    		format)
#define sru_debug_(format...)	g_log (G_LOG_DOMAIN,		\
			    		G_LOG_LEVEL_DEBUG,	\
			    		format)
#else
#define sru_info_ 	g_message
#define sru_debug_ 	g_message
#endif /* G_HAVE_ISO_VARARGS */

extern GLogLevelFlags sru_log_flags;
extern GLogLevelFlags sru_log_stack_flags;

#define sru_check_flag(flags, flag)	((flags) & (flag))
#define sru_check_log_flag(flag)	sru_check_flag (sru_log_flags, flag)
#define sru_check_log_stack_flag(flag)	sru_check_flag (sru_log_stack_flags, flag)
#define sru_prgname			g_get_prgname()

#define sru_assert(X)						\
	{							\
	    gboolean rv_ = (X) ? TRUE : FALSE;			\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_ERROR) && !rv_)\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_ERROR))		\
		g_assert (rv_);					\
	    if (!rv_)						\
		exit(1);					\
	}
#define sru_assert_not_reached()				\
	{							\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_ERROR))	\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_ERROR))		\
		g_assert_not_reached ();			\
	    exit (1);						\
	}

#define sru_return_if_fail(cond)				\
	{							\
	    gboolean rv_ = (cond) ? TRUE : FALSE;		\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_CRITICAL) && !rv_)\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_CRITICAL))	\
		g_return_if_fail (rv_);				\
	    if (!rv_)						\
		return;						\
	}
#define sru_return_val_if_fail(cond, val)			\
	{							\
	    gboolean rv_ = (cond) ? TRUE : FALSE;		\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_CRITICAL) && !rv_)\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_CRITICAL))	\
		g_return_val_if_fail (rv_, val);		\
	    if (!rv_)						\
		return val;					\
	}	
#ifdef G_HAVE_ISO_VARARGS
#define sru_error(...)						\
	{							\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_ERROR))	\
	    	g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_ERROR))		\
		g_error (__VA_ARGS__);				\
	    exit (1);						\
	}
#define sru_warning(...)					\
	{							\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_WARNING))	\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_WARNING))		\
		g_warning (__VA_ARGS__);			\
	}
#define sru_message(...)					\
	{							\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_MESSAGE))	\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_MESSAGE))		\
		g_message (__VA_ARGS__);			\
	}
#define sru_info(...)						\
	{							\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_INFO))	\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_INFO))		\
		sru_info_ (__VA_ARGS__);			\
	}
#define sru_debug(...)						\
	{							\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_DEBUG))	\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_DEBUG))		\
		sru_debug_ (__VA_ARGS__);			\
	}
#elif defined (G_HAVE_GNUC_VARARGS)
#define sru_error(format...)					\
	{							\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_ERROR))	\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_ERROR))		\
		g_error (format);				\
	    exit (1);						\
	}
#define sru_warning(format...)					\
	{							\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_WARNING))	\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_WARNING))		\
		g_warning (format);				\
	}
#define sru_message(format...)					\
	{							\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_MESSAGE))	\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_MESSAGE))		\
		g_message (format);				\
	}
#define sru_info(format...)					\
	{							\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_INFO))	\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_INFO))		\
		sru_info_ (format);				\
	}
#define sru_debug(format...)					\
	{							\
	    if(sru_check_log_stack_flag(G_LOG_LEVEL_DEBUG))	\
		g_on_error_stack_trace (sru_prgname);		\
	    if(sru_check_log_flag(G_LOG_LEVEL_DEBUG))		\
		sru_debug_ (format);				\
	}
#else
#define sru_assert		g_assert
#define sru_assert_not_reached	g_assert_not_reached
#define sru_return_if_fail	g_return_if_fail
#define sru_return_val_if_fail	g_return_val_if_fail
#define sru_error		g_error
#define sru_warning		g_warning
#define sru_message		g_message
#define sru_info		sru_info_
#define sru_debug		sru_debug_
#endif /* G_HAVE_ISO_VARARGS */

gboolean sru_log_init();
gboolean sru_log_terminate();
#else /* !SRU_PARANOIA */
#define sru_assert		g_assert
#define sru_assert_not_reached	g_assert_not_reached

#define sru_return_if_fail	g_return_if_fail
#define sru_return_val_if_fail	g_return_val_if_fail

#define sru_error		g_error
#define sru_warning		g_warning
#define sru_message		g_message

#ifdef G_HAVE_ISO_VARARGS
#define sru_info(...)
#define sru_debug(...)
#elif defined(G_HAVE_GNUC_VARARGS)
#define sru_info(format...)
#define sru_debug(format...)
#else
static void sru_info (...) {};
static void sru_debug(...) {};
#endif /* G_HAVE_ISO_VARARGS */
#define sru_log_init()
#define sru_log_terminate()
#endif /* SRU_PARANOIA */

#endif /* _SR_MESSAGES_H_ */
