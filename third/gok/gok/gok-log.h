/*
* gok-log.h
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
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

#include <stdarg.h>
#include "gok-log-priv.h"

#if defined(ENABLE_LOGGING_NORMAL)

#if defined(HAVE_ISO_C99_VARIADIC)
#define gok_log(...) gok_log_impl("", -1, "", __VA_ARGS__)
#elif defined(HAVE_GNU_VARIADIC)
#define gok_log(args...) gok_log_impl("", -1, "", args)
#else /* no variadic macros */
static void gok_log (const char* format, ...)
{
    va_list ap;
    va_start (ap, format);
    gok_log_impl_v ("", -1, "", format, ap);
    va_end (ap);
}
#endif /* variadic macros */

#else /* no ENABLE_LOGGING_NORMAL */

#if defined(HAVE_ISO_C99_VARIADIC)
#define gok_log(...)
#elif defined(HAVE_GNU_VARIADIC)
#define gok_log(args...)
#else /* no variadic macros */
static void gok_log(const char* format, ...)
{
}
#endif /* variadic macros */

#endif /* ENABLE_LOGGING_NORMAL */


#if defined(ENABLE_LOGGING_EXCEPTIONAL)

#if defined(HAVE_FUNC)

#if defined(HAVE_ISO_C99_VARIADIC)
#define gok_log_x(...) gok_log_impl(__FILE__, __LINE__, __func__, __VA_ARGS__)
#elif defined(HAVE_GNU_VARIADIC)
#define gok_log_x(args...) gok_log_impl(__FILE__, __LINE__, __func__, args)
#else /* no variadic macros */
static void gok_log_x (const char* format, ...)
{
    va_list ap;
    va_start (ap, format);
    gok_log_impl_v ("", -1, "", format, ap);
    va_end (ap);
}
#endif /* variadic macros */

#else /* no HAVE_FUNC */

#if defined(HAVE_ISO_C99_VARIADIC)
#define gok_log_x(...) gok_log_impl(__FILE__, __LINE__, "", __VA_ARGS__)
#elif defined(HAVE_GNU_VARIADIC)
#define gok_log_x(args...) gok_log_impl(__FILE__, __LINE__, "", args)
#else /* no variadic macros */
static void gok_log_x (const char* format, ...)
{
    va_list ap;
    va_start (ap, format);
    gok_log_impl_v ("", -1, "", format, ap);
    va_end (ap);
}
#endif /* variadic macros */

#endif /* HAVE_FUNC */

#else /* no ENABLE_LOGGING_EXCEPTIONAL */

#if defined(HAVE_ISO_C99_VARIADIC)
#define gok_log_x(...)
#elif defined(HAVE_GNU_VARIADIC)
#define gok_log_x(args...)
#else /* no variadic macros */
static void gok_log_x(const char* format, ...)
{
}
#endif /* variadic macros */

#endif /* ENABLE_LOGGING_EXCEPTIONAL */

/* #if defined(ENABLE_LOGGING_NORMAL) || defined(ENABLE_LOGGING_EXCEPTIONAL) */
#if defined(ENABLE_LOGGING_NORMAL)

#if defined(HAVE_FUNC)
#define gok_log_enter() gok_log_enter_impl (__func__)
#else /* no HAVE_FUNC */
#define gok_log_enter() gok_log_enter_impl ("")
#endif /* HAVE_FUNC */

#define gok_log_leave() gok_log_leave_impl()

#else /* no logging */

#define gok_log_enter()
#define gok_log_leave()

#endif /* logging */
