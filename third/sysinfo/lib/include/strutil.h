/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not sold 
 * for profit or used for commercial gain and the author is credited 
 * appropriately.
 */

/*
 * $Revision: 1.1.1.1 $
 *
 * String utility header
 */

/*
 * Are flags f set in b?
 */
#ifndef FLAGS_ON
#define FLAGS_ON(b,f)		((b != 0) && (b & f))
#endif
#ifndef FLAGS_OFF
#define FLAGS_OFF(b,f)		((b == 0) || ((b != 0) && !(b & f)))
#endif

/*
 * StrToArgv 
 */
#define STRTO_SKIPRP		0x01		/* Skip repeating sep chars */

