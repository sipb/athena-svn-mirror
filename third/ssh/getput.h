/*

getput.h

Author: Tatu Ylonen <ylo@cs.hut.fi>

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>, Espoo, Finland
                   All rights reserved

Created: Wed Jun 28 22:36:30 1995 ylo

Macros for storing and retrieving data in msb first and lsb first order.

*/

/*
 * $Id: getput.h,v 1.1.1.2 1999-03-08 17:43:33 danw Exp $
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  1996/02/18  21:38:11  ylo
 * 	Imported ssh-1.2.13.
 *
 * Revision 1.2  1995/07/13  01:24:09  ylo
 * 	Removed "Last modified" header.
 * 	Added cvs log.
 *
 * $Endlog$
 */

#ifndef GETPUT_H
#define GETPUT_H

/*------------ macros for storing/extracting msb first words -------------*/

#define GET_32BIT(cp) (((unsigned long)(unsigned char)(cp)[0] << 24) | \
  		       ((unsigned long)(unsigned char)(cp)[1] << 16) | \
		       ((unsigned long)(unsigned char)(cp)[2] << 8) | \
		       ((unsigned long)(unsigned char)(cp)[3]))

#define GET_16BIT(cp) (((unsigned long)(unsigned char)(cp)[0] << 8) | \
		       ((unsigned long)(unsigned char)(cp)[1]))

#define PUT_32BIT(cp, value) do { \
  (cp)[0] = (value) >> 24; \
  (cp)[1] = (value) >> 16; \
  (cp)[2] = (value) >> 8; \
  (cp)[3] = (value); } while (0)

#define PUT_16BIT(cp, value) do { \
  (cp)[0] = (value) >> 8; \
  (cp)[1] = (value); } while (0)

/*------------ macros for storing/extracting lsb first words -------------*/

#define GET_32BIT_LSB_FIRST(cp) \
  (((unsigned long)(unsigned char)(cp)[0]) | \
  ((unsigned long)(unsigned char)(cp)[1] << 8) | \
  ((unsigned long)(unsigned char)(cp)[2] << 16) | \
  ((unsigned long)(unsigned char)(cp)[3] << 24))

#define GET_16BIT_LSB_FIRST(cp) \
  (((unsigned long)(unsigned char)(cp)[0]) | \
  ((unsigned long)(unsigned char)(cp)[1] << 8))

#define PUT_32BIT_LSB_FIRST(cp, value) do { \
  (cp)[0] = (value); \
  (cp)[1] = (value) >> 8; \
  (cp)[2] = (value) >> 16; \
  (cp)[3] = (value) >> 24; } while (0)

#define PUT_16BIT_LSB_FIRST(cp, value) do { \
  (cp)[0] = (value); \
  (cp)[1] = (value) >> 8; } while (0)

#endif /* GETPUT_H */

