/*
 * types.h
 *
 * Types.h generic system types file
 *
 * Copyright (c) 2000 Virtual Unlimited B.V.
 *
 * Author: Bob Deblier <bob@virtualunlimited.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef _BEECRYPT_TYPES_H
#define _BEECRYPT_TYPES_H

#ifndef ROTL32
# define ROTL32(x, s) (((x) << (s)) | ((x) >> (32 - (s))))
#endif
#ifndef ROTR32
# define ROTR32(x, s) (((x) >> (s)) | ((x) << (32 - (s))))
#endif

#if WIN32 && !__CYGWIN32__
# ifdef BEECRYPT_DLL_EXPORT
#  define BEECRYPTAPI __declspec(dllexport)
# else
#  define BEECRYPTAPI __declspec(dllimport)
# endif
#else
# define BEECRYPTAPI
typedef unsigned char	byte;
#endif

/*@-typeuse@*/
typedef char	int8;
/*@=typeuse@*/
typedef short	int16;
typedef int	int32;
typedef long long	int64;

typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;
/*@-duplicatequals@*/
typedef unsigned long long	uint64;
/*@=duplicatequals@*/

typedef char	javabyte;
typedef short	javashort;
typedef int	javaint;
typedef long long	javalong;

typedef unsigned short	javachar;

typedef float	javafloat;
typedef double	javadouble;

#endif /* _BEECRYPT_TYPES_H */
