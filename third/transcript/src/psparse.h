/* psparse.h
 *
 * Copyright (C) 1990,1992 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See notice of rights in Notice file in TranScript
 * library directory -- probably /usr/lib/ps/Notice
 * RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/psparse.h,v 1.1.1.1 1996-10-07 20:25:52 ghudson Exp $
 *
 * interface file for psparse package.
 *
 * $Log: not supported by cvs2svn $
 * Revision 3.4  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.3  1992/07/14  22:36:41  snichols
 * Updated copyright.
 *
 * Revision 3.2  1992/05/18  20:09:06  snichols
 * support for shrink to fit, and translating by page length.
 *
 * Revision 3.1  1992/05/05  22:10:53  snichols
 * support for adding showpage.
 *
 * Revision 3.0  1991/06/17  16:46:10  snichols
 * Release3.0
 *
 * Revision 1.5  1991/01/23  16:32:36  snichols
 * Added support for landscape, cleaned up some defaults, and
 * handled *PageRegion features better.
 *
 * Revision 1.4  91/01/16  14:14:15  snichols
 * Added support for LZW compression and ascii85 encoding for Level 2 printers.
 * 
 * Revision 1.3  90/12/12  10:37:35  snichols
 * added range information.
 * 
 *
 */

#define PSBUFSIZ 255

#define MAXRANGES 30

struct range {
    int begin;
    int end;
};

struct comfeatures {
    char keyword[50];
    char option[50];
    char *value;
};

struct controlswitches {
    int force;
    int reverse;
    int norearrange;
    int noparse;
    int strip;
    int compress;
    int landscape;
    int addshowpage;
    int squeeze;
    int upper;
    struct range ranges[MAXRANGES];
    struct comfeatures features[20];
    int nfeatures;
};

enum status { notps, notcon, handled, success };

enum status HandleComments(/* FILE *input, char *printer, char
			      *resourcepath, struct
			      controlswitches control */);
void HandleOutput(/* FILE *input, FILE *output, struct controlswitches control */);



