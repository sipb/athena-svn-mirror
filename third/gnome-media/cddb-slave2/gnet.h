/* GNet - Networking library
 * Copyright (C) 2000  David Helder
 * Copyright (C) 2000  Andrew Lanoix
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
 * Boston, MA  02111-1307, USA.
 */

#ifndef _GNET_H
#define _GNET_H

#include <glib.h>

#include "inetaddr.h"
#include "iochannel.h"
#include "socks.h"
#include "tcp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef GNET_WIN32
#  define GNET_EXPORT extern __declspec(dllimport)
#else
#  define GNET_EXPORT extern
#endif

GNET_EXPORT const guint gnet_major_version;
GNET_EXPORT const guint gnet_minor_version;
GNET_EXPORT const guint gnet_micro_version;
GNET_EXPORT const guint gnet_interface_age;
GNET_EXPORT const guint gnet_binary_age;

/**
 *  GNET_CHECK_VERSION:
 *  @major: Major version number
 *  @minor: Minor version number
 *  @micro: Micro version number
 *
 *  Check if the version given is compatable with this version of the
 *  library.  For example, GNET_CHECK_VERSION(1.2.3) would return TRUE
 *  if the version is 1.2.5, and FALSE if 1.1.0.
 *
 **/
#define GNET_CHECK_VERSION(major,minor,micro)    \
    (GNET_MAJOR_VERSION > (major) || \
     (GNET_MAJOR_VERSION == (major) && GNET_MINOR_VERSION > (minor)) || \
     (GNET_MAJOR_VERSION == (major) && GNET_MINOR_VERSION == (minor) && \
      GNET_MICRO_VERSION >= (micro)))



#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif /* _GNET_H */
