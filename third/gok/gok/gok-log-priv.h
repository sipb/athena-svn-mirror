/*
* gok-log-priv.h
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto
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

/* These are private functions please do not use them */

/**
 * gok_log_impl:
 *
 * This is a private function please do not use.
 */
void gok_log_impl (const char* file, int line,
		   const char* func, const char* format, ...);

/**
 * gok_log_impl_v:
 *
 * This is a private function please do not use.
 */
void gok_log_impl_v (const char* file, int line,
		     const char* func, const char* format, va_list arg);

/**
 * gok_log_enter_impl:
 *
 * This is a private function please do not use.
 */
void gok_log_enter_impl ();

/**
 * gok_log_leave_impl:
 *
 * This is a private function please do not use.
 */
void gok_log_leave_impl ();
