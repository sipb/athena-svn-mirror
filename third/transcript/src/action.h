
/* action.h
 *
 * Copyright (C) 1985,1987 Adobe Systems Incorporated. All rights reserved.
 * GOVERNMENT END USERS: See notice of rights in Notice file in TranScript
 * library directory -- probably /usr/lib/ps/Notice
 * RCSID: $Header: /afs/dev.mit.edu/source/repository/third/transcript/src/action.h,v 1.1.1.1 1996-10-07 20:25:46 ghudson Exp $
 *
 * action table types for pscat/pscatmap/ptroff character mapping
 *
 * RCSLOG:
 * $Log: not supported by cvs2svn $
 * Revision 3.1  1992/08/21  16:26:32  snichols
 * Release 4.0
 *
 * Revision 3.0  1991/06/17  16:50:45  snichols
 * Release 3.0
 *
 * Revision 2.2  1987/11/17  16:49:17  byron
 * *** empty log message ***
 *
 * Revision 2.2  87/11/17  16:49:17  byron
 * Release 2.1
 * 
 * Revision 2.1.1.2  87/11/12  13:39:51  byron
 * Changed Government user's notice.
 * 
 * Revision 2.1.1.1  87/04/23  10:25:01  byron
 * Copyright notice.
 * 
 * Revision 2.2  86/11/02  14:13:27  shore
 * Product Update
 * 
 * Revision 2.1  85/11/24  12:07:51  shore
 * Product Release 2.0
 * 
 * Revision 1.2  85/05/14  11:21:08  shore
 * 
 * 
 *
 */

#define PFONT 1
#define PLIG 2
#define PPROC 3
#define PNONE 4

struct chAction {
    unsigned char	actCode;
    char *		actName;
};


struct map {
	int	wid;
	int	x,y;
	int	font;
	int	pschar;
	int	action;
	int	pswidth;
};
