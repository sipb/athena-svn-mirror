#ifndef lint
static char *RCSid = "$Id: kbd-sun.c,v 1.1.1.1 1996-10-07 20:16:54 ghudson Exp $";
#endif

/*
 * The code contained in this file is based on xkeycaps by 
 * Jamie Zawinski <jwz@lucid.com>.  It is been modified slightly
 * for use with sysinfo.
 */

/* xkeycaps, Copyright (c) 1991, 1992, 1993 Jamie Zawinski <jwz@lucid.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

/* SunOS-specific stuff: if we're on console, we can query the keyboard
   hardware directly to find out what kind it is.  I would have just put
   this code in guess.c, but vuid_event.h defines a `struct keyboard' 
   that conflicts with our own...
 */

#include "defs.h"

#if __STDC__
#include <stdlib.h>
#include <unistd.h>
extern char *strdup (const char *);
#endif

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#if	defined(SVR4) || (defined(sunos) && OSMVER == 5)
#include <sys/vuid_event.h>
#include <sys/kbio.h>
#include <sys/kbd.h>
#else
#include <sundev/vuid_event.h>
#include <sundev/kbio.h>
#include <sundev/kbd.h>
#endif

#define KBD_DEVICE		"/dev/kbd"

char *
xkeycaps_guess_local_keyboard_type ()
{
  int type = -1, layout = 0;
  int kbdfd;

  if ((kbdfd = open (KBD_DEVICE, O_WRONLY)) <= 0) {
      if (Debug) Error("Cannot open %s: %s.", KBD_DEVICE, SYSERR);
      return 0;
  }
  if (ioctl (kbdfd, KIOCTYPE, &type) == -1)
    {
      if (Debug) Error("ioctl KIOCTYPE of %s failed: %s.", KBD_DEVICE, SYSERR);
      close (kbdfd);
      return 0;
    }
  if (ioctl (kbdfd, KIOCLAYOUT, &layout) == -1)
      if (Debug) Error("ioctl KIOCLAYOUT of %s failed: %s.",KBD_DEVICE,SYSERR);
  close (kbdfd);

  switch (type) {
  case -1:	  return 0;
  case KB_ASCII:  return "Generic ASCII Terminal";	/* Ascii terminal */
  case KB_KLUNK:  return "MS103SD32-2";	/* Micro Switch 103SD32-2 */
  case KB_VT100:  return "Sun VT100";	/* Keytronics VT100 compatible */
  case KB_VT220:  return "Sun VT220";	/* vt220 Emulation */
  case KB_VT220I: return "Sun VT220i";	/* International vt220 Emulation */
  case KB_SUN2:   return "Sun Type-2";
  case KB_SUN3:   return "Sun Type-3";
  case KB_SUN4:
    switch (layout) {
    case  0: return "Sun Type-4 US Unix"; /* Part 320-1005-02 REV A. */
    case  1: return "Sun Type-4 US Unix"; /* Part 320-1005-01 REV B.  Seems identical... */
    case  2: return "Sun Type-4 Belgium/France";
    case  3: return "Sun Type-4 French Canadian";
    case  4: return "Sun Type-4 Danish";
    case  5: return "Sun Type-4 German";
    case  6: return "Sun Type-4 Italian";
    case  7: return "Sun Type-4 Dutch";
    case  8: return "Sun Type-4 Norwegian";
    case  9: return "Sun Type-4 Portuguese";
    case 10: return "Sun Type-4 Spanish";
    case 11: return "Sun Type-4 Swedish/Finnish";
    case 12: return "Sun Type-4 Swiss/French";
    case 13: return "Sun Type-4 Swiss/German";
    case 14: return "Sun Type-4 UK";
    case 33: return "Sun Type-5 US PC";
    case 34: return "Sun Type-5 US Unix";
    case 35: return "Sun Type-5 French";
    case 36: return "Sun Type-5 Danish";
    case 37: return "Sun Type-5 German";
    case 38: return "Sun Type-5 Italian";
    case 39: return "Sun Type-5 Dutch";
    case 40: return "Sun Type-5 Norwegian";
    case 41: return "Sun Type-5 Portuguese";
    case 42: return "Sun Type-5 Spanish";
    case 43: return "Sun Type-5 Swedish/Finnish";
    case 44: return "Sun Type-5 Swiss/French";
    case 45: return "Sun Type-5 Swiss/German";
    case 46: return "Sun Type-5 UK";
    case 47: return "Sun Type-5 Korean";
    case 48: return "Sun Type-5 Taiwanese";
    case 49: return "Sun Type-5 Nihon-go";
    case 50: return "Sun Type-5 French Canadian";
    case 80: return "Sun Compact 1 US PC";
    case 81: return "Sun Compact 1 US Unix";
    case 82: return "Sun Compact 1 French";
    case 83: return "Sun Compact 1 Danish";
    case 84: return "Sun Compact 1 German";
    case 85: return "Sun Compact 1 Italian";
    case 86: return "Sun Compact 1 Dutch";
    case 87: return "Sun Compact 1 Norwegian";
    case 88: return "Sun Compact 1 Portuguese";
    case 89: return "Sun Compact 1 Spanish";
    case 90: return "Sun Compact 1 Swedish/Finnish";
    case 91: return "Sun Compact 1 Swiss/French";
    case 92: return "Sun Compact 1 Swiss/German";
    case 93: return "Sun Compact 1 UK";
    case 94: return "Sun Compact 1 Korean";
    case 95: return "Sun Compact 1 Taiwanese";
    case 96: return "Sun Compact 1 Nihon-go";
    case 97: return "Sun Compact 1 French Canadian";
    default:
      {
	char buf [255];
	sprintf (buf, "Sun Type-4 Layout %d", layout);
	return strdup (buf);
      }
    }
  default:
    {
      char buf [255];
      if (layout)
	sprintf (buf, "Sun Type-%d Layout %d", type, layout);
      else
	sprintf (buf, "Sun Type-%d", type);
      return strdup (buf);
    }
  }
}

/*
 * Keyboard Probe routine.
 */
extern DevInfo_t *ProbeKbd(TreePtr)
    DevInfo_t		      **TreePtr;
{
    DevInfo_t		       *DevInfo;
    DevInfo_t		       *KbdDev;
    static char		       *KbdType = NULL;

    /*
     * If we've already been called, return now.
     */
    if (KbdType)
	return((DevInfo_t *) NULL);

    KbdType = xkeycaps_guess_local_keyboard_type();
    if (!KbdType)
	return((DevInfo_t *) NULL);

    if (!(KbdDev = NewDevInfo(NULL))) {
	Error("Cannot create new kbd device.");
	return((DevInfo_t *) NULL);
    }

    KbdDev->Name = "kbd";
    KbdDev->Type = DT_KEYBOARD;
    KbdDev->Model = KbdType;

    if (TreePtr && *TreePtr) {
	DevInfo = *TreePtr;
	/*
	 * Set device master to be the top node if the top node
	 * has no peers.
	 */
	if (!DevInfo->Name &&
	    DevInfo->Slaves && !DevInfo->Slaves->Next)
	    KbdDev->Master = DevInfo->Slaves;
    }

    return(KbdDev);
}
