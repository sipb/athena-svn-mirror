/*
 * This file is part of the OLC On-Line Consulting system.
 * It contains definitions for operating-system routines that don't
 * appear to be defined elsewhere, at least in BSD.
 *
 * Copyright (C) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h".
 *
 *	$Id: system.h,v 1.8 1999-01-22 23:13:31 ghudson Exp $
 */

#include <mit-copyright.h>

#include <sys/types.h>
#ifdef KERBEROS
#include <krb.h>
#endif /* KERBEROS */

#ifdef HESIOD
#include <hesiod.h>
#endif
